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

#include "Common.hlsli"
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
#if Z_RECONSTRUCTION || SPHEREMAP_TRANSFORM
    float4x4 InvView;
#endif
    float3 ViewerPos;
    //float4x4 InvProj;
};

StructuredBuffer<LightData> gLightBuffer : register(t4);
Texture2DArray<float> gLightShadowArrayTex : register(t5);
Texture2D<float> gDepthTex : register(t8);
Texture2D<uint2> gStencilTex : register(t9);

Texture2D<float3> gRt0 : register(t10);
Texture2D<float4> gRt1 : register(t11);
Texture2D<float4> gRt2 : register(t12);

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
    if (depthUInt != 0)
    {
        InterlockedMin(gSharedMinDepthUInt, depthUInt);
        InterlockedMax(gSharedMaxDepthUInt, depthUInt);
    }

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
    
    float4 nearMinPoint = float4(((float) Gid.x * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) Gid.y * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, tileMinDepth, 1.0f);
    float4 farMinPoint = float4(((float) Gid.x * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) Gid.y * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, tileMaxDepth, 1.0f);
    float4 nearMaxPoint = float4(((float) (Gid.x + 1) * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) (Gid.y + 1) * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, tileMinDepth, 1.0f);
    float4 farMaxPoint = float4(((float) (Gid.x + 1) * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) (Gid.y + 1) * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, tileMaxDepth, 1.0f);
    
    nearMinPoint.y *= -1.0f;
    farMinPoint.y *= -1.0f;
    nearMaxPoint.y *= -1.0f;
    farMaxPoint.y *= -1.0f;
    
    nearMinPoint = mul(InvViewProj, nearMinPoint);
    farMinPoint = mul(InvViewProj, farMinPoint);
    nearMaxPoint = mul(InvViewProj, nearMaxPoint);
    farMaxPoint = mul(InvViewProj, farMaxPoint);
    
    nearMinPoint /= nearMinPoint.w;
    farMinPoint /= farMinPoint.w;
    nearMaxPoint /= nearMaxPoint.w;
    farMaxPoint /= farMaxPoint.w;
    
    float4 aabbMinPoint = min(min(nearMinPoint, farMinPoint), min(nearMaxPoint, farMaxPoint));
    float4 aabbMaxPoint = max(max(nearMinPoint, farMinPoint), max(nearMaxPoint, farMaxPoint));
    
    float4 centerAABB = (aabbMinPoint + aabbMaxPoint) / 2.0f;
    float4 extentAABB = abs(aabbMaxPoint - centerAABB);
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
        
        if (lightIndex > MAX_LIGHTS)
        {
            continue;
        }
        //lightIndex = min(lightIndex, MAX_LIGHTS);
        
        LightData lightData = gLightBuffer[lightIndex];
        float3 lightWorldPos = lightData.pos;
        float lightCullRadius = sqrt(lightData.radiusSq);
        
#if AABB_BASED_CULLING
        // Arvo Intersection Test
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
        const float fLightMin = (lightWorldPos.z + lightCullRadius) * ViewProjMatrix._33 + ViewProjMatrix._43;
        const float fLightMax = (lightWorldPos.z - lightCullRadius) * ViewProjMatrix._33 + ViewProjMatrix._43;
        const uint lightMaskCellIndexStart = max(0, min(32, floor((fLightMin - tileMinDepth) * depthRangeRecip)));
        const uint lightMaskCellIndexEnd = max(0, min(32, floor((fLightMax - tileMinDepth) * depthRangeRecip)));
        
        uint c = 0;
        for (c = lightMaskCellIndexStart; c <= lightMaskCellIndexEnd; ++c)
        {
            localDepthMask |= 1 << c;
        }
        
        if (gSharedDepthMask & localDepthMask)  // overlapping ← depthMaskT ∧ depthMaskL
#endif
        {
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
            }
        }
    }
    GroupMemoryBarrierWithGroupSync();
    
    uint stencil = gStencilTex[DTid.xy].g;
    if (!stencil)
    {
        return;
    }
    
    if (depth <= 0.0f)
    {
        //gOutputTexture[DTid.xy] += 0;
        return;
    }
    
#if LIGHT_DENSITY
    float density = (float) (gSharedVisibleLightCountSphere + gSharedVisibleLightCountCone + gSharedVisibleLightCountConeShadowed) / (float) MAX_LIGHTS;
    gOutputTexture[DTid.xy] += float4(ConvertToRadarColor(density), 1);
    return;
#else
    
    float3 color = 0;
    
    color = gRt0[DTid.xy];
    float4 rt1Data = gRt1[DTid.xy];
    
#if NO_ENCODING || BASELINE
    float3 normal = (float3) BaseDecode(rt1Data);
#elif Z_RECONSTRUCTION || SPHEREMAP_TRANSFORM
    float3 normal = (float3) BaseDecode(rt1Data.xy, InvView);
#elif SPHERICAL_COORDNATES || OCTAHEDRON_NORMAL
    float3 normal = (float3) BaseDecode(rt1Data.xy);
#elif OCT24
    float3 normal = (float3) BaseDecode(rt1Data.xyz);
#endif
    
    float4 rt2Data = gRt2[DTid.xy];
    float3 diffuseAlbedo = rt2Data.xyz;
    float specularMask = rt2Data.a;
    
    if (dot(normal, 1.0) == 0.0 && dot(diffuseAlbedo, 1.0) == 0.0)
    {
        return;
    }
    float gloss = rt1Data.a * 256.0;
    
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
    
#if FALSE_POSITIVE_RATE
    uint falsePositiveCount = 0;
#endif
    uint lightIt = 0;
    for (; lightIt < gSharedVisibleLightCountSphere; ++lightIt)
    {
        uint lightIndex = gSharedVisibleLightIndicesSphere[lightIt];
        LightData lightData = gLightBuffer[lightIndex];
#if FALSE_POSITIVE_RATE
        float3 lightDir = lightData.pos - worldPos.xyz;
        float lightDistSq = dot(lightDir, lightDir);
        float invLightDist = rsqrt(lightDistSq);
        lightDir *= invLightDist;

        // modify 1/d^2 * R^2 to fall off at a fixed radius
        // (R/d)^2 - d/R = [(1/d^2) - (1/R^2)*(d/R)] * R^2
        float distanceFalloff = lightData.radiusSq * (invLightDist * invLightDist);
        distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));

        falsePositiveCount += (distanceFalloff == 0.0);
        
        float3 pointLightColor = ApplyPointLight(POINT_LIGHT_ARGS);
        //falsePositiveCount += (pointLightColor.r == 0 && pointLightColor.g == 0 && pointLightColor.b == 0);
        color += pointLightColor;
#else
        color += ApplyPointLight(POINT_LIGHT_ARGS);
#endif
    }

    for (lightIt = 0; lightIt < gSharedVisibleLightCountCone; ++lightIt)
    {
        uint lightIndex = gSharedVisibleLightIndicesCone[lightIt];
        LightData lightData = gLightBuffer[lightIndex];
#if FALSE_POSITIVE_RATE
        float3 lightDir = lightData.pos - worldPos.xyz;
        float lightDistSq = dot(lightDir, lightDir);
        float invLightDist = rsqrt(lightDistSq);
        lightDir *= invLightDist;

        // modify 1/d^2 * R^2 to fall off at a fixed radius
        // (R/d)^2 - d/R = [(1/d^2) - (1/R^2)*(d/R)] * R^2
        float distanceFalloff = lightData.radiusSq * (invLightDist * invLightDist);
        distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));

        float coneFalloff = dot(-lightDir, lightData.coneDir);
        coneFalloff = saturate((coneFalloff - lightData.coneAngles.y) * lightData.coneAngles.x);
        
        falsePositiveCount += (distanceFalloff * coneFalloff == 0.0);
        
        float3 coneLightColor = ApplyConeLight(CONE_LIGHT_ARGS);
        //falsePositiveCount += (coneLightColor.r == 0 && coneLightColor.g == 0 && coneLightColor.b == 0);
        color += coneLightColor;
#else
        color += ApplyConeLight(CONE_LIGHT_ARGS);
#endif
    }
    
    for (lightIt = 0; lightIt < gSharedVisibleLightCountConeShadowed; ++lightIt)
    {
        uint lightIndex = gSharedVisibleLightIndicesConeShadowed[lightIt];
        LightData lightData = gLightBuffer[lightIndex];
#if FALSE_POSITIVE_RATE
        float3 lightDir = lightData.pos - worldPos.xyz;
        float lightDistSq = dot(lightDir, lightDir);
        float invLightDist = rsqrt(lightDistSq);
        lightDir *= invLightDist;

        // modify 1/d^2 * R^2 to fall off at a fixed radius
        // (R/d)^2 - d/R = [(1/d^2) - (1/R^2)*(d/R)] * R^2
        float distanceFalloff = lightData.radiusSq * (invLightDist * invLightDist);
        distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));

        float coneFalloff = dot(-lightDir, lightData.coneDir);
        coneFalloff = saturate((coneFalloff - lightData.coneAngles.y) * lightData.coneAngles.x);
        
        falsePositiveCount += (distanceFalloff * coneFalloff == 0.0);
        
        float3 coneShadowedLightColor = ApplyConeShadowedLight(SHADOWED_LIGHT_ARGS);
        //falsePositiveCount += (coneShadowedLightColor.r == 0 && coneShadowedLightColor.g == 0 && coneShadowedLightColor.b == 0);
        color += coneShadowedLightColor;
#else
        color += ApplyConeShadowedLight(SHADOWED_LIGHT_ARGS);
#endif
    }
    
#if FALSE_POSITIVE_RATE
    uint totalLightsCount = gSharedVisibleLightCountSphere + gSharedVisibleLightCountCone + gSharedVisibleLightCountConeShadowed;
    if (totalLightsCount)
    {
        float fpr = (float) falsePositiveCount / (float) totalLightsCount;
        gOutputTexture[DTid.xy] = float4(ConvertToRadarColor(fpr), 1);
    }
    else
    {
        gOutputTexture[DTid.xy] = 0;
    }
    return;
#else
    gOutputTexture[DTid.xy] += float4(color, 1);
    //gOutputTexture[DTid.xy] += float4(lightDensity, lightDensity, lightDensity, 1);
#endif
#endif
}