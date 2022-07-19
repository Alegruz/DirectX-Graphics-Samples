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
};

StructuredBuffer<LightData> lightBuffer : register(t0);
Texture2D<float> depthTex : register(t1);
RWByteAddressBuffer lightCluster : register(u0);
RWByteAddressBuffer lightClusterBitMask : register(u1);

groupshared uint minDepthUInt;
groupshared uint maxDepthUInt;

groupshared uint clusterLightCountSphere;
groupshared uint clusterLightCountCone;
groupshared uint clusterLightCountConeShadowed;

groupshared uint clusterLightIndicesSphere[MAX_LIGHTS];
groupshared uint clusterLightIndicesCone[MAX_LIGHTS];
groupshared uint clusterLightIndicesConeShadowed[MAX_LIGHTS];

groupshared uint4 clusterLightBitMask;

#define _RootSig \
    "RootFlags(0), " \
    "CBV(b0), " \
    "DescriptorTable(SRV(t0, numDescriptors = 2))," \
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
        clusterLightCountSphere = 0;
        clusterLightCountCone = 0;
        clusterLightCountConeShadowed = 0;
        clusterLightBitMask = 0;
        minDepthUInt = 0xffffffff;
        maxDepthUInt = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    // Cluster Key
    
    // Read all depth values for this tile and compute the tile min and max values
    for (uint dx = GTid.x; dx < WORK_GROUP_SIZE_X; dx += 8)
    {
        for (uint dy = GTid.y; dy < WORK_GROUP_SIZE_Y; dy += 8)
        {
            uint2 DTid = Gid.xy * uint2(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y) + uint2(dx, dy);
                
            // If pixel coordinates are in bounds...
            if (DTid.x < ViewportWidth && DTid.y < ViewportHeight)
            {
                // Load and compare depth
                uint depthUInt = asuint(depthTex[DTid.xy]);
                
                InterlockedMin(minDepthUInt, depthUInt);
                InterlockedMax(maxDepthUInt, depthUInt);
            }
        }
    }
    
    float clusterMinDepth = (1.0f / WORK_GROUP_SIZE_Z) * (Gid.z);
    float clusterMaxDepth = (1.0f / WORK_GROUP_SIZE_Z) * (Gid.z + 1);
    
    GroupMemoryBarrierWithGroupSync();
    
    //float tileMinDepth = asfloat(minDepthUInt);
    //float tileMaxDepth = asfloat(maxDepthUInt);
    float tileMinDepth = (rcp(asfloat(maxDepthUInt)) - 1.0) * RcpZMagic;
    float tileMaxDepth = (rcp(asfloat(minDepthUInt)) - 1.0) * RcpZMagic;
    
    tileMinDepth = 2.0f * atan(999.0f * tileMinDepth) / PI;
    tileMaxDepth = 2.0f * atan(999.0f * tileMaxDepth) / PI;
    
    clusterMinDepth = min(clusterMinDepth, tileMaxDepth);
    clusterMaxDepth = min(clusterMaxDepth, tileMinDepth);
    
    GroupMemoryBarrierWithGroupSync();
    
    //float tileDepthRange = tileMaxDepth - tileMinDepth;
    float clusterDepthRange = clusterMaxDepth - clusterMinDepth;
    if (clusterDepthRange < 0.0)
    {
        return;
    }
    clusterDepthRange = max(clusterDepthRange, FLT_MIN);
    //tileDepthRange = max(tileDepthRange, FLT_MIN); // don't allow a depth range of 0
    //float invTileDepthRange = rcp(tileDepthRange);
    float invClusterDepthRange = rcp(clusterDepthRange);
    // TODO: near/far clipping planes seem to be falling apart at or near the max depth with infinite projections

    // construct transform from world space to tile space (projection space constrained to tile area)
    float2 invTileSize2X = float2(ViewportWidth, ViewportHeight) * InvTileDim;
    // D3D-specific [0, 1] depth range ortho projection
    // (but without negation of Z, since we already have that from the projection matrix)
    //float3 tileBias = float3(
    //    -2.0 * float(Gid.x) + invTileSize2X.x - 1.0,
    //    -2.0 * float(Gid.y) + invTileSize2X.y - 1.0,
    //    -tileMinDepth * invTileDepthRange);
    float3 clusterBias = float3(
        -2.0 * float(Gid.x) + invTileSize2X.x - 1.0,
        -2.0 * float(Gid.y) + invTileSize2X.y - 1.0,
        -clusterMinDepth * invClusterDepthRange);
    //float4x4 projToTile = float4x4(
    //    invTileSize2X.x, 0, 0, tileBias.x,
    //    0, -invTileSize2X.y, 0, tileBias.y,
    //    0, 0, invTileDepthRange, tileBias.z,
    //    0, 0, 0, 1
    //    );
    float4x4 projToCluster = float4x4(
        invTileSize2X.x, 0, 0, clusterBias.x,
        0, -invTileSize2X.y, 0, clusterBias.y,
        0, 0, invClusterDepthRange, clusterBias.z,
        0, 0, 0, 1
        );
    //float4x4 tileMVP = mul(projToTile, ViewProjMatrix);
    float4x4 clusterMVP = mul(projToCluster, ViewProjMatrix);
    
    // extract frustum planes (these will be in world space)
    //float4 frustumPlanes[6];
    //frustumPlanes[0] = tileMVP[3] + tileMVP[0];
    //frustumPlanes[1] = tileMVP[3] - tileMVP[0];
    //frustumPlanes[2] = tileMVP[3] + tileMVP[1];
    //frustumPlanes[3] = tileMVP[3] - tileMVP[1];
    //frustumPlanes[4] = tileMVP[3] + tileMVP[2];
    //frustumPlanes[5] = tileMVP[3] - tileMVP[2];
    //for (int n = 0; n < 6; n++)
    //{
    //    frustumPlanes[n] *= rsqrt(dot(frustumPlanes[n].xyz, frustumPlanes[n].xyz));
    //}
    
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
    
    //uint tileIndex = GetTileIndex(Gid.xy, TileCount.x);
    uint clusterIndex = GetClusterIndex(Gid.xyz, TileCount);
    //uint tileOffset = GetTileOffset(tileIndex);
    uint clusterOffset = GetClusterOffset(clusterIndex);

    uint4 perThreadLightBitMask = 0;

    // find set of lights that overlap this tile
    for (uint lightIndex = GI; lightIndex < MAX_LIGHTS; lightIndex += 64)
    {
        LightData lightData = lightBuffer[lightIndex];
        float3 lightWorldPos = lightData.pos;
        float lightCullRadius = sqrt(lightData.radiusSq);

        bool overlapping = true;
        for (int p = 0; p < 6; p++)
        {
            //float d = dot(lightWorldPos, frustumPlanes[p].xyz) + frustumPlanes[p].w;
            float d = dot(lightWorldPos, clusterPlanes[p].xyz) + clusterPlanes[p].w;
            if (d < -lightCullRadius)
            {
                overlapping = false;
            }
        }
        
        if (!overlapping)
            continue;

        uint slot;

        switch (lightData.type)
        {
        case 0: // sphere
            InterlockedAdd(clusterLightCountSphere, 1, slot);
            clusterLightIndicesSphere[slot] = lightIndex;
            break;

        case 1: // cone
            InterlockedAdd(clusterLightCountCone, 1, slot);
            clusterLightIndicesCone[slot] = lightIndex;
            break;

        case 2: // cone w/ shadow map
            InterlockedAdd(clusterLightCountConeShadowed, 1, slot);
            clusterLightIndicesConeShadowed[slot] = lightIndex;
            break;
        }

        // update bitmask
        InterlockedOr(clusterLightBitMask[lightIndex / 32], 1 << (lightIndex % 32));
    }

    GroupMemoryBarrierWithGroupSync();

    if (GI == 0)
    {
        uint lightCount = 
            ((clusterLightCountSphere & 0xff) << 0) |
            ((clusterLightCountCone & 0xff) << 8) |
            ((clusterLightCountConeShadowed & 0xff) << 16);
        //lightGrid.Store(tileOffset + 0, lightCount);
        lightCluster.Store(clusterOffset + 0, lightCount);

        //uint storeOffset = tileOffset + 4;
        uint storeOffset = clusterOffset + 4;
        uint n;
        for (n = 0; n < clusterLightCountSphere; n++)
        {
            lightCluster.Store(storeOffset, clusterLightIndicesSphere[n]);
            storeOffset += 4;
        }
        for (n = 0; n < clusterLightCountCone; n++)
        {
            lightCluster.Store(storeOffset, clusterLightIndicesCone[n]);
            storeOffset += 4;
        }
        for (n = 0; n < clusterLightCountConeShadowed; n++)
        {
            lightCluster.Store(storeOffset, clusterLightIndicesConeShadowed[n]);
            storeOffset += 4;
        }

        //lightGridBitMask.Store4(tileIndex * 16, clusterLightBitMask);
        lightClusterBitMask.Store4(clusterIndex * 16, clusterLightBitMask);
    }
}
