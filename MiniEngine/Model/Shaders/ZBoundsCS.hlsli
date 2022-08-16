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

// Stewart, Jason, “Compute-Based Tiled Culling,” in Wolfgang Engel, ed., GPU Pro 6, CRC Press, pp. 435–458, 2015.

#include "LightGrid.hlsli"

// outdated warning about for-loop variable scope
#pragma warning (disable: 3078)

#define FLT_MIN         1.175494351e-38F        // min positive value
#define FLT_MAX         3.402823466e+38F        // max value
#define PI				3.1415926535f
#define TWOPI			6.283185307f

Texture2D<float> g_SceneDepthBuffer : register(t1);
RWTexture2D<float2> g_DepthBounds : register(u0);

#define NUM_THREADS_1D_X (WORK_GROUP_SIZE_X / 2)
#define NUM_THREADS_1D_Y (WORK_GROUP_SIZE_Y / 2)
#define NUM_THREADS (NUM_THREADS_1D_X * NUM_THREADS_1D_Y)

groupshared float ldsZMin[NUM_THREADS];
groupshared float ldsZMax[NUM_THREADS];

#define _RootSig \
    "RootFlags(0), " \
    "CBV(b0), " \
    "DescriptorTable(SRV(t0, numDescriptors = 3))," \
    "DescriptorTable(UAV(u0, numDescriptors = 2))"

[RootSignature(_RootSig)]
[numthreads(NUM_THREADS_1D_X, NUM_THREADS_1D_Y, 1)]
void main(
    uint2 Gid : SV_GroupID,
    uint2 GTid : SV_GroupThreadID,
    uint GI : SV_GroupIndex,
    uint3 DTid : SV_DispatchThreadID)
{
    uint2 sampleIdx = DTid.xy * 2;
    
    // Load four depth samples
    float depth00 = g_SceneDepthBuffer.Load(uint3(sampleIdx.x, sampleIdx.y, 0)).x;
    float depth01 = g_SceneDepthBuffer.Load(uint3(sampleIdx.x, sampleIdx.y + 1, 0)).x;
    float depth10 = g_SceneDepthBuffer.Load(uint3(sampleIdx.x + 1, sampleIdx.y, 0)).x;
    float depth11 = g_SceneDepthBuffer.Load(uint3(sampleIdx.x + 1, sampleIdx.y + 1, 0)).x;
    
    uint threadNum = GTid.x + GTid.y * NUM_THREADS_1D_X;

    // Use parallel reduction to calculate the depth bounds
    {
        // Parts of the depth buffer that were never written
        // (e.g., the sky) will be zero (the companion code uses
        // inverted 32-bit float depth for better precision)
        float minZ00 = (depth00 != 0.f) ? depth00 : FLT_MAX;
        float minZ01 = (depth01 != 0.f) ? depth01 : FLT_MAX;
        float minZ10 = (depth10 != 0.f) ? depth10 : FLT_MAX;
        float minZ11 = (depth11 != 0.f) ? depth11 : FLT_MAX;

        float maxZ00 = (depth00 != 0.f) ? depth00 : 0.0f;
        float maxZ01 = (depth01 != 0.f) ? depth01 : 0.0f;
        float maxZ10 = (depth10 != 0.f) ? depth10 : 0.0f;
        float maxZ11 = (depth11 != 0.f) ? depth11 : 0.0f;

        // Initialize shared memory
        ldsZMin[threadNum] = min(minZ00, min(minZ01, min(minZ10, minZ11)));
        ldsZMax[threadNum] = max(maxZ00, max(maxZ01, max(maxZ10, maxZ11)));
        GroupMemoryBarrierWithGroupSync();

        // Minimum and maximum using parallel reduction, with the 
        // loop manually unrolled for 8x8 thread groups (64 threads
        // per thread group)
        uint offset = NUM_THREADS / 2;
        if (threadNum < offset)
        {
            while (offset > 0)
            {
                ldsZMin[threadNum] = min(ldsZMin[threadNum], ldsZMin[threadNum + offset]);
                ldsZMax[threadNum] = max(ldsZMax[threadNum], ldsZMax[threadNum + offset]);
                offset /= 2;
            }
        }
    }
    GroupMemoryBarrierWithGroupSync();

    float minZ = ldsZMin[0];
    float maxZ = ldsZMax[0];

    // The first thread writes to the depth bounds texture
    if (threadNum == 0)
    {
        uint index = (Gid.x + Gid.y * WORK_GROUP_SIZE_X) * 8;
        g_DepthBounds[Gid.xy] = float2(minZ, maxZ);
    }
}
