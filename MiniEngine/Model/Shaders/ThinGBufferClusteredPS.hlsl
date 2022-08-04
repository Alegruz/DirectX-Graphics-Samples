﻿//
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
#include "LightingClustered.hlsli"

cbuffer StartVertex : register(b1)
{
    uint materialIdx;
    float4x4 ModelToProjection;
    float4x4 InvViewProj;
    float4x4 InvProj;
#if Z_RECONSTRUCTION || SPHEREMAP_TRANSFORM
    float4x4 InvView;
#endif
    float4x4 modelToShadow;
    float4 ViewerPos;
    uint2 ViewportSize;
    float NearZ;
    float FarZ;
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
Texture2D<float3> texRt0	: register(t21);
Texture2D<float4> texRt1	: register(t22);
Texture2D<float4> texRt2	: register(t23);

struct VSOutput
{
	sample float4 projPos : SV_Position;
	sample float3 worldPos : WorldPos;
    sample float2 uv : TexCoord0;
};

float4 TransformScreenSpaceToViewSpace(float4 vec);
float4 TransformClipSpaceToViewSpace(float4 vec);

float4 TransformScreenSpaceToViewSpace(float4 vec)
{
    // Convert to NDC
    float2 texCoord = vec.xy / float2(ViewportSize);
    
    // Convert to clip space
    float4 clipSpace = float4(texCoord * 2.0f - 1.0f, vec.z, vec.w);
    
    return TransformClipSpaceToViewSpace(clipSpace);
}

float4 TransformClipSpaceToViewSpace(float4 vec)
{
    // View space transformation
    float4 viewSpace = mul(InvProj, vec);
    
    // Perspective projection
    viewSpace /= viewSpace.w;
    
    return viewSpace;
}

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
    //return sqrt((float) slice / (float) TileCount[2]);
    return sqrt(depth);
#endif
    
    color  = texRt0[pixelPos];
    
#if LIGHT_ACCUMULATION
    return color;
#endif
    
    float4 rt1Data = texRt1[pixelPos];
    
#if NO_ENCODING || BASELINE
    float3 normal = (float3) BaseDecode(rt1Data);
#elif Z_RECONSTRUCTION || SPHEREMAP_TRANSFORM
    float3 normal = (float3) BaseDecode(rt1Data.xy, InvView);
#elif SPHERICAL_COORDNATES || OCTAHEDRON_NORMAL
    float3 normal = (float3) BaseDecode(rt1Data.xy);
#endif
    
#if NORMAL
    return normal;
#endif
    
    float4 rt2Data = texRt2[pixelPos];
    float specularMask = rt2Data.a;
    
    float4 clipSpacePosition = float4((vsOutput.projPos.x / (float) ViewportSize.x) * 2.0f - 1.0f, -((vsOutput.projPos.y / (float) ViewportSize.y) * 2.0f - 1.0f), depth, 1.0f);
    float4 worldPos = mul(InvViewProj, clipSpacePosition);
    worldPos /= worldPos.w;
    
#if SPEC_INTENSITY
    return specularMask;
#endif
    float3 diffuseAlbedo = rt2Data.rgb;
    if (dot(normal, 1.0) == 0.0 && dot(diffuseAlbedo, 1.0) == 0.0)
    {
        discard;
    }
    float gloss = rt1Data.a * 256.0;
#if GLOSS
    return gloss / 256.0;
#endif
#if DIFFUSE_ALBEDO
    return diffuseAlbedo;
#endif
  
    float3 specularAlbedo = float3( 0.56, 0.56, 0.56 );
    //float3 viewDir = normalize(float3(vsOutput.projPos.xy, depth) - ViewerPos);
    float3 viewDir = normalize(worldPos.xyz - ViewerPos.xyz);
    float3 shadowCoord = mul(modelToShadow, float4(worldPos.xyz, 1.0f)).xyz;
    
    //float viewSpaceDepth = TransformScreenSpaceToViewSpace(float4(0.0f, 0.0f, depth, 1.0f));
    float4 viewDepth = mul(InvProj, clipSpacePosition);
    viewDepth /= viewDepth.w;

    ShadeLights(
		color,
		pixelPos,
		diffuseAlbedo,
		specularAlbedo,
    	specularMask,
		gloss,
		normal,
		viewDir,
		worldPos.xyz,
        viewDepth.z,
        NearZ,
        FarZ,
        ViewportSize
		);

    // Thibieroz, Nicolas, “Deferred Shading with Multisampling Anti-Aliasing in DirectX 10,” in Wolfgang Engel, ed., ShaderX7, Charles River Media, pp. 225–242, 2009.
    if (dot(color, 1.0f) == 0)
    {
        discard;
    }
    
    return color;
}
