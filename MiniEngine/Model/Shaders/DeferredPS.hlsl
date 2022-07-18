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

Texture2D<float3> texDiffuse		: register(t0);
Texture2D<float3> texSpecular		: register(t1);
//Texture2D<float4> texEmissive		: register(t2);
Texture2D<float3> texNormal			: register(t3);
//Texture2D<float4> texLightmap		: register(t4);
//Texture2D<float4> texReflection	: register(t5);
Texture2D<float> texSSAO			: register(t12);
Texture2D<float> texShadow : register(t13);

struct VSOutput
{
	sample float4 position : SV_Position;
	sample float3 worldPos : WorldPos;
    sample float2 uv : TexCoord0;
	sample float3 viewDir : TexCoord1;
	sample float3 shadowCoord : TexCoord2;
	sample float3 normal : Normal;
	sample float3 tangent : Tangent;
    sample float3 bitangent : Bitangent;
};

struct MRT
{
	float4 WorldPos : SV_Target0;
    float4 NormalDepth : SV_Target1;
    float4 Albedo : SV_Target2;
    float4 Specular : SV_Target3;
};

[RootSignature(Renderer_RootSig)]
MRT main(VSOutput vsOutput)
{
	MRT mrt;
	
	uint2 pixelPos = uint2(vsOutput.position.xy);
	
# define SAMPLE_TEX(texName) texName.Sample(defaultSampler, vsOutput.uv)
	
    //mrt.WorldPos = float4(SAMPLE_TEX(texDiffuse), SAMPLE_TEX(texSpecular).g);
    mrt.WorldPos = float4(vsOutput.worldPos, 1.0f);
    
    float gloss = 128.0;
    float3 normal;
    {
        normal = SAMPLE_TEX(texNormal) * 2.0 - 1.0;
        AntiAliasSpecular(normal, gloss);
        float3x3 tbn = float3x3(normalize(vsOutput.tangent), normalize(vsOutput.bitangent), normalize(vsOutput.normal));
        normal = normalize(mul(normal, tbn));
    }
    mrt.NormalDepth = float4(normal, vsOutput.position.z / vsOutput.position.w);
    mrt.Albedo = float4(SAMPLE_TEX(texDiffuse), 0.0f);
    float3 specular = SAMPLE_TEX(texSpecular);
    mrt.Specular = float4(specular, gloss);
    
	return mrt;
}
