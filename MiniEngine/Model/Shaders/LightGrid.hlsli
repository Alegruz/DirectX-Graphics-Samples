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

// keep in sync with C code
#define MAX_LIGHTS (192)
#define TILE_SIZE (4 + MAX_LIGHTS * 4)
#define CLUSTER_SIZE (4 + MAX_LIGHTS * 4)

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

uint2 GetTilePos(float2 pos, float2 invTileDim)
{
    return pos * invTileDim;
}
uint3 GetClusterPos(float3 worldPos, float3 scale, float3 bias)
{
    return uint3(worldPos * scale + bias);
}
uint3 GetClusterPos(float3 pos, float3 invTileDim)
{
    return uint3(pos.xy * invTileDim.xy, pos.z / invTileDim.z);
}
uint GetTileIndex(uint2 tilePos, uint tileCountX)
{
    return tilePos.y * tileCountX + tilePos.x;
}
uint GetClusterIndex(uint3 clusterPos, uint2 clusterCount)
{
    return clusterPos.z * (clusterCount.y * clusterCount.x) + clusterPos.y * clusterCount.x + clusterPos.x;
}
uint GetTileOffset(uint tileIndex)
{
    return tileIndex * TILE_SIZE;
}
uint GetClusterOffset(uint clusterIndex)
{
    return clusterIndex * CLUSTER_SIZE;
}
