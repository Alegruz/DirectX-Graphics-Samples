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
    float InvTileDim;
    float RcpZMagic;
    uint2 TileCount;
    float4x4 ProjMatrix;
    float4x4 ViewMatrix;
};

struct VolumeTileAABB
{
    float4 MinPoint;
    float4 MaxPoint;
};

StructuredBuffer<LightData> lightBuffer : register(t0);
Texture2D<float> depthTex : register(t1);
StructuredBuffer<VolumeTileAABB> lightClusterAABB : register(t2);
RWByteAddressBuffer lightCluster : register(u0);
RWByteAddressBuffer lightClusterBitMask : register(u1);

groupshared uint gClusterLightCountSphere;
groupshared uint gClusterLightCountCone;
groupshared uint gClusterLightCountConeShadowed;

groupshared uint gClusterLightIndicesSphere[CLUSTER_MAX_LIGHTS];
groupshared uint gClusterLightIndicesCone[CLUSTER_MAX_LIGHTS];
groupshared uint gClusterLightIndicesConeShadowed[CLUSTER_MAX_LIGHTS];

#define _RootSig \
    "RootFlags(0), " \
    "CBV(b0), " \
    "DescriptorTable(SRV(t0, numDescriptors = 3))," \
    "DescriptorTable(UAV(u0, numDescriptors = 2))"

[RootSignature(_RootSig)]
[numthreads(8, 8, 1)]
void main(
    uint3 Gid : SV_GroupID,
    uint3 GTid : SV_GroupThreadID,
    uint GI : SV_GroupIndex)
{
    // initialize shared data
    if (GI == 0)
    {
        gClusterLightCountSphere = 0;
        gClusterLightCountCone = 0;
        gClusterLightCountConeShadowed = 0;
    }
    GroupMemoryBarrierWithGroupSync();
    
    uint clusterIndex = GetClusterIndex(Gid.xyz, TileCount);
    uint clusterOffset = GetClusterOffset(clusterIndex);

    float4 centerAABB = (lightClusterAABB[clusterIndex].MinPoint + lightClusterAABB[clusterIndex].MaxPoint) / 2.0;
    float4 extentAABB = abs(lightClusterAABB[clusterIndex].MaxPoint - centerAABB);
    
    // find set of lights that overlap this tile
    for (uint lightIndex = GI; lightIndex < MAX_LIGHTS; lightIndex += 64)
    {
        LightData lightData = lightBuffer[lightIndex];

        // Arvo Intersection Test
        //bool bIsOverlapping = false;
        
        //if (lightData.type == 0)
        //{
        //float4 viewSpaceLightPos = mul(ViewMatrix, float4(lightData.pos, 1.0));
        //float3 vDelta = max(0, abs(centerAABB.xyz - viewSpaceLightPos.xyz) - extentAABB.xyz);
            float3 vDelta = max(0, abs(centerAABB.xyz - lightData.pos) - extentAABB.xyz);
            float fDistSq = dot(vDelta, vDelta);
        
            //bIsOverlapping = (fDistSq > lightData.radiusSq);
        if (fDistSq > lightData.radiusSq)
        {
            continue;
        }
        //}
        //else
        //{ 
        //  // Wronski, Bartlomiej, ¡°Cull That Cone! Improved Cone/Spotlight Visibility Tests for Tiled and Clustered Lighting,¡± Bart Wronski blog, Apr. 13, 2017. 
        //    const float3 v = centerAABB.xyz - lightData.pos;
        //    const float vLengthSq = dot(v, v);
        //    const float v1Length = dot(v, lightData.coneDir);
        //    const float distanceClosestPoint0 = cos(lightData.coneAngles[0]) * sqrt(vLengthSq - v1Length * v1Length) - v1Length * sin(lightData.coneAngles[0]);
        //    const float distanceClosestPoint1 = cos(lightData.coneAngles[1]) * sqrt(vLengthSq - v1Length * v1Length) - v1Length * sin(lightData.coneAngles[1]);
        //    const float distanceClosestPoint = min(distanceClosestPoint0, distanceClosestPoint1);
        //    
        //    const bool bIsAngleCull = distanceClosestPoint * distanceClosestPoint > sizeSq;
        //    const bool bIsFrontCull = v1Length > sqrt(sizeSq) + sqrt(lightData.radiusSq);
        //    const bool bIsBackCull = v1Length < -sqrt(sizeSq);
        //    bIsOverlapping = !(bIsAngleCull || bIsFrontCull || bIsBackCull);
        //}
        
        //if (!bIsOverlapping)
        //{
        //    continue;
        //}

        uint slot;

        switch (lightData.type)
        {
        case 0: // sphere
                if (gClusterLightCountSphere >= CLUSTER_MAX_LIGHTS)
                {
                    break;
                }
            InterlockedAdd(gClusterLightCountSphere, 1, slot);
            gClusterLightIndicesSphere[slot] = lightIndex;
            break;

            case 1: // cone
                if (gClusterLightCountCone >= CLUSTER_MAX_LIGHTS)
                {
                    break;
                }
            InterlockedAdd(gClusterLightCountCone, 1, slot);
            gClusterLightIndicesCone[slot] = lightIndex;
            break;

            case 2: // cone w/ shadow map
                if (gClusterLightCountConeShadowed >= CLUSTER_MAX_LIGHTS)
                {
                    break;
                }
            InterlockedAdd(gClusterLightCountConeShadowed, 1, slot);
            gClusterLightIndicesConeShadowed[slot] = lightIndex;
            break;
        }
    }

    GroupMemoryBarrierWithGroupSync();

    if (GI == 0)
    {
        uint lightCount = 
            ((gClusterLightCountSphere & 0xff) << 0) |
            ((gClusterLightCountCone & 0xff) << 8) |
            ((gClusterLightCountConeShadowed & 0xff) << 16);
        lightCluster.Store(clusterOffset + 0, lightCount);
        
        uint storeOffset = clusterOffset + 4;
        uint n;
        for (n = 0; n < gClusterLightCountSphere; n++)
        {
            lightCluster.Store(storeOffset, gClusterLightIndicesSphere[n]);
            storeOffset += 4;
        }
        for (n = 0; n < gClusterLightCountCone; n++)
        {
            lightCluster.Store(storeOffset, gClusterLightIndicesCone[n]);
            storeOffset += 4;
        }
        for (n = 0; n < gClusterLightCountConeShadowed; n++)
        {
            lightCluster.Store(storeOffset, gClusterLightIndicesConeShadowed[n]);
            storeOffset += 4;
        }
    }
}