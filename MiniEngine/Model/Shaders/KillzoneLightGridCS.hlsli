//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.tileLightCountSphere
//
// Developed by Minigraph
//
// Author(s):	Alex Nankervis
//

#include "LightGrid.hlsli"

// outdated warning about for-loop variable scope
#pragma warning (disable: 3078)

#define FLT_MIN         1.175494351e-38F        // min positive value
#define FLT_MAX         3.402823466e+38F        // max value
#define PI				3.1415926535f
#define TWOPI			6.283185307f

cbuffer CSConstants : register(b0)
{
    uint ViewportWidth, ViewportHeight;
    float InvTileDim;
    float RcpZMagic;
    uint TileCountX;
    float4x4 ViewProjMatrix;
    float4x4 InvViewProj;
    float3 ViewerPos;
    //float4x4 InvProj;
};

StructuredBuffer<LightData> gLightBuffer : register(t4);
Texture2DArray<float> gLightShadowArrayTex : register(t5);
Texture2D<float> gDepthTex : register(t8);
Texture2D<float3> gRt0 : register(t10);
//Texture2D<float4> gRt0 : register(t10);
Texture2D<float4> gRt1 : register(t11);
Texture2D<float4> gRt2 : register(t12);
Texture2D<float4> gRt3 : register(t13);

RWTexture2D<float4> gOutputTexture : register(u0);

SamplerComparisonState gShadowSampler : register(s0);

#if LIGHT_CULLING_2_5
// Harada, T., “A 2.5D culling for Forward+,” in SIGGRAPH Asia 2012 Technical Briefs, ACM, pp. 18:1–18:4, Dec. 2012
groupshared uint gSharedDepthMask;
#endif

groupshared uint gSharedMinDepthUInt;
groupshared uint gSharedMaxDepthUInt;

groupshared uint gSharedVisibleLightCountSphere;
groupshared uint gSharedVisibleLightCountCone;
groupshared uint gSharedVisibleLightCountConeShadowed;

groupshared uint gSharedVisibleLightIndicesSphere[MAX_LIGHTS];
groupshared uint gSharedVisibleLightIndicesCone[MAX_LIGHTS];
groupshared uint gSharedVisibleLightIndicesConeShadowed[MAX_LIGHTS];

//groupshared uint4 tileLightBitMask;

#define _RootSig \
    "RootFlags(0), " \
    "CBV(b0), " \
    "DescriptorTable(SRV(t0, numDescriptors = 10))," \
    "DescriptorTable(SRV(t10, numDescriptors = 4))," \
    "DescriptorTable(UAV(u0, numDescriptors = 1))," \
    "StaticSampler(s0, visibility = SHADER_VISIBILITY_ALL," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "comparisonFunc = COMPARISON_GREATER_EQUAL," \
        "filter = FILTER_MIN_MAG_LINEAR_MIP_POINT)" \

// Apply fresnel to modulate the specular albedo
void FSchlick(inout float3 specular, inout float3 diffuse, float3 lightDir, float3 halfVec)
{
    float fresnel = pow(1.0 - saturate(dot(lightDir, halfVec)), 5.0);
    specular = lerp(specular, 1, fresnel);
    diffuse = lerp(diffuse, 0, fresnel);
}

float GetShadowConeLight(uint lightIndex, float3 shadowCoord)
{
    float result = gLightShadowArrayTex.SampleCmpLevelZero(
        gShadowSampler, float3(shadowCoord.xy, lightIndex), shadowCoord.z);
    return result * result;
}

float3 ApplyLightCommon(
    float3 diffuseColor, // Diffuse albedo
    float3 specularColor, // Specular albedo
    float specularMask, // Where is it shiny or dingy?
    float gloss, // Specular power
    float3 normal, // World-space normal
    float3 viewDir, // World-space vector from eye to point
    float3 lightDir, // World-space vector from point to light
    float3 lightColor // Radiance of directional light
    )
{
    float3 halfVec = normalize(lightDir - viewDir);
    float nDotH = saturate(dot(halfVec, normal));

    FSchlick(diffuseColor, specularColor, lightDir, halfVec);

    float specularFactor = specularMask * pow(nDotH, gloss) * (gloss + 2) / 8;

    float nDotL = saturate(dot(normal, lightDir));

    return nDotL * lightColor * (diffuseColor + specularFactor * specularColor);
}

float3 ApplyPointLight(
    float3 diffuseColor, // Diffuse albedo
    float3 specularColor, // Specular albedo
    float specularMask, // Where is it shiny or dingy?
    float gloss, // Specular power
    float3 normal, // World-space normal
    float3 viewDir, // World-space vector from eye to point
    float3 worldPos, // World-space fragment position
    float3 lightPos, // World-space light position
    float lightRadiusSq,
    float3 lightColor // Radiance of directional light
    )
{
    float3 lightDir = lightPos - worldPos;
    float lightDistSq = dot(lightDir, lightDir);
    float invLightDist = rsqrt(lightDistSq);
    lightDir *= invLightDist;

    // modify 1/d^2 * R^2 to fall off at a fixed radius
    // (R/d)^2 - d/R = [(1/d^2) - (1/R^2)*(d/R)] * R^2
    float distanceFalloff = lightRadiusSq * (invLightDist * invLightDist);
    distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));

    //float3 commonLight = ApplyLightCommon(
    //    diffuseColor,
    //    specularColor,
    //    specularMask,
    //    gloss,
    //    normal,
    //    viewDir,
    //    lightDir,
    //    lightColor
    //    );
    //
    //return distanceFalloff * commonLight;
    return distanceFalloff * ApplyLightCommon(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        lightDir,
        lightColor
        );
}

float3 ApplyConeLight(
    float3 diffuseColor, // Diffuse albedo
    float3 specularColor, // Specular albedo
    float specularMask, // Where is it shiny or dingy?
    float gloss, // Specular power
    float3 normal, // World-space normal
    float3 viewDir, // World-space vector from eye to point
    float3 worldPos, // World-space fragment position
    float3 lightPos, // World-space light position
    float lightRadiusSq,
    float3 lightColor, // Radiance of directional light
    float3 coneDir,
    float2 coneAngles
    )
{
    float3 lightDir = lightPos - worldPos;
    float lightDistSq = dot(lightDir, lightDir);
    float invLightDist = rsqrt(lightDistSq);
    lightDir *= invLightDist;

    // modify 1/d^2 * R^2 to fall off at a fixed radius
    // (R/d)^2 - d/R = [(1/d^2) - (1/R^2)*(d/R)] * R^2
    float distanceFalloff = lightRadiusSq * (invLightDist * invLightDist);
    distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));

    float coneFalloff = dot(-lightDir, coneDir);
    coneFalloff = saturate((coneFalloff - coneAngles.y) * coneAngles.x);

    return (coneFalloff * distanceFalloff) * ApplyLightCommon(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        lightDir,
        lightColor
        );
}

float3 ApplyConeShadowedLight(
    float3 diffuseColor, // Diffuse albedo
    float3 specularColor, // Specular albedo
    float specularMask, // Where is it shiny or dingy?
    float gloss, // Specular power
    float3 normal, // World-space normal
    float3 viewDir, // World-space vector from eye to point
    float3 worldPos, // World-space fragment position
    float3 lightPos, // World-space light position
    float lightRadiusSq,
    float3 lightColor, // Radiance of directional light
    float3 coneDir,
    float2 coneAngles,
    float4x4 shadowTextureMatrix,
    uint lightIndex
    )
{
    float4 shadowCoord = mul(shadowTextureMatrix, float4(worldPos, 1.0));
    shadowCoord.xyz *= rcp(shadowCoord.w);
    float shadow = GetShadowConeLight(lightIndex, shadowCoord.xyz);

    return shadow * ApplyConeLight(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        worldPos,
        lightPos,
        lightRadiusSq,
        lightColor,
        coneDir,
        coneAngles
        );
}

// Andersson, Johan, “Parallel Graphics in Frostbite—Current & Future,” SIGGRAPH Beyond Programmable Shading course, Aug. 2009
[RootSignature(_RootSig)]
[numthreads(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y, 1)]
void main(
    uint2 Gid : SV_GroupID,
    uint2 GTid : SV_GroupThreadID,
    uint GI : SV_GroupIndex,
    uint3 DTid : SV_DispatchThreadID)
{
    // initialize shared data
    if (GI == 0)
    {
        gSharedVisibleLightCountSphere = 0;
        gSharedVisibleLightCountCone = 0;
        gSharedVisibleLightCountConeShadowed = 0;
        //tileLightBitMask = 0;
        gSharedMinDepthUInt = 0xffffffff;
        gSharedMaxDepthUInt = 0;
#if LIGHT_CULLING_2_5
        gSharedDepthMask = 0;
#endif
    }
    GroupMemoryBarrierWithGroupSync();

    // Find the max / min depth value of the tile
    uint depthUInt = asuint(gDepthTex[DTid.xy]);
    InterlockedMin(gSharedMinDepthUInt, depthUInt);
    InterlockedMax(gSharedMaxDepthUInt, depthUInt);

    GroupMemoryBarrierWithGroupSync();
    
    // Construct the tile frustum in world space
    float tileMinDepth = (rcp(asfloat(gSharedMaxDepthUInt)) - 1.0) * RcpZMagic;
    float tileMaxDepth = (rcp(asfloat(gSharedMinDepthUInt)) - 1.0) * RcpZMagic;
    float depth = (rcp(asfloat(depthUInt)) - 1.0) * RcpZMagic;
    
    float tileDepthRange = tileMaxDepth - tileMinDepth;
    tileDepthRange = max(tileDepthRange, FLT_MIN); // don't allow a depth range of 0
    float invTileDepthRange = rcp(tileDepthRange);
    // TODO: near/far clipping planes seem to be falling apart at or near the max depth with infinite projections
    
#if AABB_BASED_CULLING
    // https://wickedengine.net/2018/01/10/optimizing-tile-based-light-culling/
    // AABB based culling
    float4 minAABB = float4(((float) Gid.x * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) Gid.y * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, tileMinDepth, 1.0f);
    minAABB.y *= -1.0f;
    minAABB = mul(InvViewProj, minAABB);
    minAABB /= minAABB.w;
    float4 maxAABB = float4(((float) (Gid.x + 1) * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) (Gid.y + 1) * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, tileMaxDepth, 1.0f);
    maxAABB.y *= -1.0f;
    maxAABB = mul(InvViewProj, maxAABB);
    maxAABB /= maxAABB.w;
    
    float4 centerAABB = (minAABB + maxAABB) / 2.0f;
    float4 extentAABB = abs(maxAABB - centerAABB);
#else
    // construct transform from world space to tile space (projection space constrained to tile area)
    float2 invTileSize2X = float2(ViewportWidth, ViewportHeight) * InvTileDim;
    // D3D-specific [0, 1] depth range ortho projection
    // (but without negation of Z, since we already have that from the projection matrix)
    float3 tileBias = float3(
        -2.0 * float(Gid.x) + invTileSize2X.x - 1.0,
        -2.0 * float(Gid.y) + invTileSize2X.y - 1.0,
        -tileMinDepth * invTileDepthRange);
    float4x4 projToTile = float4x4(
        invTileSize2X.x, 0, 0, tileBias.x,
        0, -invTileSize2X.y, 0, tileBias.y,
        0, 0, invTileDepthRange, tileBias.z,
        0, 0, 0, 1
        );
    float4x4 tileMVP = mul(projToTile, ViewProjMatrix);
    
    // extract frustum planes (these will be in world space)
    float4 frustumPlanes[6];
    frustumPlanes[0] = tileMVP[3] + tileMVP[0];
    frustumPlanes[1] = tileMVP[3] - tileMVP[0];
    frustumPlanes[2] = tileMVP[3] + tileMVP[1];
    frustumPlanes[3] = tileMVP[3] - tileMVP[1];
    frustumPlanes[4] = tileMVP[3] + tileMVP[2];
    frustumPlanes[5] = tileMVP[3] - tileMVP[2];
    for (int n = 0; n < 6; n++)
    {
        frustumPlanes[n] *= rsqrt(dot(frustumPlanes[n].xyz, frustumPlanes[n].xyz));
    }
#endif
    
#if LIGHT_CULLING_2_5
    const float depthRangeRecip = 32.f * invTileDepthRange;
    const uint depthMaskCellIndex = max(0, min(32, floor(depth - tileMinDepth) * depthRangeRecip));
    InterlockedOr(gSharedDepthMask, 1 << depthMaskCellIndex);   // depthMaskT ← atomOr(1 << getCellIndex(z))
    GroupMemoryBarrierWithGroupSync();
#endif
    
    // Cull lights
    uint threadCount = WORK_GROUP_SIZE_X * WORK_GROUP_SIZE_Y;
    uint passCount = (MAX_LIGHTS + threadCount - 1) / threadCount;
    
    for (uint passIt = 0; passIt < passCount; ++passIt)
    {
        uint lightIndex = passIt * threadCount + GI;
        
        lightIndex = min(lightIndex, MAX_LIGHTS);
        
        LightData lightData = gLightBuffer[lightIndex];
        float3 lightWorldPos = lightData.pos;
        float lightCullRadius = sqrt(lightData.radiusSq);
        
#if AABB_BASED_CULLING
        float3 vDelta = max(0, abs(centerAABB.xyz - lightWorldPos) - extentAABB.xyz);
        float fDistSq = dot(vDelta, vDelta);
        
        if (fDistSq > lightData.radiusSq)
            continue;
#else
        bool overlapping = true;
        for (int p = 0; p < 6; p++)
        {
            float d = dot(lightWorldPos, frustumPlanes[p].xyz) + frustumPlanes[p].w;
            if (d < -lightCullRadius)
            {
                overlapping = false;
            }
        }
        
        if (!overlapping)
            continue;
#endif
        
#if LIGHT_CULLING_2_5
        // depthMaskL ← Compute mask using light extent
        uint localDepthMask = 0;
        const float fLightMin = (lightWorldPos.z - lightCullRadius) * ViewProjMatrix._33 + ViewProjMatrix._43;
        const float fLightMax = (lightWorldPos.z + lightCullRadius) * ViewProjMatrix._33 + ViewProjMatrix._43;
        const uint lightMaskCellIndexStart = max(0, min(32, floor((fLightMin - tileMinDepth) * depthRangeRecip)));
        const uint lightMaskCellIndexEnd = max(0, min(32, floor((fLightMax - tileMinDepth) * depthRangeRecip)));
        
        uint c = 0;
        for (c = lightMaskCellIndexStart; c <= lightMaskCellIndexEnd; ++c)
        {
            localDepthMask |= 1 << c;
        }
        
        if (gSharedDepthMask & localDepthMask)  // overlapping ← depthMaskT ∧ depthMaskL
        {
#endif
            // Fill in the light indices to the local shared indices array
            uint offset = 0;
        
            switch (lightData.type)
            {
                case 0: // sphere
                    InterlockedAdd(gSharedVisibleLightCountSphere, 1, offset);
                    gSharedVisibleLightIndicesSphere[offset] = lightIndex;
                    break;

                case 1: // cone
                    InterlockedAdd(gSharedVisibleLightCountCone, 1, offset);
                    gSharedVisibleLightIndicesCone[offset] = lightIndex;
                    break;

                case 2: // cone w/ shadow map
                    InterlockedAdd(gSharedVisibleLightCountConeShadowed, 1, offset);
                    gSharedVisibleLightIndicesConeShadowed[offset] = lightIndex;
                    break;
#if LIGHT_CULLING_2_5
            }
#endif
        }
    }
    GroupMemoryBarrierWithGroupSync();
    
    if (depth <= 0.0f)
    {
        //gOutputTexture[DTid.xy] += 0;
        return;
    }
    
    float3 color = 0;
    
    //float4 rt0Data = gRt0[DTid.xy];
    //color = rt0Data.rgb;
    color = gRt0[DTid.xy];
    //float gloss = rt0Data.a * 256.0;
    float4 rt1Data = gRt1[DTid.xy];
    float3 normal = rt1Data.xyz;
    if (normal.x == 0 && normal.y == 0 && normal.z == 0)
    {
        return;
    }
    float4 rt2Data = gRt2[DTid.xy];
    float specularMask = rt2Data.a;
    float4 rt3Data = gRt3[DTid.xy];
    float3 diffuseAlbedo = rt3Data.rgb;
    float gloss = rt3Data.a * 256.0;
    
    float4 clipSpacePosition = float4(((float) DTid.x / (float) ViewportWidth) * 2.0f - 1.0f, ((float) DTid.y / (float) ViewportHeight) * 2.0f - 1.0f, depth, 1.0f);
    clipSpacePosition.y *= -1.0f;
    float4 worldPos = mul(InvViewProj, clipSpacePosition);
    worldPos /= worldPos.w;
    
    float3 viewDir = normalize(worldPos.xyz - ViewerPos);
    float3 specularAlbedo = float3(0.56, 0.56, 0.56);
    
#define POINT_LIGHT_ARGS \
    diffuseAlbedo, \
    specularAlbedo, \
    specularMask, \
    gloss, \
    normal, \
    viewDir, \
    worldPos.xyz, \
    lightData.pos, \
    lightData.radiusSq, \
    lightData.color

#define CONE_LIGHT_ARGS \
    POINT_LIGHT_ARGS, \
    lightData.coneDir, \
    lightData.coneAngles

#define SHADOWED_LIGHT_ARGS \
    CONE_LIGHT_ARGS, \
    lightData.shadowTextureMatrix, \
    lightIndex
    
    uint lightIt = 0;
    for (; lightIt < gSharedVisibleLightCountSphere; ++lightIt)
    {
        uint lightIndex = gSharedVisibleLightIndicesSphere[lightIt];
        LightData lightData = gLightBuffer[lightIndex];
        color += ApplyPointLight(POINT_LIGHT_ARGS);
    }

    for (lightIt = 0; lightIt < gSharedVisibleLightCountCone; ++lightIt)
    {
        uint lightIndex = gSharedVisibleLightIndicesCone[lightIt];
        LightData lightData = gLightBuffer[lightIndex];
        color += ApplyConeLight(CONE_LIGHT_ARGS);
    }
    
    for (lightIt = 0; lightIt < gSharedVisibleLightCountConeShadowed; ++lightIt)
    {
        uint lightIndex = gSharedVisibleLightIndicesConeShadowed[lightIt];
        LightData lightData = gLightBuffer[lightIndex];
        color += ApplyConeShadowedLight(SHADOWED_LIGHT_ARGS);
    }
    
    gOutputTexture[DTid.xy] += float4(color, 1);
    //gOutputTexture[DTid.xy] += float4(lightDensity, lightDensity, lightDensity, 1);
}