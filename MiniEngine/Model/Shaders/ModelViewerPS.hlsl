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
//Texture2D<float4> texEmissive		: register(t2);
Texture2D<float3> texNormal : register(t3);
//Texture2D<float4> texLightmap		: register(t4);
//Texture2D<float4> texReflection	: register(t5);
Texture2D<float> texSSAO : register(t12);
Texture2D<float> texShadow : register(t13);

struct VSOutput
{
    sample float4 position : SV_Position;
    sample float3 projPosition : ProjPos;
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
	
	//ShadeLights(colorSum, pixelPos,
	//	diffuseAlbedo,
	//	specularAlbedo,
	//	specularMask,
	//	gloss,
	//	normal,
	//	viewDir,
	//	vsOutput.worldPos
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
    float3 worldPos = vsOutput.worldPos;
#undef POINT_LIGHT_ARGS
#define POINT_LIGHT_ARGS \
    diffuseAlbedo, \
    specularAlbedo, \
    specularMask, \
    gloss, \
    normal, \
    viewDir, \
    worldPos, \
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
	
    // sphere
    uint n;
    for (n = 0; n < tileLightCountSphere; n++, tileLightLoadOffset += 4)
    {
        uint lightIndex = lightGrid.Load(tileLightLoadOffset);
        LightData lightData = lightBuffer[lightIndex];
        
        float3 lightDir = lightData.pos - worldPos;
        float lightDistSq = dot(lightDir, lightDir);
        float invLightDist = rsqrt(lightDistSq);
        lightDir *= invLightDist;
        
        float distanceFalloff = lightData.radiusSq * (invLightDist * invLightDist);
        distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));
        
        float3 halfVec = normalize(lightDir - viewDir);
        float nDotH = saturate(dot(halfVec, normal));
        
        mrt.Color = nDotH;
        
        return mrt;
        //colorSum += ApplyPointLight(POINT_LIGHT_ARGS);
    }
	
    mrt.Color = float3(1.0f, 0.0f, 0.0f);
    //mrt.Color = float3(normalize(float2(tilePos)), 0.0f);
	
    return mrt;
}
