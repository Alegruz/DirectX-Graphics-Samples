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

//Texture2D<float3> texDiffuse : register(t0);
//Texture2D<float3> texSpecular : register(t1);
//Texture2D<float4> texEmissive		: register(t2);
//Texture2D<float3> texNormal : register(t3);
//Texture2D<float4> texLightmap		: register(t4);
//Texture2D<float4> texReflection	: register(t5);
Texture2D<float> texSSAO : register(t12);
Texture2D<float> texShadow : register(t13);
Texture2D<float4> gbufferRtv0	: register(t21);
Texture2D<float4> gbufferRtv1	: register(t22);
Texture2D<float4> gbufferRtv2	: register(t23);
Texture2D<float4> gbufferNormal : register(t24);


struct VSOutput
{
	sample float4 position : SV_Position;
	sample float3 worldPos : WorldPos;
    sample float2 uv : TexCoord0;
	sample float3 viewDir : TexCoord1;
	sample float3 shadowCoord : TexCoord2;
};

struct MRT
{
	float3 Color : SV_Target0;
};

[RootSignature(Renderer_RootSig)]
MRT main(VSOutput vsOutput)
{
	MRT mrt;
	
	uint2 pixelPos = uint2(vsOutput.position.xy);
	
#define SAMPLE_TEX(texName) texName.Sample(defaultSampler, vsOutput.uv)
	
    //float4 rtv0Data = SAMPLE_TEX(gbufferRtv0);
    float4 rtv0Data = gbufferRtv0[pixelPos];
    float3 diffuseAlbedo = rtv0Data.rgb;
    float specularMask = rtv0Data.w;
	
    float3 colorSum = 0;
	{
        float ao = texSSAO[pixelPos];
	    colorSum += ApplyAmbientLight( diffuseAlbedo, ao, AmbientColor );
    }
	
    float gloss = gbufferRtv1[pixelPos].x;
    float3 normal = gbufferNormal[pixelPos].xyz;
	
    float3 specularAlbedo = float3( 0.56, 0.56, 0.56 );
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
	
    mrt.Color = colorSum;
	
	return mrt;
}
