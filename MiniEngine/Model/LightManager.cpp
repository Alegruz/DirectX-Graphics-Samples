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
// Author(s):  Alex Nankervis
//             James Stanard
//

#include <cwchar>

#include "LightManager.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "CommandContext.h"
#include "Camera.h"
#include "BufferManager.h"
#include "TemporalEffects.h"
#include "Model.h"

#include "CompiledShaders/FillLightGridCS_8.h"
#include "CompiledShaders/FillLightGridCS_16.h"
#include "CompiledShaders/FillLightGridCS_24.h"
#include "CompiledShaders/FillLightGridCS_32.h"

#include "CompiledShaders/FillLight2_5DGridCS_8.h"
#include "CompiledShaders/FillLight2_5DGridCS_16.h"
#include "CompiledShaders/FillLight2_5DGridCS_24.h"
#include "CompiledShaders/FillLight2_5DGridCS_32.h"

#include "CompiledShaders/FillLight2_5DAABBGridCS_8.h"
#include "CompiledShaders/FillLight2_5DAABBGridCS_16.h"
#include "CompiledShaders/FillLight2_5DAABBGridCS_24.h"
#include "CompiledShaders/FillLight2_5DAABBGridCS_32.h"

#include "CompiledShaders/FillLightClusterCS_8_8.h"
#include "CompiledShaders/FillLightClusterCS_16_16.h"
#include "CompiledShaders/FillLightClusterCS_24_16.h"
#include "CompiledShaders/FillLightClusterCS_32_16.h"
#include "CompiledShaders/FillLightClusterCS_32_32.h"
#include "CompiledShaders/FillLightClusterCS_64_16.h"
#include "CompiledShaders/FillLightClusterCS_64_32.h"

#include "CompiledShaders/FillLightClusterAABBCS_8_8.h"
#include "CompiledShaders/FillLightClusterAABBCS_16_16.h"
#include "CompiledShaders/FillLightClusterAABBCS_24_16.h"
#include "CompiledShaders/FillLightClusterAABBCS_32_16.h"
#include "CompiledShaders/FillLightClusterAABBCS_32_32.h"
#include "CompiledShaders/FillLightClusterAABBCS_64_16.h"
#include "CompiledShaders/FillLightClusterAABBCS_64_32.h"

#if KILLZONE_GBUFFER
#include "CompiledShaders/KillzoneLightGridCS_8.h"
#include "CompiledShaders/KillzoneLightGridCS_16.h"
#include "CompiledShaders/KillzoneLightGridCS_24.h"
#include "CompiledShaders/KillzoneLightGridCS_32.h"

#include "CompiledShaders/KillzoneLightGridDensityCS_8.h"
#include "CompiledShaders/KillzoneLightGridDensityCS_16.h"
#include "CompiledShaders/KillzoneLightGridDensityCS_24.h"
#include "CompiledShaders/KillzoneLightGridDensityCS_32.h"

#include "CompiledShaders/KillzoneLightGridFPRCS_8.h"
#include "CompiledShaders/KillzoneLightGridFPRCS_16.h"
#include "CompiledShaders/KillzoneLightGridFPRCS_24.h"
#include "CompiledShaders/KillzoneLightGridFPRCS_32.h"

#include "CompiledShaders/KillzoneLightCullingGridCS_8.h"
#include "CompiledShaders/KillzoneLightCullingGridCS_16.h"
#include "CompiledShaders/KillzoneLightCullingGridCS_24.h"
#include "CompiledShaders/KillzoneLightCullingGridCS_32.h"

#include "CompiledShaders/KillzoneLightCullingGridDensityCS_8.h"
#include "CompiledShaders/KillzoneLightCullingGridDensityCS_16.h"
#include "CompiledShaders/KillzoneLightCullingGridDensityCS_24.h"
#include "CompiledShaders/KillzoneLightCullingGridDensityCS_32.h"

#include "CompiledShaders/KillzoneLightCullingGridFPRCS_8.h"
#include "CompiledShaders/KillzoneLightCullingGridFPRCS_16.h"
#include "CompiledShaders/KillzoneLightCullingGridFPRCS_24.h"
#include "CompiledShaders/KillzoneLightCullingGridFPRCS_32.h"

#include "CompiledShaders/KillzoneLightAABBCullingGridCS_8.h"
#include "CompiledShaders/KillzoneLightAABBCullingGridCS_16.h"
#include "CompiledShaders/KillzoneLightAABBCullingGridCS_24.h"
#include "CompiledShaders/KillzoneLightAABBCullingGridCS_32.h"

#include "CompiledShaders/KillzoneLightAABBCullingGridDensityCS_8.h"
#include "CompiledShaders/KillzoneLightAABBCullingGridDensityCS_16.h"
#include "CompiledShaders/KillzoneLightAABBCullingGridDensityCS_24.h"
#include "CompiledShaders/KillzoneLightAABBCullingGridDensityCS_32.h"

#include "CompiledShaders/KillzoneLightAABBCullingGridFPRCS_8.h"
#include "CompiledShaders/KillzoneLightAABBCullingGridFPRCS_16.h"
#include "CompiledShaders/KillzoneLightAABBCullingGridFPRCS_24.h"
#include "CompiledShaders/KillzoneLightAABBCullingGridFPRCS_32.h"

#include "CompiledShaders/KillzoneIntelLightGridCS_8.h"
#include "CompiledShaders/KillzoneIntelLightGridCS_16.h"
#include "CompiledShaders/KillzoneIntelLightGridCS_24.h"
#include "CompiledShaders/KillzoneIntelLightGridCS_32.h"

#include "CompiledShaders/KillzoneIntelLightGridDensityCS_8.h"
#include "CompiledShaders/KillzoneIntelLightGridDensityCS_16.h"
#include "CompiledShaders/KillzoneIntelLightGridDensityCS_24.h"
#include "CompiledShaders/KillzoneIntelLightGridDensityCS_32.h"

#include "CompiledShaders/KillzoneIntelLightGridFPRCS_8.h"
#include "CompiledShaders/KillzoneIntelLightGridFPRCS_16.h"
#include "CompiledShaders/KillzoneIntelLightGridFPRCS_24.h"
#include "CompiledShaders/KillzoneIntelLightGridFPRCS_32.h"
#elif THIN_GBUFFER
#include "CompiledShaders/ThinGBufferTiledDiceCS_8.h"
#include "CompiledShaders/ThinGBufferTiledDiceCS_16.h"
#include "CompiledShaders/ThinGBufferTiledDiceCS_24.h"
#include "CompiledShaders/ThinGBufferTiledDiceCS_32.h"

#include "CompiledShaders/ThinGBufferTiledDiceDensityCS_8.h"
#include "CompiledShaders/ThinGBufferTiledDiceDensityCS_16.h"
#include "CompiledShaders/ThinGBufferTiledDiceDensityCS_24.h"
#include "CompiledShaders/ThinGBufferTiledDiceDensityCS_32.h"

#include "CompiledShaders/ThinGBufferTiledDiceFPRCS_8.h"
#include "CompiledShaders/ThinGBufferTiledDiceFPRCS_16.h"
#include "CompiledShaders/ThinGBufferTiledDiceFPRCS_24.h"
#include "CompiledShaders/ThinGBufferTiledDiceFPRCS_32.h"

#include "CompiledShaders/ThinGBufferTiledDice2_5DCullingCS_8.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DCullingCS_16.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DCullingCS_24.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DCullingCS_32.h"

#include "CompiledShaders/ThinGBufferTiledDice2_5DCullingDensityCS_8.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DCullingDensityCS_16.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DCullingDensityCS_24.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DCullingDensityCS_32.h"

#include "CompiledShaders/ThinGBufferTiledDice2_5DCullingFPRCS_8.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DCullingFPRCS_16.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DCullingFPRCS_24.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DCullingFPRCS_32.h"

#include "CompiledShaders/ThinGBufferTiledDice2_5DAABBCullingCS_8.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DAABBCullingCS_16.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DAABBCullingCS_24.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DAABBCullingCS_32.h"

#include "CompiledShaders/ThinGBufferTiledDice2_5DAABBCullingDensityCS_8.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DAABBCullingDensityCS_16.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DAABBCullingDensityCS_24.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DAABBCullingDensityCS_32.h"

#include "CompiledShaders/ThinGBufferTiledDice2_5DAABBCullingFPRCS_8.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DAABBCullingFPRCS_16.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DAABBCullingFPRCS_24.h"
#include "CompiledShaders/ThinGBufferTiledDice2_5DAABBCullingFPRCS_32.h"

#include "CompiledShaders/ThinGBufferTiledIntelCS_8.h"
#include "CompiledShaders/ThinGBufferTiledIntelCS_16.h"
#include "CompiledShaders/ThinGBufferTiledIntelCS_24.h"
#include "CompiledShaders/ThinGBufferTiledIntelCS_32.h"

#include "CompiledShaders/ThinGBufferTiledIntelDensityCS_8.h"
#include "CompiledShaders/ThinGBufferTiledIntelDensityCS_16.h"
#include "CompiledShaders/ThinGBufferTiledIntelDensityCS_24.h"
#include "CompiledShaders/ThinGBufferTiledIntelDensityCS_32.h"

#include "CompiledShaders/ThinGBufferTiledIntelFPRCS_8.h"
#include "CompiledShaders/ThinGBufferTiledIntelFPRCS_16.h"
#include "CompiledShaders/ThinGBufferTiledIntelFPRCS_24.h"
#include "CompiledShaders/ThinGBufferTiledIntelFPRCS_32.h"
#endif

using namespace Math;
using namespace Graphics;

// must keep in sync with HLSL
struct LightData
{
    float pos[3];
    float radiusSq;
    float color[3];

    uint32_t type;
    float coneDir[3];
    float coneAngles[2];

    float shadowTextureMatrix[16];
};

struct VolumeTileAABB
{
    float minPoint[4];
    float maxPoint[4];
};

enum { kMinLightGridDim = 8 };

namespace Lighting
{
    IntVar LightGridDim("Application/Forward+/Light Grid Dim", 16, kMinLightGridDim, 32, 8 );

    uint32_t aLightClusterDimensions[static_cast<size_t>(eClusterType::COUNT)][2] =
    {
        {8, 8},
        {16, 16},
        {24, 16},
        {32, 16},
        {32, 32},
        {64, 16},
        {64, 32}
    };
    eClusterType LightClusterType = eClusterType::_16x16x16;

    RootSignature m_FillLightRootSig;
    RootSignature m_FillLightClusterSig;
#if KILLZONE_GBUFFER
    RootSignature m_KillzoneLightRootSig;
#elif THIN_GBUFFER
    RootSignature m_ThinGBufferLightRootSig;
#endif

    ComputePSO m_aForwardPSOs[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eClusterType::COUNT)] =
    {
        // Forward
        {
            { (L"Fill Light Grid 8 CS") },
            { (L"Fill Light Grid 16 CS") },
            { (L"Fill Light Grid 24 CS") },
            { (L"Fill Light Grid 32 CS") },
        },
        // Forward+
        {
            { (L"Fill Light Grid 8 CS") }, 
            { (L"Fill Light Grid 16 CS") }, 
            { (L"Fill Light Grid 24 CS") }, 
            { (L"Fill Light Grid 32 CS") },
        },
        // Forward+ 2.5D Culling
        {
            { (L"Fill Light 2.5D Culling Grid 8 CS") },
            { (L"Fill Light 2.5D Culling Grid 16 CS") },
            { (L"Fill Light 2.5D Culling Grid 24 CS") },
            { (L"Fill Light 2.5D Culling Grid 32 CS") },
        },
        // Forward+ 2.5D AABB Culling
        {
            { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
            { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
            { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
            { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
        },
        // Forward+ (DICE)
        {
            { (L"Fill Light Grid 8 CS") },
            { (L"Fill Light Grid 16 CS") },
            { (L"Fill Light Grid 24 CS") },
            { (L"Fill Light Grid 32 CS") },
        },
        // Forward+ (DICE 2.5)
        {
            { (L"Fill Light 2.5D Culling Grid 8 CS") },
            { (L"Fill Light 2.5D Culling Grid 16 CS") },
            { (L"Fill Light 2.5D Culling Grid 24 CS") },
            { (L"Fill Light 2.5D Culling Grid 32 CS") },
        },
        // Forward+ (DICE 2.5 AABB)
        {
            { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
            { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
            { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
            { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
        },
        // Forward+ (INTEL)
        {
            { (L"Fill Light Grid 8 CS") },
            { (L"Fill Light Grid 16 CS") },
            { (L"Fill Light Grid 24 CS") },
            { (L"Fill Light Grid 32 CS") },
        },
        // Clustered?
        {
            { (L"Fill Light Cluster 8 x 8 x 8 CS") },
            { (L"Fill Light Cluster 16 x 16 x 16 CS") },
            { (L"Fill Light Cluster 24 x 24 x 16 CS") },
            { (L"Fill Light Cluster 32 x 32 x 16 CS") },
            { (L"Fill Light Cluster 32 x 32 x 32 CS") },
            { (L"Fill Light Cluster 64 x 64 x 16 CS") },
            { (L"Fill Light Cluster 64 x 64 x 32 CS") },
        }
    };

    ComputePSO m_aClusterAABBPSOs[static_cast<size_t>(eClusterType::COUNT)] =
    {
        // Clustered
        { (L"Fill Light Cluster AABB 8 x 8 x 8 CS") },
        { (L"Fill Light Cluster AABB 16 x 16 x 16 CS") },
        { (L"Fill Light Cluster AABB 24 x 24 x 16 CS") },
        { (L"Fill Light Cluster AABB 32 x 32 x 16 CS") },
        { (L"Fill Light Cluster AABB 32 x 32 x 32 CS") },
        { (L"Fill Light Cluster AABB 64 x 64 x 16 CS") },
        { (L"Fill Light Cluster AABB 64 x 64 x 32 CS") },
    };

#if KILLZONE_GBUFFER
    ComputePSO m_aKillzonePSOs[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eGBufferDataType::COUNT) + 1][static_cast<size_t>(eClusterType::COUNT)] =
    {
        // Default
        {
            // Depth
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT0_INTENSITY
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT2_MOTION_VECTORS
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT3_SUN_OCCLUSION
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // FINAL
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
        },
        // Tiled
        {
            // Depth
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT0_INTENSITY
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT2_MOTION_VECTORS
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT3_SUN_OCCLUSION
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // FINAL
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
        },
        // Tiled 2.5D Culling
        {
            // Depth
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // RT0_INTENSITY
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // RT2_MOTION_VECTORS
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // RT3_SUN_OCCLUSION
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // FINAL
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
        },
        // Tiled 2.5D AABB Culling
        {
            // Depth
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // RT0_INTENSITY
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // RT2_MOTION_VECTORS
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // RT3_SUN_OCCLUSION
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // FINAL
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
        },
        // DICE Tiled
        {
            // Depth
            {
                { (L"DICE Depth Light Grid 8 CS") },
                { (L"DICE Depth Light Grid 16 CS") },
                { (L"DICE Depth Light Grid 24 CS") },
                { (L"DICE Depth Light Grid 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"DICE Light Accumulation Light Grid 8 CS") },
                { (L"DICE Light Accumulation Light Grid 16 CS") },
                { (L"DICE Light Accumulation Light Grid 24 CS") },
                { (L"DICE Light Accumulation Light Grid 32 CS") },
            },
            // RT0_INTENSITY
            {
                { (L"DICE Intensity Light Grid 8 CS") },
                { (L"DICE Intensity Light Grid 16 CS") },
                { (L"DICE Intensity Light Grid 24 CS") },
                { (L"DICE Intensity Light Grid 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"DICE Normal Light Grid 8 CS") },
                { (L"DICE Normal Light Grid 16 CS") },
                { (L"DICE Normal Light Grid 24 CS") },
                { (L"DICE Normal Light Grid 32 CS") },
            },
            // RT2_MOTION_VECTORS
            {
                { (L"DICE Motion Vectors Light Grid 8 CS") },
                { (L"DICE Motion Vectors Light Grid 16 CS") },
                { (L"DICE Motion Vectors Light Grid 24 CS") },
                { (L"DICE Motion Vectors Light Grid 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"DICE Specular Intensity Light Grid 8 CS") },
                { (L"DICE Specular Intensity Light Grid 16 CS") },
                { (L"DICE Specular Intensity Light Grid 24 CS") },
                { (L"DICE Specular Intensity Light Grid 32 CS") },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { (L"DICE Diffuse Albedo Light Grid 8 CS") },
                { (L"DICE Diffuse Albedo Light Grid 16 CS") },
                { (L"DICE Diffuse Albedo Light Grid 24 CS") },
                { (L"DICE Diffuse Albedo Light Grid 32 CS") },
            },
            // RT3_SUN_OCCLUSION
            {
                { (L"DICE Sun Occlusion Light Grid 8 CS") },
                { (L"DICE Sun Occlusion Light Grid 16 CS") },
                { (L"DICE Sun Occlusion Light Grid 24 CS") },
                { (L"DICE Sun Occlusion Light Grid 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"DICE Light Density Light Grid 8 CS") },
                { (L"DICE Light Density Light Grid 16 CS") },
                { (L"DICE Light Density Light Grid 24 CS") },
                { (L"DICE Light Density Light Grid 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"DICE False Positive Rate Light Grid 8 CS") },
                { (L"DICE False Positive Rate Light Grid 16 CS") },
                { (L"DICE False Positive Rate Light Grid 24 CS") },
                { (L"DICE False Positive Rate Light Grid 32 CS") },
            },
            // FINAL
            {
                { (L"DICE Light Grid 8 CS") },
                { (L"DICE Light Grid 16 CS") },
                { (L"DICE Light Grid 24 CS") },
                { (L"DICE Light Grid 32 CS") },
            },
        },
        // DICE Tiled 2.5D Culling
        {
            // Depth
            {
                { (L"DICE Depth 2.5D Culling Light Grid 8 CS") },
                { (L"DICE Depth 2.5D Culling Light Grid 16 CS") },
                { (L"DICE Depth 2.5D Culling Light Grid 24 CS") },
                { (L"DICE Depth 2.5D Culling Light Grid 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"DICE Light 2.5D Culling Accumulation Light Grid 8 CS") },
                { (L"DICE Light 2.5D Culling Accumulation Light Grid 16 CS") },
                { (L"DICE Light 2.5D Culling Accumulation Light Grid 24 CS") },
                { (L"DICE Light 2.5D Culling Accumulation Light Grid 32 CS") },
            },
            // RT0_INTENSITY
            {
                { (L"DICE Intensity 2.5D Culling Light Grid 8 CS") },
                { (L"DICE Intensity 2.5D Culling Light Grid 16 CS") },
                { (L"DICE Intensity 2.5D Culling Light Grid 24 CS") },
                { (L"DICE Intensity 2.5D Culling Light Grid 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"DICE Normal 2.5D Culling Light Grid 8 CS") },
                { (L"DICE Normal 2.5D Culling Light Grid 16 CS") },
                { (L"DICE Normal 2.5D Culling Light Grid 24 CS") },
                { (L"DICE Normal 2.5D Culling Light Grid 32 CS") },
            },
            // RT2_MOTION_VECTORS
            {
                { (L"DICE Motion Vectors 2.5D Culling Light Grid 8 CS") },
                { (L"DICE Motion Vectors 2.5D Culling Light Grid 16 CS") },
                { (L"DICE Motion Vectors 2.5D Culling Light Grid 24 CS") },
                { (L"DICE Motion Vectors 2.5D Culling Light Grid 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"DICE Specular Intensity 2.5D Culling Light Grid 8 CS") },
                { (L"DICE Specular Intensity 2.5D Culling Light Grid 16 CS") },
                { (L"DICE Specular Intensity 2.5D Culling Light Grid 24 CS") },
                { (L"DICE Specular Intensity 2.5D Culling Light Grid 32 CS") },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { (L"DICE Diffuse Albedo 2.5D Culling Light Grid 8 CS") },
                { (L"DICE Diffuse Albedo 2.5D Culling Light Grid 16 CS") },
                { (L"DICE Diffuse Albedo 2.5D Culling Light Grid 24 CS") },
                { (L"DICE Diffuse Albedo 2.5D Culling Light Grid 32 CS") },
            },
            // RT3_SUN_OCCLUSION
            {
                { (L"DICE Sun Occlusion 2.5D Culling Light Grid 8 CS") },
                { (L"DICE Sun Occlusion 2.5D Culling Light Grid 16 CS") },
                { (L"DICE Sun Occlusion 2.5D Culling Light Grid 24 CS") },
                { (L"DICE Sun Occlusion 2.5D Culling Light Grid 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"DICE Light Density 2.5D Culling Light Grid 8 CS") },
                { (L"DICE Light Density 2.5D Culling Light Grid 16 CS") },
                { (L"DICE Light Density 2.5D Culling Light Grid 24 CS") },
                { (L"DICE Light Density 2.5D Culling Light Grid 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"DICE False Positive Rate 2.5D Culling Light Grid 8 CS") },
                { (L"DICE False Positive Rate 2.5D Culling Light Grid 16 CS") },
                { (L"DICE False Positive Rate 2.5D Culling Light Grid 24 CS") },
                { (L"DICE False Positive Rate 2.5D Culling Light Grid 32 CS") },
            },
            // FINAL
            {
                { (L"DICE 2.5D Culling Light Grid 8 CS") },
                { (L"DICE 2.5D Culling Light Grid 16 CS") },
                { (L"DICE 2.5D Culling Light Grid 24 CS") },
                { (L"DICE 2.5D Culling Light Grid 32 CS") },
            },
        },
        // DICE Tiled 2.5D AABB Culling
        {
            // Depth
            {
                { (L"DICE Depth 2.5D AABB Culling Light Grid 8 CS") },
                { (L"DICE Depth 2.5D AABB Culling Light Grid 16 CS") },
                { (L"DICE Depth 2.5D AABB Culling Light Grid 24 CS") },
                { (L"DICE Depth 2.5D AABB Culling Light Grid 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"DICE Light 2.5D AABB Culling Accumulation Light Grid 8 CS") },
                { (L"DICE Light 2.5D AABB Culling Accumulation Light Grid 16 CS") },
                { (L"DICE Light 2.5D AABB Culling Accumulation Light Grid 24 CS") },
                { (L"DICE Light 2.5D AABB Culling Accumulation Light Grid 32 CS") },
            },
            // RT0_INTENSITY
            {
                { (L"DICE Intensity 2.5D AABB Culling Light Grid 8 CS") },
                { (L"DICE Intensity 2.5D AABB Culling Light Grid 16 CS") },
                { (L"DICE Intensity 2.5D AABB Culling Light Grid 24 CS") },
                { (L"DICE Intensity 2.5D AABB Culling Light Grid 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"DICE Normal 2.5D AABB Culling Light Grid 8 CS") },
                { (L"DICE Normal 2.5D AABB Culling Light Grid 16 CS") },
                { (L"DICE Normal 2.5D AABB Culling Light Grid 24 CS") },
                { (L"DICE Normal 2.5D AABB Culling Light Grid 32 CS") },
            },
            // RT2_MOTION_VECTORS
            {
                { (L"DICE Motion Vectors 2.5D AABB Culling Light Grid 8 CS") },
                { (L"DICE Motion Vectors 2.5D AABB Culling Light Grid 16 CS") },
                { (L"DICE Motion Vectors 2.5D AABB Culling Light Grid 24 CS") },
                { (L"DICE Motion Vectors 2.5D AABB Culling Light Grid 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"DICE Specular Intensity 2.5D AABB Culling Light Grid 8 CS") },
                { (L"DICE Specular Intensity 2.5D AABB Culling Light Grid 16 CS") },
                { (L"DICE Specular Intensity 2.5D AABB Culling Light Grid 24 CS") },
                { (L"DICE Specular Intensity 2.5D AABB Culling Light Grid 32 CS") },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { (L"DICE Diffuse Albedo 2.5D AABB Culling Light Grid 8 CS") },
                { (L"DICE Diffuse Albedo 2.5D AABB Culling Light Grid 16 CS") },
                { (L"DICE Diffuse Albedo 2.5D AABB Culling Light Grid 24 CS") },
                { (L"DICE Diffuse Albedo 2.5D AABB Culling Light Grid 32 CS") },
            },
            // RT3_SUN_OCCLUSION
            {
                { (L"DICE Sun Occlusion 2.5D AABB Culling Light Grid 8 CS") },
                { (L"DICE Sun Occlusion 2.5D AABB Culling Light Grid 16 CS") },
                { (L"DICE Sun Occlusion 2.5D AABB Culling Light Grid 24 CS") },
                { (L"DICE Sun Occlusion 2.5D AABB Culling Light Grid 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"DICE Light Density 2.5D AABB Culling Light Grid 8 CS") },
                { (L"DICE Light Density 2.5D AABB Culling Light Grid 16 CS") },
                { (L"DICE Light Density 2.5D AABB Culling Light Grid 24 CS") },
                { (L"DICE Light Density 2.5D AABB Culling Light Grid 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"DICE False Positive Rate 2.5D AABB Culling Light Grid 8 CS") },
                { (L"DICE False Positive Rate 2.5D AABB Culling Light Grid 16 CS") },
                { (L"DICE False Positive Rate 2.5D AABB Culling Light Grid 24 CS") },
                { (L"DICE False Positive Rate 2.5D AABB Culling Light Grid 32 CS") },
            },
            // FINAL
            {
                { (L"DICE 2.5D AABB Culling Light Grid 8 CS") },
                { (L"DICE 2.5D AABB Culling Light Grid 16 CS") },
                { (L"DICE 2.5D AABB Culling Light Grid 24 CS") },
                { (L"DICE 2.5D AABB Culling Light Grid 32 CS") },
            },
        },
        // INTEL Tiled
        {
            // Depth
            {
                { (L"Intel Depth Light Grid 8 CS") },
                { (L"Intel Depth Light Grid 16 CS") },
                { (L"Intel Depth Light Grid 24 CS") },
                { (L"Intel Depth Light Grid 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"Intel Light Accumulation Light Grid 8 CS") },
                { (L"Intel Light Accumulation Light Grid 16 CS") },
                { (L"Intel Light Accumulation Light Grid 24 CS") },
                { (L"Intel Light Accumulation Light Grid 32 CS") },
            },
            // RT0_INTENSITY
            {
                { (L"Intel Intensity Light Grid 8 CS") },
                { (L"Intel Intensity Light Grid 16 CS") },
                { (L"Intel Intensity Light Grid 24 CS") },
                { (L"Intel Intensity Light Grid 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"Intel Normal Light Grid 8 CS") },
                { (L"Intel Normal Light Grid 16 CS") },
                { (L"Intel Normal Light Grid 24 CS") },
                { (L"Intel Normal Light Grid 32 CS") },
            },
            // RT2_MOTION_VECTORS
            {
                { (L"Intel Motion Vectors Light Grid 8 CS") },
                { (L"Intel Motion Vectors Light Grid 16 CS") },
                { (L"Intel Motion Vectors Light Grid 24 CS") },
                { (L"Intel Motion Vectors Light Grid 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"Intel Specular Intensity Light Grid 8 CS") },
                { (L"Intel Specular Intensity Light Grid 16 CS") },
                { (L"Intel Specular Intensity Light Grid 24 CS") },
                { (L"Intel Specular Intensity Light Grid 32 CS") },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { (L"Intel Diffuse Albedo Light Grid 8 CS") },
                { (L"Intel Diffuse Albedo Light Grid 16 CS") },
                { (L"Intel Diffuse Albedo Light Grid 24 CS") },
                { (L"Intel Diffuse Albedo Light Grid 32 CS") },
            },
            // RT3_SUN_OCCLUSION
            {
                { (L"Intel Sun Occlusion Light Grid 8 CS") },
                { (L"Intel Sun Occlusion Light Grid 16 CS") },
                { (L"Intel Sun Occlusion Light Grid 24 CS") },
                { (L"Intel Sun Occlusion Light Grid 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"Intel Light Density Light Grid 8 CS") },
                { (L"Intel Light Density Light Grid 16 CS") },
                { (L"Intel Light Density Light Grid 24 CS") },
                { (L"Intel Light Density Light Grid 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"Intel False Positive Rate Light Grid 8 CS") },
                { (L"Intel False Positive Rate Light Grid 16 CS") },
                { (L"Intel False Positive Rate Light Grid 24 CS") },
                { (L"Intel False Positive Rate Light Grid 32 CS") },
            },
            // FINAL
            {
                { (L"Intel Light Grid 8 CS") },
                { (L"Intel Light Grid 16 CS") },
                { (L"Intel Light Grid 24 CS") },
                { (L"Intel Light Grid 32 CS") },
            },
        },
        // Clustered?
        {
            // Depth
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // RT0_INTENSITY
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // RT2_MOTION_VECTORS
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // RT3_SUN_OCCLUSION
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // FINAL
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
        }
    };
#elif THIN_GBUFFER
    ComputePSO m_aThinGBufferPSOs[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eGBufferDataType::COUNT) + 1][static_cast<size_t>(eClusterType::COUNT)] =
    {
        // Default
        {
            // Depth
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT1_GLOSSINESS
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT3_SPEC_INTENSITY
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // FINAL
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
        },
        // Tiled
        {
            // Depth
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT1_GLOSSINESS
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
            // FINAL
            {
                { (L"Fill Light Grid 8 CS") },
                { (L"Fill Light Grid 16 CS") },
                { (L"Fill Light Grid 24 CS") },
                { (L"Fill Light Grid 32 CS") },
            },
        },
        // Tiled 2.5D Culling
        {
            // Depth
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // RT1_GLOSSINESS
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
            // FINAL
            {
                { (L"Fill Light 2.5D Culling Grid 8 CS") },
                { (L"Fill Light 2.5D Culling Grid 16 CS") },
                { (L"Fill Light 2.5D Culling Grid 24 CS") },
                { (L"Fill Light 2.5D Culling Grid 32 CS") },
            },
        },
        // Tiled 2.5D AABB Culling
        {
            // Depth
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // RT1_GLOSSINESS
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
            // FINAL
            {
                { (L"Fill Light 2.5D AABB Culling Grid 8 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 16 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 24 CS") },
                { (L"Fill Light 2.5D AABB Culling Grid 32 CS") },
            },
        },
        // DICE Tiled
        {
            // Depth
            {
                { (L"DICE Depth Tiled 8 CS") },
                { (L"DICE Depth Tiled 16 CS") },
                { (L"DICE Depth Tiled 24 CS") },
                { (L"DICE Depth Tiled 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"DICE Light Accumulation Tiled 8 CS") },
                { (L"DICE Light Accumulation Tiled 16 CS") },
                { (L"DICE Light Accumulation Tiled 24 CS") },
                { (L"DICE Light Accumulation Tiled 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"DICE Normal Tiled 8 CS") },
                { (L"DICE Normal Tiled 16 CS") },
                { (L"DICE Normal Tiled 24 CS") },
                { (L"DICE Normal Tiled 32 CS") },
            },
            // RT1_GLOSSINESS
            {
                { (L"DICE Glossiness Tiled 8 CS") },
                { (L"DICE Glossiness Tiled 16 CS") },
                { (L"DICE Glossiness Tiled 24 CS") },
                { (L"DICE Glossiness Tiled 32 CS") },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { (L"DICE Diffuse Albedo Tiled 8 CS") },
                { (L"DICE Diffuse Albedo Tiled 16 CS") },
                { (L"DICE Diffuse Albedo Tiled 24 CS") },
                { (L"DICE Diffuse Albedo Tiled 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"DICE Specular Intensity Tiled 8 CS") },
                { (L"DICE Specular Intensity Tiled 16 CS") },
                { (L"DICE Specular Intensity Tiled 24 CS") },
                { (L"DICE Specular Intensity Tiled 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"DICE Light Density Tiled 8 CS") },
                { (L"DICE Light Density Tiled 16 CS") },
                { (L"DICE Light Density Tiled 24 CS") },
                { (L"DICE Light Density Tiled 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"DICE False Positive Rate Tiled 8 CS") },
                { (L"DICE False Positive Rate Tiled 16 CS") },
                { (L"DICE False Positive Rate Tiled 24 CS") },
                { (L"DICE False Positive Rate Tiled 32 CS") },
            },
            // FINAL
            {
                { (L"DICE Tiled 8 CS") },
                { (L"DICE Tiled 16 CS") },
                { (L"DICE Tiled 24 CS") },
                { (L"DICE Tiled 32 CS") },
            },
        },
        // DICE Tiled 2.5D Culling
        {
            // Depth
            {
                { (L"DICE Depth Tiled 2.5D Culling 8 CS") },
                { (L"DICE Depth Tiled 2.5D Culling 16 CS") },
                { (L"DICE Depth Tiled 2.5D Culling 24 CS") },
                { (L"DICE Depth Tiled 2.5D Culling 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"DICE Light Accumulation Tiled 2.5D Culling 8 CS") },
                { (L"DICE Light Accumulation Tiled 2.5D Culling 16 CS") },
                { (L"DICE Light Accumulation Tiled 2.5D Culling 24 CS") },
                { (L"DICE Light Accumulation Tiled 2.5D Culling 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"DICE Normal Tiled 2.5D Culling 8 CS") },
                { (L"DICE Normal Tiled 2.5D Culling 16 CS") },
                { (L"DICE Normal Tiled 2.5D Culling 24 CS") },
                { (L"DICE Normal Tiled 2.5D Culling 32 CS") },
            },
            // RT1_GLOSSINESS
            {
                { (L"DICE Glossiness Tiled 2.5D Culling 8 CS") },
                { (L"DICE Glossiness Tiled 2.5D Culling 16 CS") },
                { (L"DICE Glossiness Tiled 2.5D Culling 24 CS") },
                { (L"DICE Glossiness Tiled 2.5D Culling 32 CS") },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { (L"DICE Diffuse Albedo Tiled 2.5D Culling 8 CS") },
                { (L"DICE Diffuse Albedo Tiled 2.5D Culling 16 CS") },
                { (L"DICE Diffuse Albedo Tiled 2.5D Culling 24 CS") },
                { (L"DICE Diffuse Albedo Tiled 2.5D Culling 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"DICE Specular Intensity Tiled 2.5D Culling 8 CS") },
                { (L"DICE Specular Intensity Tiled 2.5D Culling 16 CS") },
                { (L"DICE Specular Intensity Tiled 2.5D Culling 24 CS") },
                { (L"DICE Specular Intensity Tiled 2.5D Culling 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"DICE Light Density Tiled 2.5D Culling 8 CS") },
                { (L"DICE Light Density Tiled 2.5D Culling 16 CS") },
                { (L"DICE Light Density Tiled 2.5D Culling 24 CS") },
                { (L"DICE Light Density Tiled 2.5D Culling 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"DICE False Positive Rate Tiled 2.5D Culling 8 CS") },
                { (L"DICE False Positive Rate Tiled 2.5D Culling 16 CS") },
                { (L"DICE False Positive Rate Tiled 2.5D Culling 24 CS") },
                { (L"DICE False Positive Rate Tiled 2.5D Culling 32 CS") },
            },
            // FINAL
            {
                { (L"DICE Tiled 2.5D Culling 8 CS") },
                { (L"DICE Tiled 2.5D Culling 16 CS") },
                { (L"DICE Tiled 2.5D Culling 24 CS") },
                { (L"DICE Tiled 2.5D Culling 32 CS") },
            },
        },
        // DICE Tiled 2.5D AABB Culling
        {
            // Depth
            {
                { (L"DICE Depth Tiled 2.5D AABB Culling 8 CS") },
                { (L"DICE Depth Tiled 2.5D AABB Culling 16 CS") },
                { (L"DICE Depth Tiled 2.5D AABB Culling 24 CS") },
                { (L"DICE Depth Tiled 2.5D AABB Culling 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"DICE Light Accumulation Tiled 2.5D AABB Culling 8 CS") },
                { (L"DICE Light Accumulation Tiled 2.5D AABB Culling 16 CS") },
                { (L"DICE Light Accumulation Tiled 2.5D AABB Culling 24 CS") },
                { (L"DICE Light Accumulation Tiled 2.5D AABB Culling 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"DICE Normal Tiled 2.5D AABB Culling 8 CS") },
                { (L"DICE Normal Tiled 2.5D AABB Culling 16 CS") },
                { (L"DICE Normal Tiled 2.5D AABB Culling 24 CS") },
                { (L"DICE Normal Tiled 2.5D AABB Culling 32 CS") },
            },
            // RT1_GLOSSINESS
            {
                { (L"DICE Glossiness Tiled 2.5D AABB Culling 8 CS") },
                { (L"DICE Glossiness Tiled 2.5D AABB Culling 16 CS") },
                { (L"DICE Glossiness Tiled 2.5D AABB Culling 24 CS") },
                { (L"DICE Glossiness Tiled 2.5D AABB Culling 32 CS") },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { (L"DICE Diffuse Albedo Tiled 2.5D AABB Culling 8 CS") },
                { (L"DICE Diffuse Albedo Tiled 2.5D AABB Culling 16 CS") },
                { (L"DICE Diffuse Albedo Tiled 2.5D AABB Culling 24 CS") },
                { (L"DICE Diffuse Albedo Tiled 2.5D AABB Culling 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"DICE Specular Intensity Tiled 2.5D AABB Culling 8 CS") },
                { (L"DICE Specular Intensity Tiled 2.5D AABB Culling 16 CS") },
                { (L"DICE Specular Intensity Tiled 2.5D AABB Culling 24 CS") },
                { (L"DICE Specular Intensity Tiled 2.5D AABB Culling 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"DICE Light Density Tiled 2.5D AABB Culling 8 CS") },
                { (L"DICE Light Density Tiled 2.5D AABB Culling 16 CS") },
                { (L"DICE Light Density Tiled 2.5D AABB Culling 24 CS") },
                { (L"DICE Light Density Tiled 2.5D AABB Culling 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"DICE False Positive Rate Tiled 2.5D AABB Culling 8 CS") },
                { (L"DICE False Positive Rate Tiled 2.5D AABB Culling 16 CS") },
                { (L"DICE False Positive Rate Tiled 2.5D AABB Culling 24 CS") },
                { (L"DICE False Positive Rate Tiled 2.5D AABB Culling 32 CS") },
            },
            // FINAL
            {
                { (L"DICE Tiled 2.5D AABB Culling 8 CS") },
                { (L"DICE Tiled 2.5D AABB Culling 16 CS") },
                { (L"DICE Tiled 2.5D AABB Culling 24 CS") },
                { (L"DICE Tiled 2.5D AABB Culling 32 CS") },
            },
        },
        // INTEL Tiled
        {
            // Depth
            {
                { (L"Intel Depth Tiled 8 CS") },
                { (L"Intel Depth Tiled 16 CS") },
                { (L"Intel Depth Tiled 24 CS") },
                { (L"Intel Depth Tiled 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"Intel Light Accumulation Tiled 8 CS") },
                { (L"Intel Light Accumulation Tiled 16 CS") },
                { (L"Intel Light Accumulation Tiled 24 CS") },
                { (L"Intel Light Accumulation Tiled 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"Intel Normal Tiled 8 CS") },
                { (L"Intel Normal Tiled 16 CS") },
                { (L"Intel Normal Tiled 24 CS") },
                { (L"Intel Normal Tiled 32 CS") },
            },
            // RT1_GLOSSINESS
            {
                { (L"Intel Glossiness Tiled 8 CS") },
                { (L"Intel Glossiness Tiled 16 CS") },
                { (L"Intel Glossiness Tiled 24 CS") },
                { (L"Intel Glossiness Tiled 32 CS") },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { (L"Intel Diffuse Albedo Tiled 8 CS") },
                { (L"Intel Diffuse Albedo Tiled 16 CS") },
                { (L"Intel Diffuse Albedo Tiled 24 CS") },
                { (L"Intel Diffuse Albedo Tiled 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"Intel Specular Intensity Tiled 8 CS") },
                { (L"Intel Specular Intensity Tiled 16 CS") },
                { (L"Intel Specular Intensity Tiled 24 CS") },
                { (L"Intel Specular Intensity Tiled 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"Intel Light Density Tiled 8 CS") },
                { (L"Intel Light Density Tiled 16 CS") },
                { (L"Intel Light Density Tiled 24 CS") },
                { (L"Intel Light Density Tiled 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"Intel False Positive Rate Tiled 8 CS") },
                { (L"Intel False Positive Rate Tiled 16 CS") },
                { (L"Intel False Positive Rate Tiled 24 CS") },
                { (L"Intel False Positive Rate Tiled 32 CS") },
            },
            // FINAL
            {
                { (L"Intel Tiled 8 CS") },
                { (L"Intel Tiled 16 CS") },
                { (L"Intel Tiled 24 CS") },
                { (L"Intel Tiled 32 CS") },
            },
        },
        // Clustered?
        {
            // Depth
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // RT1_NORMAL
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // RT1_GLOSSINESS
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // RT2_SPEC_INTENSITY
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // LIGHT_DENSITY
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // FALSE_POSITIVE_RATE
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
            // FINAL
            {
                { (L"Fill Light Cluster 8 x 8 x 8 CS") },
                { (L"Fill Light Cluster 16 x 16 x 16 CS") },
                { (L"Fill Light Cluster 24 x 24 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 16 CS") },
                { (L"Fill Light Cluster 32 x 32 x 32 CS") },
                { (L"Fill Light Cluster 64 x 64 x 16 CS") },
                { (L"Fill Light Cluster 64 x 64 x 32 CS") },
            },
        }
    };
#endif

    std::pair<const unsigned char* const, size_t> m_aForwardComputeShaders[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eClusterType::COUNT)] =
    {
        // Forward
        {
            { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
            { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
            { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
            { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
        },
        // Forward+
        {
            { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
            { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
            { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
            { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
        },
        // Forward+ 2.5D Culling
        {
            { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
            { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
            { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
            { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
        },
        // Forward+ 2.5D AABB Culling
        {
            { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
            { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
            { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
            { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
        },
        // Forward+ (DICE)
        {
            { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
            { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
            { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
            { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
        },
        // Forward+ (DICE 2.5)
        {
            { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
            { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
            { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
            { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
        },
        // Forward+ (DICE 2.5 AABB)
        {
            { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
            { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
            { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
            { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
        },
        // Forward+ (INTEL)
        {
            { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
            { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
            { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
            { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
        },
        // Clustered?
        {
            { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
            { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
            { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
            { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
            { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
            { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
            { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
        }
    };

    std::pair<const unsigned char* const, size_t> m_aClusterAABBComputeShaders[static_cast<size_t>(eClusterType::COUNT)] =
    {
        // Clustered?
        { g_pFillLightClusterAABBCS_8_8, sizeof(g_pFillLightClusterAABBCS_8_8) },
        { g_pFillLightClusterAABBCS_16_16, sizeof(g_pFillLightClusterAABBCS_16_16) },
        { g_pFillLightClusterAABBCS_24_16, sizeof(g_pFillLightClusterAABBCS_24_16) },
        { g_pFillLightClusterAABBCS_32_16, sizeof(g_pFillLightClusterAABBCS_32_16) },
        { g_pFillLightClusterAABBCS_32_32, sizeof(g_pFillLightClusterAABBCS_32_32) },
        { g_pFillLightClusterAABBCS_64_16, sizeof(g_pFillLightClusterAABBCS_64_16) },
        { g_pFillLightClusterAABBCS_64_32, sizeof(g_pFillLightClusterAABBCS_64_32) },
    };

#if KILLZONE_GBUFFER
    std::pair<const unsigned char* const, size_t> m_aKillzoneComputeShaders[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eGBufferDataType::COUNT) + 1][static_cast<size_t>(eClusterType::COUNT)] =
    {
        // Default
        {
            // Depth
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT0_INTENSITY
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT2_MOTION_VECTORS
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT3_SUN_OCCLUSION
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // FINAL
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
        },
        // Tiled
        {
            // Depth
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT0_INTENSITY
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT2_MOTION_VECTORS
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT3_SUN_OCCLUSION
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // FINAL
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
        },
        // Tiled 2.5D Culling
        {
            // Depth
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // RT0_INTENSITY
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // RT2_MOTION_VECTORS
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // RT3_SUN_OCCLUSION
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // FINAL
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
        },
        // Tiled 2.5D AABB Culling
        {
            // Depth
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // RT0_INTENSITY
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // RT2_MOTION_VECTORS
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // RT3_SUN_OCCLUSION
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // FINAL
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
        },
        // DICE Tiled
        {
            // Depth
            {
                { g_pKillzoneLightGridCS_8, sizeof(g_pKillzoneLightGridCS_8) },
                { g_pKillzoneLightGridCS_16, sizeof(g_pKillzoneLightGridCS_16) },
                { g_pKillzoneLightGridCS_24, sizeof(g_pKillzoneLightGridCS_24) },
                { g_pKillzoneLightGridCS_32, sizeof(g_pKillzoneLightGridCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pKillzoneLightGridCS_8, sizeof(g_pKillzoneLightGridCS_8) },
                { g_pKillzoneLightGridCS_16, sizeof(g_pKillzoneLightGridCS_16) },
                { g_pKillzoneLightGridCS_24, sizeof(g_pKillzoneLightGridCS_24) },
                { g_pKillzoneLightGridCS_32, sizeof(g_pKillzoneLightGridCS_32) },
            },
            // RT0_INTENSITY
            {
                { g_pKillzoneLightGridCS_8, sizeof(g_pKillzoneLightGridCS_8) },
                { g_pKillzoneLightGridCS_16, sizeof(g_pKillzoneLightGridCS_16) },
                { g_pKillzoneLightGridCS_24, sizeof(g_pKillzoneLightGridCS_24) },
                { g_pKillzoneLightGridCS_32, sizeof(g_pKillzoneLightGridCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pKillzoneLightGridCS_8, sizeof(g_pKillzoneLightGridCS_8) },
                { g_pKillzoneLightGridCS_16, sizeof(g_pKillzoneLightGridCS_16) },
                { g_pKillzoneLightGridCS_24, sizeof(g_pKillzoneLightGridCS_24) },
                { g_pKillzoneLightGridCS_32, sizeof(g_pKillzoneLightGridCS_32) },
            },
            // RT2_MOTION_VECTORS
            {
                { g_pKillzoneLightGridCS_8, sizeof(g_pKillzoneLightGridCS_8) },
                { g_pKillzoneLightGridCS_16, sizeof(g_pKillzoneLightGridCS_16) },
                { g_pKillzoneLightGridCS_24, sizeof(g_pKillzoneLightGridCS_24) },
                { g_pKillzoneLightGridCS_32, sizeof(g_pKillzoneLightGridCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pKillzoneLightGridCS_8, sizeof(g_pKillzoneLightGridCS_8) },
                { g_pKillzoneLightGridCS_16, sizeof(g_pKillzoneLightGridCS_16) },
                { g_pKillzoneLightGridCS_24, sizeof(g_pKillzoneLightGridCS_24) },
                { g_pKillzoneLightGridCS_32, sizeof(g_pKillzoneLightGridCS_32) },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { g_pKillzoneLightGridCS_8, sizeof(g_pKillzoneLightGridCS_8) },
                { g_pKillzoneLightGridCS_16, sizeof(g_pKillzoneLightGridCS_16) },
                { g_pKillzoneLightGridCS_24, sizeof(g_pKillzoneLightGridCS_24) },
                { g_pKillzoneLightGridCS_32, sizeof(g_pKillzoneLightGridCS_32) },
            },
            // RT3_SUN_OCCLUSION
            {
                { g_pKillzoneLightGridCS_8, sizeof(g_pKillzoneLightGridCS_8) },
                { g_pKillzoneLightGridCS_16, sizeof(g_pKillzoneLightGridCS_16) },
                { g_pKillzoneLightGridCS_24, sizeof(g_pKillzoneLightGridCS_24) },
                { g_pKillzoneLightGridCS_32, sizeof(g_pKillzoneLightGridCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pKillzoneLightGridDensityCS_8, sizeof(g_pKillzoneLightGridDensityCS_8) },
                { g_pKillzoneLightGridDensityCS_16, sizeof(g_pKillzoneLightGridDensityCS_16) },
                { g_pKillzoneLightGridDensityCS_24, sizeof(g_pKillzoneLightGridDensityCS_24) },
                { g_pKillzoneLightGridDensityCS_32, sizeof(g_pKillzoneLightGridDensityCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pKillzoneLightGridFPRCS_8, sizeof(g_pKillzoneLightGridFPRCS_8)},
                { g_pKillzoneLightGridFPRCS_16, sizeof(g_pKillzoneLightGridFPRCS_16)},
                { g_pKillzoneLightGridFPRCS_24, sizeof(g_pKillzoneLightGridFPRCS_24)},
                { g_pKillzoneLightGridFPRCS_32, sizeof(g_pKillzoneLightGridFPRCS_32)},
            },
            // FINAL
            {
                { g_pKillzoneLightGridCS_8, sizeof(g_pKillzoneLightGridCS_8)},
                { g_pKillzoneLightGridCS_16, sizeof(g_pKillzoneLightGridCS_16)},
                { g_pKillzoneLightGridCS_24, sizeof(g_pKillzoneLightGridCS_24)},
                { g_pKillzoneLightGridCS_32, sizeof(g_pKillzoneLightGridCS_32)},
            },
        },
        // DICE Tiled 2.5D Culling
        {
            // Depth
            {
                { g_pKillzoneLightCullingGridCS_8, sizeof(g_pKillzoneLightCullingGridCS_8) },
                { g_pKillzoneLightCullingGridCS_16, sizeof(g_pKillzoneLightCullingGridCS_16) },
                { g_pKillzoneLightCullingGridCS_24, sizeof(g_pKillzoneLightCullingGridCS_24) },
                { g_pKillzoneLightCullingGridCS_32, sizeof(g_pKillzoneLightCullingGridCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pKillzoneLightCullingGridCS_8, sizeof(g_pKillzoneLightCullingGridCS_8) },
                { g_pKillzoneLightCullingGridCS_16, sizeof(g_pKillzoneLightCullingGridCS_16) },
                { g_pKillzoneLightCullingGridCS_24, sizeof(g_pKillzoneLightCullingGridCS_24) },
                { g_pKillzoneLightCullingGridCS_32, sizeof(g_pKillzoneLightCullingGridCS_32) },
            },
            // RT0_INTENSITY
            {
                { g_pKillzoneLightCullingGridCS_8, sizeof(g_pKillzoneLightCullingGridCS_8) },
                { g_pKillzoneLightCullingGridCS_16, sizeof(g_pKillzoneLightCullingGridCS_16) },
                { g_pKillzoneLightCullingGridCS_24, sizeof(g_pKillzoneLightCullingGridCS_24) },
                { g_pKillzoneLightCullingGridCS_32, sizeof(g_pKillzoneLightCullingGridCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pKillzoneLightCullingGridCS_8, sizeof(g_pKillzoneLightCullingGridCS_8) },
                { g_pKillzoneLightCullingGridCS_16, sizeof(g_pKillzoneLightCullingGridCS_16) },
                { g_pKillzoneLightCullingGridCS_24, sizeof(g_pKillzoneLightCullingGridCS_24) },
                { g_pKillzoneLightCullingGridCS_32, sizeof(g_pKillzoneLightCullingGridCS_32) },
            },
            // RT2_MOTION_VECTORS
            {
                { g_pKillzoneLightCullingGridCS_8, sizeof(g_pKillzoneLightCullingGridCS_8) },
                { g_pKillzoneLightCullingGridCS_16, sizeof(g_pKillzoneLightCullingGridCS_16) },
                { g_pKillzoneLightCullingGridCS_24, sizeof(g_pKillzoneLightCullingGridCS_24) },
                { g_pKillzoneLightCullingGridCS_32, sizeof(g_pKillzoneLightCullingGridCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pKillzoneLightCullingGridCS_8, sizeof(g_pKillzoneLightCullingGridCS_8) },
                { g_pKillzoneLightCullingGridCS_16, sizeof(g_pKillzoneLightCullingGridCS_16) },
                { g_pKillzoneLightCullingGridCS_24, sizeof(g_pKillzoneLightCullingGridCS_24) },
                { g_pKillzoneLightCullingGridCS_32, sizeof(g_pKillzoneLightCullingGridCS_32) },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { g_pKillzoneLightCullingGridCS_8, sizeof(g_pKillzoneLightCullingGridCS_8) },
                { g_pKillzoneLightCullingGridCS_16, sizeof(g_pKillzoneLightCullingGridCS_16) },
                { g_pKillzoneLightCullingGridCS_24, sizeof(g_pKillzoneLightCullingGridCS_24) },
                { g_pKillzoneLightCullingGridCS_32, sizeof(g_pKillzoneLightCullingGridCS_32) },
            },
            // RT3_SUN_OCCLUSION
            {
                { g_pKillzoneLightCullingGridCS_8, sizeof(g_pKillzoneLightCullingGridCS_8) },
                { g_pKillzoneLightCullingGridCS_16, sizeof(g_pKillzoneLightCullingGridCS_16) },
                { g_pKillzoneLightCullingGridCS_24, sizeof(g_pKillzoneLightCullingGridCS_24) },
                { g_pKillzoneLightCullingGridCS_32, sizeof(g_pKillzoneLightCullingGridCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pKillzoneLightCullingGridDensityCS_8, sizeof(g_pKillzoneLightCullingGridDensityCS_8) },
                { g_pKillzoneLightCullingGridDensityCS_16, sizeof(g_pKillzoneLightCullingGridDensityCS_16) },
                { g_pKillzoneLightCullingGridDensityCS_24, sizeof(g_pKillzoneLightCullingGridDensityCS_24) },
                { g_pKillzoneLightCullingGridDensityCS_32, sizeof(g_pKillzoneLightCullingGridDensityCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pKillzoneLightCullingGridFPRCS_8, sizeof(g_pKillzoneLightCullingGridFPRCS_8)},
                { g_pKillzoneLightCullingGridFPRCS_16, sizeof(g_pKillzoneLightCullingGridFPRCS_16)},
                { g_pKillzoneLightCullingGridFPRCS_24, sizeof(g_pKillzoneLightCullingGridFPRCS_24)},
                { g_pKillzoneLightCullingGridFPRCS_32, sizeof(g_pKillzoneLightCullingGridFPRCS_32)},
            },
            // FINAL
            {
                { g_pKillzoneLightCullingGridCS_8, sizeof(g_pKillzoneLightCullingGridCS_8)},
                { g_pKillzoneLightCullingGridCS_16, sizeof(g_pKillzoneLightCullingGridCS_16)},
                { g_pKillzoneLightCullingGridCS_24, sizeof(g_pKillzoneLightCullingGridCS_24)},
                { g_pKillzoneLightCullingGridCS_32, sizeof(g_pKillzoneLightCullingGridCS_32)},
            },
        },
        // DICE Tiled 2.5D AABB Culling
        {
            // Depth
            {
                { g_pKillzoneLightAABBCullingGridCS_8, sizeof(g_pKillzoneLightAABBCullingGridCS_8) },
                { g_pKillzoneLightAABBCullingGridCS_16, sizeof(g_pKillzoneLightAABBCullingGridCS_16) },
                { g_pKillzoneLightAABBCullingGridCS_24, sizeof(g_pKillzoneLightAABBCullingGridCS_24) },
                { g_pKillzoneLightAABBCullingGridCS_32, sizeof(g_pKillzoneLightAABBCullingGridCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pKillzoneLightAABBCullingGridCS_8, sizeof(g_pKillzoneLightAABBCullingGridCS_8) },
                { g_pKillzoneLightAABBCullingGridCS_16, sizeof(g_pKillzoneLightAABBCullingGridCS_16) },
                { g_pKillzoneLightAABBCullingGridCS_24, sizeof(g_pKillzoneLightAABBCullingGridCS_24) },
                { g_pKillzoneLightAABBCullingGridCS_32, sizeof(g_pKillzoneLightAABBCullingGridCS_32) },
            },
            // RT0_INTENSITY
            {
                { g_pKillzoneLightAABBCullingGridCS_8, sizeof(g_pKillzoneLightAABBCullingGridCS_8) },
                { g_pKillzoneLightAABBCullingGridCS_16, sizeof(g_pKillzoneLightAABBCullingGridCS_16) },
                { g_pKillzoneLightAABBCullingGridCS_24, sizeof(g_pKillzoneLightAABBCullingGridCS_24) },
                { g_pKillzoneLightAABBCullingGridCS_32, sizeof(g_pKillzoneLightAABBCullingGridCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pKillzoneLightAABBCullingGridCS_8, sizeof(g_pKillzoneLightAABBCullingGridCS_8) },
                { g_pKillzoneLightAABBCullingGridCS_16, sizeof(g_pKillzoneLightAABBCullingGridCS_16) },
                { g_pKillzoneLightAABBCullingGridCS_24, sizeof(g_pKillzoneLightAABBCullingGridCS_24) },
                { g_pKillzoneLightAABBCullingGridCS_32, sizeof(g_pKillzoneLightAABBCullingGridCS_32) },
            },
            // RT2_MOTION_VECTORS
            {
                { g_pKillzoneLightAABBCullingGridCS_8, sizeof(g_pKillzoneLightAABBCullingGridCS_8) },
                { g_pKillzoneLightAABBCullingGridCS_16, sizeof(g_pKillzoneLightAABBCullingGridCS_16) },
                { g_pKillzoneLightAABBCullingGridCS_24, sizeof(g_pKillzoneLightAABBCullingGridCS_24) },
                { g_pKillzoneLightAABBCullingGridCS_32, sizeof(g_pKillzoneLightAABBCullingGridCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pKillzoneLightAABBCullingGridCS_8, sizeof(g_pKillzoneLightAABBCullingGridCS_8) },
                { g_pKillzoneLightAABBCullingGridCS_16, sizeof(g_pKillzoneLightAABBCullingGridCS_16) },
                { g_pKillzoneLightAABBCullingGridCS_24, sizeof(g_pKillzoneLightAABBCullingGridCS_24) },
                { g_pKillzoneLightAABBCullingGridCS_32, sizeof(g_pKillzoneLightAABBCullingGridCS_32) },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { g_pKillzoneLightAABBCullingGridCS_8, sizeof(g_pKillzoneLightAABBCullingGridCS_8) },
                { g_pKillzoneLightAABBCullingGridCS_16, sizeof(g_pKillzoneLightAABBCullingGridCS_16) },
                { g_pKillzoneLightAABBCullingGridCS_24, sizeof(g_pKillzoneLightAABBCullingGridCS_24) },
                { g_pKillzoneLightAABBCullingGridCS_32, sizeof(g_pKillzoneLightAABBCullingGridCS_32) },
            },
            // RT3_SUN_OCCLUSION
            {
                { g_pKillzoneLightAABBCullingGridCS_8, sizeof(g_pKillzoneLightAABBCullingGridCS_8) },
                { g_pKillzoneLightAABBCullingGridCS_16, sizeof(g_pKillzoneLightAABBCullingGridCS_16) },
                { g_pKillzoneLightAABBCullingGridCS_24, sizeof(g_pKillzoneLightAABBCullingGridCS_24) },
                { g_pKillzoneLightAABBCullingGridCS_32, sizeof(g_pKillzoneLightAABBCullingGridCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pKillzoneLightAABBCullingGridDensityCS_8, sizeof(g_pKillzoneLightAABBCullingGridDensityCS_8) },
                { g_pKillzoneLightAABBCullingGridDensityCS_16, sizeof(g_pKillzoneLightAABBCullingGridDensityCS_16) },
                { g_pKillzoneLightAABBCullingGridDensityCS_24, sizeof(g_pKillzoneLightAABBCullingGridDensityCS_24) },
                { g_pKillzoneLightAABBCullingGridDensityCS_32, sizeof(g_pKillzoneLightAABBCullingGridDensityCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pKillzoneLightAABBCullingGridFPRCS_8, sizeof(g_pKillzoneLightAABBCullingGridFPRCS_8)},
                { g_pKillzoneLightAABBCullingGridFPRCS_16, sizeof(g_pKillzoneLightAABBCullingGridFPRCS_16)},
                { g_pKillzoneLightAABBCullingGridFPRCS_24, sizeof(g_pKillzoneLightAABBCullingGridFPRCS_24)},
                { g_pKillzoneLightAABBCullingGridFPRCS_32, sizeof(g_pKillzoneLightAABBCullingGridFPRCS_32)},
            },
            // FINAL
            {
                { g_pKillzoneLightAABBCullingGridCS_8, sizeof(g_pKillzoneLightAABBCullingGridCS_8)},
                { g_pKillzoneLightAABBCullingGridCS_16, sizeof(g_pKillzoneLightAABBCullingGridCS_16)},
                { g_pKillzoneLightAABBCullingGridCS_24, sizeof(g_pKillzoneLightAABBCullingGridCS_24)},
                { g_pKillzoneLightAABBCullingGridCS_32, sizeof(g_pKillzoneLightAABBCullingGridCS_32)},
            },
        },
        // INTEL Tiled
        {
            // Depth
            {
                { g_pKillzoneIntelLightGridCS_8, sizeof(g_pKillzoneIntelLightGridCS_8) },
                { g_pKillzoneIntelLightGridCS_16, sizeof(g_pKillzoneIntelLightGridCS_16) },
                { g_pKillzoneIntelLightGridCS_24, sizeof(g_pKillzoneIntelLightGridCS_24) },
                { g_pKillzoneIntelLightGridCS_32, sizeof(g_pKillzoneIntelLightGridCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pKillzoneIntelLightGridCS_8, sizeof(g_pKillzoneIntelLightGridCS_8) },
                { g_pKillzoneIntelLightGridCS_16, sizeof(g_pKillzoneIntelLightGridCS_16) },
                { g_pKillzoneIntelLightGridCS_24, sizeof(g_pKillzoneIntelLightGridCS_24) },
                { g_pKillzoneIntelLightGridCS_32, sizeof(g_pKillzoneIntelLightGridCS_32) },
            },
            // RT0_INTENSITY
            {
                { g_pKillzoneIntelLightGridCS_8, sizeof(g_pKillzoneIntelLightGridCS_8) },
                { g_pKillzoneIntelLightGridCS_16, sizeof(g_pKillzoneIntelLightGridCS_16) },
                { g_pKillzoneIntelLightGridCS_24, sizeof(g_pKillzoneIntelLightGridCS_24) },
                { g_pKillzoneIntelLightGridCS_32, sizeof(g_pKillzoneIntelLightGridCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pKillzoneIntelLightGridCS_8, sizeof(g_pKillzoneIntelLightGridCS_8) },
                { g_pKillzoneIntelLightGridCS_16, sizeof(g_pKillzoneIntelLightGridCS_16) },
                { g_pKillzoneIntelLightGridCS_24, sizeof(g_pKillzoneIntelLightGridCS_24) },
                { g_pKillzoneIntelLightGridCS_32, sizeof(g_pKillzoneIntelLightGridCS_32) },
            },
            // RT2_MOTION_VECTORS
            {
                { g_pKillzoneIntelLightGridCS_8, sizeof(g_pKillzoneIntelLightGridCS_8) },
                { g_pKillzoneIntelLightGridCS_16, sizeof(g_pKillzoneIntelLightGridCS_16) },
                { g_pKillzoneIntelLightGridCS_24, sizeof(g_pKillzoneIntelLightGridCS_24) },
                { g_pKillzoneIntelLightGridCS_32, sizeof(g_pKillzoneIntelLightGridCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pKillzoneIntelLightGridCS_8, sizeof(g_pKillzoneIntelLightGridCS_8) },
                { g_pKillzoneIntelLightGridCS_16, sizeof(g_pKillzoneIntelLightGridCS_16) },
                { g_pKillzoneIntelLightGridCS_24, sizeof(g_pKillzoneIntelLightGridCS_24) },
                { g_pKillzoneIntelLightGridCS_32, sizeof(g_pKillzoneIntelLightGridCS_32) },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { g_pKillzoneIntelLightGridCS_8, sizeof(g_pKillzoneIntelLightGridCS_8) },
                { g_pKillzoneIntelLightGridCS_16, sizeof(g_pKillzoneIntelLightGridCS_16) },
                { g_pKillzoneIntelLightGridCS_24, sizeof(g_pKillzoneIntelLightGridCS_24) },
                { g_pKillzoneIntelLightGridCS_32, sizeof(g_pKillzoneIntelLightGridCS_32) },
            },
            // RT3_SUN_OCCLUSION
            {
                { g_pKillzoneIntelLightGridCS_8, sizeof(g_pKillzoneIntelLightGridCS_8) },
                { g_pKillzoneIntelLightGridCS_16, sizeof(g_pKillzoneIntelLightGridCS_16) },
                { g_pKillzoneIntelLightGridCS_24, sizeof(g_pKillzoneIntelLightGridCS_24) },
                { g_pKillzoneIntelLightGridCS_32, sizeof(g_pKillzoneIntelLightGridCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pKillzoneIntelLightGridDensityCS_8, sizeof(g_pKillzoneIntelLightGridDensityCS_8) },
                { g_pKillzoneIntelLightGridDensityCS_16, sizeof(g_pKillzoneIntelLightGridDensityCS_16) },
                { g_pKillzoneIntelLightGridDensityCS_24, sizeof(g_pKillzoneIntelLightGridDensityCS_24) },
                { g_pKillzoneIntelLightGridDensityCS_32, sizeof(g_pKillzoneIntelLightGridDensityCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pKillzoneIntelLightGridFPRCS_8, sizeof(g_pKillzoneIntelLightGridFPRCS_8)},
                { g_pKillzoneIntelLightGridFPRCS_16, sizeof(g_pKillzoneIntelLightGridFPRCS_16)},
                { g_pKillzoneIntelLightGridFPRCS_24, sizeof(g_pKillzoneIntelLightGridFPRCS_24)},
                { g_pKillzoneIntelLightGridFPRCS_32, sizeof(g_pKillzoneIntelLightGridFPRCS_32)},
            },
            // FINAL
            {
                { g_pKillzoneIntelLightGridCS_8, sizeof(g_pKillzoneIntelLightGridCS_8)},
                { g_pKillzoneIntelLightGridCS_16, sizeof(g_pKillzoneIntelLightGridCS_16)},
                { g_pKillzoneIntelLightGridCS_24, sizeof(g_pKillzoneIntelLightGridCS_24)},
                { g_pKillzoneIntelLightGridCS_32, sizeof(g_pKillzoneIntelLightGridCS_32)},
            },
        },
        // Clustered?
        {
            // Depth
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // RT0_INTENSITY
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // RT1_NORMAL
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // RT2_MOTION_VECTORS
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // RT3_SUN_OCCLUSION
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // FINAL
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
        },
    };
#elif THIN_GBUFFER
    std::pair<const unsigned char* const, size_t> m_aThinGBufferComputeShaders[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eGBufferDataType::COUNT) + 1][static_cast<size_t>(eClusterType::COUNT)] =
    {
        // Default
        {
            // Depth
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT1_GLOSSINESS
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // FINAL
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
        },
        // Tiled
        {
            // Depth
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT1_GLOSSINESS
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
            // FINAL
            {
                { g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8) },
                { g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16) },
                { g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24) },
                { g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32) },
            },
        },
        // Tiled 2.5D Culling
        {
            // Depth
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // RT1_GLOSSINESS
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
            // FINAL
            {
                { g_pFillLight2_5DGridCS_8, sizeof(g_pFillLight2_5DGridCS_8) },
                { g_pFillLight2_5DGridCS_16, sizeof(g_pFillLight2_5DGridCS_16) },
                { g_pFillLight2_5DGridCS_24, sizeof(g_pFillLight2_5DGridCS_24) },
                { g_pFillLight2_5DGridCS_32, sizeof(g_pFillLight2_5DGridCS_32) },
            },
        },
        // Tiled 2.5D AABB Culling
        {
            // Depth
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // RT1_GLOSSINESS
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
            // FINAL
            {
                { g_pFillLight2_5DAABBGridCS_8, sizeof(g_pFillLight2_5DAABBGridCS_8) },
                { g_pFillLight2_5DAABBGridCS_16, sizeof(g_pFillLight2_5DAABBGridCS_16) },
                { g_pFillLight2_5DAABBGridCS_24, sizeof(g_pFillLight2_5DAABBGridCS_24) },
                { g_pFillLight2_5DAABBGridCS_32, sizeof(g_pFillLight2_5DAABBGridCS_32) },
            },
        },
        // DICE Tiled
        {
            // Depth
            {
                { g_pThinGBufferTiledDiceCS_8,  sizeof(g_pThinGBufferTiledDiceCS_8)  },
                { g_pThinGBufferTiledDiceCS_16, sizeof(g_pThinGBufferTiledDiceCS_16) },
                { g_pThinGBufferTiledDiceCS_24, sizeof(g_pThinGBufferTiledDiceCS_24) },
                { g_pThinGBufferTiledDiceCS_32, sizeof(g_pThinGBufferTiledDiceCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pThinGBufferTiledDiceCS_8,  sizeof(g_pThinGBufferTiledDiceCS_8)  },
                { g_pThinGBufferTiledDiceCS_16, sizeof(g_pThinGBufferTiledDiceCS_16) },
                { g_pThinGBufferTiledDiceCS_24, sizeof(g_pThinGBufferTiledDiceCS_24) },
                { g_pThinGBufferTiledDiceCS_32, sizeof(g_pThinGBufferTiledDiceCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pThinGBufferTiledDiceCS_8,  sizeof(g_pThinGBufferTiledDiceCS_8)  },
                { g_pThinGBufferTiledDiceCS_16, sizeof(g_pThinGBufferTiledDiceCS_16) },
                { g_pThinGBufferTiledDiceCS_24, sizeof(g_pThinGBufferTiledDiceCS_24) },
                { g_pThinGBufferTiledDiceCS_32, sizeof(g_pThinGBufferTiledDiceCS_32) },
            },
            // RT1_GLOSSINESS
            {
                { g_pThinGBufferTiledDiceCS_8,  sizeof(g_pThinGBufferTiledDiceCS_8)  },
                { g_pThinGBufferTiledDiceCS_16, sizeof(g_pThinGBufferTiledDiceCS_16) },
                { g_pThinGBufferTiledDiceCS_24, sizeof(g_pThinGBufferTiledDiceCS_24) },
                { g_pThinGBufferTiledDiceCS_32, sizeof(g_pThinGBufferTiledDiceCS_32) },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { g_pThinGBufferTiledDiceCS_8,  sizeof(g_pThinGBufferTiledDiceCS_8)  },
                { g_pThinGBufferTiledDiceCS_16, sizeof(g_pThinGBufferTiledDiceCS_16) },
                { g_pThinGBufferTiledDiceCS_24, sizeof(g_pThinGBufferTiledDiceCS_24) },
                { g_pThinGBufferTiledDiceCS_32, sizeof(g_pThinGBufferTiledDiceCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pThinGBufferTiledDiceCS_8,  sizeof(g_pThinGBufferTiledDiceCS_8)  },
                { g_pThinGBufferTiledDiceCS_16, sizeof(g_pThinGBufferTiledDiceCS_16) },
                { g_pThinGBufferTiledDiceCS_24, sizeof(g_pThinGBufferTiledDiceCS_24) },
                { g_pThinGBufferTiledDiceCS_32, sizeof(g_pThinGBufferTiledDiceCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pThinGBufferTiledDiceDensityCS_8,  sizeof(g_pThinGBufferTiledDiceDensityCS_8)  },
                { g_pThinGBufferTiledDiceDensityCS_16, sizeof(g_pThinGBufferTiledDiceDensityCS_16) },
                { g_pThinGBufferTiledDiceDensityCS_24, sizeof(g_pThinGBufferTiledDiceDensityCS_24) },
                { g_pThinGBufferTiledDiceDensityCS_32, sizeof(g_pThinGBufferTiledDiceDensityCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pThinGBufferTiledDiceFPRCS_8,  sizeof(g_pThinGBufferTiledDiceFPRCS_8) },
                { g_pThinGBufferTiledDiceFPRCS_16, sizeof(g_pThinGBufferTiledDiceFPRCS_16)},
                { g_pThinGBufferTiledDiceFPRCS_24, sizeof(g_pThinGBufferTiledDiceFPRCS_24)},
                { g_pThinGBufferTiledDiceFPRCS_32, sizeof(g_pThinGBufferTiledDiceFPRCS_32)},
            },
            // FINAL
            {
                { g_pThinGBufferTiledDiceCS_8,  sizeof(g_pThinGBufferTiledDiceCS_8)  },
                { g_pThinGBufferTiledDiceCS_16, sizeof(g_pThinGBufferTiledDiceCS_16) },
                { g_pThinGBufferTiledDiceCS_24, sizeof(g_pThinGBufferTiledDiceCS_24) },
                { g_pThinGBufferTiledDiceCS_32, sizeof(g_pThinGBufferTiledDiceCS_32) },
            },
        },
        // DICE Tiled 2.5D Culling
        {
            // Depth
            {
                { g_pThinGBufferTiledDice2_5DCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pThinGBufferTiledDice2_5DCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pThinGBufferTiledDice2_5DCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_32) },
            },
            // RT1_GLOSSINESS
            {
                { g_pThinGBufferTiledDice2_5DCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_32) },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { g_pThinGBufferTiledDice2_5DCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pThinGBufferTiledDice2_5DCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pThinGBufferTiledDice2_5DCullingDensityCS_8,  sizeof(g_pThinGBufferTiledDice2_5DCullingDensityCS_8)  },
                { g_pThinGBufferTiledDice2_5DCullingDensityCS_16, sizeof(g_pThinGBufferTiledDice2_5DCullingDensityCS_16) },
                { g_pThinGBufferTiledDice2_5DCullingDensityCS_24, sizeof(g_pThinGBufferTiledDice2_5DCullingDensityCS_24) },
                { g_pThinGBufferTiledDice2_5DCullingDensityCS_32, sizeof(g_pThinGBufferTiledDice2_5DCullingDensityCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pThinGBufferTiledDice2_5DCullingFPRCS_8,  sizeof(g_pThinGBufferTiledDice2_5DCullingFPRCS_8) },
                { g_pThinGBufferTiledDice2_5DCullingFPRCS_16, sizeof(g_pThinGBufferTiledDice2_5DCullingFPRCS_16)},
                { g_pThinGBufferTiledDice2_5DCullingFPRCS_24, sizeof(g_pThinGBufferTiledDice2_5DCullingFPRCS_24)},
                { g_pThinGBufferTiledDice2_5DCullingFPRCS_32, sizeof(g_pThinGBufferTiledDice2_5DCullingFPRCS_32)},
            },
            // FINAL
            {
                { g_pThinGBufferTiledDice2_5DCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DCullingCS_32) },
            },
        },
        // DICE Tiled 2.5D AABB Culling
        {
            // Depth
            {
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_32) },
            },
            // RT1_GLOSSINESS
            {
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_32) },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pThinGBufferTiledDice2_5DAABBCullingDensityCS_8,  sizeof(g_pThinGBufferTiledDice2_5DAABBCullingDensityCS_8)  },
                { g_pThinGBufferTiledDice2_5DAABBCullingDensityCS_16, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingDensityCS_16) },
                { g_pThinGBufferTiledDice2_5DAABBCullingDensityCS_24, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingDensityCS_24) },
                { g_pThinGBufferTiledDice2_5DAABBCullingDensityCS_32, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingDensityCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pThinGBufferTiledDice2_5DAABBCullingFPRCS_8,  sizeof(g_pThinGBufferTiledDice2_5DAABBCullingFPRCS_8)},
                { g_pThinGBufferTiledDice2_5DAABBCullingFPRCS_16, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingFPRCS_16)},
                { g_pThinGBufferTiledDice2_5DAABBCullingFPRCS_24, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingFPRCS_24)},
                { g_pThinGBufferTiledDice2_5DAABBCullingFPRCS_32, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingFPRCS_32)},
            },
            // FINAL
            {
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_8,  sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_8)  },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_16, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_16) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_24, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_24) },
                { g_pThinGBufferTiledDice2_5DAABBCullingCS_32, sizeof(g_pThinGBufferTiledDice2_5DAABBCullingCS_32) },
            },
        },
        // INTEL Tiled
        {
            // Depth
            {
                { g_pThinGBufferTiledIntelCS_8,  sizeof(g_pThinGBufferTiledIntelCS_8)  },
                { g_pThinGBufferTiledIntelCS_16, sizeof(g_pThinGBufferTiledIntelCS_16) },
                { g_pThinGBufferTiledIntelCS_24, sizeof(g_pThinGBufferTiledIntelCS_24) },
                { g_pThinGBufferTiledIntelCS_32, sizeof(g_pThinGBufferTiledIntelCS_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pThinGBufferTiledIntelCS_8,  sizeof(g_pThinGBufferTiledIntelCS_8)  },
                { g_pThinGBufferTiledIntelCS_16, sizeof(g_pThinGBufferTiledIntelCS_16) },
                { g_pThinGBufferTiledIntelCS_24, sizeof(g_pThinGBufferTiledIntelCS_24) },
                { g_pThinGBufferTiledIntelCS_32, sizeof(g_pThinGBufferTiledIntelCS_32) },
            },
            // RT1_NORMAL
            {
                { g_pThinGBufferTiledIntelCS_8,  sizeof(g_pThinGBufferTiledIntelCS_8)  },
                { g_pThinGBufferTiledIntelCS_16, sizeof(g_pThinGBufferTiledIntelCS_16) },
                { g_pThinGBufferTiledIntelCS_24, sizeof(g_pThinGBufferTiledIntelCS_24) },
                { g_pThinGBufferTiledIntelCS_32, sizeof(g_pThinGBufferTiledIntelCS_32) },
            },
            // RT2_MOTION_VECTORS
            {
                { g_pThinGBufferTiledIntelCS_8,  sizeof(g_pThinGBufferTiledIntelCS_8)  },
                { g_pThinGBufferTiledIntelCS_16, sizeof(g_pThinGBufferTiledIntelCS_16) },
                { g_pThinGBufferTiledIntelCS_24, sizeof(g_pThinGBufferTiledIntelCS_24) },
                { g_pThinGBufferTiledIntelCS_32, sizeof(g_pThinGBufferTiledIntelCS_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pThinGBufferTiledIntelCS_8,  sizeof(g_pThinGBufferTiledIntelCS_8)  },
                { g_pThinGBufferTiledIntelCS_16, sizeof(g_pThinGBufferTiledIntelCS_16) },
                { g_pThinGBufferTiledIntelCS_24, sizeof(g_pThinGBufferTiledIntelCS_24) },
                { g_pThinGBufferTiledIntelCS_32, sizeof(g_pThinGBufferTiledIntelCS_32) },
            },
            // RT3_DIFFUSE_ALBEDO
            {
                { g_pThinGBufferTiledIntelCS_8,  sizeof(g_pThinGBufferTiledIntelCS_8)  },
                { g_pThinGBufferTiledIntelCS_16, sizeof(g_pThinGBufferTiledIntelCS_16) },
                { g_pThinGBufferTiledIntelCS_24, sizeof(g_pThinGBufferTiledIntelCS_24) },
                { g_pThinGBufferTiledIntelCS_32, sizeof(g_pThinGBufferTiledIntelCS_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pThinGBufferTiledIntelDensityCS_8,  sizeof(g_pThinGBufferTiledIntelDensityCS_8)  },
                { g_pThinGBufferTiledIntelDensityCS_16, sizeof(g_pThinGBufferTiledIntelDensityCS_16) },
                { g_pThinGBufferTiledIntelDensityCS_24, sizeof(g_pThinGBufferTiledIntelDensityCS_24) },
                { g_pThinGBufferTiledIntelDensityCS_32, sizeof(g_pThinGBufferTiledIntelDensityCS_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pThinGBufferTiledIntelFPRCS_8,  sizeof(g_pThinGBufferTiledIntelFPRCS_8) },
                { g_pThinGBufferTiledIntelFPRCS_16, sizeof(g_pThinGBufferTiledIntelFPRCS_16)},
                { g_pThinGBufferTiledIntelFPRCS_24, sizeof(g_pThinGBufferTiledIntelFPRCS_24)},
                { g_pThinGBufferTiledIntelFPRCS_32, sizeof(g_pThinGBufferTiledIntelFPRCS_32)},
            },
            // FINAL
            {
                { g_pThinGBufferTiledIntelCS_8,  sizeof(g_pThinGBufferTiledIntelCS_8)  },
                { g_pThinGBufferTiledIntelCS_16, sizeof(g_pThinGBufferTiledIntelCS_16) },
                { g_pThinGBufferTiledIntelCS_24, sizeof(g_pThinGBufferTiledIntelCS_24) },
                { g_pThinGBufferTiledIntelCS_32, sizeof(g_pThinGBufferTiledIntelCS_32) },
            },
        },
        // Clustered?
        {
            // Depth
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // RT0_LIGHT_ACCUMULATION
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // RT1_NORMAL
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // RT1_GLOSSINESS
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // RT2_DIFFUSE_ALBEDO
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // RT2_SPEC_INTENSITY
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // LIGHT_DENSITY
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // FALSE_POSITIVE_RATE
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
            // FINAL
            {
                { g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8) },
                { g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16) },
                { g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16) },
                { g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16) },
                { g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32) },
                { g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16) },
                { g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32) },
            },
        },
    };
#endif

    LightData m_LightOriginalData[MaxLights];
    LightData m_LightData[MaxLights];
    StructuredBuffer m_LightBuffer;
    ByteAddressBuffer m_LightGrid;
    ByteAddressBuffer m_LightCluster;
    StructuredBuffer m_LightClusterAABB;

    ByteAddressBuffer m_LightGridBitMask;
    ByteAddressBuffer m_LightClusterBitMask;
    uint32_t m_FirstConeLight;
    uint32_t m_FirstConeShadowedLight;

    enum {shadowDim = 512};
    ColorBuffer m_LightShadowArray;
    ShadowBuffer m_LightShadowTempBuffer;
    Matrix4 m_LightShadowMatrix[MaxLights];

    //std::bitset<MaxLights> m_LightShadowParity;
    uint32_t m_LightShadowParity[MaxLights / 32] =
    {
        0,
    };

    constexpr const WCHAR A_SZ_LIGHT_PROF_NAME[static_cast<size_t>(eLightType::COUNT)][32] =
    {
        L"Default",
        L"Tiled",
        L"Tiled 2.5D",
        L"Tiled 2.5D AABB",
        L"Tiled (D)",
        L"Tiled 2.5D (D)",
        L"Tiled 2.5D AABB (D)",
        L"Tiled (I)",
        L"Clustered",
    };

    bool m_bUpdateLightsToggle = false;

    void InitializeResources(void);
    void CreateRandomLights(const Vector3 minBound, const Vector3 maxBound);
    //void FillLightGrid(GraphicsContext& gfxContext, const Camera& camera);
    void Shutdown(void);
}

void Lighting::InitializeResources( void )
{
    m_FillLightRootSig.Reset(3, 0);
    m_FillLightRootSig[0].InitAsConstantBuffer(0);
    m_FillLightRootSig[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2);
    m_FillLightRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 2);
    m_FillLightRootSig.Finalize(L"FillLightRS");

    m_FillLightClusterSig.Reset(3, 0);
    m_FillLightClusterSig[0].InitAsConstantBuffer(0);
    m_FillLightClusterSig[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 3);
    m_FillLightClusterSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 2);
    m_FillLightClusterSig.Finalize(L"FillLightClusterRS");

#if KILLZONE_GBUFFER
    m_KillzoneLightRootSig.Reset(4, 1);
    m_KillzoneLightRootSig.InitStaticSampler(0, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_ALL);
    m_KillzoneLightRootSig[0].InitAsConstantBuffer(0);
    m_KillzoneLightRootSig[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10);
    m_KillzoneLightRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 4);
    m_KillzoneLightRootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
    m_KillzoneLightRootSig.Finalize(L"KillzoneLightRS");
#elif THIN_GBUFFER
    m_ThinGBufferLightRootSig.Reset(4, 1);
    m_ThinGBufferLightRootSig.InitStaticSampler(0, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_ALL);
    m_ThinGBufferLightRootSig[0].InitAsConstantBuffer(0);
    m_ThinGBufferLightRootSig[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10);
    m_ThinGBufferLightRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 4);
    m_ThinGBufferLightRootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
    m_ThinGBufferLightRootSig.Finalize(L"ThinGBufferLightRS");
#endif

    for (size_t lightTypeIndex = 0; lightTypeIndex < static_cast<size_t>(eLightType::COUNT); ++lightTypeIndex)
    {
        bool bIsCluster = (static_cast<eLightType>(lightTypeIndex) == eLightType::CLUSTERED);
        size_t gridCount = bIsCluster ? static_cast<size_t>(eClusterType::COUNT) : 4;
        RootSignature& rootSignature = bIsCluster ? m_FillLightClusterSig : m_FillLightRootSig;

        for (size_t gridIndex = 0; gridIndex < gridCount; ++gridIndex)
        {
            m_aForwardPSOs[lightTypeIndex][gridIndex].SetRootSignature(rootSignature);
            m_aForwardPSOs[lightTypeIndex][gridIndex].SetComputeShader(m_aForwardComputeShaders[lightTypeIndex][gridIndex].first, m_aForwardComputeShaders[lightTypeIndex][gridIndex].second);
            m_aForwardPSOs[lightTypeIndex][gridIndex].Finalize();
        }

        if (bIsCluster)
        {
            for (size_t gridIndex = 0; gridIndex < gridCount; ++gridIndex)
            {
                m_aClusterAABBPSOs[gridIndex].SetRootSignature(rootSignature);
                m_aClusterAABBPSOs[gridIndex].SetComputeShader(m_aClusterAABBComputeShaders[gridIndex].first, m_aClusterAABBComputeShaders[gridIndex].second);
                m_aClusterAABBPSOs[gridIndex].Finalize();
            }
        }
    }

    for (size_t lightTypeIndex = 0; lightTypeIndex < static_cast<size_t>(eLightType::COUNT); ++lightTypeIndex)
    {
        for (size_t gbufferTypeIndex = 0; gbufferTypeIndex < static_cast<size_t>(eGBufferDataType::COUNT) + 1; ++gbufferTypeIndex)
        {
            bool bIsCluster = (static_cast<eLightType>(lightTypeIndex) == eLightType::CLUSTERED);
            bool bIsComputeShaderShading = (static_cast<eLightType>(lightTypeIndex) == eLightType::TILED_DICE ||
                static_cast<eLightType>(lightTypeIndex) == eLightType::TILED_DICE_2_5 ||
                static_cast<eLightType>(lightTypeIndex) == eLightType::TILED_DICE_2_5_AABB ||
                static_cast<eLightType>(lightTypeIndex) == eLightType::TILED_INTEL);
            size_t gridCount = bIsCluster ? static_cast<size_t>(eClusterType::COUNT) : 4;
            for (size_t gridIndex = 0; gridIndex < gridCount; ++gridIndex)
            {
#if KILLZONE_GBUFFER
                m_aKillzonePSOs[lightTypeIndex][gbufferTypeIndex][gridIndex].SetRootSignature(bIsComputeShaderShading ? m_KillzoneLightRootSig : (bIsCluster ? m_FillLightClusterSig : m_FillLightRootSig));
                m_aKillzonePSOs[lightTypeIndex][gbufferTypeIndex][gridIndex].SetComputeShader(m_aKillzoneComputeShaders[lightTypeIndex][gbufferTypeIndex][gridIndex].first, m_aKillzoneComputeShaders[lightTypeIndex][gbufferTypeIndex][gridIndex].second);
                m_aKillzonePSOs[lightTypeIndex][gbufferTypeIndex][gridIndex].Finalize();
#elif THIN_GBUFFER
                m_aThinGBufferPSOs[lightTypeIndex][gbufferTypeIndex][gridIndex].SetRootSignature(bIsComputeShaderShading ? m_ThinGBufferLightRootSig : (bIsCluster ? m_FillLightClusterSig : m_FillLightRootSig));
                m_aThinGBufferPSOs[lightTypeIndex][gbufferTypeIndex][gridIndex].SetComputeShader(m_aThinGBufferComputeShaders[lightTypeIndex][gbufferTypeIndex][gridIndex].first, m_aThinGBufferComputeShaders[lightTypeIndex][gbufferTypeIndex][gridIndex].second);
                m_aThinGBufferPSOs[lightTypeIndex][gbufferTypeIndex][gridIndex].Finalize();
#endif
            }
        }
    }

    // Assumes max resolution of 3840x2160
    uint32_t lightGridCells = Math::DivideByMultiple(3840, kMinLightGridDim) * Math::DivideByMultiple(2160, kMinLightGridDim);
    uint32_t lightGridSizeBytes = lightGridCells * (4 + MaxLights * 4);
    m_LightGrid.Create(L"m_LightGrid", lightGridSizeBytes, 1);

    uint32_t lightClusterCells = Math::DivideByMultiple(3840, kMinLightGridDim) * Math::DivideByMultiple(2160, kMinLightGridDim) * 32;
    //uint32_t lightClusterSizeBytes = lightClusterCells * (4 + MaxLights * 4);
    uint32_t lightClusterSizeBytes = lightClusterCells * (4 + MaxLights * 4);
    m_LightCluster.Create(L"m_LightCluster", lightClusterSizeBytes, 1);

    uint32_t lightGridBitMaskSizeBytes = lightGridCells * 4 * 4;
    m_LightGridBitMask.Create(L"m_LightGridBitMask", lightGridBitMaskSizeBytes, 1);

    uint32_t lightClusterBitMaskSizeBytes = lightClusterCells * 4 * 4;
    m_LightClusterBitMask.Create(L"m_LightClusterBitMask", lightClusterBitMaskSizeBytes, 1);

    m_LightShadowArray.CreateArray(L"m_LightShadowArray", shadowDim, shadowDim, MaxLights, DXGI_FORMAT_R16_UNORM);
    m_LightShadowTempBuffer.Create(L"m_LightShadowTempBuffer", shadowDim, shadowDim);

    uint32_t tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0]);
    uint32_t tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0]);
    uint32_t tileCountZ = aLightClusterDimensions[static_cast<size_t>(LightClusterType)][1];
    m_LightBuffer.Create(L"m_LightBuffer", MaxLights, sizeof(LightData));
    m_LightClusterAABB.Create(
        L"m_LightClusterAABB",
        tileCountX * tileCountY * tileCountZ,
        sizeof(VolumeTileAABB)
    );
}

void Lighting::CreateRandomLights( const Vector3 minBound, const Vector3 maxBound )
{
    Vector3 posScale = maxBound - minBound;
    Vector3 posBias = minBound;

    // todo: replace this with MT
    srand(12645);
    auto randUint = []() -> uint32_t
    {
        return rand(); // [0, RAND_MAX]
    };
    auto randFloat = [randUint]() -> float
    {
        return randUint() * (1.0f / RAND_MAX); // convert [0, RAND_MAX] to [0, 1]
    };
    auto randVecUniform = [randFloat]() -> Vector3
    {
        return Vector3(randFloat(), randFloat(), randFloat());
    };
    auto randGaussian = [randFloat]() -> float
    {
        // polar box-muller
        static bool gaussianPair = true;
        static float y2;

        if (gaussianPair)
        {
            gaussianPair = false;

            float x1, x2, w;
            do
            {
                x1 = 2 * randFloat() - 1;
                x2 = 2 * randFloat() - 1;
                w = x1 * x1 + x2 * x2;
            } while (w >= 1);

            w = sqrtf(-2 * logf(w) / w);
            y2 = x2 * w;
            return x1 * w;
        }
        else
        {
            gaussianPair = true;
            return y2;
        }
    };
    auto randVecGaussian = [randGaussian]() -> Vector3
    {
        return Normalize(Vector3(randGaussian(), randGaussian(), randGaussian()));
    };

    const float pi = 3.14159265359f;
    for (uint32_t n = 0; n < MaxLights; n++)
    {
        Vector3 pos = randVecUniform() * posScale + posBias;
        float lightRadius = randFloat() * 800.0f + 200.0f;

        Vector3 color = randVecUniform();
        float colorScale = randFloat() * .3f + .3f;
        color = color * colorScale;

        uint32_t type;
        // force types to match 32-bit boundaries for the BIT_MASK_SORTED case
        if (n < MaxPointLights)
            type = 0;
        else if (n < MaxPointLights + MaxConeLights)
            type = 1;
        else
            type = 2;

        Vector3 coneDir = randVecGaussian();
        float coneInner = (randFloat() * .2f + .025f) * pi;
        float coneOuter = coneInner + randFloat() * .1f * pi;

        if (type == 1 || type == 2)
        {
            // emphasize cone lights
            color = color * 5.0f;
        }

        Math::Camera shadowCamera;
        shadowCamera.SetEyeAtUp(pos, pos + coneDir, Vector3(0, 1, 0));
        shadowCamera.SetPerspectiveMatrix(coneOuter * 2, 1.0f, lightRadius * .05f, lightRadius * 1.0f);
        shadowCamera.Update();
        m_LightShadowMatrix[n] = shadowCamera.GetViewProjMatrix();
        m_LightShadowParity[n / 32] |= 1u << (n % 32);
        Matrix4 shadowTextureMatrix = Matrix4(AffineTransform(Matrix3::MakeScale( 0.5f, -0.5f, 1.0f ), Vector3(0.5f, 0.5f, 0.0f))) * m_LightShadowMatrix[n];

        m_LightData[n].pos[0] = pos.GetX();
        m_LightData[n].pos[1] = pos.GetY();
        m_LightData[n].pos[2] = pos.GetZ();
        m_LightData[n].radiusSq = lightRadius * lightRadius;
        m_LightData[n].color[0] = color.GetX();
        m_LightData[n].color[1] = color.GetY();
        m_LightData[n].color[2] = color.GetZ();
        m_LightData[n].type = type;
        m_LightData[n].coneDir[0] = coneDir.GetX();
        m_LightData[n].coneDir[1] = coneDir.GetY();
        m_LightData[n].coneDir[2] = coneDir.GetZ();
        m_LightData[n].coneAngles[0] = 1.0f / (cosf(coneInner) - cosf(coneOuter));
        m_LightData[n].coneAngles[1] = cosf(coneOuter);
        std::memcpy(m_LightData[n].shadowTextureMatrix, &shadowTextureMatrix, sizeof(shadowTextureMatrix));
        //*(Matrix4*)(m_LightData[n].shadowTextureMatrix) = shadowTextureMatrix;
    }
    // sort lights by type, needed for efficiency in the BIT_MASK approach
    /*	{
    Matrix4 copyLightShadowMatrix[MaxLights];
    memcpy(copyLightShadowMatrix, m_LightShadowMatrix, sizeof(Matrix4) * MaxLights);
    LightData copyLightData[MaxLights];
    memcpy(copyLightData, m_LightData, sizeof(LightData) * MaxLights);

    uint32_t sortArray[MaxLights];
    for (uint32_t n = 0; n < MaxLights; n++)
    {
    sortArray[n] = n;
    }
    std::sort(sortArray, sortArray + MaxLights,
    [this](const uint32_t &a, const uint32_t &b) -> bool
    {
    return this->m_LightData[a].type < this->m_LightData[b].type;
    });
    for (uint32_t n = 0; n < MaxLights; n++)
    {
    m_LightShadowMatrix[n] = copyLightShadowMatrix[sortArray[n]];
    m_LightData[n] = copyLightData[sortArray[n]];
    }
    }*/
    for (uint32_t n = 0; n < MaxLights; n++)
    {
        if (m_LightData[n].type == 1)
        {
            m_FirstConeLight = n;
            break;
        }
    }
    for (uint32_t n = 0; n < MaxLights; n++)
    {
        if (m_LightData[n].type == 2)
        {
            m_FirstConeShadowedLight = n;
            break;
        }
    }

    std::memcpy(m_LightOriginalData, m_LightData, MaxLights * sizeof(LightData));
    CommandContext::InitializeBuffer(m_LightBuffer, m_LightData, MaxLights * sizeof(LightData));
}

void Lighting::Shutdown(void)
{
    m_LightBuffer.Destroy();
    m_LightCluster.Destroy();
    m_LightClusterAABB.Destroy();
    m_LightGrid.Destroy();
    m_LightClusterBitMask.Destroy();
    m_LightGridBitMask.Destroy();
    m_LightShadowArray.Destroy();
    m_LightShadowTempBuffer.Destroy();
}

void Lighting::ResetLights(void)
{
    std::memcpy(m_LightData, m_LightOriginalData, MaxLights * sizeof(LightData));
}

void Lighting::UpdateLights(float deltaTime)
{
    if (!m_bUpdateLightsToggle)
    {
        return;
    }

    XMMATRIX rotationY = XMMatrixRotationY(deltaTime * 0.1f);
    XMMATRIX rotationX = XMMatrixRotationX(deltaTime * 0.1f);

    //for (size_t i = MaxPointLights + MaxConeLights; i < MaxLights; i += 2)
    for (size_t i = 0; i < MaxLights; i += 2)
    {
        XMVECTOR position = XMVector4Transform(XMVectorSet(m_LightData[i].pos[0], m_LightData[i].pos[1], m_LightData[i].pos[2], 1.0f), rotationY);
        m_LightData[i].pos[0] = XMVectorGetX(position);
        m_LightData[i].pos[1] = XMVectorGetY(position);
        m_LightData[i].pos[2] = XMVectorGetZ(position);
        Vector3 pos(position);
        Vector3 coneDir(m_LightData[i].coneDir[0], m_LightData[i].coneDir[1], m_LightData[i].coneDir[2]);
        float lightRadius = sqrtf(m_LightData[i].radiusSq);
        Math::Camera shadowCamera;
        shadowCamera.SetEyeAtUp(pos, pos + coneDir, Vector3(0, 1, 0));
        shadowCamera.SetPerspectiveMatrix(acosf(m_LightData[i].coneAngles[1]) * 2, 1.0f, lightRadius * .05f, lightRadius * 1.0f);
        shadowCamera.Update();
        m_LightShadowMatrix[i] = shadowCamera.GetViewProjMatrix();
        //m_LightShadowParity.reset(i);
        m_LightShadowParity[i / 32] |= (1u << (i % 32));
        Matrix4 shadowTextureMatrix = Matrix4(AffineTransform(Matrix3::MakeScale(0.5f, -0.5f, 1.0f), Vector3(0.5f, 0.5f, 0.0f))) * m_LightShadowMatrix[i];
        std::memcpy(m_LightData[i].shadowTextureMatrix, &shadowTextureMatrix, sizeof(shadowTextureMatrix));
    }

    //for (size_t i = MaxPointLights + MaxConeLights + 1; i < MaxLights; i += 2)
    for (size_t i = 1; i < MaxLights; i += 2)
    {
        XMVECTOR position = XMVector4Transform(XMVectorSet(m_LightData[i].pos[0], m_LightData[i].pos[1], m_LightData[i].pos[2], 1.0f), rotationX);
        m_LightData[i].pos[0] = XMVectorGetX(position);
        m_LightData[i].pos[1] = XMVectorGetY(position);
        m_LightData[i].pos[2] = XMVectorGetZ(position);
        Vector3 pos(position);
        Vector3 coneDir(m_LightData[i].coneDir[0], m_LightData[i].coneDir[1], m_LightData[i].coneDir[2]);
        float lightRadius = sqrtf(m_LightData[i].radiusSq);
        Math::Camera shadowCamera;
        shadowCamera.SetEyeAtUp(pos, pos + coneDir, Vector3(0, 1, 0));
        shadowCamera.SetPerspectiveMatrix(acosf(m_LightData[i].coneAngles[1]) * 2, 1.0f, lightRadius * .05f, lightRadius * 1.0f);
        shadowCamera.Update();
        m_LightShadowMatrix[i] = shadowCamera.GetViewProjMatrix();
        //m_LightShadowParity.reset(i);
        m_LightShadowParity[i / 32] |= (1u << (i % 32));
        Matrix4 shadowTextureMatrix = Matrix4(AffineTransform(Matrix3::MakeScale(0.5f, -0.5f, 1.0f), Vector3(0.5f, 0.5f, 0.0f))) * m_LightShadowMatrix[i];
        std::memcpy(m_LightData[i].shadowTextureMatrix, &shadowTextureMatrix, sizeof(shadowTextureMatrix));
    }

    CommandContext::InitializeBuffer(m_LightBuffer, m_LightData, MaxLights * sizeof(LightData));
    
    //UNREFERENCED_PARAMETER(deltaTime);
}

void Lighting::FillLightGrid(GraphicsContext& gfxContext, const Math::Camera& camera, Graphics::eLightType lightType)
{
    ScopedTimer _prof(A_SZ_LIGHT_PROF_NAME[static_cast<size_t>(lightType)], gfxContext);

    ComputeContext& Context = gfxContext.GetComputeContext();

    bool bIsCluster = lightType == eLightType::CLUSTERED;

    float FarClipDist = camera.GetFarClip();
    float NearClipDist = camera.GetNearClip();
    const float RcpZMagic = NearClipDist / (FarClipDist - NearClipDist);

    uint32_t tileCountX = 0;
    uint32_t tileCountY = 0;
    uint32_t tileCountZ = 1;

    ColorBuffer& LinearDepth = g_LinearDepth[TemporalEffects::GetFrameIndexMod2()];

    if (bIsCluster)
    {
        Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0]);
        tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0]);
        tileCountZ = aLightClusterDimensions[static_cast<size_t>(LightClusterType)][1];

        // Cluster AABB
        {
            Context.SetRootSignature(m_FillLightClusterSig);
            Context.SetPipelineState(Lighting::m_aClusterAABBPSOs[static_cast<size_t>(LightClusterType)]);

            Context.TransitionResource(m_LightClusterAABB, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
            Context.SetDynamicDescriptor(1, 1, LinearDepth.GetSRV());

            Context.SetDynamicDescriptor(2, 0, m_LightClusterAABB.GetUAV());
            //Context.SetDynamicDescriptor(2, 1, m_LightClusterBitMask.GetUAV());
            
            struct CSConstants
            {
                uint32_t ViewportWidth, ViewportHeight;
                //Matrix4 ViewProjMatrix;
                Matrix4 InvProjMatrix;
                Matrix4 InvViewMatrix;
                uint32_t TileCount[4];
                float FarZ;
                float NearZ;
            } csConstants;
            // todo: assumes 1920x1080 resolution
            csConstants.ViewportWidth = g_SceneColorBuffer.GetWidth();
            csConstants.ViewportHeight = g_SceneColorBuffer.GetHeight();
            XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetProjMatrix()), camera.GetProjMatrix());
            csConstants.InvProjMatrix = Matrix4(
                Vector4(invProj.r[0]),
                Vector4(invProj.r[1]),
                Vector4(invProj.r[2]),
                Vector4(invProj.r[3])
            );

            XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewMatrix()), camera.GetViewMatrix());
            csConstants.InvViewMatrix = Matrix4(
                Vector4(invView.r[0]),
                Vector4(invView.r[1]),
                Vector4(invView.r[2]),
                Vector4(invView.r[3])
            );
            csConstants.TileCount[0] = tileCountX;
            csConstants.TileCount[1] = tileCountY;
            csConstants.TileCount[2] = tileCountZ;
            csConstants.FarZ = camera.GetFarClip();
            csConstants.NearZ = camera.GetNearClip();
            Context.SetDynamicConstantBufferView(0, sizeof(CSConstants), &csConstants);

            // todo: assumes 1920x1080 resolution

            Context.Dispatch(tileCountX, tileCountY, tileCountZ);

            Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            Context.TransitionResource(m_LightClusterAABB, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }

        // Cluster Light
        {
            //Context.SetRootSignature(m_FillLightClusterSig);
            Context.SetPipelineState(Lighting::m_aForwardPSOs[static_cast<size_t>(lightType)][static_cast<size_t>(LightClusterType)]);

            Context.TransitionResource(m_LightCluster, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            Context.TransitionResource(m_LightClusterBitMask, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            Context.TransitionResource(m_LightClusterAABB, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
            Context.SetDynamicDescriptor(1, 1, LinearDepth.GetSRV());
            Context.SetDynamicDescriptor(1, 2, m_LightClusterAABB.GetSRV());

            Context.SetDynamicDescriptor(2, 0, m_LightCluster.GetUAV());
            //Context.SetDynamicDescriptor(2, 1, m_LightClusterBitMask.GetUAV());

            struct CSConstants
            {
                uint32_t ViewportWidth, ViewportHeight;
                float InvTileDim;
                float RcpZMagic;
                uint32_t TileCount[2];
                Matrix4 ViewProjMatrix;
                Matrix4 InvViewMatrix;
                //Matrix4 InvProjMatrix;
                //Matrix4 InvViewProj;
                //float FarZ;
                //float NearZ;
            } csConstants;
            // todo: assumes 1920x1080 resolution
            csConstants.ViewportWidth = g_SceneColorBuffer.GetWidth();
            csConstants.ViewportHeight = g_SceneColorBuffer.GetHeight();
            csConstants.InvTileDim = 1.0f / aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0];
            csConstants.RcpZMagic = RcpZMagic;
            csConstants.TileCount[0] = tileCountX;
            csConstants.TileCount[1] = tileCountY;
            csConstants.ViewProjMatrix = camera.GetViewProjMatrix();
            //csConstants.ViewMatrix = camera.GetViewMatrix();
            //XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewProjMatrix()), camera.GetViewProjMatrix());
            //csConstants.InvViewProj = Matrix4(
            //    Vector4(invViewProj.r[0]),
            //    Vector4(invViewProj.r[1]),
            //    Vector4(invViewProj.r[2]),
            //    Vector4(invViewProj.r[3])
            //);
            //XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetProjMatrix()), camera.GetProjMatrix());
            //csConstants.InvProjMatrix = Matrix4(
            //    Vector4(invProj.r[0]),
            //    Vector4(invProj.r[1]),
            //    Vector4(invProj.r[2]),
            //    Vector4(invProj.r[3])
            //);
            XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewMatrix()), camera.GetViewMatrix());
            csConstants.InvViewMatrix = Matrix4(
                Vector4(invView.r[0]),
                Vector4(invView.r[1]),
                Vector4(invView.r[2]),
                Vector4(invView.r[3])
            );
            //csConstants.FarZ = camera.GetFarClip();
            //csConstants.NearZ = camera.GetNearClip();
            Context.SetDynamicConstantBufferView(0, sizeof(CSConstants), &csConstants);

            // todo: assumes 1920x1080 resolution

            Context.Dispatch(tileCountX, tileCountY, tileCountZ);

            Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            Context.TransitionResource(m_LightCluster, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            //Context.TransitionResource(m_LightClusterBitMask, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }
    }
    else
    {
        Context.SetRootSignature(m_FillLightRootSig);
        Context.SetPipelineState(Lighting::m_aForwardPSOs[static_cast<size_t>(lightType)][(static_cast<int>(LightGridDim) / 8) - 1]);

        Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
        Context.SetDynamicDescriptor(1, 1, LinearDepth.GetSRV());

        Context.SetDynamicDescriptor(2, 0, m_LightGrid.GetUAV());
        Context.SetDynamicDescriptor(2, 1, m_LightGridBitMask.GetUAV());

        tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), LightGridDim);
        tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), LightGridDim);

        struct CSConstants
        {
            uint32_t ViewportWidth, ViewportHeight;
            float InvTileDim;
            float RcpZMagic;
            uint32_t TileCount;
            Matrix4 ViewProjMatrix;
            Matrix4 InvViewProj;
        } csConstants;
        // todo: assumes 1920x1080 resolution
        csConstants.ViewportWidth = g_SceneColorBuffer.GetWidth();
        csConstants.ViewportHeight = g_SceneColorBuffer.GetHeight();
        csConstants.InvTileDim = 1.0f / LightGridDim;
        csConstants.RcpZMagic = RcpZMagic;
        csConstants.TileCount = tileCountX;
        csConstants.ViewProjMatrix = camera.GetViewProjMatrix();
        XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewProjMatrix()), camera.GetViewProjMatrix());
        csConstants.InvViewProj = Matrix4(
            Vector4(invViewProj.r[0]),
            Vector4(invViewProj.r[1]),
            Vector4(invViewProj.r[2]),
            Vector4(invViewProj.r[3])
        );
        Context.SetDynamicConstantBufferView(0, sizeof(CSConstants), &csConstants);

        // todo: assumes 1920x1080 resolution

        Context.Dispatch(tileCountX, tileCountY, tileCountZ);

        Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
}

void Lighting::FillAndShadeLightGrid(GraphicsContext& gfxContext, const Math::Camera& camera, const DescriptorHandle& gBufferHandle, Graphics::eLightType lightType, Graphics::eGBufferDataType gbufferType)
{
    ScopedTimer _prof(A_SZ_LIGHT_PROF_NAME[static_cast<size_t>(lightType)], gfxContext);

    ASSERT(lightType == eLightType::TILED_DICE || lightType == eLightType::TILED_DICE_2_5 || lightType == eLightType::TILED_DICE_2_5_AABB || lightType == eLightType::TILED_INTEL);

    ComputeContext& Context = gfxContext.GetComputeContext();

#if KILLZONE_GBUFFER
    Context.SetRootSignature(m_KillzoneLightRootSig);
#elif THIN_GBUFFER
    Context.SetRootSignature(m_ThinGBufferLightRootSig);
#endif

    bool bIsCluster = lightType == eLightType::CLUSTERED;
    if (bIsCluster)
    {
#if KILLZONE_GBUFFER
        Context.SetPipelineState(Lighting::m_aKillzonePSOs[static_cast<size_t>(lightType)][static_cast<size_t>(gbufferType)][static_cast<size_t>(LightClusterType)]);
#elif THIN_GBUFFER
        Context.SetPipelineState(Lighting::m_aThinGBufferPSOs[static_cast<size_t>(lightType)][static_cast<size_t>(gbufferType)][static_cast<size_t>(LightClusterType)]);
#endif
    }
    else
    {
#if KILLZONE_GBUFFER
        Context.SetPipelineState(Lighting::m_aKillzonePSOs[static_cast<size_t>(lightType)][static_cast<size_t>(gbufferType)][(static_cast<int>(LightGridDim) / 8) - 1]);
#elif THIN_GBUFFER
        Context.SetPipelineState(Lighting::m_aThinGBufferPSOs[static_cast<size_t>(lightType)][static_cast<size_t>(gbufferType)][(static_cast<int>(LightGridDim) / 8) - 1]);
#endif
    }

    float FarClipDist = camera.GetFarClip();
    float NearClipDist = camera.GetNearClip();
    const float RcpZMagic = NearClipDist / (FarClipDist - NearClipDist);

    ColorBuffer& LinearDepth = g_LinearDepth[TemporalEffects::GetFrameIndexMod2()];

    Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    //g_aSceneGBuffers[static_cast<size_t>(eGBufferType::COUNT)]
    for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
    {
        Context.TransitionResource(g_aSceneGBuffers[i], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    Context.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    Context.SetDynamicDescriptor(1, 8, LinearDepth.GetSRV());
    Context.SetDynamicDescriptor(1, 4, m_LightBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 5, m_LightShadowArray.GetSRV());
    Context.SetDescriptorTable(2, gBufferHandle);
    Context.SetDynamicDescriptor(3, 0, g_SceneColorBuffer.GetUAV());

    // todo: assumes 1920x1080 resolution
    uint32_t tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), LightGridDim);
    uint32_t tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), LightGridDim);

    struct CSConstants
    {
        uint32_t ViewportWidth, ViewportHeight;
        float InvTileDim;
        float RcpZMagic;
        uint32_t TileCount;
        Matrix4 ViewProjMatrix;
        Matrix4 InvViewProj;
        //Matrix4 InvProj;
        Vector3 ViewerPos;
    } csConstants;
    // todo: assumes 1920x1080 resolution
    csConstants.ViewportWidth = g_SceneColorBuffer.GetWidth();
    csConstants.ViewportHeight = g_SceneColorBuffer.GetHeight();
    csConstants.InvTileDim = 1.0f / LightGridDim;
    csConstants.RcpZMagic = RcpZMagic;
    csConstants.TileCount = tileCountX;
    csConstants.ViewProjMatrix = camera.GetViewProjMatrix();
    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewProjMatrix()), camera.GetViewProjMatrix());
    csConstants.InvViewProj = Matrix4(
        Vector4(invViewProj.r[0]),
        Vector4(invViewProj.r[1]),
        Vector4(invViewProj.r[2]),
        Vector4(invViewProj.r[3])
    );
    csConstants.ViewerPos = camera.GetPosition();
    Context.SetDynamicConstantBufferView(0, sizeof(CSConstants), &csConstants);

    Context.Dispatch(tileCountX, tileCountY, 1);

    for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
    {
        Context.TransitionResource(g_aSceneGBuffers[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
    Context.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
