//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):	James Stanard

#include "Common.hlsli"
#include "Lighting.hlsli"

cbuffer StartVertex : register(b1)
{
    uint materialIdx;
    float4x4 ModelToProjection;
    float4x4 InvViewProj;
    float4x4 modelToShadow;
    float3 ViewerPos;
//    float NearZ;
//    float FarZ;
//    float2 ViewportSize;
//    float CameraForward;
};

//Texture2D<float3> texDiffuse : register(t0);
//Texture2D<float3> texSpecular : register(t1);
//Texture2D<float4> texEmissive		: register(t2);
//Texture2D<float3> texNormal : register(t3);
//Texture2D<float4> texLightmap		: register(t4);
//Texture2D<float4> texReflection	: register(t5);
//Texture2D<float> texSSAO : register(t12);
//Texture2D<float> texShadow : register(t13);
Texture2D<float> texDepth : register(t18);

// GBuffers
Texture2D<float4> texRt0	: register(t21);
//Texture2D<float2> texRt1	: register(t22);
Texture2D<float3> texRt1    : register(t22);
//Texture2D<float4> texRt1	: register(t22);
Texture2D<float4> texRt2	: register(t23);
Texture2D<float4> texRt3    : register(t24);

struct VSOutput
{
	sample float4 projPos : SV_Position;
	sample float3 worldPos : WorldPos;
    sample float2 uv : TexCoord0;
};

[RootSignature(Renderer_RootSig)]
float3 main(VSOutput vsOutput) : SV_Target
{
    float3 color = 0.0;
	
	uint2 pixelPos = uint2(vsOutput.projPos.xy);
	
    float depth = texDepth[pixelPos];
    if (depth <= 0.0)
    {
        color = 0.0;
        return color;
    }
#if DEPTH
    return sqrt(depth);
#endif
    float4 rt0Data = texRt0[pixelPos];
    color = rt0Data.rgb;
    float gloss = rt0Data.a * 256.0;
#if LIGHT_ACCUMULATION
    return color;
#endif
#if GLOSS
    return gloss / 256.0;
#endif
    //float2 rt1Data = texRt1[pixelPos];
    //float3 normal = normalize(float3(rt1Data.x, rt1Data.y, sqrt(1.0 - rt1Data.x * rt1Data.x - rt1Data.y * rt1Data.y)));
    float4 rt2Data = texRt2[pixelPos];
    //float3 worldPos = rt2Data.xyz;
    float specularMask = rt2Data.a;
    
    float4 clipSpacePosition = float4((vsOutput.projPos.x / 1920) * 2.0f - 1.0f, (vsOutput.projPos.y / 1080) * 2.0f - 1.0f, depth, 1.0f);
    clipSpacePosition.y *= -1.0f;
    float4 worldPos = mul(InvViewProj, clipSpacePosition);
    worldPos /= worldPos.w;
    
    //if (dot(normal, normalize(ViewerPos - worldPos.xyz)) < 0.0)
    //{
    //    normal.z *= -1.0f;
    //}

#if MOTION_VECTOR
    return normalize(worldPos.xyz);
#endif
#if SPEC_INTENSITY
    return specularMask;
#endif
    float4 rt3Data = texRt3[pixelPos];
    float3 diffuseAlbedo = rt3Data.rgb;
#if DIFFUSE_ALBEDO
    return diffuseAlbedo;
#endif
#if SUN_OCCLUSION
    return rt3Data.a;
#endif
  
    float3 specularAlbedo = float3( 0.56, 0.56, 0.56 );
    float3 viewDir = normalize(worldPos.xyz - ViewerPos);
    float3 shadowCoord = mul(modelToShadow, float4(worldPos.xyz, 1.0)).xyz;

	//ShadeLights(
	//	color,
	//	pixelPos,
	//	diffuseAlbedo,
	//	specularAlbedo,
	//	specularMask,
	//	gloss,
	//	normal,
	//	viewDir,
	//	worldPos.xyz
	//	);
    
    uint2 tilePos = GetTilePos(pixelPos, InvTileDim.xy);
    uint tileIndex = GetTileIndex(tilePos, TileCount.x);
    uint tileOffset = GetTileOffset(tileIndex);
    uint lightBitMaskGroups[4] = { 0, 0, 0, 0 };
    
    uint tileLightCount = lightGrid.Load(tileOffset + 0);
    uint tileLightCountSphere = (tileLightCount >> 0) & 0xff;
    uint tileLightCountCone = (tileLightCount >> 8) & 0xff;
    uint tileLightCountConeShadowed = (tileLightCount >> 16) & 0xff;

    uint tileLightLoadOffset = tileOffset + 4;
    
#undef POINT_LIGHT_ARGS
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
#undef CONE_LIGHT_ARGS
#define CONE_LIGHT_ARGS \
    POINT_LIGHT_ARGS, \
    lightData.coneDir, \
    lightData.coneAngles
#undef SHADOWED_LIGHT_ARGS
#define SHADOWED_LIGHT_ARGS \
    CONE_LIGHT_ARGS, \
    lightData.shadowTextureMatrix, \
    lightIndex
	
    float3 normal = texRt1[pixelPos];
#if NORMAL
    return normal;
#endif
    // sphere
    uint n;
    for (n = 0; n < tileLightCountSphere; n++, tileLightLoadOffset += 4)
    {
        uint lightIndex = lightGrid.Load(tileLightLoadOffset);
        LightData lightData = lightBuffer[lightIndex];
        
        //color += ApplyPointLight(POINT_LIGHT_ARGS);
        float3 lightDir = lightData.pos - worldPos.xyz;
        float lightDistSq = dot(lightDir, lightDir);
        float invLightDist = rsqrt(lightDistSq);
        lightDir *= invLightDist;
        
        float distanceFalloff = lightData.radiusSq * (invLightDist * invLightDist);
        distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));
        
        float3 halfVec = normalize(lightDir - viewDir);
        float nDotH = saturate(dot(halfVec, normal));
        
        return nDotH;
    }
	
    return float3(1.0f, 0.0f, 0.0f);
}
