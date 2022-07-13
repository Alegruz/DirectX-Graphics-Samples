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

#if !defined(GBUFFER_NORMAL_DEPTH)
#if !defined(GBUFFER_SPECULAR)
Texture2D<float3> texDiffuse		: register(t0);
#endif
#if !defined(GBUFFER_ALBEDO)
Texture2D<float3> texSpecular		: register(t1);
//Texture2D<float4> texEmissive		: register(t2);
#endif
#endif
#if !defined(GBUFFER_ALBEDO) && !defined(GBUFFER_SPECULAR)
Texture2D<float3> texNormal			: register(t3);
//Texture2D<float4> texLightmap		: register(t4);
//Texture2D<float4> texReflection	: register(t5);
#if !defined(GBUFFER_NORMAL_DEPTH)
Texture2D<float> texSSAO			: register(t12);
Texture2D<float> texShadow : register(t13);
#endif
#endif

struct VSOutput
{
	sample float4 position : SV_Position;
    sample float3 projPosition : ProjPos;
	sample float3 worldPos : WorldPos;
    sample float2 uv : TexCoord0;
#if !defined(GBUFFER_ALBEDO) && !defined(GBUFFER_SPECULAR)
	sample float3 viewDir : TexCoord1;
	sample float3 shadowCoord : TexCoord2;
	sample float3 normal : Normal;
	sample float3 tangent : Tangent;
    sample float3 bitangent : Bitangent;
#endif
};

struct MRT
{
	float3 Color : SV_Target0;
#if !defined(GBUFFER_NORMAL_DEPTH) && !defined(GBUFFER_ALBEDO) && !defined(GBUFFER_SPECULAR)
	float4 Normal : SV_Target1;
#endif
};

[RootSignature(Renderer_RootSig)]
MRT main(VSOutput vsOutput)
{
	MRT mrt;

#if !defined(GBUFFER_WORLD_POS)
#if !defined(GBUFFER_NORMAL_DEPTH) && !defined(GBUFFER_ALBEDO) && !defined(GBUFFER_SPECULAR)
	uint2 pixelPos = uint2(vsOutput.position.xy);
#endif
	
#define SAMPLE_TEX(texName) texName.Sample(defaultSampler, vsOutput.uv)
	
#if !defined(GBUFFER_NORMAL_DEPTH) && !defined(GBUFFER_SPECULAR)
    float3 diffuseAlbedo = SAMPLE_TEX(texDiffuse);
	
#if !defined(GBUFFER_ALBEDO)
    float3 colorSum = 0;
	{
        float ao = texSSAO[pixelPos];
	    colorSum += ApplyAmbientLight( diffuseAlbedo, ao, AmbientColor );
    }
#endif
#endif

#if !defined(GBUFFER_ALBEDO)
#if !defined(GBUFFER_SPECULAR)
    float gloss = 128.0;
    float3 normal;
    {
        normal = SAMPLE_TEX(texNormal) * 2.0 - 1.0;
        AntiAliasSpecular(normal, gloss);
        float3x3 tbn = float3x3(normalize(vsOutput.tangent), normalize(vsOutput.bitangent), normalize(vsOutput.normal));
        normal = normalize(mul(normal, tbn));
    }
#endif
	
#if !defined(GBUFFER_NORMAL_DEPTH)
#if !defined(GBUFFER_SPECULAR)
    float3 specularAlbedo = float3( 0.56, 0.56, 0.56 );
#endif
    float specularMask = SAMPLE_TEX(texSpecular).g;
#if !defined(GBUFFER_SPECULAR)
    float3 viewDir = normalize(vsOutput.viewDir);
	
    colorSum += ApplyDirectionalLight(diffuseAlbedo, specularAlbedo, specularMask, gloss, normal, viewDir, SunDirection, SunColor, vsOutput.shadowCoord, texShadow);
	
	ShadeLights(colorSum, pixelPos,
		diffuseAlbedo,
		specularAlbedo,
		specularMask,
		gloss,
		normal,
		viewDir,
		vsOutput.worldPos
		);
#endif
#endif
#endif
#endif
	
#ifdef GBUFFER_WORLD_POS
    mrt.Color = vsOutput.worldPos;
#elif defined(GBUFFER_NORMAL_DEPTH)
	mrt.Color = mrt.Color = normal;
#elif defined(GBUFFER_ALBEDO)
	mrt.Color = diffuseAlbedo;
#elif defined(GBUFFER_SPECULAR)
    mrt.Color = SAMPLE_TEX(texSpecular);
#else
    mrt.Normal = float4(normal, 0);
    mrt.Color = colorSum;
    //mrt.Color = normalize(vsOutput.worldPos);
#endif
	return mrt;
}
