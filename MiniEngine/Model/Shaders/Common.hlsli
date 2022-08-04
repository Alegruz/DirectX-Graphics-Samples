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

#define KILLZONE_GBUFFER (0)
#define THIN_GBUFFER (1)

#define NO_ENCODING (0)
#define BASELINE (0)
#define Z_RECONSTRUCTION (0)
#define SPHERICAL_COORDNATES (0)
#define SPHEREMAP_TRANSFORM (0)
#define OCTAHEDRON_NORMAL (1)

#if SPHEREMAP_TRANSFORM
#define CRYENGINE3_SPHEREMAP_TRANSFORM (0)
#define LAMBERT_AZIMUTHAL_EQUAL_AREA_PROJECTION (1)
#endif

#define PI_F (3.14159265)
#define FLT_MIN         1.175494351e-38F        // min positive value

#if OCTAHEDRON_NORMAL
half2 WrapOctahedron(half2 v)
{
    return (1.0 - abs(v.yx)) * (v.xy >= 0.0 ? 1.0 : -1.0);
}
#endif

#if SPHEREMAP_TRANSFORM || OCTAHEDRON_NORMAL
half2
#else
half4 
#endif
BaseEncode(half3 n)
{
#if NO_ENCODING
    return half4(n, 0.0);
#elif BASELINE
    return half4(n.xyz * 0.5 + 0.5, 0.0);
#elif Z_RECONSTRUCTION
    return half4(n.xy * 0.5 + 0.5, 0.0, 0.0);
#elif SPHERICAL_COORDNATES
    return half4((half2(atan2(n.y, n.x) / PI_F, n.z) + 1.0) * 0.5, 0.0, 0.0);
#elif SPHEREMAP_TRANSFORM    
#if CRYENGINE3_SPHEREMAP_TRANSFORM
    half2 enc = normalize(n.xy) * (sqrt(-n.z * 0.5 + 0.5));
    enc = enc * 0.5 + 0.5;
    return enc;
#elif LAMBERT_AZIMUTHAL_EQUAL_AREA_PROJECTION
    half f = sqrt(8.0 * n.z + 8.0);
    return n.xy / f + 0.5;
#endif
#elif OCTAHEDRON_NORMAL
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    n.xy = n.z >= 0.0 ? n.xy : WrapOctahedron(n.xy);
    n.xy = n.xy * 0.5 + 0.5;
    return n.xy;
#endif
}

half3 BaseDecode(
#if NO_ENCODING || BASELINE
    half4 enc
#elif Z_RECONSTRUCTION || SPHEREMAP_TRANSFORM
    half2 enc, float4x4 invViewMatrix
#elif SPHERICAL_COORDNATES || OCTAHEDRON_NORMAL
    half2 enc
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
    n.z = sqrt(1.0 - dot(n.xy, n.xy));
    n.z = n.z * !((asuint(n.z) & 0x7fffffff) > 0x7f800000) + FLT_MIN * ((asuint(n.z) & 0x7fffffff) > 0x7f800000);
    //n.z = sqrt(1.0 - dot(n.xy, n.xy)) * (2.0 * !!enc.z - 1.0);
    n = normalize(mul(invViewMatrix, float4(n, 0.0))).xyz;
    return n;
#elif SPHERICAL_COORDNATES
    half2 ang = enc * 2.0 - 1.0;
    half2 scth;
    sincos(ang.x * PI_F, scth.x, scth.y);
    half2 scphi = half2(sqrt(1.0 - ang.y * ang.y), ang.y);
    return half3(scth.y * scphi.x, scth.x * scphi.x, scphi.y);
#elif SPHEREMAP_TRANSFORM
#if CRYENGINE3_SPHEREMAP_TRANSFORM
    half4 nn = half4(enc, 0.0, 0.0) * half4(2.0, 2.0, 0.0, 0.0) + half4(-1.0, -1.0, 1.0, -1.0);
    half l = dot(nn.xyz, -nn.xyw);
    l = l < 0.0f ? FLT_MIN : l;
    nn.z = l;
    nn.xy *= sqrt(l);
    half3 n = nn.xyz * 2.0 + half3(0.0, 0.0, -1.0);
    n = normalize(mul(invViewMatrix, float4(n, 0.0))).xyz;
    return n;
#elif LAMBERT_AZIMUTHAL_EQUAL_AREA_PROJECTION
    half2 fenc = enc * 4.0 - 2.0;
    half f = dot(fenc, fenc);
    half g = 1.0 - f / 4.0;
    if (g < 0.0)
    {
        return 0;
    }
    g = sqrt(g);
    half3 n;
    n.xy = fenc * g;
    n.z = 1.0 - f / 2.0;
    n = normalize(mul(invViewMatrix, float4(n, 0.0))).xyz;
    return n;
#endif
#elif OCTAHEDRON_NORMAL
    enc = enc * 2.0 - 1.0;
    
    // https://twitter.com/Stubbesaurus/status/937994790553227264
    half3 n = half3(enc.xy, 1.0 - abs(enc.x) - abs(enc.y));
    float t = saturate(-n.z);
    n.xy += n.xy >= 0.0 ? -t : t;
    return normalize(n);
#endif
}

#endif // __COMMON_HLSLI__
