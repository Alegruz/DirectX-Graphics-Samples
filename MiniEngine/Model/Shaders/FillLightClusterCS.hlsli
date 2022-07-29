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
    float4x4 ViewProjMatrix;
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

//groupshared uint gSharedMinDepthUInt;
//groupshared uint gSharedMaxDepthUInt;

groupshared uint gClusterLightCountSphere;
groupshared uint gClusterLightCountCone;
groupshared uint gClusterLightCountConeShadowed;

groupshared uint gClusterLightIndicesSphere[MAX_LIGHTS];
groupshared uint gClusterLightIndicesCone[MAX_LIGHTS];
groupshared uint gClusterLightIndicesConeShadowed[MAX_LIGHTS];

//groupshared uint4 gClusterLightBitMask;

float GetDistSqPointAABB(float3 position, uint tileIndex);
bool TestSphereAABB(uint lightIndex, uint tileIndex);

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
        //gClusterLightBitMask = 0;
        
        //gSharedMinDepthUInt = 0xffffffff;
        //gSharedMaxDepthUInt = 0;
        //gSharedMinDepthUInt = asuint(((1.0f / (float) WORK_GROUP_SIZE_Z) * (Gid.z + 1)) + FLT_MIN);
        //gSharedMaxDepthUInt = asuint(((1.0f / (float) WORK_GROUP_SIZE_Z) * (Gid.z)) - FLT_MIN);
        //gClusterMinDepth = ((FarPlane - NearPlane) / WORK_GROUP_SIZE_Z) * (Gid.z + 1);
        //gClusterMaxDepth = ((FarPlane - NearPlane) / WORK_GROUP_SIZE_Z) * (Gid.z);
    }
    GroupMemoryBarrierWithGroupSync();

    // Cluster Key
    
    // Read all depth values for this tile and compute the tile min and max values
    //for (uint dx = GTid.x; dx < WORK_GROUP_SIZE_X; dx += 8)
    //{
    //    for (uint dy = GTid.y; dy < WORK_GROUP_SIZE_Y; dy += 8)
    //    {
    //        uint2 DTid = Gid.xy * uint2(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y) + uint2(dx, dy);
    //            
    //        // If pixel coordinates are in bounds...
    //        if (DTid.x < ViewportWidth && DTid.y < ViewportHeight)
    //        {
    //            // Load and compare depth
    //            uint depthUInt = asuint(depthTex[DTid.xy]);
    //            
    //            InterlockedMin(gSharedMinDepthUInt, depthUInt);
    //            InterlockedMax(gSharedMaxDepthUInt, depthUInt);
    //        }
    //    }
    //}
    //
    //GroupMemoryBarrierWithGroupSync();
    
    /*
    float clusterMinDepth = (rcp(asfloat(gSharedMaxDepthUInt)) - 1.0) * RcpZMagic;
    float clusterMaxDepth = (rcp(asfloat(gSharedMinDepthUInt)) - 1.0) * RcpZMagic;
    

    float clusterDepthRange = clusterMaxDepth - clusterMinDepth;

    clusterDepthRange = max(clusterDepthRange, FLT_MIN); // don't allow a depth range of 0
    //float invTileDepthRange = rcp(tileDepthRange);
    float invClusterDepthRange = rcp(clusterDepthRange);
    // TODO: near/far clipping planes seem to be falling apart at or near the max depth with infinite projections

    // construct transform from world space to tile space (projection space constrained to tile area)
    float2 invTileSize2X = float2(ViewportWidth, ViewportHeight) * InvTileDim;
    // D3D-specific [0, 1] depth range ortho projection
    // (but without negation of Z, since we already have that from the projection matrix)
    float3 clusterBias = float3(
        -2.0 * float(Gid.x) + invTileSize2X.x - 1.0,
        -2.0 * float(Gid.y) + invTileSize2X.y - 1.0,
        -clusterMinDepth * invClusterDepthRange);
    float4x4 projToCluster = float4x4(
        invTileSize2X.x, 0, 0, clusterBias.x,
        0, -invTileSize2X.y, 0, clusterBias.y,
        0, 0, invClusterDepthRange, clusterBias.z,
        0, 0, 0, 1
        );
    float4x4 clusterMVP = mul(projToCluster, ViewProjMatrix);
    
    float4 clusterPlanes[6];
    clusterPlanes[0] = clusterMVP[3] + clusterMVP[0];
    clusterPlanes[1] = clusterMVP[3] - clusterMVP[0];
    clusterPlanes[2] = clusterMVP[3] + clusterMVP[1];
    clusterPlanes[3] = clusterMVP[3] - clusterMVP[1];
    clusterPlanes[4] = clusterMVP[3] + clusterMVP[2];
    clusterPlanes[5] = clusterMVP[3] - clusterMVP[2];
    for (int n = 0; n < 6; n++)
    {
        clusterPlanes[n] *= rsqrt(dot(clusterPlanes[n].xyz, clusterPlanes[n].xyz));
    }
    */
    
    //uint tileIndex = GetTileIndex(Gid.xy, TileCount.x);
    uint clusterIndex = GetClusterIndex(Gid.xyz, TileCount);
    //uint tileOffset = GetTileOffset(tileIndex);
    uint clusterOffset = GetClusterOffset(clusterIndex);

    // find set of lights that overlap this tile
    for (uint lightIndex = GI; lightIndex < MAX_LIGHTS; lightIndex += 64)
    {
        LightData lightData = lightBuffer[lightIndex];
        //float3 lightWorldPos = lightData.pos;
        //float lightCullRadius = sqrt(lightData.radiusSq);

        bool overlapping = TestSphereAABB(lightIndex, clusterIndex);
        
        if (!overlapping)
            continue;

        uint slot;

        switch (lightData.type)
        {
        case 0: // sphere
            InterlockedAdd(gClusterLightCountSphere, 1, slot);
            gClusterLightIndicesSphere[slot] = lightIndex;
            break;

        case 1: // cone
            InterlockedAdd(gClusterLightCountCone, 1, slot);
            gClusterLightIndicesCone[slot] = lightIndex;
            break;

        case 2: // cone w/ shadow map
            InterlockedAdd(gClusterLightCountConeShadowed, 1, slot);
            gClusterLightIndicesConeShadowed[slot] = lightIndex;
            break;
        }

        // update bitmask
        //InterlockedOr(gClusterLightBitMask[lightIndex / 32], 1 << (lightIndex % 32));
    }

    GroupMemoryBarrierWithGroupSync();

    if (GI == 0)
    {
        uint lightCount = 
            ((gClusterLightCountSphere & 0xff) << 0) |
            ((gClusterLightCountCone & 0xff) << 8) |
            ((gClusterLightCountConeShadowed & 0xff) << 16);
        //lightGrid.Store(tileOffset + 0, lightCount);
        lightCluster.Store(clusterOffset + 0, lightCount);

        //uint storeOffset = tileOffset + 4;
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

        //lightGridBitMask.Store4(tileIndex * 16, gClusterLightBitMask);
        //lightClusterBitMask.Store4(clusterIndex * 16, gClusterLightBitMask);
    }
}


bool TestSphereAABB(uint lightIndex, uint tileIndex)
{
    float radiusSq = lightBuffer[lightIndex].radiusSq;
    float3 center = mul(ViewMatrix, float4(lightBuffer[lightIndex].pos, 1.0)).xyz;
    float distSq = GetDistSqPointAABB(center, tileIndex);
    
    return distSq <= radiusSq;
}

float GetDistSqPointAABB(float3 position, uint tileIndex)
{
    float distSq = 0.0;
    
    VolumeTileAABB currentCell = lightClusterAABB[tileIndex];
    
    currentCell.MaxPoint.z = tileIndex;
    
    for (uint i = 0; i < 3; ++i)
    {
        float v = position[i];
        if (v < currentCell.MinPoint[i])
        {
            distSq += (currentCell.MinPoint[i] - v) * (currentCell.MinPoint[i] - v);
        }
        
        if (v < currentCell.MaxPoint[i])
        {
            distSq += (v - currentCell.MaxPoint[i]) * (v - currentCell.MaxPoint[i]);
        }
    }
    
    return distSq;
}