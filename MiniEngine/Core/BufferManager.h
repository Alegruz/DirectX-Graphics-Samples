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

#pragma once

#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "ShadowBuffer.h"
#include "GpuBuffer.h"
#include "GraphicsCore.h"

namespace Graphics
{
#define SIMPLE_GBUFFER (0)
#define KILLZONE_GBUFFER (1)
#define THIN_GBUFFER (1)

    enum class eRenderType : UINT8
    {
        FORWARD,
        DEFERRED,
        DEFERRED_THIN,
        COUNT,
    };

    enum class eForwardType : UINT8
    {
        WORLD_POS,
        DIFFUSE_ALBEDO,
        GLOSS,
        NORMAL,
        SPECULAR_INTENSITY,
        LIGHT_DENSITY,
        FALSE_POSITIVE_RATE,
        COUNT,
    };

    enum class eLightType : UINT8
    {
        DEFAULT,
        TILED,
        TILED_2_5,
        TILED_2_5_AABB,
        TILED_DICE,
        TILED_DICE_2_5,
        TILED_DICE_2_5_AABB,
        TILED_INTEL,
        CLUSTERED,
        COUNT,
    };

    enum class eGBufferType : UINT8
    {
#if SIMPLE_GBUFFER
        WORLD_POS,
        NORMAL_DEPTH,
        ALBEDO,
        SPECULAR,
#elif KILLZONE_GBUFFER
        RT0,    // Light accumulation + intensity
        RT1,    // Normal.XY
        RT2,    // Motion Vectors + spec-intensity
        RT3,    // diffuse albedo + sun-occlusion
#endif
        COUNT,
    };

    enum class eThinGBufferType : UINT8
    {
        RT0,    // Light accumulation
        RT1,    // Normal X, Y, Z Sign, Glossiness
        RT2,    // Diffuse Albedo, Specular Intensity
        COUNT,
    };

    enum class eGBufferDataType : UINT8
    {
#if SIMPLE_GBUFFER
        RT0_WORLD_POS,
        RT1_NORMAL,
        RT1_DEPTH,
        RT2_ALBEDO,
        RT3_SPECULAR,
        RT3_GLOSS,
#elif KILLZONE_GBUFFER
        DEPTH,
        RT0_LIGHT_ACCUMULATION,
        RT0_INTENSITY,
        RT1_NORMAL,
        RT2_MOTION_VECTORS,
        RT2_SPEC_INTENSITY,
        RT3_DIFFUSE_ALBEDO,
        RT3_SUN_OCCLUSION,
#endif
        LIGHT_DENSITY,
        FALSE_POSITIVE_RATE,
        COUNT,
    };

    enum class eThinGBufferDataType : UINT8
    {
        DEPTH,
        RT0_LIGHT_ACCUMULATION,
        RT1_NORMAL,
        RT1_GLOSSINESS,
        //RT1_Z_SIGN,
        RT2_DIFFUSE_ALBEDO,
        RT2_SPEC_INTENSITY,
        LIGHT_DENSITY,
        FALSE_POSITIVE_RATE,
        COUNT,
    };

    extern DepthBuffer g_SceneDepthBuffer;  // D32_FLOAT_S8_UINT
    extern ColorBuffer g_SceneColorBuffer;  // R11G11B10_FLOAT
    extern ColorBuffer g_aSceneGBuffers[static_cast<size_t>(eGBufferType::COUNT)];
    extern ColorBuffer g_aSceneThinGBuffers[static_cast<size_t>(eThinGBufferType::COUNT)];
    extern ColorBuffer g_SceneNormalBuffer; // R16G16B16A16_FLOAT
    extern ColorBuffer g_PostEffectsBuffer; // R32_UINT (to support Read-Modify-Write with a UAV)
    extern ColorBuffer g_OverlayBuffer;     // R8G8B8A8_UNORM
    extern ColorBuffer g_HorizontalBuffer;  // For separable (bicubic) upsampling

    extern ColorBuffer g_VelocityBuffer;    // R10G10B10  (3D velocity)
    extern ShadowBuffer g_ShadowBuffer;

    extern ColorBuffer g_SSAOFullScreen;	// R8_UNORM
    extern ColorBuffer g_LinearDepth[2];	// Normalized planar distance (0 at eye, 1 at far plane) computed from the SceneDepthBuffer
    extern ColorBuffer g_MinMaxDepth8;		// Min and max depth values of 8x8 tiles
    extern ColorBuffer g_MinMaxDepth16;		// Min and max depth values of 16x16 tiles
    extern ColorBuffer g_MinMaxDepth32;		// Min and max depth values of 16x16 tiles
    extern ColorBuffer g_DepthDownsize1;
    extern ColorBuffer g_DepthDownsize2;
    extern ColorBuffer g_DepthDownsize3;
    extern ColorBuffer g_DepthDownsize4;
    extern ColorBuffer g_DepthTiled1;
    extern ColorBuffer g_DepthTiled2;
    extern ColorBuffer g_DepthTiled3;
    extern ColorBuffer g_DepthTiled4;
    extern ColorBuffer g_AOMerged1;
    extern ColorBuffer g_AOMerged2;
    extern ColorBuffer g_AOMerged3;
    extern ColorBuffer g_AOMerged4;
    extern ColorBuffer g_AOSmooth1;
    extern ColorBuffer g_AOSmooth2;
    extern ColorBuffer g_AOSmooth3;
    extern ColorBuffer g_AOHighQuality1;
    extern ColorBuffer g_AOHighQuality2;
    extern ColorBuffer g_AOHighQuality3;
    extern ColorBuffer g_AOHighQuality4;

    extern ColorBuffer g_DoFTileClass[2];
    extern ColorBuffer g_DoFPresortBuffer;
    extern ColorBuffer g_DoFPrefilter;
    extern ColorBuffer g_DoFBlurColor[2];
    extern ColorBuffer g_DoFBlurAlpha[2];
    extern StructuredBuffer g_DoFWorkQueue;
    extern StructuredBuffer g_DoFFastQueue;
    extern StructuredBuffer g_DoFFixupQueue;

    extern ColorBuffer g_MotionPrepBuffer;		// R10G10B10A2
    extern ColorBuffer g_LumaBuffer;
    extern ColorBuffer g_TemporalColor[2];
    extern ColorBuffer g_TemporalMinBound;
    extern ColorBuffer g_TemporalMaxBound;

    extern ColorBuffer g_aBloomUAV1[2];		// 640x384 (1/3)
    extern ColorBuffer g_aBloomUAV2[2];		// 320x192 (1/6)  
    extern ColorBuffer g_aBloomUAV3[2];		// 160x96  (1/12)
    extern ColorBuffer g_aBloomUAV4[2];		// 80x48   (1/24)
    extern ColorBuffer g_aBloomUAV5[2];		// 40x24   (1/48)
    extern ColorBuffer g_LumaLR;
    extern ByteAddressBuffer g_Histogram;
    extern ByteAddressBuffer g_FXAAWorkQueue;
    extern TypedBuffer g_FXAAColorQueue;

    void InitializeRenderingBuffers(uint32_t NativeWidth, uint32_t NativeHeight );
    void ResizeDisplayDependentBuffers(uint32_t NativeWidth, uint32_t NativeHeight);
    void DestroyRenderingBuffers();

} // namespace Graphics
