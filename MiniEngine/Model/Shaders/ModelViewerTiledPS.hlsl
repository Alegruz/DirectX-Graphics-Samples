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

Texture2D<float3> texDiffuse : register(t0);
Texture2D<float3> texSpecular : register(t1);
Texture2D<float3> texNormal : register(t3);
Texture2D<float> texSSAO : register(t12);
Texture2D<float> texShadow : register(t13);

struct VSOutput
{
    sample float4 position : SV_Position;
    sample float3 viewPos : ViewPos;
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
    float3 Color : SV_Target0;
    float4 Normal : SV_Target1;
};

[RootSignature(Renderer_RootSig)]
MRT main(VSOutput vsOutput)
{
    MRT mrt;
	
    uint2 pixelPos = uint2(vsOutput.position.xy);
	
#define SAMPLE_TEX(texName) texName.Sample(defaultSampler, vsOutput.uv)
	
    float3 diffuseAlbedo = SAMPLE_TEX(texDiffuse);

    float gloss = 128.0;
    float3 normal;
    {
        normal = SAMPLE_TEX(texNormal) * 2.0 - 1.0;
        AntiAliasSpecular(normal, gloss);
        float3x3 tbn = float3x3(normalize(vsOutput.tangent), normalize(vsOutput.bitangent), normalize(vsOutput.normal));
        normal = normalize(mul(normal, tbn));
    }
	
    float specularMask = SAMPLE_TEX(texSpecular).g;
	
    mrt.Normal = float4(normal, 0.0f);
    
#if WORLD_POS
	mrt.Color = vsOutput.worldPos;
	
	return mrt;
#elif DIFFUSE_ALBEDO 
	mrt.Color = diffuseAlbedo;
	
	return mrt;
#elif GLOSS
	mrt.Color = gloss / 256.0f;
	
	return mrt;
#elif NORMAL
	mrt.Color = normal;
	
	return mrt;
#elif SPEC_INTENSITY
	mrt.Color = specularMask;
	
	return mrt;
#endif
	
    float3 colorSum = 0;
	{
        float ao = texSSAO[pixelPos];
        colorSum += ApplyAmbientLight(diffuseAlbedo, ao, AmbientColor);
    }
	
    float3 specularAlbedo = float3(0.56, 0.56, 0.56);
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
    
    // Thibieroz, Nicolas, “Deferred Shading with Multisampling Anti-Aliasing in DirectX 10,” in Wolfgang Engel, ed., ShaderX7, Charles River Media, pp. 225–242, 2009.
    if (dot(colorSum, 1.0f) == 0)
    {
        discard;
    }
    
    mrt.Color = colorSum;
    
    return mrt;
}
