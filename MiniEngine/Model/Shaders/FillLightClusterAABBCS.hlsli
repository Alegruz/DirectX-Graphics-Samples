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
    //float4x4 InvProjMatrix;
    //float4x4 InvViewMatrix;
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

// Function Prototypes
//float4 TransformScreenSpaceToViewSpace(float4 vec);
//float4 TransformClipSpaceToViewSpace(float4 vec);
//float3 ConvertLineIntersectionToZPlane(float3 lineA, float3 lineB, float zDistance);

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
    /*
    const float3 eyePosition = 0.0;
    
    uint2 tileSizePx = uint2(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y);
    uint tileIndex = Gid.x + Gid.y * TileCount.x + Gid.z * (TileCount.x * TileCount.y);
    
    // Calculating the min / max point in screen space
    float4 screenSpaceMaxPoint = float4(float2(Gid.x + 1, Gid.y + 1) * tileSizePx, 0.0, 1.0); // Top Right
    float4 screenSpaceMinPoint = float4(float2(Gid.x, Gid.y) * tileSizePx, 1.0, 1.0); // Left Bottom
    
    // Pass min / max to view space
    float3 viewSpaceMaxPoint = TransformScreenSpaceToViewSpace(screenSpaceMaxPoint).xyz;
    float3 viewSpaceMinPoint = TransformScreenSpaceToViewSpace(screenSpaceMinPoint).xyz;
    
    // Near and far values of the cluster in view space
    float tileNear = -NearZ * pow(FarZ / NearZ, (float) Gid.z / (float) TileCount.z);
    float tileFar = -NearZ * pow(FarZ / NearZ, (float) (Gid.z + 1) / (float) TileCount.z);
    
    //Finding the 4 intersection points made from the maxPoint to the cluster near/far plane
    float3 nearMinPoint = ConvertLineIntersectionToZPlane(eyePosition, viewSpaceMinPoint, tileNear);
    float3 farMinPoint = ConvertLineIntersectionToZPlane(eyePosition, viewSpaceMinPoint, tileFar);
    float3 nearMaxPoint = ConvertLineIntersectionToZPlane(eyePosition, viewSpaceMaxPoint, tileNear);
    float3 farMaxPoint = ConvertLineIntersectionToZPlane(eyePosition, viewSpaceMaxPoint, tileFar);
    
    float3 aabbMinPoint = min(min(nearMinPoint, farMinPoint), min(nearMaxPoint, farMaxPoint));
    float3 aabbMaxPoint = max(max(nearMinPoint, farMinPoint), max(nearMaxPoint, farMaxPoint));
    
    float4 worldSpaceAabbMinPoint = mul(InvViewMatrix, float4(aabbMinPoint, 1.0f));
    float4 worldSpaceAabbMaxPoint = mul(InvViewMatrix, float4(aabbMaxPoint, 1.0f));
    
    lightClusterAABB[tileIndex].MinPoint = float4(worldSpaceAabbMinPoint.xyz, 1.0f);
    lightClusterAABB[tileIndex].MaxPoint = float4(worldSpaceAabbMaxPoint.xyz, 1.0f);
    */
    
    uint tileIndex = Gid.x + Gid.y * TileCount.x + Gid.z * (TileCount.x * TileCount.y);
    
    // Near and far values of the cluster in view space
    float tileNear = -NearZ * pow(FarZ / NearZ, (float) Gid.z / (float) TileCount.z);
    float tileFar = -NearZ * pow(FarZ / NearZ, (float) (Gid.z + 1) / (float) TileCount.z);
    //float tileMinDepth = (ProjMatrix._33 * tileNear + ProjMatrix._34) / (ProjMatrix._43 * tileNear + ProjMatrix._44);
    //float tileMaxDepth = (ProjMatrix._33 * tileFar + ProjMatrix._34) / (ProjMatrix._43 * tileFar + ProjMatrix._44);

    float4 minDepth = mul(ProjMatrix, float4(0.0, 0.0, tileNear, 1.0));
    minDepth /= minDepth.w;
    float4 maxDepth = mul(ProjMatrix, float4(0.0, 0.0, tileFar, 1.0));
    maxDepth /= maxDepth.w;
    
    //float4 nearMinPoint = float4(((float) Gid.x * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) Gid.y * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, tileMinDepth, 1.0f);
    //float4 farMinPoint = float4(((float) Gid.x * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) Gid.y * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, tileMaxDepth, 1.0f);
    //float4 nearMaxPoint = float4(((float) (Gid.x + 1) * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) (Gid.y + 1) * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, tileMinDepth, 1.0f);
    //float4 farMaxPoint = float4(((float) (Gid.x + 1) * WORK_GROUP_SIZE_X / (float) ViewportWidth) * 2.0f - 1.0f, ((float) (Gid.y + 1) * WORK_GROUP_SIZE_Y / (float) ViewportHeight) * 2.0f - 1.0f, tileMaxDepth, 1.0f);
    
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

//float4 TransformScreenSpaceToViewSpace(float4 vec)
//{
//    // Convert to NDC
//    float2 texCoord = vec.xy / float2(ViewportWidth, ViewportHeight);
//    
//    // Convert to clip space
//    float4 clipSpace = float4(texCoord * 2.0f - 1.0f, vec.z, vec.w);
//    
//    return TransformClipSpaceToViewSpace(clipSpace);
//}
//
//float4 TransformClipSpaceToViewSpace(float4 vec)
//{
//    // View space transformation
//    float4 viewSpace = mul(InvProjMatrix, vec);
//    viewSpace.y *= -1.0;
//    
//    // Perspective projection
//    viewSpace /= viewSpace.w;
//    
//    return viewSpace;
//}
//
////Creates a line from the eye to the screenpoint, then finds its intersection
////With a z oriented plane located at the given distance to the origin
//float3 ConvertLineIntersectionToZPlane(float3 lineA, float3 lineB, float zDistance)
//{
//    // Because this is a Z based normal this is fixed
//    float3 normal = float3(0.0, 0.0, 1.0);
//    
//    float3 ab = lineB - lineA;
//    
//    // Computing the intersection length for the line and the plane
//    float t = (zDistance - dot(normal, lineA)) / dot(normal, ab);
//    
//    // Computing the actual xyz position of the point along the line
//    float3 result = 0.0;
//    if (0 <= t && t <= 1.0f)
//    {
//        result = lineA + t * ab;
//    }
//    
//    return result;
//}