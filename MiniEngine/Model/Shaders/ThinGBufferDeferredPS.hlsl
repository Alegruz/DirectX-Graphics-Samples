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
    float3 RT0 : SV_Target0;
    float4 RT1 : SV_Target1;
    float4 RT2 : SV_Target2;
};

[RootSignature(Renderer_RootSig)]
MRT main(VSOutput vsOutput)
{
	MRT mrt;
	
    uint2 pixelPos = uint2(vsOutput.position.xy);
	
#define SAMPLE_TEX(texName) texName.Sample(defaultSampler, vsOutput.uv)
	
    float3 diffuseAlbedo = SAMPLE_TEX(texDiffuse);

    float3 colorSum = 0;
    float ao = texSSAO[pixelPos];
	{
        colorSum += ApplyAmbientLight(diffuseAlbedo, ao, AmbientColor);
    }
    
    float gloss = 128.0;
    float3 normal;
    {
        normal = SAMPLE_TEX(texNormal) * 2.0 - 1.0;
        AntiAliasSpecular(normal, gloss);
        float3x3 tbn = float3x3(normalize(vsOutput.tangent), normalize(vsOutput.bitangent), normalize(vsOutput.normal));
        normal = normalize(mul(normal, tbn));
    }
    
    float4 rt1Data = float4(
#if NO_ENCODING || BASELINE
        BaseEncode(half3(
            normal.x * !((asuint(normal.x) & 0x7fffffff) > 0x7f800000),
            normal.y * !((asuint(normal.y) & 0x7fffffff) > 0x7f800000),
            normal.z * !((asuint(normal.z) & 0x7fffffff) > 0x7f800000)
        )).xyz,
#elif Z_RECONSTRUCTION
        BaseEncode(half3(
            normal.x * !((asuint(normal.x) & 0x7fffffff) > 0x7f800000),
            normal.y * !((asuint(normal.y) & 0x7fffffff) > 0x7f800000),
            normal.z * !((asuint(normal.z) & 0x7fffffff) > 0x7f800000)
        )).xy,
#endif
        normal.z < 0.0f,
        gloss / 256.0
    );
        
    mrt.RT1 = rt1Data;
	
    float3 specularAlbedo = float3(0.56, 0.56, 0.56);
    float specularMask = SAMPLE_TEX(texSpecular).g;

    float3 viewDir = normalize(vsOutput.viewDir);
	
    // Deferred Shading in S.T.A.L.K.E.R., Oleksandr Shyshkovtsov, GPU Gems 2
    float nDotL = saturate(dot(normal, SunDirection));
    // Pixels facing away from the sun don't need to have complex shading applied
    // Pixels with AO term of zero can be excluded
    if (nDotL * ao > 0.0)
    {
        colorSum += ApplyDirectionalLight(diffuseAlbedo, specularAlbedo, specularMask, gloss, normal, viewDir, SunDirection, SunColor, vsOutput.shadowCoord, texShadow);
    }
    
    //mrt.RT0 = float4(colorSum, gloss / 256.0);
    mrt.RT0 = float3(colorSum);
    mrt.RT2 = float4(diffuseAlbedo, specularMask);
    
	return mrt;
}
