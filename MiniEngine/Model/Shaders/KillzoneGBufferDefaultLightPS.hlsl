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
#include "LightingDefault.hlsli"

cbuffer StartVertex : register(b1)
{
    uint materialIdx;
    float4x4 ModelToProjection;
    float4x4 InvViewProj;
    float4x4 InvProj;
    float4x4 modelToShadow;
    float4 ViewerPos;
    uint2 ViewportSize;
};

Texture2D<float> texDepth : register(t18);

// GBuffers
Texture2D<float3> texRt0	: register(t21);
Texture2D<half4> texRt1 : register(t22);
Texture2D<float4> texRt2	: register(t23);
Texture2D<float4> texRt3    : register(t24);

struct VSOutput
{
	sample float4 projPos : SV_Position;
};

[RootSignature(Renderer_RootSig)]
float3 main(VSOutput vsOutput) : SV_Target
{
    uint2 pixelPos = uint2(vsOutput.projPos.xy);
	
    float depth = texDepth[pixelPos];
    float3 color = texRt0[pixelPos];
    half4 rt1Data = texRt1[pixelPos];
    float4 rt2Data = texRt2[pixelPos];
    float4 rt3Data = texRt3[pixelPos];
    if (depth <= 0.0)
    {
        color = 0.0;
        return color;
    }
#if DEPTH
    return sqrt(depth);
#endif
#if LIGHT_ACCUMULATION
    return color;
#endif
    
    float3 normal = rt1Data.xyz;
#if NORMAL
    return normal;
#endif
    
    float specularMask = rt2Data.a;
    
    // Deferred Shading, Shawn Hargreaves, GDC 2004.
    // Deferred Shading, Shawn Hargreaves. Mark Harris, NDC 2004.
    // Reconstruct position from depth
    float4 clipSpacePosition = float4((vsOutput.projPos.x / ViewportSize.x) * 2.0f - 1.0f, (vsOutput.projPos.y / ViewportSize.y) * 2.0f - 1.0f, depth, 1.0f);
    clipSpacePosition.y *= -1.0f;
    float4 worldPos = mul(InvViewProj, clipSpacePosition);
    worldPos /= worldPos.w;
    
#if MOTION_VECTOR
    return normalize(worldPos.xyz);
#endif
#if SPEC_INTENSITY
    return specularMask;
#endif
    float3 diffuseAlbedo = rt3Data.rgb;
    float gloss = rt3Data.a * 256.0;
#if GLOSS
    return gloss / 256.0;
#endif
#if DIFFUSE_ALBEDO
    return diffuseAlbedo;
#endif
#if SUN_OCCLUSION
    return rt3Data.a;
#endif
  
    float3 specularAlbedo = float3( 0.56, 0.56, 0.56 );
    float3 viewDir = normalize(worldPos.xyz - ViewerPos.xyz);
    float3 shadowCoord = mul(modelToShadow, float4(worldPos.xyz, 1.0)).xyz;
    
	ShadeLights(
		color,
		pixelPos,
		diffuseAlbedo,
		specularAlbedo,
		specularMask,
		gloss,
		normal,
		viewDir,
		worldPos.xyz
		);
    
    return color;
}
