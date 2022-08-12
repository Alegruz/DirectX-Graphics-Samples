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
    float3 ViewerPos;
};

//Texture2D<float3> texDiffuse : register(t0);
//Texture2D<float3> texSpecular : register(t1);
//Texture2D<float4> texEmissive		: register(t2);
//Texture2D<float3> texNormal : register(t3);
//Texture2D<float4> texLightmap		: register(t4);
//Texture2D<float4> texReflection	: register(t5);
Texture2D<float> texSSAO : register(t12);
Texture2D<float> texShadow : register(t13);
Texture2D<float> texDepth : register(t18);

// GBuffers
Texture2D<float4> texWorldPos	: register(t21);
Texture2D<float4> texNormalDepth	: register(t22);
Texture2D<float4> texAlbedo	: register(t23);
Texture2D<float4> texSpecular : register(t24);

struct VSOutput
{
	sample float4 projPos : SV_Position;
	//sample float3 worldPos : WorldPos;
    //sample float2 uv : TexCoord0;
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
#if GBUFFER_DEPTH
    return sqrt(sqrt(depth));
#endif
    float4 worldPos = texWorldPos[pixelPos];
#if GBUFFER_WORLD_POS
    return normalize(worldPos.xyz);
#endif
    
    float4 albedoData = texAlbedo[pixelPos];
    float3 diffuseAlbedo = albedoData.rgb;
#if GBUFFER_ALBEDO
    return diffuseAlbedo;
#endif
    float gloss = texSpecular[pixelPos].a;
#if GBUFFER_GLOSS
    return gloss / 256.0;
#endif
    float specularMask = texSpecular[pixelPos].g;
#if GBUFFER_SPECULAR
    return specularMask;
#endif
    float3 normal = texNormalDepth[pixelPos].xyz;
#if GBUFFER_NORMAL
    return normalize(normal);
#endif
    
    float3 colorSum = 0;
	{
        float ao = texSSAO[pixelPos];
	    colorSum += ApplyAmbientLight( diffuseAlbedo, ao, AmbientColor );
    }
	
    float3 specularAlbedo = float3( 0.56, 0.56, 0.56 );
    float3 viewDir = normalize(worldPos.xyz - ViewerPos);
    float3 shadowCoord = mul(modelToShadow, float4(worldPos.xyz, 1.0)).xyz;
    colorSum += ApplyDirectionalLight(diffuseAlbedo, specularAlbedo, specularMask, gloss, normal, viewDir, SunDirection, SunColor, shadowCoord, texShadow);
	
	ShadeLights(
		colorSum, 
		pixelPos,
		diffuseAlbedo,
		specularAlbedo,
		specularMask,
		gloss,
		normal,
		viewDir,
		worldPos.xyz
		);
	
    color = colorSum;
	
    return color;
}
