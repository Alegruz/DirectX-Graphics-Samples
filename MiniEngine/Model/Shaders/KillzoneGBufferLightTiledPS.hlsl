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
    float4x4 InvProj;
    float4x4 modelToShadow;
    float4 ViewerPos;
//    float NearZ;
//    float FarZ;
    uint2 ViewportSize;
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
//Texture2D<float4> texRt0 : register(t21);
//Texture2D<float2> texRt1	: register(t22);
//Texture2D<float3> texRt1    : register(t22);
//Texture2D<float4> texRt1	: register(t22);
Texture2D<half4> texRt1 : register(t22);    // Baseline
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
    //float4 rt0Data = texRt0[pixelPos];
    //color = rt0Data.rgb;
    color = texRt0[pixelPos];
    //float gloss = rt0Data.a * 256.0;
#if LIGHT_ACCUMULATION
    return color;
#endif
//#if GLOSS
//    return gloss / 256.0;
//#endif

    //float2 rt1Data = texRt1[pixelPos];
    //float4 rt1Data = texRt1[pixelPos];
    half4 rt1Data = texRt1[pixelPos];

    float3 normal = (float3) rt1Data.xyz;
    
#if NORMAL
    return normal;
#endif
    
    float4 rt2Data = texRt2[pixelPos];
    //float3 worldPos = rt2Data.xyz;
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
    float4 rt3Data = texRt3[pixelPos];
    float3 diffuseAlbedo = rt3Data.rgb;
    if (dot(normal, 1.0) == 0.0 && dot(diffuseAlbedo, 1.0) == 0.0)
    {
        discard;
    }
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
    //float3 viewDir = normalize(float3(vsOutput.projPos.xy, depth) - ViewerPos);
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
    
    // Thibieroz, Nicolas, “Deferred Shading with Multisampling Anti-Aliasing in DirectX 10,” in Wolfgang Engel, ed., ShaderX7, Charles River Media, pp. 225–242, 2009.
    if (dot(color, 1.0f) == 0)
    {
        discard;
    }
    
    return color;
}
