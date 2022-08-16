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
// Author(s):	Alex Nankervis
//

#include "LightGrid.hlsli"

// outdated warning about for-loop variable scope
#pragma warning (disable: 3078)

#define FLT_MIN         1.175494351e-38F        // min positive value
#define FLT_MAX         3.402823466e+38F        // max value
#define PI				3.1415926535f
#define TWOPI			6.283185307f

cbuffer CSConstants : register(b0)
{
    uint ViewportWidth, ViewportHeight;
    float4x4 InvViewProj;
    float4x4 ProjMatrix;
    uint4 TileCount;
    float FarZ;
    float NearZ;
};

struct VolumeTileAABB
{
    float4 MinPoint;
    float4 MaxPoint;
};

StructuredBuffer<LightData> lightBuffer : register(t0);
Texture2D<float> depthTex : register(t1);
RWStructuredBuffer<VolumeTileAABB> lightClusterAABB : register(u0);

#define _RootSig \
    "RootFlags(0), " \
    "CBV(b0), " \
    "DescriptorTable(SRV(t0, numDescriptors = 2))," \
    "DescriptorTable(UAV(u0, numDescriptors = 2))"

[RootSignature(_RootSig)]
[numthreads(1, 1, 1)]
void main(
    uint3 Gid : SV_GroupID,
    uint3 GTid : SV_GroupThreadID,
    uint GI : SV_GroupIndex,
    uint3 DTid : SV_DispatchThreadID)
{
    uint tileIndex = Gid.x + Gid.y * TileCount.x + Gid.z * (TileCount.x * TileCount.y);
    
    // Near and far values of the cluster in view space
    float tileNear = -NearZ * pow(FarZ / NearZ, (float) Gid.z / (float) TileCount.z);
    float tileFar = -NearZ * pow(FarZ / NearZ, (float) (Gid.z + 1) / (float) TileCount.z);
    
    float4 minDepth = mul(ProjMatrix, float4(0.0, 0.0, tileNear, 1.0));
    minDepth /= minDepth.w;
    float4 maxDepth = mul(ProjMatrix, float4(0.0, 0.0, tileFar, 1.0));
    maxDepth /= maxDepth.w;

    float4 nearMinPoint = float4(((float) Gid.x * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) Gid.y * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, minDepth.z, 1.0f);
    float4 farMinPoint = float4(((float) Gid.x * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) Gid.y * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, maxDepth.z, 1.0f);
    float4 nearMaxPoint = float4(((float) (Gid.x + 1) * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) (Gid.y + 1) * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, minDepth.z, 1.0f);
    float4 farMaxPoint = float4(((float) (Gid.x + 1) * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) (Gid.y + 1) * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, maxDepth.z, 1.0f);
    
    nearMinPoint.y *= -1.0f;
    farMinPoint.y *= -1.0f;
    nearMaxPoint.y *= -1.0f;
    farMaxPoint.y *= -1.0f;
    
    nearMinPoint = mul(InvViewProj, nearMinPoint);
    farMinPoint = mul(InvViewProj, farMinPoint);
    nearMaxPoint = mul(InvViewProj, nearMaxPoint);
    farMaxPoint = mul(InvViewProj, farMaxPoint);
    
    nearMinPoint /= nearMinPoint.w;
    farMinPoint /= farMinPoint.w;
    nearMaxPoint /= nearMaxPoint.w;
    farMaxPoint /= farMaxPoint.w;
    
    float4 aabbMinPoint = min(min(nearMinPoint, farMinPoint), min(nearMaxPoint, farMaxPoint));
    float4 aabbMaxPoint = max(max(nearMinPoint, farMinPoint), max(nearMaxPoint, farMaxPoint));
    lightClusterAABB[tileIndex].MinPoint = float4(aabbMinPoint.xyz, 1.0f);
    lightClusterAABB[tileIndex].MaxPoint = float4(aabbMaxPoint.xyz, 1.0f);
}