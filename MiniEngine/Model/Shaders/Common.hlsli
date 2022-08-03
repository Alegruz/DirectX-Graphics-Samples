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
// Author:  James Stanard 
//

#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

// outdated warning about for-loop variable scope
#pragma warning (disable: 3078)
// single-iteration loop
#pragma warning (disable: 3557)

#define Renderer_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_VERTEX), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(SRV(t0, numDescriptors = 10), visibility = SHADER_VISIBILITY_PIXEL)," \
    "DescriptorTable(Sampler(s0, numDescriptors = 10), visibility = SHADER_VISIBILITY_PIXEL)," \
    "DescriptorTable(SRV(t10, numDescriptors = 10), visibility = SHADER_VISIBILITY_PIXEL)," \
    "CBV(b1), " \
    "SRV(t20, visibility = SHADER_VISIBILITY_VERTEX), " \
    "DescriptorTable(SRV(t21, numDescriptors = 4), visibility = SHADER_VISIBILITY_PIXEL)," \
    "StaticSampler(s10, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," \
    "StaticSampler(s11, visibility = SHADER_VISIBILITY_PIXEL," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "comparisonFunc = COMPARISON_GREATER_EQUAL," \
        "filter = FILTER_MIN_MAG_LINEAR_MIP_POINT)," \
    "StaticSampler(s12, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)"

// Common (static) samplers
SamplerState defaultSampler : register(s10);
SamplerComparisonState shadowSampler : register(s11);
SamplerState cubeMapSampler : register(s12);

#ifndef ENABLE_TRIANGLE_ID
    #define ENABLE_TRIANGLE_ID 0
#endif

#if ENABLE_TRIANGLE_ID

uint HashTriangleID(uint vertexID)
{
	// TBD SM6.1 stuff
	uint Index0 = EvaluateAttributeAtVertex(vertexID, 0);
	uint Index1 = EvaluateAttributeAtVertex(vertexID, 1);
	uint Index2 = EvaluateAttributeAtVertex(vertexID, 2);

	// When triangles are clipped (to the near plane?) their interpolants can sometimes
	// be reordered.  To stabilize the ID generation, we need to sort the indices before
	// forming the hash.
	uint I0 = __XB_Min3_U32(Index0, Index1, Index2);
	uint I1 = __XB_Med3_U32(Index0, Index1, Index2);
	uint I2 = __XB_Max3_U32(Index0, Index1, Index2);
	return (I2 & 0xFF) << 16 | (I1 & 0xFF) << 8 | (I0 & 0xFF0000FF);
}

#endif // ENABLE_TRIANGLE_ID

float3 ConvertToRadarColor(float scale)
{
    if (scale >= 1.0f)
    {
        return 0;   // WHITE
    }
    else if (scale >= 0.9375f)
    {
        return float3(195.f, 163.f, 212.f) / 255.f; // VIOLET
    }
    else if (scale >= 0.875f)
    {
        return float3(121.f, 51.f, 160.f) / 255.f; // LIGHT VIOLET
    }
    else if (scale >= 0.8125f)
    {
        return float3(121.f, 0.f, 109.f) / 255.f; // DARK RED
    }
    else if (scale >= 0.75f)
    {
        return float3(188.f, 0.f, 54.f) / 255.f; // MED RED
    }
    else if (scale >= 0.6875f)
    {
        return float3(221.f, 0.f, 27.f) / 255.f; // ADDED
    }
    else if (scale >= 0.625)
    {
        return float3(1.f, 0.f, 0.f); // RED
    }
    else if (scale >= 0.5625f)
    {
        return float3(255.f, 152.f, 0.f) / 255.f; // LIGHT ORANGE
    }
    else if (scale >= 0.5f)
    {
        return float3(255.f, 214.f, 0.f) / 255.f; // YELLOW
    }
    else if (scale >= 0.4375f)
    {
        return float3(8.f, 175.f, 20.f) / 255.f; // DARK GREEN
    }
    else if (scale >= 0.375f)
    {
        return float3(0.f, 1.f, 0.f); // GREEN
    }
    else if (scale >= 0.3125f)
    {
        return float3(9.f, 130.f, 175.f) / 255.f; // LIGHT GREEN
    }
    else if (scale >= 0.25f)
    {
        return float3(0.f, 0.f, 1.f); // BLUE
    }
    else if (scale >= 0.1875f)
    {
        return float3(0.f, 157.f, 255.f) / 255.f; // LIGHT BLUE
    }
    else if (scale >= 0.125f)
    {
        return float3(0.f, 1.f, 1.f); // CYAN
    }
    else
    {
        return 0;
    }
}

#define NO_ENCODING (0)
#define BASELINE (0)
#define Z_RECONSTRUCTION (1)

half4 BaseEncode(half3 n)
{
#if NO_ENCODING
    return half4(n, 0.0);
#elif BASELINE
    return half4(n.xyz * 0.5 + 0.5, 0.0);
#elif Z_RECONSTRUCTION
    return half4(n.xy * 0.5 + 0.5, 0.0, 0.0);
#endif
}

half3 BaseDecode(
#if NO_ENCODING || BASELINE
    half4 enc
#elif Z_RECONSTRUCTION
    half3 enc
#endif
)
{
#if NO_ENCODING
    return enc.xyz;
#elif BASELINE
    return enc.xyz * 2.0 - 1.0;
#elif Z_RECONSTRUCTION
    half3 n;
    n.xy = enc.xy * 2.0 - 1.0;
    n.z = sqrt(1.0 - dot(n.xy, n.xy)) * (2.0 * !!enc.z - 1.0);
    return n;
#endif
}

#endif // __COMMON_HLSLI__
