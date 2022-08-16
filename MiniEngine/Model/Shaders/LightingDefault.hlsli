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
//				Alex Nankervis
//
// Thanks to Michal Drobot for his feedback.

#include "Common.hlsli"

#define MAX_LIGHTS (192)

struct LightData
{
    float3 pos;
    float radiusSq;

    float3 color;
    uint type;

    float3 coneDir;
    float2 coneAngles; // x = 1.0f / (cos(coneInner) - cos(coneOuter)), y = cos(coneOuter)

    float4x4 shadowTextureMatrix;
};

cbuffer PSConstants : register(b0)
{
    float3 SunDirection;
    float3 SunColor;
    float3 AmbientColor;
    float4 ShadowTexelSize;

    float4 InvTileDim;
    uint4 TileCount;
    uint4 FirstLightIndex;

    uint FrameIndexMod2;
}

StructuredBuffer<LightData> lightBuffer : register(t14);
Texture2DArray<float> lightShadowArrayTex : register(t15);
ByteAddressBuffer lightGrid : register(t16);
ByteAddressBuffer lightGridBitMask : register(t17);

void AntiAliasSpecular( inout float3 texNormal, inout float gloss )
{
    float normalLenSq = dot(texNormal, texNormal);
    float invNormalLen = rsqrt(normalLenSq);
    texNormal *= invNormalLen;
    float normalLen = normalLenSq * invNormalLen;
	float flatness = saturate(1 - abs(ddx(normalLen)) - abs(ddy(normalLen)));
	gloss = exp2(lerp(0, log2(gloss), flatness));
}

// Apply fresnel to modulate the specular albedo
void FSchlick( inout float3 specular, inout float3 diffuse, float3 lightDir, float3 halfVec )
{
    float fresnel = pow(1.0 - saturate(dot(lightDir, halfVec)), 5.0);
    specular = lerp(specular, 1, fresnel);
    diffuse = lerp(diffuse, 0, fresnel);
}

float3 ApplyAmbientLight(
    float3	diffuse,	// Diffuse albedo
    float	ao,			// Pre-computed ambient-occlusion
    float3	lightColor	// Radiance of ambient light
    )
{
    return ao * diffuse * lightColor;
}

float GetDirectionalShadow( float3 ShadowCoord, Texture2D<float> texShadow )
{
#ifdef SINGLE_SAMPLE
    float result = texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy, ShadowCoord.z );
#else
    const float Dilation = 2.0;
    float d1 = Dilation * ShadowTexelSize.x * 0.125;
    float d2 = Dilation * ShadowTexelSize.x * 0.875;
    float d3 = Dilation * ShadowTexelSize.x * 0.625;
    float d4 = Dilation * ShadowTexelSize.x * 0.375;
    float result = (
        2.0 * texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy, ShadowCoord.z ) +
        texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2(-d2,  d1), ShadowCoord.z ) +
        texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2(-d1, -d2), ShadowCoord.z ) +
        texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2( d2, -d1), ShadowCoord.z ) +
        texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2( d1,  d2), ShadowCoord.z ) +
        texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2(-d4,  d3), ShadowCoord.z ) +
        texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2(-d3, -d4), ShadowCoord.z ) +
        texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2( d4, -d3), ShadowCoord.z ) +
        texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2( d3,  d4), ShadowCoord.z )
        ) / 10.0;
#endif
    return result * result;
}

float GetShadowConeLight(uint lightIndex, float3 shadowCoord)
{
    float result = lightShadowArrayTex.SampleCmpLevelZero(
        shadowSampler, float3(shadowCoord.xy, lightIndex), shadowCoord.z);
    return result * result;
}

float3 ApplyLightCommon(
    float3	diffuseColor,	// Diffuse albedo
    float3	specularColor,	// Specular albedo
    float	specularMask,	// Where is it shiny or dingy?
    float	gloss,			// Specular power
    float3	normal,			// World-space normal
    float3	viewDir,		// World-space vector from eye to point
    float3	lightDir,		// World-space vector from point to light
    float3	lightColor		// Radiance of directional light
    )
{
    float3 halfVec = normalize(lightDir - viewDir);
    float nDotH = saturate(dot(halfVec, normal));

    FSchlick( diffuseColor, specularColor, lightDir, halfVec );

    float specularFactor = specularMask * pow(nDotH, gloss) * (gloss + 2) / 8;

    float nDotL = saturate(dot(normal, lightDir));

    return nDotL * lightColor * (diffuseColor + specularFactor * specularColor);
}

float3 ApplyDirectionalLight(
    float3	diffuseColor,	// Diffuse albedo
    float3	specularColor,	// Specular albedo
    float	specularMask,	// Where is it shiny or dingy?
    float	gloss,			// Specular power
    float3	normal,			// World-space normal
    float3	viewDir,		// World-space vector from eye to point
    float3	lightDir,		// World-space vector from point to light
    float3	lightColor,		// Radiance of directional light
    float3	shadowCoord,	// Shadow coordinate (Shadow map UV & light-relative Z)
	Texture2D<float> ShadowMap
    )
{
    float shadow = GetDirectionalShadow(shadowCoord, ShadowMap);

    return shadow * ApplyLightCommon(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        lightDir,
        lightColor
        );
}

float3 ApplyPointLight(
    float3	diffuseColor,	// Diffuse albedo
    float3	specularColor,	// Specular albedo
    float	specularMask,	// Where is it shiny or dingy?
    float	gloss,			// Specular power
    float3	normal,			// World-space normal
    float3	viewDir,		// World-space vector from eye to point
    float3	worldPos,		// World-space fragment position
    float3	lightPos,		// World-space light position
    float	lightRadiusSq,
    float3	lightColor		// Radiance of directional light
    )
{
    float3 lightDir = lightPos - worldPos;
    float lightDistSq = dot(lightDir, lightDir);
    float invLightDist = rsqrt(lightDistSq);
    lightDir *= invLightDist;

    // modify 1/d^2 * R^2 to fall off at a fixed radius
    // (R/d)^2 - d/R = [(1/d^2) - (1/R^2)*(d/R)] * R^2
    float distanceFalloff = lightRadiusSq * (invLightDist * invLightDist);
    distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));
    
    return distanceFalloff * ApplyLightCommon(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        lightDir,
        lightColor
        );
}

float3 ApplyConeLight(
    float3	diffuseColor,	// Diffuse albedo
    float3	specularColor,	// Specular albedo
    float	specularMask,	// Where is it shiny or dingy?
    float	gloss,			// Specular power
    float3	normal,			// World-space normal
    float3	viewDir,		// World-space vector from eye to point
    float3	worldPos,		// World-space fragment position
    float3	lightPos,		// World-space light position
    float	lightRadiusSq,
    float3	lightColor,		// Radiance of directional light
    float3	coneDir,
    float2	coneAngles
    )
{
    float3 lightDir = lightPos - worldPos;
    float lightDistSq = dot(lightDir, lightDir);
    float invLightDist = rsqrt(lightDistSq);
    lightDir *= invLightDist;

    // modify 1/d^2 * R^2 to fall off at a fixed radius
    // (R/d)^2 - d/R = [(1/d^2) - (1/R^2)*(d/R)] * R^2
    float distanceFalloff = lightRadiusSq * (invLightDist * invLightDist);
    distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));

    float coneFalloff = dot(-lightDir, coneDir);
    coneFalloff = saturate((coneFalloff - coneAngles.y) * coneAngles.x);

    return (coneFalloff * distanceFalloff) * ApplyLightCommon(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        lightDir,
        lightColor
        );
}

float3 ApplyConeShadowedLight(
    float3	diffuseColor,	// Diffuse albedo
    float3	specularColor,	// Specular albedo
    float	specularMask,	// Where is it shiny or dingy?
    float	gloss,			// Specular power
    float3	normal,			// World-space normal
    float3	viewDir,		// World-space vector from eye to point
    float3	worldPos,		// World-space fragment position
    float3	lightPos,		// World-space light position
    float	lightRadiusSq,
    float3	lightColor,		// Radiance of directional light
    float3	coneDir,
    float2	coneAngles,
    float4x4 shadowTextureMatrix,
    uint	lightIndex
    )
{
    float4 shadowCoord = mul(shadowTextureMatrix, float4(worldPos, 1.0));
    shadowCoord.xyz *= rcp(shadowCoord.w);
    float shadow = GetShadowConeLight(lightIndex, shadowCoord.xyz);

    return shadow * ApplyConeLight(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        worldPos,
        lightPos,
        lightRadiusSq,
        lightColor,
        coneDir,
        coneAngles
        );
}

// options for F+ variants and optimizations
#if 0 // SM6.0
#define _WAVE_OP
#endif

// options for F+ variants and optimizations
#ifdef _WAVE_OP // SM 6.0 (new shader compiler)

// choose one of these:
//# define BIT_MASK
# define BIT_MASK_SORTED
//# define SCALAR_LOOP
//# define SCALAR_BRANCH

// enable to amortize latency of vector read in exchange for additional VGPRs being held
# define LIGHT_GRID_PRELOADING

// configured for 32 sphere lights, 64 cone lights, and 32 cone shadowed lights
# define POINT_LIGHT_GROUPS			1
# define SPOT_LIGHT_GROUPS			2
# define SHADOWED_SPOT_LIGHT_GROUPS	1
# define POINT_LIGHT_GROUPS_TAIL			POINT_LIGHT_GROUPS
# define SPOT_LIGHT_GROUPS_TAIL				POINT_LIGHT_GROUPS_TAIL + SPOT_LIGHT_GROUPS
# define SHADOWED_SPOT_LIGHT_GROUPS_TAIL	SPOT_LIGHT_GROUPS_TAIL + SHADOWED_SPOT_LIGHT_GROUPS

uint GetGroupBits(uint groupIndex, uint tileIndex, uint lightBitMaskGroups[4])
{
#ifdef LIGHT_GRID_PRELOADING
    return lightBitMaskGroups[groupIndex];
#else
    return lightGridBitMask.Load(tileIndex * 16 + groupIndex * 4);
#endif
}

uint WaveOr(uint mask)
{
    return WaveActiveBitOr(mask);
}

uint64_t Ballot64(bool b)
{
    uint4 ballots = WaveActiveBallot(b);
    return (uint64_t)ballots.y << 32 | (uint64_t)ballots.x;
}

#endif // _WAVE_OP

// Helper function for iterating over a sparse list of bits.  Gets the offset of the next
// set bit, clears it, and returns the offset.
uint PullNextBit( inout uint bits )
{
    uint bitIndex = firstbitlow(bits);
    bits ^= 1 << bitIndex;
    return bitIndex;
}

void ShadeLights(inout float3 colorSum, uint2 pixelPos,
	float3	diffuseAlbedo,	// Diffuse albedo
	float3	specularAlbedo,	// Specular albedo
	float	specularMask,	// Where is it shiny or dingy?
	float gloss,
	float3 normal,
	float3 viewDir,
	float3 worldPos
	)
{
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

#define CONE_LIGHT_ARGS \
    POINT_LIGHT_ARGS, \
    lightData.coneDir, \
    lightData.coneAngles

#define SHADOWED_LIGHT_ARGS \
    CONE_LIGHT_ARGS, \
    lightData.shadowTextureMatrix, \
    lightIndex

#if defined(BIT_MASK)
    uint64_t threadMask = Ballot64(tileIndex != ~0); // attempt to get starting exec mask

    for (uint groupIndex = 0; groupIndex < 4; groupIndex++)
    {
        // combine across threads
        uint groupBits = WaveOr(GetGroupBits(groupIndex, tileIndex, lightBitMaskGroups));

        while (groupBits != 0)
        {
            uint bitIndex = PullNextBit(groupBits);
            uint lightIndex = 32 * groupIndex + bitIndex;

            LightData lightData = lightBuffer[lightIndex];

            if (lightIndex < FirstLightIndex.x) // sphere
            {
                colorSum += ApplyPointLight(POINT_LIGHT_ARGS);
            }
            else if (lightIndex < FirstLightIndex.y) // cone
            {
                colorSum += ApplyConeLight(CONE_LIGHT_ARGS);
            }
            else // cone w/ shadow map
            {
                colorSum += ApplyConeShadowedLight(SHADOWED_LIGHT_ARGS);
            }
        }
    }

#elif defined(BIT_MASK_SORTED)

    // Get light type groups - these can be predefined as compile time constants to enable unrolling and better scheduling of vector reads
    uint pointLightGroupTail		= POINT_LIGHT_GROUPS_TAIL;
    uint spotLightGroupTail			= SPOT_LIGHT_GROUPS_TAIL;
    uint spotShadowLightGroupTail	= SHADOWED_SPOT_LIGHT_GROUPS_TAIL;

    uint groupBitsMasks[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < 4; i++)
    {
        // combine across threads
        groupBitsMasks[i] = WaveOr(GetGroupBits(i, tileIndex, lightBitMaskGroups));
    }

    uint groupIndex;

    for (groupIndex = 0; groupIndex < pointLightGroupTail; groupIndex++)
    {
        uint groupBits = groupBitsMasks[groupIndex];

        while (groupBits != 0)
        {
            uint bitIndex = PullNextBit(groupBits);
            uint lightIndex = 32 * groupIndex + bitIndex;

            // sphere
            LightData lightData = lightBuffer[lightIndex];
            colorSum += ApplyPointLight(POINT_LIGHT_ARGS);
        }
    }

    for (groupIndex = pointLightGroupTail; groupIndex < spotLightGroupTail; groupIndex++)
    {
        uint groupBits = groupBitsMasks[groupIndex];

        while (groupBits != 0)
        {
            uint bitIndex = PullNextBit(groupBits);
            uint lightIndex = 32 * groupIndex + bitIndex;

            // cone
            LightData lightData = lightBuffer[lightIndex];
            colorSum += ApplyConeLight(CONE_LIGHT_ARGS);
        }
    }

    for (groupIndex = spotLightGroupTail; groupIndex < spotShadowLightGroupTail; groupIndex++)
    {
        uint groupBits = groupBitsMasks[groupIndex];

        while (groupBits != 0)
        {
            uint bitIndex = PullNextBit(groupBits);
            uint lightIndex = 32 * groupIndex + bitIndex;

            // cone w/ shadow map
            LightData lightData = lightBuffer[lightIndex];
            colorSum += ApplyConeShadowedLight(SHADOWED_LIGHT_ARGS);
        }
    }

#else // SM 5.0 (no wave intrinsics)
    
#if FALSE_POSITIVE_RATE || LIGHT_DENSITY
    float falsePositiveCount = 0;
    uint lightCount = 0;
#endif
    // sphere
    uint lightIndex;
    for (lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex)
    {
        LightData lightData = lightBuffer[lightIndex];
        switch (lightData.type)
        {
            case 0:
            // Sphere
            {
#if FALSE_POSITIVE_RATE || LIGHT_DENSITY
                float3 lightDir = lightData.pos - worldPos.xyz;
                float lightDistSq = dot(lightDir, lightDir);
                float invLightDist = rsqrt(lightDistSq);
                lightDir *= invLightDist;

                // modify 1/d^2 * R^2 to fall off at a fixed radius
                // (R/d)^2 - d/R = [(1/d^2) - (1/R^2)*(d/R)] * R^2
                float distanceFalloff = lightData.radiusSq * (invLightDist * invLightDist);
                distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));

                falsePositiveCount += (distanceFalloff == 0.0);
                lightCount += (distanceFalloff != 0.0);
#else
                colorSum += ApplyPointLight(POINT_LIGHT_ARGS);
#endif
            }
                break;
            case 1:
            // Cone
            {
#if FALSE_POSITIVE_RATE || LIGHT_DENSITY
                float3 lightDir = lightData.pos - worldPos.xyz;
                float lightDistSq = dot(lightDir, lightDir);
                float invLightDist = rsqrt(lightDistSq);
                lightDir *= invLightDist;

                // modify 1/d^2 * R^2 to fall off at a fixed radius
                // (R/d)^2 - d/R = [(1/d^2) - (1/R^2)*(d/R)] * R^2
                float distanceFalloff = lightData.radiusSq * (invLightDist * invLightDist);
                distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));

                float coneFalloff = dot(-lightDir, lightData.coneDir);
                coneFalloff = saturate((coneFalloff - lightData.coneAngles.y) * lightData.coneAngles.x);
        
                falsePositiveCount += (distanceFalloff * coneFalloff == 0.0);
                lightCount += (distanceFalloff * coneFalloff != 0.0);
#else
                colorSum += ApplyConeLight(CONE_LIGHT_ARGS);
#endif
            }
                break;
            case 2:
            // Cone Shadowed
            {
#if FALSE_POSITIVE_RATE || LIGHT_DENSITY
                float3 lightDir = lightData.pos - worldPos.xyz;
                float lightDistSq = dot(lightDir, lightDir);
                float invLightDist = rsqrt(lightDistSq);
                lightDir *= invLightDist;

                // modify 1/d^2 * R^2 to fall off at a fixed radius
                // (R/d)^2 - d/R = [(1/d^2) - (1/R^2)*(d/R)] * R^2
                float distanceFalloff = lightData.radiusSq * (invLightDist * invLightDist);
                distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));

                float coneFalloff = dot(-lightDir, lightData.coneDir);
                coneFalloff = saturate((coneFalloff - lightData.coneAngles.y) * lightData.coneAngles.x);
        
                falsePositiveCount += (distanceFalloff * coneFalloff == 0.0);
                lightCount += (distanceFalloff * coneFalloff != 0.0);
#else
                colorSum += ApplyConeShadowedLight(SHADOWED_LIGHT_ARGS);
#endif
            }
                break;
            default:
                break;
        }
    }
    
#if LIGHT_DENSITY   
    float lightDensity = (float) lightCount / (float) MAX_LIGHTS;
    colorSum = ConvertToRadarColor(lightDensity);
#elif FALSE_POSITIVE_RATE
    if (MAX_LIGHTS)
    {
        float fpr = (float) falsePositiveCount / (float) MAX_LIGHTS;
        colorSum = ConvertToRadarColor(fpr);
    }
    else
    {
        colorSum = 0;
    }
#endif
#endif
}