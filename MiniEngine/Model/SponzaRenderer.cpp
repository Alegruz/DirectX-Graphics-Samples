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

// From Core
#include "GraphicsCore.h"
#include "BufferManager.h"
#include "Camera.h"
#include "CommandContext.h"
#include "TemporalEffects.h"
#include "SSAO.h"
#include "SystemTime.h"
#include "ShadowCamera.h"
#include "ParticleEffects.h"
#include "SponzaRenderer.h"
#include "Renderer.h"

// From Model
#include "ModelH3D.h"

// From ModelViewer
#include "LightManager.h"

#include "CompiledShaders/DepthViewerVS.h"
#include "CompiledShaders/DepthViewerPS.h"
#include "CompiledShaders/ModelViewerVS.h"

// Default
#include "CompiledShaders/ModelViewerForwardPS.h"

#include "CompiledShaders/ModelViewerForwardDensityPS.h"
#include "CompiledShaders/ModelViewerForwardFPRPS.h"
#include "CompiledShaders/ModelViewerForwardDiffuseAlbedoPS.h"
#include "CompiledShaders/ModelViewerForwardGlossPS.h"
#include "CompiledShaders/ModelViewerForwardNormalPS.h"
#include "CompiledShaders/ModelViewerForwardSpecularIntensityPS.h"
#include "CompiledShaders/ModelViewerForwardWorldPosPS.h"

// Tiled
#include "CompiledShaders/ModelViewerTiledPS.h"

#include "CompiledShaders/ModelViewerTiledDensityPS.h"
#include "CompiledShaders/ModelViewerTiledFPRPS.h"
#include "CompiledShaders/ModelViewerDiffuseAlbedoPS.h"
#include "CompiledShaders/ModelViewerGlossPS.h"
#include "CompiledShaders/ModelViewerNormalPS.h"
#include "CompiledShaders/ModelViewerSpecularIntensityPS.h"
#include "CompiledShaders/ModelViewerWorldPosPS.h"

// Clustered
#include "CompiledShaders/ModelViewerClusteredPS.h"

#include "CompiledShaders/ModelViewerClusteredDensityPS.h"
#include "CompiledShaders/ModelViewerClusteredFPRPS.h"
#include "CompiledShaders/ModelViewerClusteredDiffuseAlbedoPS.h"
#include "CompiledShaders/ModelViewerClusteredGlossPS.h"
#include "CompiledShaders/ModelViewerClusteredNormalPS.h"
#include "CompiledShaders/ModelViewerClusteredSpecularIntensityPS.h"
#include "CompiledShaders/ModelViewerClusteredWorldPosPS.h"

#include "CompiledShaders/GBufferLightVS.h"
#if SIMPLE_GBUFFER
#include "CompiledShaders/DeferredPS.h"
#include "CompiledShaders/GBufferAlbedoPS.h"
#include "CompiledShaders/GBufferDepthPS.h"
#include "CompiledShaders/GBufferGlossPS.h"
#include "CompiledShaders/GBufferLightPS.h"
#include "CompiledShaders/GBufferNormalPS.h"
#include "CompiledShaders/GBufferSpecularPS.h"
#include "CompiledShaders/GBufferWorldPosPS.h"
#elif KILLZONE_GBUFFER
#include "CompiledShaders/KillzoneDeferredPS.h"

// Default
#include "CompiledShaders/KillzoneGBufferDefaultLightPS.h"
#include "CompiledShaders/KillzoneGBufferDefaultLightDensityPS.h"
#include "CompiledShaders/KillzoneGBufferDefaultLightFPRPS.h"

#include "CompiledShaders/KillzoneGBufferDefaultDepthPS.h"
#include "CompiledShaders/KillzoneGBufferDefaultDiffuseAlbedoPS.h"
#include "CompiledShaders/KillzoneGBufferDefaultGlossPS.h"
#include "CompiledShaders/KillzoneGBufferDefaultLightAccumulationPS.h"
#include "CompiledShaders/KillzoneGBufferDefaultMotionVectorPS.h"
#include "CompiledShaders/KillzoneGBufferDefaultNormalPS.h"
#include "CompiledShaders/KillzoneGBufferDefaultSpecularIntensityPS.h"
#include "CompiledShaders/KillzoneGBufferDefaultSunOcclusionPS.h"

// Tiled
#include "CompiledShaders/KillzoneGBufferLightTiledPS.h"
#include "CompiledShaders/KillzoneGBufferLightTiledDensityPS.h"
#include "CompiledShaders/KillzoneGBufferLightTiledFPRPS.h"

#include "CompiledShaders/KillzoneGBufferDepthPS.h"
#include "CompiledShaders/KillzoneGBufferDiffuseAlbedoPS.h"
#include "CompiledShaders/KillzoneGBufferGlossPS.h"
#include "CompiledShaders/KillzoneGBufferLightAccumulationPS.h"
#include "CompiledShaders/KillzoneGBufferMotionVectorPS.h"
#include "CompiledShaders/KillzoneGBufferNormalPS.h"
#include "CompiledShaders/KillzoneGBufferSpecularIntensityPS.h"
#include "CompiledShaders/KillzoneGBufferSunOcclusionPS.h"

// Clustered
#include "CompiledShaders/KillzoneGBufferLightClusteredPS.h"
#include "CompiledShaders/KillzoneGBufferLightClusteredDensityPS.h"
#include "CompiledShaders/KillzoneGBufferLightClusteredFPRPS.h"

#include "CompiledShaders/KillzoneGBufferDepthClusteredPS.h"
#include "CompiledShaders/KillzoneGBufferDiffuseAlbedoClusteredPS.h"
#include "CompiledShaders/KillzoneGBufferGlossClusteredPS.h"
#include "CompiledShaders/KillzoneGBufferLightAccumulationClusteredPS.h"
#include "CompiledShaders/KillzoneGBufferMotionVectorClusteredPS.h"
#include "CompiledShaders/KillzoneGBufferNormalClusteredPS.h"
#include "CompiledShaders/KillzoneGBufferSpecularIntensityClusteredPS.h"
#include "CompiledShaders/KillzoneGBufferSunOcclusionClusteredPS.h"
#endif

#include "CompiledShaders/ThinGBufferVS.h"
#include "CompiledShaders/ThinGBufferDeferredPS.h"

// Default
#include "CompiledShaders/ThinGBufferDefaultPS.h"
#include "CompiledShaders/ThinGBufferDefaultDensityPS.h"
#include "CompiledShaders/ThinGBufferDefaultFPRPS.h"

#include "CompiledShaders/ThinGBufferDefaultDepthPS.h"
#include "CompiledShaders/ThinGBufferDefaultDiffuseAlbedoPS.h"
#include "CompiledShaders/ThinGBufferDefaultGlossPS.h"
#include "CompiledShaders/ThinGBufferDefaultLightAccumulationPS.h"
#include "CompiledShaders/ThinGBufferDefaultNormalPS.h"
#include "CompiledShaders/ThinGBufferDefaultSpecularIntensityPS.h"

// Tiled
#include "CompiledShaders/ThinGBufferTiledPS.h"
#include "CompiledShaders/ThinGBufferTiledDensityPS.h"
#include "CompiledShaders/ThinGBufferTiledFPRPS.h"

#include "CompiledShaders/ThinGBufferTiledDepthPS.h"
#include "CompiledShaders/ThinGBufferTiledDiffuseAlbedoPS.h"
#include "CompiledShaders/ThinGBufferTiledGlossPS.h"
#include "CompiledShaders/ThinGBufferTiledLightAccumulationPS.h"
#include "CompiledShaders/ThinGBufferTiledNormalPS.h"
#include "CompiledShaders/ThinGBufferTiledSpecularIntensityPS.h"

// Clustered
#include "CompiledShaders/ThinGBufferClusteredPS.h"
#include "CompiledShaders/ThinGBufferClusteredDensityPS.h"
#include "CompiledShaders/ThinGBufferClusteredFPRPS.h"

#include "CompiledShaders/ThinGBufferClusteredDepthPS.h"
#include "CompiledShaders/ThinGBufferClusteredDiffuseAlbedoPS.h"
#include "CompiledShaders/ThinGBufferClusteredGlossPS.h"
#include "CompiledShaders/ThinGBufferClusteredLightAccumulationPS.h"
#include "CompiledShaders/ThinGBufferClusteredNormalPS.h"
#include "CompiledShaders/ThinGBufferClusteredSpecularIntensityPS.h"

using namespace Math;
using namespace Graphics;

namespace Sponza
{
    void RenderLightShadows(GraphicsContext& gfxContext, const Camera& camera);

    enum eObjectFilter { kOpaque = 0x1, kCutout = 0x2, kTransparent = 0x4, kAll = 0xF, kNone = 0x0 };
    void RenderDeferredObjects(GraphicsContext& Context, const Camera& camera, const Vector3& viewerPos);
    void RenderDeferredObjectsThinGBuffer(GraphicsContext& Context, const Camera& camera, const Vector3& viewerPos);
    void RenderDeferredClusteredObjects(GraphicsContext& Context, const Camera& camera, const Vector3& viewerPos);
    void RenderDeferredClusteredObjectsThinGBuffer(GraphicsContext& Context, const Camera& camera, const Vector3& viewerPos);
    void RenderObjects( GraphicsContext& Context, const Camera& camera, const Vector3& viewerPos, eObjectFilter Filter = kAll );
    void RenderObjectsThinGBuffer(GraphicsContext& Context, const Camera& camera, const Vector3& viewerPos, eObjectFilter Filter = kAll);
    void RenderClusteredObjects(GraphicsContext& Context, const Camera& camera, const Vector3& viewerPos, eObjectFilter Filter = kAll);
    void RenderObjects(GraphicsContext& Context, const Matrix4& ViewProjMatrix, const Vector3& viewerPos, eObjectFilter Filter = kAll);

    GraphicsPSO m_DepthPSO = { (L"Sponza: Depth PSO") };
    GraphicsPSO m_CutoutDepthPSO = { (L"Sponza: Cutout Depth PSO") };
    GraphicsPSO m_CutoutModelPSO = { (L"Sponza: Cutout Color PSO") };
    GraphicsPSO m_ShadowPSO(L"Sponza: Shadow PSO");
    GraphicsPSO m_CutoutShadowPSO(L"Sponza: Cutout Shadow PSO");

    GraphicsPSO m_GBufferPSO = { (L"Sponza: GBuffer PSO") };
    GraphicsPSO m_ThinGBufferPSO = { (L"Sponza: GBuffer PSO") };

    GraphicsPSO m_aForwardPSOs[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eForwardType::COUNT) + 1] =
    {
        // Forward
        {
            { (L"Sponza: Forward World Position PSO") },
            { (L"Sponza: Forward Diffuse Albedo PSO") },
            { (L"Sponza: Forward Glossiness PSO") },
            { (L"Sponza: Forward Normal PSO") },
            { (L"Sponza: Forward Specular Intensity PSO") },
            { (L"Sponza: Forward Light Density PSO") },
            { (L"Sponza: Forward False Positive Rate PSO") },
            { (L"Sponza: Forward Color PSO") },
        },
        // Forward+
        {
            { (L"Sponza: Tiled World Position PSO") },
            { (L"Sponza: Tiled Diffuse Albedo PSO") },
            { (L"Sponza: Tiled Glossiness PSO") },
            { (L"Sponza: Tiled Normal PSO") },
            { (L"Sponza: Tiled Specular Intensity PSO") },
            { (L"Sponza: Tiled Light Density PSO") },
            { (L"Sponza: Tiled False Positive Rate PSO") },
            { (L"Sponza: Tiled Color PSO") },
        },
        // Forward+ 2.5D Culling
        {
            { (L"Sponza: Tiled World Position PSO") },
            { (L"Sponza: Tiled Diffuse Albedo PSO") },
            { (L"Sponza: Tiled Glossiness PSO") },
            { (L"Sponza: Tiled Normal PSO") },
            { (L"Sponza: Tiled Specular Intensity PSO") },
            { (L"Sponza: Tiled Light Density PSO") },
            { (L"Sponza: Tiled False Positive Rate PSO") },
            { (L"Sponza: Tiled Color PSO") },
        },
        // Forward+ 2.5D AABB Culling
        {
            { (L"Sponza: Tiled World Position PSO") },
            { (L"Sponza: Tiled Diffuse Albedo PSO") },
            { (L"Sponza: Tiled Glossiness PSO") },
            { (L"Sponza: Tiled Normal PSO") },
            { (L"Sponza: Tiled Specular Intensity PSO") },
            { (L"Sponza: Tiled Light Density PSO") },
            { (L"Sponza: Tiled False Positive Rate PSO") },
            { (L"Sponza: Tiled Color PSO") },
        },
        // Forward+ (DICE)
        {
            { (L"Sponza: Tiled World Position PSO") },
            { (L"Sponza: Tiled Diffuse Albedo PSO") },
            { (L"Sponza: Tiled Glossiness PSO") },
            { (L"Sponza: Tiled Normal PSO") },
            { (L"Sponza: Tiled Specular Intensity PSO") },
            { (L"Sponza: Tiled Light Density PSO") },
            { (L"Sponza: Tiled False Positive Rate PSO") },
            { (L"Sponza: Tiled Color PSO") },
        },
        // Forward+ (DICE 2.5)
        {
            { (L"Sponza: Tiled World Position PSO") },
            { (L"Sponza: Tiled Diffuse Albedo PSO") },
            { (L"Sponza: Tiled Glossiness PSO") },
            { (L"Sponza: Tiled Normal PSO") },
            { (L"Sponza: Tiled Specular Intensity PSO") },
            { (L"Sponza: Tiled Light Density PSO") },
            { (L"Sponza: Tiled False Positive Rate PSO") },
            { (L"Sponza: Tiled Color PSO") },
        },
        // Forward+ (DICE 2.5 AABB)
        {
            { (L"Sponza: Tiled World Position PSO") },
            { (L"Sponza: Tiled Diffuse Albedo PSO") },
            { (L"Sponza: Tiled Glossiness PSO") },
            { (L"Sponza: Tiled Normal PSO") },
            { (L"Sponza: Tiled Specular Intensity PSO") },
            { (L"Sponza: Tiled Light Density PSO") },
            { (L"Sponza: Tiled False Positive Rate PSO") },
            { (L"Sponza: Tiled Color PSO") },
        },
        // Forward+ (INTEL)
        {
            { (L"Sponza: Tiled World Position PSO") },
            { (L"Sponza: Tiled Diffuse Albedo PSO") },
            { (L"Sponza: Tiled Glossiness PSO") },
            { (L"Sponza: Tiled Normal PSO") },
            { (L"Sponza: Tiled Specular Intensity PSO") },
            { (L"Sponza: Tiled Light Density PSO") },
            { (L"Sponza: Tiled False Positive Rate PSO") },
            { (L"Sponza: Tiled Color PSO") },
        },
        // Clustered?
        {
            { (L"Sponza: Clustered World Position PSO") },
            { (L"Sponza: Clustered Diffuse Albedo PSO") },
            { (L"Sponza: Clustered Glossiness PSO") },
            { (L"Sponza: Clustered Normal PSO") },
            { (L"Sponza: Clustered Specular Intensity PSO") },
            { (L"Sponza: Clustered Light Density PSO") },
            { (L"Sponza: Clustered False Positive Rate PSO") },
            { (L"Sponza: Clustered Color PSO") },
        },
    };

    GraphicsPSO m_aGBufferPSOs[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eGBufferDataType::COUNT) + 1] =
    {
        // DEFAULT
        {
#if SIMPLE_GBUFFER
            { (L"Sponza: GBuffer RT0 World Position PSO") },
            { (L"Sponza: GBuffer RT1 Normal PSO") },
            { (L"Sponza: GBuffer RT1 Depth PSO") },
            { (L"Sponza: GBuffer RT2 Albedo PSO") },
            { (L"Sponza: GBuffer RT3 Specular PSO") },
            { (L"Sponza: GBuffer RT3 Gloss PSO") },
            { (L"Sponza: GBuffer Light PSO") },
#elif KILLZONE_GBUFFER
            { (L"Sponza: GBuffer Depth PSO") },
            { (L"Sponza: GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: GBuffer RT0 Intensity PSO") },
            { (L"Sponza: GBuffer RT1 Normal PSO") },
            { (L"Sponza: GBuffer RT2 Motion Vector PSO") },
            { (L"Sponza: GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: GBuffer RT3 Diffuse Albedo PSO") },
            { (L"Sponza: GBuffer RT3 Sun-Occlusion PSO") },
            { (L"Sponza: GBuffer Light Density PSO") },
            { (L"Sponza: GBuffer False Positive Rate PSO") },
            { (L"Sponza: GBuffer Light PSO") },
#endif
        },
        // TILED
        {
#if SIMPLE_GBUFFER
            { (L"Sponza: Tiled GBuffer RT0 World Position PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Depth PSO") },
            { (L"Sponza: Tiled GBuffer RT2 Albedo PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Specular PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Gloss PSO") },
            { (L"Sponza: Tiled GBuffer Light PSO") },
#elif KILLZONE_GBUFFER
            { (L"Sponza: Tiled GBuffer Depth PSO") },
            { (L"Sponza: Tiled GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: Tiled GBuffer RT0 Intensity PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled GBuffer RT2 Motion Vector PSO") },
            { (L"Sponza: Tiled GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Diffuse Albedo PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Sun-Occlusion PSO") },
            { (L"Sponza: Tiled GBuffer Light Density PSO") },
            { (L"Sponza: Tiled GBuffer False Positive Rate PSO") },
            { (L"Sponza: Tiled GBuffer Light PSO") },
#endif
        },
        // TILED 2.5D Culling
        {
#if SIMPLE_GBUFFER
            { (L"Sponza: Tiled GBuffer RT0 World Position PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Depth PSO") },
            { (L"Sponza: Tiled GBuffer RT2 Albedo PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Specular PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Gloss PSO") },
            { (L"Sponza: Tiled GBuffer Light PSO") },
#elif KILLZONE_GBUFFER
            { (L"Sponza: Tiled 2.5D GBuffer Depth PSO") },
            { (L"Sponza: Tiled 2.5D GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: Tiled 2.5D GBuffer RT0 Intensity PSO") },
            { (L"Sponza: Tiled 2.5D GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled 2.5D GBuffer RT2 Motion Vector PSO") },
            { (L"Sponza: Tiled 2.5D GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: Tiled 2.5D GBuffer RT3 Diffuse Albedo PSO") },
            { (L"Sponza: Tiled 2.5D GBuffer RT3 Sun-Occlusion PSO") },
            { (L"Sponza: Tiled 2.5D GBuffer Light Density PSO") },
            { (L"Sponza: Tiled 2.5D GBuffer False Positive Rate PSO") },
            { (L"Sponza: Tiled 2.5D GBuffer Light PSO") },
#endif
        },
        // TILED 2.5D AABB Culling
        {
#if SIMPLE_GBUFFER
            { (L"Sponza: Tiled GBuffer RT0 World Position PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Depth PSO") },
            { (L"Sponza: Tiled GBuffer RT2 Albedo PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Specular PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Gloss PSO") },
            { (L"Sponza: Tiled GBuffer Light PSO") },
#elif KILLZONE_GBUFFER
            { (L"Sponza: Tiled 2.5D AABB GBuffer Depth PSO") },
            { (L"Sponza: Tiled 2.5D AABB GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: Tiled 2.5D AABB GBuffer RT0 Intensity PSO") },
            { (L"Sponza: Tiled 2.5D AABB GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled 2.5D AABB GBuffer RT2 Motion Vector PSO") },
            { (L"Sponza: Tiled 2.5D AABB GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: Tiled 2.5D AABB GBuffer RT3 Diffuse Albedo PSO") },
            { (L"Sponza: Tiled 2.5D AABB GBuffer RT3 Sun-Occlusion PSO") },
            { (L"Sponza: Tiled 2.5D AABB GBuffer Light Density PSO") },
            { (L"Sponza: Tiled 2.5D AABB GBuffer False Positive Rate PSO") },
            { (L"Sponza: Tiled 2.5D AABB GBuffer Light PSO") },
#endif
        },
        // TILED DICE
        {
#if SIMPLE_GBUFFER
            { (L"Sponza: Tiled GBuffer RT0 World Position PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Depth PSO") },
            { (L"Sponza: Tiled GBuffer RT2 Albedo PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Specular PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Gloss PSO") },
            { (L"Sponza: Tiled GBuffer Light PSO") },
#elif KILLZONE_GBUFFER
            { (L"Sponza: DICE Tiled GBuffer Depth PSO") },
            { (L"Sponza: DICE Tiled GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: DICE Tiled GBuffer RT0 Intensity PSO") },
            { (L"Sponza: DICE Tiled GBuffer RT1 Normal PSO") },
            { (L"Sponza: DICE Tiled GBuffer RT2 Motion Vector PSO") },
            { (L"Sponza: DICE Tiled GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: DICE Tiled GBuffer RT3 Diffuse Albedo PSO") },
            { (L"Sponza: DICE Tiled GBuffer RT3 Sun-Occlusion PSO") },
            { (L"Sponza: DICE Tiled GBuffer Light Density PSO") },
            { (L"Sponza: DICE Tiled GBuffer False Positive Rate PSO") },
            { (L"Sponza: DICE Tiled GBuffer Light PSO") },
#endif
        },
        // TILED DICE 2.5
        {
#if SIMPLE_GBUFFER
            { (L"Sponza: Tiled GBuffer RT0 World Position PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Depth PSO") },
            { (L"Sponza: Tiled GBuffer RT2 Albedo PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Specular PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Gloss PSO") },
            { (L"Sponza: Tiled GBuffer Light PSO") },
#elif KILLZONE_GBUFFER
            { (L"Sponza: DICE Tiled 2.5 Culling GBuffer Depth PSO") },
            { (L"Sponza: DICE Tiled 2.5 Culling GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: DICE Tiled 2.5 Culling GBuffer RT0 Intensity PSO") },
            { (L"Sponza: DICE Tiled 2.5 Culling GBuffer RT1 Normal PSO") },
            { (L"Sponza: DICE Tiled 2.5 Culling GBuffer RT2 Motion Vector PSO") },
            { (L"Sponza: DICE Tiled 2.5 Culling GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: DICE Tiled 2.5 Culling GBuffer RT3 Diffuse Albedo PSO") },
            { (L"Sponza: DICE Tiled 2.5 Culling GBuffer RT3 Sun-Occlusion PSO") },
            { (L"Sponza: DICE Tiled 2.5 Culling GBuffer Light Density PSO") },
            { (L"Sponza: DICE Tiled 2.5 Culling GBuffer False Positive Rate PSO") },
            { (L"Sponza: DICE Tiled 2.5 Culling GBuffer Light PSO") },
#endif
        },
        // TILED DICE 2.5 AABB
        {
#if SIMPLE_GBUFFER
            { (L"Sponza: Tiled GBuffer RT0 World Position PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Depth PSO") },
            { (L"Sponza: Tiled GBuffer RT2 Albedo PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Specular PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Gloss PSO") },
            { (L"Sponza: Tiled GBuffer Light PSO") },
#elif KILLZONE_GBUFFER
            { (L"Sponza: DICE Tiled 2.5 AABB Culling GBuffer Depth PSO") },
            { (L"Sponza: DICE Tiled 2.5 AABB Culling GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: DICE Tiled 2.5 AABB Culling GBuffer RT0 Intensity PSO") },
            { (L"Sponza: DICE Tiled 2.5 AABB Culling GBuffer RT1 Normal PSO") },
            { (L"Sponza: DICE Tiled 2.5 AABB Culling GBuffer RT2 Motion Vector PSO") },
            { (L"Sponza: DICE Tiled 2.5 AABB Culling GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: DICE Tiled 2.5 AABB Culling GBuffer RT3 Diffuse Albedo PSO") },
            { (L"Sponza: DICE Tiled 2.5 AABB Culling GBuffer RT3 Sun-Occlusion PSO") },
            { (L"Sponza: DICE Tiled 2.5 AABB Culling GBuffer Light Density PSO") },
            { (L"Sponza: DICE Tiled 2.5 AABB Culling GBuffer False Positive Rate PSO") },
            { (L"Sponza: DICE Tiled 2.5 AABB Culling GBuffer Light PSO") },
#endif
        },
        // TILED INTEL
        {
#if SIMPLE_GBUFFER
            { (L"Sponza: Tiled GBuffer RT0 World Position PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Depth PSO") },
            { (L"Sponza: Tiled GBuffer RT2 Albedo PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Specular PSO") },
            { (L"Sponza: Tiled GBuffer RT3 Gloss PSO") },
            { (L"Sponza: Tiled GBuffer Light PSO") },
#elif KILLZONE_GBUFFER
            { (L"Sponza: INTEL Tiled GBuffer Depth PSO") },
            { (L"Sponza: INTEL Tiled GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: INTEL Tiled GBuffer RT0 Intensity PSO") },
            { (L"Sponza: INTEL Tiled GBuffer RT1 Normal PSO") },
            { (L"Sponza: INTEL Tiled GBuffer RT2 Motion Vector PSO") },
            { (L"Sponza: INTEL Tiled GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: INTEL Tiled GBuffer RT3 Diffuse Albedo PSO") },
            { (L"Sponza: INTEL Tiled GBuffer RT3 Sun-Occlusion PSO") },
            { (L"Sponza: INTEL Tiled GBuffer Light Density PSO") },
            { (L"Sponza: INTEL Tiled GBuffer False Positive Rate PSO") },
            { (L"Sponza: INTEL Tiled GBuffer Light PSO") },
#endif
        },
        // CLUSTERED
        {
#if SIMPLE_GBUFFER
            { (L"Sponza: Clustered GBuffer RT0 World Position PSO") },
            { (L"Sponza: Clustered GBuffer RT1 Normal PSO") },
            { (L"Sponza: Clustered GBuffer RT1 Depth PSO") },
            { (L"Sponza: Clustered GBuffer RT2 Albedo PSO") },
            { (L"Sponza: Clustered GBuffer RT3 Specular PSO") },
            { (L"Sponza: Clustered GBuffer RT3 Gloss PSO") },
            { (L"Sponza: Clustered GBuffer Light PSO") },
#elif KILLZONE_GBUFFER
            { (L"Sponza: Clustered GBuffer Depth PSO") },
            { (L"Sponza: Clustered GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: Clustered GBuffer RT0 Intensity PSO") },
            { (L"Sponza: Clustered GBuffer RT1 Normal PSO") },
            { (L"Sponza: Clustered GBuffer RT2 Motion Vector PSO") },
            { (L"Sponza: Clustered GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: Clustered GBuffer RT3 Diffuse Albedo PSO") },
            { (L"Sponza: Clustered GBuffer RT3 Sun-Occlusion PSO") },
            { (L"Sponza: Clustered GBuffer Light Density PSO") },
            { (L"Sponza: Clustered GBuffer False Positive Rate PSO") },
            { (L"Sponza: Clustered GBuffer Light PSO") },
#endif
        },
    };

    GraphicsPSO m_aThinGBufferPSOs[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eThinGBufferDataType::COUNT) + 1] =
    {
        // DEFAULT
        {
            { (L"Sponza: GBuffer Depth PSO") },
            { (L"Sponza: GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: GBuffer RT1 Normal PSO") },
            { (L"Sponza: GBuffer RT1 Glossiness PSO") },
            { (L"Sponza: GBuffer RT2 Diffuse Albedo PSO") },
            { (L"Sponza: GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: GBuffer Light Density PSO") },
            { (L"Sponza: GBuffer False Positive Rate PSO") },
            { (L"Sponza: GBuffer Light PSO") },
        },
        // TILED
        {
            { (L"Sponza: Tiled GBuffer Depth PSO") },
            { (L"Sponza: Tiled GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled GBuffer RT1 Glossiness PSO") },
            { (L"Sponza: Tiled GBuffer RT2 Diffuse Albedo PSO") },
            { (L"Sponza: Tiled GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: Tiled GBuffer Light Density PSO") },
            { (L"Sponza: Tiled GBuffer False Positive Rate PSO") },
            { (L"Sponza: Tiled GBuffer Light PSO") },
        },
        // TILED 2.5D Culling
        {
            { (L"Sponza: Tiled 2.5D Culling GBuffer Depth PSO") },
            { (L"Sponza: Tiled 2.5D Culling GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: Tiled 2.5D Culling GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled 2.5D Culling GBuffer RT1 Glossiness PSO") },
            { (L"Sponza: Tiled 2.5D Culling GBuffer RT2 Diffuse Albedo PSO") },
            { (L"Sponza: Tiled 2.5D Culling GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: Tiled 2.5D Culling GBuffer Light Density PSO") },
            { (L"Sponza: Tiled 2.5D Culling GBuffer False Positive Rate PSO") },
            { (L"Sponza: Tiled 2.5D Culling GBuffer Light PSO") },
        },
        // TILED 2.5D AABB Culling
        {
            { (L"Sponza: Tiled 2.5D AABB Culling GBuffer Depth PSO") },
            { (L"Sponza: Tiled 2.5D AABB Culling GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: Tiled 2.5D AABB Culling GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled 2.5D AABB Culling GBuffer RT1 Glossiness PSO") },
            { (L"Sponza: Tiled 2.5D AABB Culling GBuffer RT2 Diffuse Albedo PSO") },
            { (L"Sponza: Tiled 2.5D AABB Culling GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: Tiled 2.5D AABB Culling GBuffer Light Density PSO") },
            { (L"Sponza: Tiled 2.5D AABB Culling GBuffer False Positive Rate PSO") },
            { (L"Sponza: Tiled 2.5D AABB Culling GBuffer Light PSO") },
        },
        // TILED DICE
        {
            { (L"Sponza: Tiled (DICE) Culling GBuffer Depth PSO") },
            { (L"Sponza: Tiled (DICE) Culling GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: Tiled (DICE) Culling GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled (DICE) Culling GBuffer RT1 Glossiness PSO") },
            { (L"Sponza: Tiled (DICE) Culling GBuffer RT2 Diffuse Albedo PSO") },
            { (L"Sponza: Tiled (DICE) Culling GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: Tiled (DICE) Culling GBuffer Light Density PSO") },
            { (L"Sponza: Tiled (DICE) Culling GBuffer False Positive Rate PSO") },
            { (L"Sponza: Tiled (DICE) Culling GBuffer Light PSO") },
        },
        // TILED DICE 2.5
        {
            { (L"Sponza: Tiled (DICE) 2.5D Culling GBuffer Depth PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D Culling GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D Culling GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D Culling GBuffer RT1 Glossiness PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D Culling GBuffer RT2 Diffuse Albedo PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D Culling GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D Culling GBuffer Light Density PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D Culling GBuffer False Positive Rate PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D Culling GBuffer Light PSO") },
        },
        // TILED DICE 2.5 AABB
        {
            { (L"Sponza: Tiled (DICE) 2.5D AABB Culling GBuffer Depth PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D AABB Culling GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D AABB Culling GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D AABB Culling GBuffer RT1 Glossiness PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D AABB Culling GBuffer RT2 Diffuse Albedo PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D AABB Culling GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D AABB Culling GBuffer Light Density PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D AABB Culling GBuffer False Positive Rate PSO") },
            { (L"Sponza: Tiled (DICE) 2.5D AABB Culling GBuffer Light PSO") },
        },
        // TILED INTEL
        {
            { (L"Sponza: Tiled (Intel) GBuffer Depth PSO") },
            { (L"Sponza: Tiled (Intel) GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: Tiled (Intel) GBuffer RT1 Normal PSO") },
            { (L"Sponza: Tiled (Intel) GBuffer RT1 Glossiness PSO") },
            { (L"Sponza: Tiled (Intel) GBuffer RT2 Diffuse Albedo PSO") },
            { (L"Sponza: Tiled (Intel) GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: Tiled (Intel) GBuffer Light Density PSO") },
            { (L"Sponza: Tiled (Intel) GBuffer False Positive Rate PSO") },
            { (L"Sponza: Tiled (Intel) GBuffer Light PSO") },
        },
        // CLUSTERED
        {
            { (L"Sponza: Clustered GBuffer Depth PSO") },
            { (L"Sponza: Clustered GBuffer RT0 Lighting Accumulation PSO") },
            { (L"Sponza: Clustered GBuffer RT1 Normal PSO") },
            { (L"Sponza: Clustered GBuffer RT1 Glossiness PSO") },
            { (L"Sponza: Clustered GBuffer RT2 Diffuse Albedo PSO") },
            { (L"Sponza: Clustered GBuffer RT2 Specular Intensity PSO") },
            { (L"Sponza: Clustered GBuffer Light Density PSO") },
            { (L"Sponza: Clustered GBuffer False Positive Rate PSO") },
            { (L"Sponza: Clustered GBuffer Light PSO") },
        },
    };

    std::pair<const unsigned char* const, size_t> m_aForwardPixelShaders[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eForwardType::COUNT) + 1] =
    {
        // Forward
        {
            { g_pModelViewerForwardWorldPosPS,          sizeof(g_pModelViewerForwardWorldPosPS)             },
            { g_pModelViewerForwardDiffuseAlbedoPS,     sizeof(g_pModelViewerForwardDiffuseAlbedoPS)        },
            { g_pModelViewerForwardGlossPS,             sizeof(g_pModelViewerForwardGlossPS)                },
            { g_pModelViewerForwardNormalPS,            sizeof(g_pModelViewerForwardNormalPS)               },
            { g_pModelViewerForwardSpecularIntensityPS, sizeof(g_pModelViewerForwardSpecularIntensityPS)    },
            { g_pModelViewerForwardDensityPS,           sizeof(g_pModelViewerForwardDensityPS)              },
            { g_pModelViewerForwardFPRPS,               sizeof(g_pModelViewerForwardFPRPS)                  },
            { g_pModelViewerForwardPS,                  sizeof(g_pModelViewerForwardPS)                     },
        },
        // Forward+
        {
            { g_pModelViewerWorldPosPS,            sizeof(g_pModelViewerWorldPosPS)            },
            { g_pModelViewerDiffuseAlbedoPS,       sizeof(g_pModelViewerDiffuseAlbedoPS)       },
            { g_pModelViewerGlossPS,               sizeof(g_pModelViewerGlossPS)               },
            { g_pModelViewerNormalPS,              sizeof(g_pModelViewerNormalPS)              },
            { g_pModelViewerSpecularIntensityPS,   sizeof(g_pModelViewerSpecularIntensityPS)   },
            { g_pModelViewerTiledDensityPS,        sizeof(g_pModelViewerTiledDensityPS)        },
            { g_pModelViewerTiledFPRPS,            sizeof(g_pModelViewerTiledFPRPS)            },
            { g_pModelViewerTiledPS,               sizeof(g_pModelViewerTiledPS)               },
        },
        // Forward+ 2.5D Culling
        {
            { g_pModelViewerWorldPosPS,            sizeof(g_pModelViewerWorldPosPS)            },
            { g_pModelViewerDiffuseAlbedoPS,       sizeof(g_pModelViewerDiffuseAlbedoPS)       },
            { g_pModelViewerGlossPS,               sizeof(g_pModelViewerGlossPS)               },
            { g_pModelViewerNormalPS,              sizeof(g_pModelViewerNormalPS)              },
            { g_pModelViewerSpecularIntensityPS,   sizeof(g_pModelViewerSpecularIntensityPS)   },
            { g_pModelViewerTiledDensityPS,        sizeof(g_pModelViewerTiledDensityPS)        },
            { g_pModelViewerTiledFPRPS,            sizeof(g_pModelViewerTiledFPRPS)            },
            { g_pModelViewerTiledPS,               sizeof(g_pModelViewerTiledPS)               },
        },
        // Forward+ 2.5D AABB Culling
        {
            { g_pModelViewerWorldPosPS,            sizeof(g_pModelViewerWorldPosPS)            },
            { g_pModelViewerDiffuseAlbedoPS,       sizeof(g_pModelViewerDiffuseAlbedoPS)       },
            { g_pModelViewerGlossPS,               sizeof(g_pModelViewerGlossPS)               },
            { g_pModelViewerNormalPS,              sizeof(g_pModelViewerNormalPS)              },
            { g_pModelViewerSpecularIntensityPS,   sizeof(g_pModelViewerSpecularIntensityPS)   },
            { g_pModelViewerTiledDensityPS,        sizeof(g_pModelViewerTiledDensityPS)        },
            { g_pModelViewerTiledFPRPS,            sizeof(g_pModelViewerTiledFPRPS)            },
            { g_pModelViewerTiledPS,               sizeof(g_pModelViewerTiledPS)               },
        },
        // Tiled DICE
        {
            { g_pModelViewerWorldPosPS,            sizeof(g_pModelViewerWorldPosPS)            },
            { g_pModelViewerDiffuseAlbedoPS,       sizeof(g_pModelViewerDiffuseAlbedoPS)       },
            { g_pModelViewerGlossPS,               sizeof(g_pModelViewerGlossPS)               },
            { g_pModelViewerNormalPS,              sizeof(g_pModelViewerNormalPS)              },
            { g_pModelViewerSpecularIntensityPS,   sizeof(g_pModelViewerSpecularIntensityPS)   },
            { g_pModelViewerTiledDensityPS,        sizeof(g_pModelViewerTiledDensityPS)        },
            { g_pModelViewerTiledFPRPS,            sizeof(g_pModelViewerTiledFPRPS)            },
            { g_pModelViewerTiledPS,               sizeof(g_pModelViewerTiledPS)               },
        },
        // Tiled DICE 2.5
        {
            { g_pModelViewerWorldPosPS,            sizeof(g_pModelViewerWorldPosPS)            },
            { g_pModelViewerDiffuseAlbedoPS,       sizeof(g_pModelViewerDiffuseAlbedoPS)       },
            { g_pModelViewerGlossPS,               sizeof(g_pModelViewerGlossPS)               },
            { g_pModelViewerNormalPS,              sizeof(g_pModelViewerNormalPS)              },
            { g_pModelViewerSpecularIntensityPS,   sizeof(g_pModelViewerSpecularIntensityPS)   },
            { g_pModelViewerTiledDensityPS,        sizeof(g_pModelViewerTiledDensityPS)        },
            { g_pModelViewerTiledFPRPS,            sizeof(g_pModelViewerTiledFPRPS)            },
            { g_pModelViewerTiledPS,               sizeof(g_pModelViewerTiledPS)               },
        },
        // Tiled DICE 2.5 AABB
        {
            { g_pModelViewerWorldPosPS,            sizeof(g_pModelViewerWorldPosPS)            },
            { g_pModelViewerDiffuseAlbedoPS,       sizeof(g_pModelViewerDiffuseAlbedoPS)       },
            { g_pModelViewerGlossPS,               sizeof(g_pModelViewerGlossPS)               },
            { g_pModelViewerNormalPS,              sizeof(g_pModelViewerNormalPS)              },
            { g_pModelViewerSpecularIntensityPS,   sizeof(g_pModelViewerSpecularIntensityPS)   },
            { g_pModelViewerTiledDensityPS,        sizeof(g_pModelViewerTiledDensityPS)        },
            { g_pModelViewerTiledFPRPS,            sizeof(g_pModelViewerTiledFPRPS)            },
            { g_pModelViewerTiledPS,               sizeof(g_pModelViewerTiledPS)               },
        },
        // Tiled INTEL
        {
            { g_pModelViewerWorldPosPS,            sizeof(g_pModelViewerWorldPosPS)            },
            { g_pModelViewerDiffuseAlbedoPS,       sizeof(g_pModelViewerDiffuseAlbedoPS)       },
            { g_pModelViewerGlossPS,               sizeof(g_pModelViewerGlossPS)               },
            { g_pModelViewerNormalPS,              sizeof(g_pModelViewerNormalPS)              },
            { g_pModelViewerSpecularIntensityPS,   sizeof(g_pModelViewerSpecularIntensityPS)   },
            { g_pModelViewerTiledDensityPS,        sizeof(g_pModelViewerTiledDensityPS)        },
            { g_pModelViewerTiledFPRPS,            sizeof(g_pModelViewerTiledFPRPS)            },
            { g_pModelViewerTiledPS,               sizeof(g_pModelViewerTiledPS)               },
        },
        // Clustered
        {
            { g_pModelViewerClusteredWorldPosPS,            sizeof(g_pModelViewerClusteredWorldPosPS)           },
            { g_pModelViewerClusteredDiffuseAlbedoPS,       sizeof(g_pModelViewerClusteredDiffuseAlbedoPS)      },
            { g_pModelViewerClusteredGlossPS,               sizeof(g_pModelViewerClusteredGlossPS)              },
            { g_pModelViewerClusteredNormalPS,              sizeof(g_pModelViewerClusteredNormalPS)             },
            { g_pModelViewerClusteredSpecularIntensityPS,   sizeof(g_pModelViewerClusteredSpecularIntensityPS)  },
            { g_pModelViewerClusteredDensityPS,             sizeof(g_pModelViewerClusteredDensityPS)            },
            { g_pModelViewerClusteredFPRPS,                 sizeof(g_pModelViewerClusteredFPRPS)                },
            { g_pModelViewerClusteredPS,                    sizeof(g_pModelViewerClusteredPS)                   },
        },
    };

    std::pair<const unsigned char* const, size_t> m_aGBufferPixelShaders[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eGBufferDataType::COUNT) + 1] =
    {
        // Default
        {
 #if SIMPLE_GBUFFER
            { g_pGBufferWorldPosPS, sizeof(g_pGBufferWorldPosPS) },
            { g_pGBufferNormalPS,   sizeof(g_pGBufferNormalPS)   },
            { g_pGBufferDepthPS,    sizeof(g_pGBufferDepthPS)    },
            { g_pGBufferAlbedoPS,   sizeof(g_pGBufferAlbedoPS)   },
            { g_pGBufferSpecularPS, sizeof(g_pGBufferSpecularPS) },
            { g_pGBufferGlossPS,    sizeof(g_pGBufferGlossPS)    },
            { g_pGBufferLightPS,    sizeof(g_pGBufferLightPS)    },
#elif KILLZONE_GBUFFER
            { g_pKillzoneGBufferDefaultDepthPS,               sizeof(g_pKillzoneGBufferDefaultDepthPS)              },
            { g_pKillzoneGBufferDefaultLightAccumulationPS,   sizeof(g_pKillzoneGBufferDefaultLightAccumulationPS)  },
            { g_pKillzoneGBufferDefaultGlossPS,               sizeof(g_pKillzoneGBufferDefaultGlossPS)              },
            { g_pKillzoneGBufferDefaultNormalPS,              sizeof(g_pKillzoneGBufferDefaultNormalPS)             },
            { g_pKillzoneGBufferDefaultMotionVectorPS,        sizeof(g_pKillzoneGBufferDefaultMotionVectorPS)       },
            { g_pKillzoneGBufferDefaultSpecularIntensityPS,   sizeof(g_pKillzoneGBufferDefaultSpecularIntensityPS)  },
            { g_pKillzoneGBufferDefaultDiffuseAlbedoPS,       sizeof(g_pKillzoneGBufferDefaultDiffuseAlbedoPS)      },
            { g_pKillzoneGBufferDefaultSunOcclusionPS,        sizeof(g_pKillzoneGBufferDefaultSunOcclusionPS)       },
            { g_pKillzoneGBufferDefaultLightDensityPS,        sizeof(g_pKillzoneGBufferDefaultLightDensityPS)  },
            { g_pKillzoneGBufferDefaultLightFPRPS,            sizeof(g_pKillzoneGBufferDefaultLightFPRPS)      },
            { g_pKillzoneGBufferDefaultLightPS,               sizeof(g_pKillzoneGBufferDefaultLightPS)         },
#endif
        },
        // Tiled
        {
 #if SIMPLE_GBUFFER
            { g_pGBufferWorldPosPS, sizeof(g_pGBufferWorldPosPS) },
            { g_pGBufferNormalPS,   sizeof(g_pGBufferNormalPS)   },
            { g_pGBufferDepthPS,    sizeof(g_pGBufferDepthPS)    },
            { g_pGBufferAlbedoPS,   sizeof(g_pGBufferAlbedoPS)   },
            { g_pGBufferSpecularPS, sizeof(g_pGBufferSpecularPS) },
            { g_pGBufferGlossPS,    sizeof(g_pGBufferGlossPS)    },
            { g_pGBufferLightPS,    sizeof(g_pGBufferLightPS)    },
#elif KILLZONE_GBUFFER
            { g_pKillzoneGBufferDepthPS,               sizeof(g_pKillzoneGBufferDepthPS)              },
            { g_pKillzoneGBufferLightAccumulationPS,   sizeof(g_pKillzoneGBufferLightAccumulationPS)  },
            { g_pKillzoneGBufferGlossPS,               sizeof(g_pKillzoneGBufferGlossPS)              },
            { g_pKillzoneGBufferNormalPS,              sizeof(g_pKillzoneGBufferNormalPS)             },
            { g_pKillzoneGBufferMotionVectorPS,        sizeof(g_pKillzoneGBufferMotionVectorPS)       },
            { g_pKillzoneGBufferSpecularIntensityPS,   sizeof(g_pKillzoneGBufferSpecularIntensityPS)  },
            { g_pKillzoneGBufferDiffuseAlbedoPS,       sizeof(g_pKillzoneGBufferDiffuseAlbedoPS)      },
            { g_pKillzoneGBufferSunOcclusionPS,        sizeof(g_pKillzoneGBufferSunOcclusionPS)       },
            { g_pKillzoneGBufferLightTiledDensityPS,   sizeof(g_pKillzoneGBufferLightTiledDensityPS)  },
            { g_pKillzoneGBufferLightTiledFPRPS,       sizeof(g_pKillzoneGBufferLightTiledFPRPS)      },
            { g_pKillzoneGBufferLightTiledPS,          sizeof(g_pKillzoneGBufferLightTiledPS)         },
#endif
        },
        // Tiled 2.5D Culling
        {
 #if SIMPLE_GBUFFER
            { g_pGBufferWorldPosPS, sizeof(g_pGBufferWorldPosPS) },
            { g_pGBufferNormalPS,   sizeof(g_pGBufferNormalPS)   },
            { g_pGBufferDepthPS,    sizeof(g_pGBufferDepthPS)    },
            { g_pGBufferAlbedoPS,   sizeof(g_pGBufferAlbedoPS)   },
            { g_pGBufferSpecularPS, sizeof(g_pGBufferSpecularPS) },
            { g_pGBufferGlossPS,    sizeof(g_pGBufferGlossPS)    },
            { g_pGBufferLightPS,    sizeof(g_pGBufferLightPS)    },
#elif KILLZONE_GBUFFER
            { g_pKillzoneGBufferDepthPS,               sizeof(g_pKillzoneGBufferDepthPS)              },
            { g_pKillzoneGBufferLightAccumulationPS,   sizeof(g_pKillzoneGBufferLightAccumulationPS)  },
            { g_pKillzoneGBufferGlossPS,               sizeof(g_pKillzoneGBufferGlossPS)              },
            { g_pKillzoneGBufferNormalPS,              sizeof(g_pKillzoneGBufferNormalPS)             },
            { g_pKillzoneGBufferMotionVectorPS,        sizeof(g_pKillzoneGBufferMotionVectorPS)       },
            { g_pKillzoneGBufferSpecularIntensityPS,   sizeof(g_pKillzoneGBufferSpecularIntensityPS)  },
            { g_pKillzoneGBufferDiffuseAlbedoPS,       sizeof(g_pKillzoneGBufferDiffuseAlbedoPS)      },
            { g_pKillzoneGBufferSunOcclusionPS,        sizeof(g_pKillzoneGBufferSunOcclusionPS)       },
            { g_pKillzoneGBufferLightTiledDensityPS,   sizeof(g_pKillzoneGBufferLightTiledDensityPS)  },
            { g_pKillzoneGBufferLightTiledFPRPS,       sizeof(g_pKillzoneGBufferLightTiledFPRPS)      },
            { g_pKillzoneGBufferLightTiledPS,          sizeof(g_pKillzoneGBufferLightTiledPS)         },
#endif
        },
        // Tiled 2.5D AABB Culling
        {
 #if SIMPLE_GBUFFER
            { g_pGBufferWorldPosPS, sizeof(g_pGBufferWorldPosPS) },
            { g_pGBufferNormalPS,   sizeof(g_pGBufferNormalPS)   },
            { g_pGBufferDepthPS,    sizeof(g_pGBufferDepthPS)    },
            { g_pGBufferAlbedoPS,   sizeof(g_pGBufferAlbedoPS)   },
            { g_pGBufferSpecularPS, sizeof(g_pGBufferSpecularPS) },
            { g_pGBufferGlossPS,    sizeof(g_pGBufferGlossPS)    },
            { g_pGBufferLightPS,    sizeof(g_pGBufferLightPS)    },
#elif KILLZONE_GBUFFER
            { g_pKillzoneGBufferDepthPS,               sizeof(g_pKillzoneGBufferDepthPS)              },
            { g_pKillzoneGBufferLightAccumulationPS,   sizeof(g_pKillzoneGBufferLightAccumulationPS)  },
            { g_pKillzoneGBufferGlossPS,               sizeof(g_pKillzoneGBufferGlossPS)              },
            { g_pKillzoneGBufferNormalPS,              sizeof(g_pKillzoneGBufferNormalPS)             },
            { g_pKillzoneGBufferMotionVectorPS,        sizeof(g_pKillzoneGBufferMotionVectorPS)       },
            { g_pKillzoneGBufferSpecularIntensityPS,   sizeof(g_pKillzoneGBufferSpecularIntensityPS)  },
            { g_pKillzoneGBufferDiffuseAlbedoPS,       sizeof(g_pKillzoneGBufferDiffuseAlbedoPS)      },
            { g_pKillzoneGBufferSunOcclusionPS,        sizeof(g_pKillzoneGBufferSunOcclusionPS)       },
            { g_pKillzoneGBufferLightTiledDensityPS,   sizeof(g_pKillzoneGBufferLightTiledDensityPS)  },
            { g_pKillzoneGBufferLightTiledFPRPS,       sizeof(g_pKillzoneGBufferLightTiledFPRPS)      },
            { g_pKillzoneGBufferLightTiledPS,          sizeof(g_pKillzoneGBufferLightTiledPS)         },
#endif
        },
        // Tiled DICE
        {
 #if SIMPLE_GBUFFER
            { g_pGBufferWorldPosPS, sizeof(g_pGBufferWorldPosPS) },
            { g_pGBufferNormalPS,   sizeof(g_pGBufferNormalPS)   },
            { g_pGBufferDepthPS,    sizeof(g_pGBufferDepthPS)    },
            { g_pGBufferAlbedoPS,   sizeof(g_pGBufferAlbedoPS)   },
            { g_pGBufferSpecularPS, sizeof(g_pGBufferSpecularPS) },
            { g_pGBufferGlossPS,    sizeof(g_pGBufferGlossPS)    },
            { g_pGBufferLightPS,    sizeof(g_pGBufferLightPS)    },
#elif KILLZONE_GBUFFER
            { g_pKillzoneGBufferDepthPS,               sizeof(g_pKillzoneGBufferDepthPS)              },
            { g_pKillzoneGBufferLightAccumulationPS,   sizeof(g_pKillzoneGBufferLightAccumulationPS)  },
            { g_pKillzoneGBufferGlossPS,               sizeof(g_pKillzoneGBufferGlossPS)              },
            { g_pKillzoneGBufferNormalPS,              sizeof(g_pKillzoneGBufferNormalPS)             },
            { g_pKillzoneGBufferMotionVectorPS,        sizeof(g_pKillzoneGBufferMotionVectorPS)       },
            { g_pKillzoneGBufferSpecularIntensityPS,   sizeof(g_pKillzoneGBufferSpecularIntensityPS)  },
            { g_pKillzoneGBufferDiffuseAlbedoPS,       sizeof(g_pKillzoneGBufferDiffuseAlbedoPS)      },
            { g_pKillzoneGBufferSunOcclusionPS,        sizeof(g_pKillzoneGBufferSunOcclusionPS)       },
            { g_pKillzoneGBufferLightTiledDensityPS,   sizeof(g_pKillzoneGBufferLightTiledDensityPS)  },
            { g_pKillzoneGBufferLightTiledFPRPS,       sizeof(g_pKillzoneGBufferLightTiledFPRPS)      },
            { g_pKillzoneGBufferLightTiledPS,          sizeof(g_pKillzoneGBufferLightTiledPS)         },
#endif
        },
        // Tiled DICE 2.5
        {
 #if SIMPLE_GBUFFER
            { g_pGBufferWorldPosPS, sizeof(g_pGBufferWorldPosPS) },
            { g_pGBufferNormalPS,   sizeof(g_pGBufferNormalPS)   },
            { g_pGBufferDepthPS,    sizeof(g_pGBufferDepthPS)    },
            { g_pGBufferAlbedoPS,   sizeof(g_pGBufferAlbedoPS)   },
            { g_pGBufferSpecularPS, sizeof(g_pGBufferSpecularPS) },
            { g_pGBufferGlossPS,    sizeof(g_pGBufferGlossPS)    },
            { g_pGBufferLightPS,    sizeof(g_pGBufferLightPS)    },
#elif KILLZONE_GBUFFER
            { g_pKillzoneGBufferDepthPS,               sizeof(g_pKillzoneGBufferDepthPS)              },
            { g_pKillzoneGBufferLightAccumulationPS,   sizeof(g_pKillzoneGBufferLightAccumulationPS)  },
            { g_pKillzoneGBufferGlossPS,               sizeof(g_pKillzoneGBufferGlossPS)              },
            { g_pKillzoneGBufferNormalPS,              sizeof(g_pKillzoneGBufferNormalPS)             },
            { g_pKillzoneGBufferMotionVectorPS,        sizeof(g_pKillzoneGBufferMotionVectorPS)       },
            { g_pKillzoneGBufferSpecularIntensityPS,   sizeof(g_pKillzoneGBufferSpecularIntensityPS)  },
            { g_pKillzoneGBufferDiffuseAlbedoPS,       sizeof(g_pKillzoneGBufferDiffuseAlbedoPS)      },
            { g_pKillzoneGBufferSunOcclusionPS,        sizeof(g_pKillzoneGBufferSunOcclusionPS)       },
            { g_pKillzoneGBufferLightTiledDensityPS,   sizeof(g_pKillzoneGBufferLightTiledDensityPS)  },
            { g_pKillzoneGBufferLightTiledFPRPS,       sizeof(g_pKillzoneGBufferLightTiledFPRPS)      },
            { g_pKillzoneGBufferLightTiledPS,          sizeof(g_pKillzoneGBufferLightTiledPS)         },
#endif
        },
        // Tiled DICE 2.5 AABB
        {
 #if SIMPLE_GBUFFER
            { g_pGBufferWorldPosPS, sizeof(g_pGBufferWorldPosPS) },
            { g_pGBufferNormalPS,   sizeof(g_pGBufferNormalPS)   },
            { g_pGBufferDepthPS,    sizeof(g_pGBufferDepthPS)    },
            { g_pGBufferAlbedoPS,   sizeof(g_pGBufferAlbedoPS)   },
            { g_pGBufferSpecularPS, sizeof(g_pGBufferSpecularPS) },
            { g_pGBufferGlossPS,    sizeof(g_pGBufferGlossPS)    },
            { g_pGBufferLightPS,    sizeof(g_pGBufferLightPS)    },
#elif KILLZONE_GBUFFER
            { g_pKillzoneGBufferDepthPS,               sizeof(g_pKillzoneGBufferDepthPS)              },
            { g_pKillzoneGBufferLightAccumulationPS,   sizeof(g_pKillzoneGBufferLightAccumulationPS)  },
            { g_pKillzoneGBufferGlossPS,               sizeof(g_pKillzoneGBufferGlossPS)              },
            { g_pKillzoneGBufferNormalPS,              sizeof(g_pKillzoneGBufferNormalPS)             },
            { g_pKillzoneGBufferMotionVectorPS,        sizeof(g_pKillzoneGBufferMotionVectorPS)       },
            { g_pKillzoneGBufferSpecularIntensityPS,   sizeof(g_pKillzoneGBufferSpecularIntensityPS)  },
            { g_pKillzoneGBufferDiffuseAlbedoPS,       sizeof(g_pKillzoneGBufferDiffuseAlbedoPS)      },
            { g_pKillzoneGBufferSunOcclusionPS,        sizeof(g_pKillzoneGBufferSunOcclusionPS)       },
            { g_pKillzoneGBufferLightTiledDensityPS,   sizeof(g_pKillzoneGBufferLightTiledDensityPS)  },
            { g_pKillzoneGBufferLightTiledFPRPS,       sizeof(g_pKillzoneGBufferLightTiledFPRPS)      },
            { g_pKillzoneGBufferLightTiledPS,          sizeof(g_pKillzoneGBufferLightTiledPS)         },
#endif
        },
        // Tiled INTEL
        {
 #if SIMPLE_GBUFFER
            { g_pGBufferWorldPosPS, sizeof(g_pGBufferWorldPosPS) },
            { g_pGBufferNormalPS,   sizeof(g_pGBufferNormalPS)   },
            { g_pGBufferDepthPS,    sizeof(g_pGBufferDepthPS)    },
            { g_pGBufferAlbedoPS,   sizeof(g_pGBufferAlbedoPS)   },
            { g_pGBufferSpecularPS, sizeof(g_pGBufferSpecularPS) },
            { g_pGBufferGlossPS,    sizeof(g_pGBufferGlossPS)    },
            { g_pGBufferLightPS,    sizeof(g_pGBufferLightPS)    },
#elif KILLZONE_GBUFFER
            { g_pKillzoneGBufferDepthPS,               sizeof(g_pKillzoneGBufferDepthPS)              },
            { g_pKillzoneGBufferLightAccumulationPS,   sizeof(g_pKillzoneGBufferLightAccumulationPS)  },
            { g_pKillzoneGBufferGlossPS,               sizeof(g_pKillzoneGBufferGlossPS)              },
            { g_pKillzoneGBufferNormalPS,              sizeof(g_pKillzoneGBufferNormalPS)             },
            { g_pKillzoneGBufferMotionVectorPS,        sizeof(g_pKillzoneGBufferMotionVectorPS)       },
            { g_pKillzoneGBufferSpecularIntensityPS,   sizeof(g_pKillzoneGBufferSpecularIntensityPS)  },
            { g_pKillzoneGBufferDiffuseAlbedoPS,       sizeof(g_pKillzoneGBufferDiffuseAlbedoPS)      },
            { g_pKillzoneGBufferSunOcclusionPS,        sizeof(g_pKillzoneGBufferSunOcclusionPS)       },
            { g_pKillzoneGBufferLightTiledDensityPS,   sizeof(g_pKillzoneGBufferLightTiledDensityPS)  },
            { g_pKillzoneGBufferLightTiledFPRPS,       sizeof(g_pKillzoneGBufferLightTiledFPRPS)      },
            { g_pKillzoneGBufferLightTiledPS,          sizeof(g_pKillzoneGBufferLightTiledPS)         },
#endif
        },
        // Clustered
        {
 #if SIMPLE_GBUFFER
            { g_pGBufferWorldPosPS, sizeof(g_pGBufferWorldPosPS) },
            { g_pGBufferNormalPS,   sizeof(g_pGBufferNormalPS)   },
            { g_pGBufferDepthPS,    sizeof(g_pGBufferDepthPS)    },
            { g_pGBufferAlbedoPS,   sizeof(g_pGBufferAlbedoPS)   },
            { g_pGBufferSpecularPS, sizeof(g_pGBufferSpecularPS) },
            { g_pGBufferGlossPS,    sizeof(g_pGBufferGlossPS)    },
            { g_pGBufferLightPS,    sizeof(g_pGBufferLightPS)    },
#elif KILLZONE_GBUFFER
            { g_pKillzoneGBufferDepthClusteredPS,               sizeof(g_pKillzoneGBufferDepthClusteredPS)              },
            { g_pKillzoneGBufferLightAccumulationClusteredPS,   sizeof(g_pKillzoneGBufferLightAccumulationClusteredPS)  },
            { g_pKillzoneGBufferGlossClusteredPS,               sizeof(g_pKillzoneGBufferGlossClusteredPS)              },
            { g_pKillzoneGBufferNormalClusteredPS,              sizeof(g_pKillzoneGBufferNormalClusteredPS)             },
            { g_pKillzoneGBufferMotionVectorClusteredPS,        sizeof(g_pKillzoneGBufferMotionVectorClusteredPS)       },
            { g_pKillzoneGBufferSpecularIntensityClusteredPS,   sizeof(g_pKillzoneGBufferSpecularIntensityClusteredPS)  },
            { g_pKillzoneGBufferDiffuseAlbedoClusteredPS,       sizeof(g_pKillzoneGBufferDiffuseAlbedoClusteredPS)      },
            { g_pKillzoneGBufferSunOcclusionClusteredPS,        sizeof(g_pKillzoneGBufferSunOcclusionClusteredPS)       },
            { g_pKillzoneGBufferLightClusteredDensityPS,        sizeof(g_pKillzoneGBufferLightClusteredDensityPS)       },
            { g_pKillzoneGBufferLightClusteredFPRPS,            sizeof(g_pKillzoneGBufferLightClusteredFPRPS)           },
            { g_pKillzoneGBufferLightClusteredPS,               sizeof(g_pKillzoneGBufferLightClusteredPS)              },
#endif
        },
    };

    std::pair<const unsigned char* const, size_t> m_aThinGBufferPixelShaders[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eThinGBufferDataType::COUNT) + 1] =
    {
        // Default
        {
            { g_pThinGBufferDefaultDepthPS,               sizeof(g_pThinGBufferDefaultDepthPS)              },
            { g_pThinGBufferDefaultLightAccumulationPS,   sizeof(g_pThinGBufferDefaultLightAccumulationPS)  },
            { g_pThinGBufferDefaultGlossPS,               sizeof(g_pThinGBufferDefaultGlossPS)              },
            { g_pThinGBufferDefaultNormalPS,              sizeof(g_pThinGBufferDefaultNormalPS)             },
            { g_pThinGBufferDefaultDiffuseAlbedoPS,       sizeof(g_pThinGBufferDefaultDiffuseAlbedoPS)      },
            { g_pThinGBufferDefaultSpecularIntensityPS,   sizeof(g_pThinGBufferDefaultSpecularIntensityPS)  },
            { g_pThinGBufferDefaultDensityPS,             sizeof(g_pThinGBufferDefaultDensityPS)            },
            { g_pThinGBufferDefaultFPRPS,                 sizeof(g_pThinGBufferDefaultFPRPS)                },
            { g_pThinGBufferDefaultPS,                    sizeof(g_pThinGBufferDefaultPS)                   },
        },
        // Tiled
        {
            { g_pThinGBufferTiledDepthPS,               sizeof(g_pThinGBufferTiledDepthPS)              },
            { g_pThinGBufferTiledLightAccumulationPS,   sizeof(g_pThinGBufferTiledLightAccumulationPS)  },
            { g_pThinGBufferTiledGlossPS,               sizeof(g_pThinGBufferTiledGlossPS)              },
            { g_pThinGBufferTiledNormalPS,              sizeof(g_pThinGBufferTiledNormalPS)             },
            { g_pThinGBufferTiledDiffuseAlbedoPS,       sizeof(g_pThinGBufferTiledDiffuseAlbedoPS)      },
            { g_pThinGBufferTiledSpecularIntensityPS,   sizeof(g_pThinGBufferTiledSpecularIntensityPS)  },
            { g_pThinGBufferTiledDensityPS,             sizeof(g_pThinGBufferTiledDensityPS)            },
            { g_pThinGBufferTiledFPRPS,                 sizeof(g_pThinGBufferTiledFPRPS)                },
            { g_pThinGBufferTiledPS,                    sizeof(g_pThinGBufferTiledPS)                   },
        },
        // Tiled 2.5D Culling
        {
            { g_pThinGBufferTiledDepthPS,               sizeof(g_pThinGBufferTiledDepthPS)              },
            { g_pThinGBufferTiledLightAccumulationPS,   sizeof(g_pThinGBufferTiledLightAccumulationPS)  },
            { g_pThinGBufferTiledGlossPS,               sizeof(g_pThinGBufferTiledGlossPS)              },
            { g_pThinGBufferTiledNormalPS,              sizeof(g_pThinGBufferTiledNormalPS)             },
            { g_pThinGBufferTiledDiffuseAlbedoPS,       sizeof(g_pThinGBufferTiledDiffuseAlbedoPS)      },
            { g_pThinGBufferTiledSpecularIntensityPS,   sizeof(g_pThinGBufferTiledSpecularIntensityPS)  },
            { g_pThinGBufferTiledDensityPS,             sizeof(g_pThinGBufferTiledDensityPS)            },
            { g_pThinGBufferTiledFPRPS,                 sizeof(g_pThinGBufferTiledFPRPS)                },
            { g_pThinGBufferTiledPS,                    sizeof(g_pThinGBufferTiledPS)                   },
        },
        // Tiled 2.5D AABB Culling
        {
            { g_pThinGBufferTiledDepthPS,               sizeof(g_pThinGBufferTiledDepthPS)              },
            { g_pThinGBufferTiledLightAccumulationPS,   sizeof(g_pThinGBufferTiledLightAccumulationPS)  },
            { g_pThinGBufferTiledGlossPS,               sizeof(g_pThinGBufferTiledGlossPS)              },
            { g_pThinGBufferTiledNormalPS,              sizeof(g_pThinGBufferTiledNormalPS)             },
            { g_pThinGBufferTiledDiffuseAlbedoPS,       sizeof(g_pThinGBufferTiledDiffuseAlbedoPS)      },
            { g_pThinGBufferTiledSpecularIntensityPS,   sizeof(g_pThinGBufferTiledSpecularIntensityPS)  },
            { g_pThinGBufferTiledDensityPS,             sizeof(g_pThinGBufferTiledDensityPS)            },
            { g_pThinGBufferTiledFPRPS,                 sizeof(g_pThinGBufferTiledFPRPS)                },
            { g_pThinGBufferTiledPS,                    sizeof(g_pThinGBufferTiledPS)                   },
        },
        // Tiled DICE
        {
            { g_pThinGBufferTiledDepthPS,               sizeof(g_pThinGBufferTiledDepthPS)              },
            { g_pThinGBufferTiledLightAccumulationPS,   sizeof(g_pThinGBufferTiledLightAccumulationPS)  },
            { g_pThinGBufferTiledGlossPS,               sizeof(g_pThinGBufferTiledGlossPS)              },
            { g_pThinGBufferTiledNormalPS,              sizeof(g_pThinGBufferTiledNormalPS)             },
            { g_pThinGBufferTiledDiffuseAlbedoPS,       sizeof(g_pThinGBufferTiledDiffuseAlbedoPS)      },
            { g_pThinGBufferTiledSpecularIntensityPS,   sizeof(g_pThinGBufferTiledSpecularIntensityPS)  },
            { g_pThinGBufferTiledDensityPS,             sizeof(g_pThinGBufferTiledDensityPS)            },
            { g_pThinGBufferTiledFPRPS,                 sizeof(g_pThinGBufferTiledFPRPS)                },
            { g_pThinGBufferTiledPS,                    sizeof(g_pThinGBufferTiledPS)                   },
        },
        // Tiled DICE 2.5
        {
            { g_pThinGBufferTiledDepthPS,               sizeof(g_pThinGBufferTiledDepthPS)              },
            { g_pThinGBufferTiledLightAccumulationPS,   sizeof(g_pThinGBufferTiledLightAccumulationPS)  },
            { g_pThinGBufferTiledGlossPS,               sizeof(g_pThinGBufferTiledGlossPS)              },
            { g_pThinGBufferTiledNormalPS,              sizeof(g_pThinGBufferTiledNormalPS)             },
            { g_pThinGBufferTiledDiffuseAlbedoPS,       sizeof(g_pThinGBufferTiledDiffuseAlbedoPS)      },
            { g_pThinGBufferTiledSpecularIntensityPS,   sizeof(g_pThinGBufferTiledSpecularIntensityPS)  },
            { g_pThinGBufferTiledDensityPS,             sizeof(g_pThinGBufferTiledDensityPS)            },
            { g_pThinGBufferTiledFPRPS,                 sizeof(g_pThinGBufferTiledFPRPS)                },
            { g_pThinGBufferTiledPS,                    sizeof(g_pThinGBufferTiledPS)                   },
        },
        // Tiled DICE 2.5 AABB
        {
            { g_pThinGBufferTiledDepthPS,               sizeof(g_pThinGBufferTiledDepthPS)              },
            { g_pThinGBufferTiledLightAccumulationPS,   sizeof(g_pThinGBufferTiledLightAccumulationPS)  },
            { g_pThinGBufferTiledGlossPS,               sizeof(g_pThinGBufferTiledGlossPS)              },
            { g_pThinGBufferTiledNormalPS,              sizeof(g_pThinGBufferTiledNormalPS)             },
            { g_pThinGBufferTiledDiffuseAlbedoPS,       sizeof(g_pThinGBufferTiledDiffuseAlbedoPS)      },
            { g_pThinGBufferTiledSpecularIntensityPS,   sizeof(g_pThinGBufferTiledSpecularIntensityPS)  },
            { g_pThinGBufferTiledDensityPS,             sizeof(g_pThinGBufferTiledDensityPS)            },
            { g_pThinGBufferTiledFPRPS,                 sizeof(g_pThinGBufferTiledFPRPS)                },
            { g_pThinGBufferTiledPS,                    sizeof(g_pThinGBufferTiledPS)                   },
        },
        // Tiled INTEL
        {
            { g_pThinGBufferTiledDepthPS,               sizeof(g_pThinGBufferTiledDepthPS)              },
            { g_pThinGBufferTiledLightAccumulationPS,   sizeof(g_pThinGBufferTiledLightAccumulationPS)  },
            { g_pThinGBufferTiledGlossPS,               sizeof(g_pThinGBufferTiledGlossPS)              },
            { g_pThinGBufferTiledNormalPS,              sizeof(g_pThinGBufferTiledNormalPS)             },
            { g_pThinGBufferTiledDiffuseAlbedoPS,       sizeof(g_pThinGBufferTiledDiffuseAlbedoPS)      },
            { g_pThinGBufferTiledSpecularIntensityPS,   sizeof(g_pThinGBufferTiledSpecularIntensityPS)  },
            { g_pThinGBufferTiledDensityPS,             sizeof(g_pThinGBufferTiledDensityPS)            },
            { g_pThinGBufferTiledFPRPS,                 sizeof(g_pThinGBufferTiledFPRPS)                },
            { g_pThinGBufferTiledPS,                    sizeof(g_pThinGBufferTiledPS)                   },
        },
        // Clustered
        {
            { g_pThinGBufferClusteredDepthPS,               sizeof(g_pThinGBufferClusteredDepthPS)              },
            { g_pThinGBufferClusteredLightAccumulationPS,   sizeof(g_pThinGBufferClusteredLightAccumulationPS)  },
            { g_pThinGBufferClusteredGlossPS,               sizeof(g_pThinGBufferClusteredGlossPS)              },
            { g_pThinGBufferClusteredNormalPS,              sizeof(g_pThinGBufferClusteredNormalPS)             },
            { g_pThinGBufferClusteredDiffuseAlbedoPS,       sizeof(g_pThinGBufferClusteredDiffuseAlbedoPS)      },
            { g_pThinGBufferClusteredSpecularIntensityPS,   sizeof(g_pThinGBufferClusteredSpecularIntensityPS)  },
            { g_pThinGBufferClusteredDensityPS,             sizeof(g_pThinGBufferClusteredDensityPS)            },
            { g_pThinGBufferClusteredFPRPS,                 sizeof(g_pThinGBufferClusteredFPRPS)                },
            { g_pThinGBufferClusteredPS,                    sizeof(g_pThinGBufferClusteredPS)                   },
        },
    };

    constexpr const WCHAR A_SZ_FORWARD_PROF_NAME[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eForwardType::COUNT) + 1][48] =
    {
        // Default
        {
            L"F World Pos",
            L"F Albedo",
            L"F Gloss",
            L"F Normal",
            L"F Spec Intensity",
            L"F Light Density",
            L"F FPR",
            L"F Color",
        },
        // Tiled
        {
            L"F+ World Pos",
            L"F+ Albedo",
            L"F+ Gloss",
            L"F+ Normal",
            L"F+ Spec Intensity",
            L"F+ Light Density",
            L"F+ FPR",
            L"F+ Color",
        },
        // Tiled 2.5D Culling
        {
            L"F+ 2.5D World Pos",
            L"F+ 2.5D Albedo",
            L"F+ 2.5D Gloss",
            L"F+ 2.5D Normal",
            L"F+ 2.5D Spec Intensity",
            L"F+ 2.5D Light Density",
            L"F+ 2.5D FPR",
            L"F+ 2.5D Color",
        },
        // Tiled 2.5D AABB-based Culling
        {
            L"F+ 2.5D AABB World Pos",
            L"F+ 2.5D AABB Albedo",
            L"F+ 2.5D AABB Gloss",
            L"F+ 2.5D AABB Normal",
            L"F+ 2.5D AABB Spec Intensity",
            L"F+ 2.5D AABB Light Density",
            L"F+ 2.5D AABB FPR",
            L"F+ 2.5D AABB Color",
        },
        // Tiled
        {
            L"F+ World Pos",
            L"F+ Albedo",
            L"F+ Gloss",
            L"F+ Normal",
            L"F+ Spec Intensity",
            L"F+ Light Density",
            L"F+ FPR",
            L"F+ Color",
        },
        // Tiled 2.5D Culling
        {
            L"F+ 2.5D World Pos",
            L"F+ 2.5D Albedo",
            L"F+ 2.5D Gloss",
            L"F+ 2.5D Normal",
            L"F+ 2.5D Spec Intensity",
            L"F+ 2.5D Light Density",
            L"F+ 2.5D FPR",
            L"F+ 2.5D Color",
        },
        // Tiled 2.5D AABB-based Culling
        {
            L"F+ 2.5D AABB World Pos",
            L"F+ 2.5D AABB Albedo",
            L"F+ 2.5D AABB Gloss",
            L"F+ 2.5D AABB Normal",
            L"F+ 2.5D AABB Spec Intensity",
            L"F+ 2.5D AABB Light Density",
            L"F+ 2.5D AABB FPR",
            L"F+ 2.5D AABB Color",
        },
        // Tiled
        {
            L"F+ World Pos",
            L"F+ Albedo",
            L"F+ Gloss",
            L"F+ Normal",
            L"F+ Spec Intensity",
            L"F+ Light Density",
            L"F+ FPR",
            L"F+ Color",
        },
        // Clustered
        {
            L"F Clustered World Pos",
            L"F Clustered Albedo",
            L"F Clustered Gloss",
            L"F Clustered Normal",
            L"F Clustered Spec Intensity",
            L"F Clustered Light Density",
            L"F Clustered FPR",
            L"F Clustered Color",
        },
    };

    constexpr const WCHAR A_SZ_DEFERRED_PROF_NAME[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eGBufferDataType::COUNT) + 1][48] =
    {
        // Default
        {
            L"D Depth",
            L"D L-Buffer",
            L"D Gloss",
            L"D Normal",
            L"D World Pos",
            L"D Spec Intensity",
            L"D Albedo",
            L"D Sun Occl",
            L"D Light Density",
            L"D FPR",
            L"D Color",
        },
        // Tiled
        {
            L"D T Depth",
            L"D T L-Buffer",
            L"D T Gloss",
            L"D T Normal",
            L"D T World Pos",
            L"D T Spec Intensity",
            L"D T Albedo",
            L"D T Sun Occl",
            L"D T Light Density",
            L"D T FPR",
            L"D T Color",
        },
        // Tiled 2.5D Culling
        {
            L"D T 2.5D Depth",
            L"D T 2.5D L-Buffer",
            L"D T 2.5D Gloss",
            L"D T 2.5D Normal",
            L"D T 2.5D World Pos",
            L"D T 2.5D Spec Intensity",
            L"D T 2.5D Albedo",
            L"D T 2.5D Sun Occl",
            L"D T 2.5D Light Density",
            L"D T 2.5D FPR",
            L"D T 2.5D Color",
        },
        // Tiled 2.5D AABB-based Culling
        {
            L"D T 2.5D AABB Depth",
            L"D T 2.5D AABB L-Buffer",
            L"D T 2.5D AABB Gloss",
            L"D T 2.5D AABB Normal",
            L"D T 2.5D AABB World Pos",
            L"D T 2.5D AABB Spec Intensity",
            L"D T 2.5D AABB Albedo",
            L"D T 2.5D AABB Sun Occl",
            L"D T 2.5D AABB Light Density",
            L"D T 2.5D AABB FPR",
            L"D T 2.5D AABB Color",
        },
        // Tiled (DICE)
        {
            L"D T (D) Depth",
            L"D T (D) L-Buffer",
            L"D T (D) Gloss",
            L"D T (D) Normal",
            L"D T (D) World Pos",
            L"D T (D) Spec Intensity",
            L"D T (D) Albedo",
            L"D T (D) Sun Occl",
            L"D T (D) Light Density",
            L"D T (D) FPR",
            L"D T (D) Color",
        },
        // Tiled 2.5D Culling
        {
            L"D T 2.5D (D) Depth",
            L"D T 2.5D (D) L-Buffer",
            L"D T 2.5D (D) Gloss",
            L"D T 2.5D (D) Normal",
            L"D T 2.5D (D) World Pos",
            L"D T 2.5D (D) Spec Intensity",
            L"D T 2.5D (D) Albedo",
            L"D T 2.5D (D) Sun Occl",
            L"D T 2.5D (D) Light Density",
            L"D T 2.5D (D) FPR",
            L"D T 2.5D (D) Color",
        },
        // Tiled 2.5D AABB-based Culling
        {
            L"D T 2.5D AABB (D) Depth",
            L"D T 2.5D AABB (D) L-Buffer",
            L"D T 2.5D AABB (D) Gloss",
            L"D T 2.5D AABB (D) Normal",
            L"D T 2.5D AABB (D) World Pos",
            L"D T 2.5D AABB (D) Spec Intensity",
            L"D T 2.5D AABB (D) Albedo",
            L"D T 2.5D AABB (D) Sun Occl",
            L"D T 2.5D AABB (D) Light Density",
            L"D T 2.5D AABB (D) FPR",
            L"D T 2.5D AABB (D) Color",
        },
        // Tiled (Intel)
        {
            L"D T (I) Depth",
            L"D T (I) L-Buffer",
            L"D T (I) Gloss",
            L"D T (I) Normal",
            L"D T (I) World Pos",
            L"D T (I) Spec Intensity",
            L"D T (I) Albedo",
            L"D T (I) Sun Occl",
            L"D T (I) Light Density",
            L"D T (I) FPR",
            L"D T (I) Color",
        },
        // Clustered
        {
            L"D C Depth",
            L"D C L-Buffer",
            L"D C Gloss",
            L"D C Normal",
            L"D C World Pos",
            L"D C Spec Intensity",
            L"D C Albedo",
            L"D C Sun Occl",
            L"D C Light Density",
            L"D C FPR",
            L"D C Color",
        },
    };

    constexpr const WCHAR A_SZ_DEFERRED_THIN_PROF_NAME[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eGBufferDataType::COUNT) + 1][48] =
    {
        // Default
        {
            L"TG Depth",
            L"TG L-Buffer",
            L"TG Gloss",
            L"TG Normal",
            L"TG Albedo",
            L"TG Spec Intensity",
            L"TG Light Density",
            L"TG FPR",
            L"TG Color",
        },
        // Tiled
        {
            L"TG T Depth",
            L"TG T L-Buffer",
            L"TG T Gloss",
            L"TG T Normal",
            L"TG T Albedo",
            L"TG T Spec Intensity",
            L"TG T Light Density",
            L"TG T FPR",
            L"TG T Color",
        },
        // Tiled 2.5D Culling
        {
            L"TG T 2.5D Depth",
            L"TG T 2.5D L-Buffer",
            L"TG T 2.5D Gloss",
            L"TG T 2.5D Normal",
            L"TG T 2.5D Albedo",
            L"TG T 2.5D Spec Intensity",
            L"TG T 2.5D Light Density",
            L"TG T 2.5D FPR",
            L"TG T 2.5D Color",
        },
        // Tiled 2.5D AABB-based Culling
        {
            L"TG T 2.5D AABB Depth",
            L"TG T 2.5D AABB L-Buffer",
            L"TG T 2.5D AABB Gloss",
            L"TG T 2.5D AABB Normal",
            L"TG T 2.5D AABB Albedo",
            L"TG T 2.5D AABB Spec Intensity",
            L"TG T 2.5D AABB Light Density",
            L"TG T 2.5D AABB FPR",
            L"TG T 2.5D AABB Color",
        },
        // Tiled (DICE)
        {
            L"TG T (D) Depth",
            L"TG T (D) L-Buffer",
            L"TG T (D) Gloss",
            L"TG T (D) Normal",
            L"TG T (D) Albedo",
            L"TG T (D) Spec Intensity",
            L"TG T (D) Light Density",
            L"TG T (D) FPR",
            L"TG T (D) Color",
        },
        // Tiled 2.5D Culling
        {
            L"TG T (D) 2.5D Depth",
            L"TG T (D) 2.5D L-Buffer",
            L"TG T (D) 2.5D Gloss",
            L"TG T (D) 2.5D Normal",
            L"TG T (D) 2.5D Albedo",
            L"TG T (D) 2.5D Spec Intensity",
            L"TG T (D) 2.5D Light Density",
            L"TG T (D) 2.5D FPR",
            L"TG T (D) 2.5D Color",
        },
        // Tiled 2.5D AABB-based Culling
        {
            L"TG T (D) 2.5D AABB Depth",
            L"TG T (D) 2.5D AABB L-Buffer",
            L"TG T (D) 2.5D AABB Gloss",
            L"TG T (D) 2.5D AABB Normal",
            L"TG T (D) 2.5D AABB Albedo",
            L"TG T (D) 2.5D AABB Spec Intensity",
            L"TG T (D) 2.5D AABB Light Density",
            L"TG T (D) 2.5D AABB FPR",
            L"TG T (D) 2.5D AABB Color",
        },
        // Tiled (Intel)
        {
            L"TG T (I) Depth",
            L"TG T (I) L-Buffer",
            L"TG T (I) Gloss",
            L"TG T (I) Normal",
            L"TG T (I) Albedo",
            L"TG T (I) Spec Intensity",
            L"TG T (I) Light Density",
            L"TG T (I) FPR",
            L"TG T (I) Color",
        },
        // Clustered
        {
            L"TG C Depth",
            L"TG C L-Buffer",
            L"TG C Gloss",
            L"TG C Normal",
            L"TG C Albedo",
            L"TG C Spec Intensity",
            L"TG C Light Density",
            L"TG C FPR",
            L"TG C Color",
        },
    };

    DescriptorHandle m_GBufferSRVs;
    DescriptorHandle m_ThinGBufferSRVs;

    eRenderType m_CurrentRenderType = eRenderType::DEFERRED_THIN;
    eForwardType m_CurrentForwardType = eForwardType::COUNT;
    eGBufferDataType m_CurrentGBufferType = eGBufferDataType::COUNT;
    eThinGBufferDataType m_CurrentThinGBufferType = eThinGBufferDataType::COUNT;
    eLightType m_CurrentLightType = eLightType::DEFAULT;

    // These are used at runtime during rendering
    struct GBufferVertex
    {
        XMFLOAT3 Position;
        XMFLOAT2 TexCoord;
    };

    ByteAddressBuffer m_GBufferGeometryBuffer;

    D3D12_VERTEX_BUFFER_VIEW m_GBufferVertexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_GBufferIndexBuffer;

    // These are allocated when constructing a model to be exported
    constexpr const size_t NUM_GBUFFER_VERTICES = 6;
    constexpr const size_t NUM_GBUFFER_INDICES = 6;
    unsigned char* m_pGBufferVertexData;
    unsigned char* m_pGBufferIndexData;

    ModelH3D m_Model;
    std::vector<bool> m_pMaterialIsCutout;

    Vector3 m_SunDirection;
    ShadowCamera m_SunShadow;

    ExpVar m_AmbientIntensity("Sponza/Lighting/Ambient Intensity", 0.1f, -16.0f, 16.0f, 0.1f);
    ExpVar m_SunLightIntensity("Sponza/Lighting/Sun Light Intensity", 4.0f, 0.0f, 16.0f, 0.1f);
    NumVar m_SunOrientation("Sponza/Lighting/Sun Orientation", -0.5f, -100.0f, 100.0f, 0.1f );
    NumVar m_SunInclination("Sponza/Lighting/Sun Inclination", 0.75f, 0.0f, 1.0f, 0.01f );
    NumVar ShadowDimX("Sponza/Lighting/Shadow Dim X", 5000, 1000, 10000, 100 );
    NumVar ShadowDimY("Sponza/Lighting/Shadow Dim Y", 3000, 1000, 10000, 100 );
    NumVar ShadowDimZ("Sponza/Lighting/Shadow Dim Z", 3000, 1000, 10000, 100 );
}

void Sponza::Startup( Camera& Camera )
{
    DXGI_FORMAT ColorFormat = g_SceneColorBuffer.GetFormat();
    DXGI_FORMAT NormalFormat = g_SceneNormalBuffer.GetFormat();
    DXGI_FORMAT DepthFormat = g_SceneDepthBuffer.GetFormat();
    //DXGI_FORMAT ShadowFormat = g_ShadowBuffer.GetFormat();

    D3D12_INPUT_ELEMENT_DESC vertElem[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Depth-only (2x rate)
    m_DepthPSO.SetRootSignature(Renderer::m_RootSig);
    m_DepthPSO.SetRasterizerState(RasterizerDefault);
    m_DepthPSO.SetBlendState(BlendNoColorWrite);
    m_DepthPSO.SetDepthStencilState(DepthStateReadWrite);
    m_DepthPSO.SetInputLayout(_countof(vertElem), vertElem);
    m_DepthPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_DepthPSO.SetRenderTargetFormats(0, nullptr, DepthFormat);
    m_DepthPSO.SetVertexShader(g_pDepthViewerVS, sizeof(g_pDepthViewerVS));
    m_DepthPSO.Finalize();

    // Depth-only shading but with alpha testing
    m_CutoutDepthPSO = m_DepthPSO;
    m_CutoutDepthPSO.SetPixelShader(g_pDepthViewerPS, sizeof(g_pDepthViewerPS));
    m_CutoutDepthPSO.SetRasterizerState(RasterizerTwoSided);
    m_CutoutDepthPSO.Finalize();

    // Depth-only but with a depth bias and/or render only backfaces
    m_ShadowPSO = m_DepthPSO;
    m_ShadowPSO.SetRasterizerState(RasterizerShadow);
    m_ShadowPSO.SetRenderTargetFormats(0, nullptr, g_ShadowBuffer.GetFormat());
    m_ShadowPSO.Finalize();

    // Shadows with alpha testing
    m_CutoutShadowPSO = m_ShadowPSO;
    m_CutoutShadowPSO.SetPixelShader(g_pDepthViewerPS, sizeof(g_pDepthViewerPS));
    m_CutoutShadowPSO.SetRasterizerState(RasterizerShadowTwoSided);
    m_CutoutShadowPSO.Finalize();

    DXGI_FORMAT formats[2] = { ColorFormat, NormalFormat };

    // Full color pass
    m_aForwardPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)] = m_DepthPSO;
    m_aForwardPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)].SetBlendState(BlendDisable);
    m_aForwardPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)].SetDepthStencilState(DepthStateTestEqual);
    m_aForwardPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)].SetRenderTargetFormats(2, formats, DepthFormat);
    m_aForwardPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)].SetVertexShader( g_pModelViewerVS, sizeof(g_pModelViewerVS) );
    m_aForwardPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)].SetPixelShader(
        m_aForwardPixelShaders[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)].first,
        m_aForwardPixelShaders[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)].second
    );
    m_aForwardPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)].Finalize();

    m_CutoutModelPSO = m_aForwardPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)];
    m_CutoutModelPSO.SetRasterizerState(RasterizerTwoSided);
    m_CutoutModelPSO.Finalize();

    for (size_t i = 0; i < static_cast<size_t>(eLightType::COUNT); ++i)
    {
        for (size_t j = 0; j < static_cast<size_t>(eForwardType::COUNT) + 1; ++j)
        {
            if (i == static_cast<size_t>(eLightType::TILED) && j == static_cast<size_t>(eForwardType::COUNT))
            {
                continue;
            }
            m_aForwardPSOs[i][j] = m_aForwardPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)];
            m_aForwardPSOs[i][j].SetPixelShader(m_aForwardPixelShaders[i][j].first, m_aForwardPixelShaders[i][j].second);
            m_aForwardPSOs[i][j].Finalize();
        }
    }

    std::vector<DXGI_FORMAT> gbufferFormats;
    gbufferFormats.reserve(static_cast<size_t>(eGBufferType::COUNT));
    for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
    {
        gbufferFormats.push_back(g_aSceneGBuffers[i].GetFormat());
    }

    m_GBufferPSO = m_aForwardPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)];
    m_GBufferPSO.SetBlendState(BlendDisable);
    m_GBufferPSO.SetRenderTargetFormats(static_cast<size_t>(eGBufferType::COUNT), gbufferFormats.data(), DepthFormat);
#if SIMPLE_GBUFFER
    m_GBufferPSO.SetPixelShader(g_pDeferredPS, sizeof(g_pDeferredPS));
#elif KILLZONE_GBUFFER
    m_GBufferPSO.SetPixelShader(g_pKillzoneDeferredPS, sizeof(g_pKillzoneDeferredPS));
#endif
    m_GBufferPSO.Finalize();

    std::vector<DXGI_FORMAT> thinGbufferFormats;
    thinGbufferFormats.reserve(static_cast<size_t>(eThinGBufferType::COUNT));
    for (size_t i = 0; i < static_cast<size_t>(eThinGBufferType::COUNT); ++i)
    {
        thinGbufferFormats.push_back(g_aSceneThinGBuffers[i].GetFormat());
    }

    m_ThinGBufferPSO = m_GBufferPSO;
    m_ThinGBufferPSO.SetRenderTargetFormats(static_cast<size_t>(eThinGBufferType::COUNT), thinGbufferFormats.data(), DepthFormat);
    m_ThinGBufferPSO.SetVertexShader(g_pThinGBufferVS, sizeof(g_pThinGBufferVS));
    m_ThinGBufferPSO.SetPixelShader(g_pThinGBufferDeferredPS, sizeof(g_pThinGBufferDeferredPS));
    m_ThinGBufferPSO.Finalize();

    // Killzone PSOs
    m_aGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eGBufferDataType::COUNT)] = m_aForwardPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)];
    m_aGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eGBufferDataType::COUNT)].SetRasterizerState(RasterizerLightPassDefault);
    m_aGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eGBufferDataType::COUNT)].SetDepthStencilState(DepthStateDisabled);
    //m_GBufferLightPSO.SetRenderTargetFormat(ColorFormat, DepthFormat);
    m_aGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eGBufferDataType::COUNT)].SetRenderTargetFormat(ColorFormat);
    m_aGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eGBufferDataType::COUNT)].SetVertexShader(g_pGBufferLightVS, sizeof(g_pGBufferLightVS));
    m_aGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eGBufferDataType::COUNT)].SetPixelShader(
        m_aGBufferPixelShaders[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eGBufferDataType::COUNT)].first, 
        m_aGBufferPixelShaders[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eGBufferDataType::COUNT)].second
    );
    m_aGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eGBufferDataType::COUNT)].Finalize();

    for (size_t i = 0; i < static_cast<size_t>(eLightType::COUNT); ++i)
    {
        for (size_t j = 0; j < static_cast<size_t>(eGBufferDataType::COUNT) + 1; ++j)
        {
            if (i == static_cast<size_t>(eLightType::TILED) && j == static_cast<size_t>(eGBufferDataType::COUNT))
            {
                continue;
            }

            m_aGBufferPSOs[i][j] = m_aGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eGBufferDataType::COUNT)];
            m_aGBufferPSOs[i][j].SetPixelShader(m_aGBufferPixelShaders[i][j].first, m_aGBufferPixelShaders[i][j].second);
            m_aGBufferPSOs[i][j].Finalize();
            // m_aGBufferPixelShaders
        }
    }

    // Thin GBuffer PSOs
    m_aThinGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eThinGBufferDataType::COUNT)] = m_aForwardPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eForwardType::COUNT)];
    m_aThinGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eThinGBufferDataType::COUNT)].SetRasterizerState(RasterizerLightPassDefault);
    m_aThinGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eThinGBufferDataType::COUNT)].SetDepthStencilState(DepthStateDisabled);
    //m_GBufferLightPSO.SetRenderTargetFormat(ColorFormat, DepthFormat);
    m_aThinGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eThinGBufferDataType::COUNT)].SetRenderTargetFormat(ColorFormat);
    m_aThinGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eThinGBufferDataType::COUNT)].SetVertexShader(g_pGBufferLightVS, sizeof(g_pGBufferLightVS));
    m_aThinGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eThinGBufferDataType::COUNT)].SetPixelShader(
        m_aThinGBufferPixelShaders[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eThinGBufferDataType::COUNT)].first,
        m_aThinGBufferPixelShaders[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eThinGBufferDataType::COUNT)].second
    );
    m_aThinGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eThinGBufferDataType::COUNT)].Finalize();

    for (size_t i = 0; i < static_cast<size_t>(eLightType::COUNT); ++i)
    {
        for (size_t j = 0; j < static_cast<size_t>(eThinGBufferDataType::COUNT) + 1; ++j)
        {
            if (i == static_cast<size_t>(eLightType::TILED) && j == static_cast<size_t>(eThinGBufferDataType::COUNT))
            {
                continue;
            }

            m_aThinGBufferPSOs[i][j] = m_aThinGBufferPSOs[static_cast<size_t>(eLightType::TILED)][static_cast<size_t>(eThinGBufferDataType::COUNT)];
            m_aThinGBufferPSOs[i][j].SetPixelShader(m_aThinGBufferPixelShaders[i][j].first, m_aThinGBufferPixelShaders[i][j].second);
            m_aThinGBufferPSOs[i][j].Finalize();
            // m_aGBufferPixelShaders
        }
    }

    ASSERT(m_Model.Load(L"Sponza/sponza.h3d"), "Failed to load model");
    ASSERT(m_Model.GetMeshCount() > 0, "Model contains no meshes");

    // The caller of this function can override which materials are considered cutouts
    m_pMaterialIsCutout.resize(m_Model.GetMaterialCount());
    for (uint32_t i = 0; i < m_Model.GetMaterialCount(); ++i)
    {
        const ModelH3D::Material& mat = m_Model.GetMaterial(i);
        if (std::string(mat.texDiffusePath).find("thorn") != std::string::npos ||
            std::string(mat.texDiffusePath).find("plant") != std::string::npos ||
            std::string(mat.texDiffusePath).find("chain") != std::string::npos)
        {
            m_pMaterialIsCutout[i] = true;
        }
        else
        {
            m_pMaterialIsCutout[i] = false;
        }
    }

    ParticleEffects::InitFromJSON(L"Sponza/particles.json");

    float modelRadius = Length(m_Model.GetBoundingBox().GetDimensions()) * 0.5f;
    const Vector3 eye = m_Model.GetBoundingBox().GetCenter() + Vector3(modelRadius * 0.5f, 0.0f, 0.0f);
    Camera.SetEyeAtUp( eye, Vector3(kZero), Vector3(kYUnitVector) );

    Lighting::CreateRandomLights(m_Model.GetBoundingBox().GetMin(), m_Model.GetBoundingBox().GetMax());

    // Killzone
    m_GBufferSRVs = Renderer::s_TextureHeap.Alloc(static_cast<uint32_t>(eGBufferType::COUNT));

    uint32_t DestCount = static_cast<uint32_t>(eGBufferType::COUNT);
    std::vector<uint32_t> SourceCounts;
    SourceCounts.reserve(DestCount);
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> aGBuffers;
    aGBuffers.reserve(DestCount);

    for (uint32_t i = 0; i < DestCount; ++i)
    {
        SourceCounts.push_back(1);
        aGBuffers.push_back(g_aSceneGBuffers[i].GetSRV());
    }

    Graphics::g_Device->CopyDescriptors(
        1,
        &m_GBufferSRVs,
        &DestCount,
        DestCount,
        aGBuffers.data(),
        SourceCounts.data(),
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );

    // Thin GBuffer
    m_ThinGBufferSRVs = Renderer::s_TextureHeap.Alloc(static_cast<uint32_t>(eThinGBufferType::COUNT));

    DestCount = static_cast<uint32_t>(eThinGBufferType::COUNT);
    SourceCounts.clear();
    SourceCounts.reserve(DestCount);
    aGBuffers.clear();
    aGBuffers.reserve(DestCount);

    for (uint32_t i = 0; i < DestCount; ++i)
    {
        SourceCounts.push_back(1);
        aGBuffers.push_back(g_aSceneThinGBuffers[i].GetSRV());
    }

    Graphics::g_Device->CopyDescriptors(
        1,
        &m_ThinGBufferSRVs,
        &DestCount,
        DestCount,
        aGBuffers.data(),
        SourceCounts.data(),
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );

    UploadBuffer geomBuffer;

    //float left = -(g_aSceneGBuffers[0].GetWidth() / 2.0f);
    float left = -1.0f;
    //float right = left + g_aSceneGBuffers[0].GetWidth();
    float right = 1.0f;
    //float top = (g_aSceneGBuffers[0].GetHeight() / 2.0f);
    float top = -1.0f;
    //float bottom = top - g_aSceneGBuffers[0].GetHeight();
    float bottom = 1.0f;

    uint32_t totalBinarySize = NUM_GBUFFER_VERTICES * sizeof(GBufferVertex) + NUM_GBUFFER_INDICES * sizeof(WORD);

    geomBuffer.Create(L"GBuffer Geometry Upload Buffer", totalBinarySize);
    uint8_t* uploadMem = (uint8_t*)geomBuffer.Map();

    m_pGBufferVertexData = uploadMem;
    m_pGBufferIndexData = m_pGBufferVertexData + NUM_GBUFFER_VERTICES * sizeof(GBufferVertex);

    WORD* pIndices = reinterpret_cast<WORD*>(m_pGBufferIndexData);

    for (WORD i = 0; i < NUM_GBUFFER_INDICES; ++i)
    {
        pIndices[i] = i;
        //pIndices[i] = NUM_GBUFFER_INDICES - 1 - i;
    }

    GBufferVertex QuadVerts[] =
    {
        { { left,  top, 1.0f }, { 0.0f, 0.0f } },
        { { right,  bottom, 1.0f }, { 1.0f, 1.0f } },
        { { left, bottom, 1.0f }, { 0.0f, 1.0f } },
        { { left, top, 1.0f }, { 0.0f, 0.0f } },
        { { right, top, 1.0f }, { 1.0f, 0.0f } },
        { { right, bottom, 1.0f }, { 1.0f, 1.0f } },
    };

    static_assert(sizeof(QuadVerts) == NUM_GBUFFER_VERTICES * sizeof(GBufferVertex), "Quad Vertices size");
    memcpy(m_pGBufferVertexData, QuadVerts, sizeof(QuadVerts));

    m_GBufferGeometryBuffer.Create(L"Geometry Buffer", totalBinarySize, 1, geomBuffer);

    m_GBufferVertexBuffer = m_GBufferGeometryBuffer.VertexBufferView(0, NUM_GBUFFER_VERTICES * sizeof(GBufferVertex), sizeof(GBufferVertex));
    m_GBufferIndexBuffer = m_GBufferGeometryBuffer.IndexBufferView(m_pGBufferIndexData - m_pGBufferVertexData, NUM_GBUFFER_INDICES * sizeof(WORD), false);

    m_pGBufferVertexData = new unsigned char[NUM_GBUFFER_VERTICES * sizeof(GBufferVertex)];
    m_pGBufferIndexData = new unsigned char[NUM_GBUFFER_INDICES * sizeof(WORD)];

    std::memcpy(m_pGBufferVertexData, uploadMem, NUM_GBUFFER_VERTICES * sizeof(GBufferVertex));
    uploadMem += NUM_GBUFFER_VERTICES * sizeof(GBufferVertex);
    std::memcpy(m_pGBufferIndexData, uploadMem, NUM_GBUFFER_INDICES * sizeof(WORD));

    geomBuffer.Unmap();
}

const ModelH3D& Sponza::GetModel()
{
    return Sponza::m_Model;
}

void Sponza::SetRenderType(eRenderType renderType) noexcept
{
    ASSERT(renderType != eRenderType::COUNT);

    switch (renderType)
    {
    case eRenderType::FORWARD:
        OutputDebugString(L"FORWARD RENDERING PIPELINE\n");
        switch (m_CurrentForwardType)
        {
        case eForwardType::WORLD_POS:
            OutputDebugString(L"\tWORLD POS PASS\n");
            break;
        case eForwardType::DIFFUSE_ALBEDO:
            OutputDebugString(L"\tDIFFUSE ALBEDO PASS\n");
            break;
        case eForwardType::GLOSS:
            OutputDebugString(L"\tGLOSSINESS PASS\n");
            break;
        case eForwardType::NORMAL:
            OutputDebugString(L"\tNORMAL PASS\n");
            break;
        case eForwardType::SPECULAR_INTENSITY:
            OutputDebugString(L"\tSPECULAR INTENSITY PASS\n");
            break;
        case eForwardType::LIGHT_DENSITY:
            OutputDebugString(L"\tLIGHT DENSITY PASS\n");
            break;
        case eForwardType::FALSE_POSITIVE_RATE:
            OutputDebugString(L"\tFALSE POSITIVE RATE PASS\n");
            break;
        case eForwardType::COUNT:
            OutputDebugString(L"\tCOLOR PASS\n");
            break;
        default:
            break;
        }
        break;
        break;
    case eRenderType::DEFERRED:
        OutputDebugString(L"DEFERRED RENDERING PIPELINE\n");
        switch (m_CurrentGBufferType)
        {
#if SIMPLE_GBUFFER
        case eGBufferDataType::RT0_WORLD_POS:
            OutputDebugString(L"\tRT0: WORLD POS PASS\n");
            break;
        case eGBufferDataType::RT1_NORMAL:
            OutputDebugString(L"\tRT1: NORMAL PASS\n");
            break;
        case eGBufferDataType::RT1_DEPTH:
            OutputDebugString(L"\tRT1: DEPTH PASS\n");
            break;
        case eGBufferDataType::RT2_ALBEDO:
            OutputDebugString(L"\tRT2: ALBEDO PASS\n");
            break;
        case eGBufferDataType::RT3_SPECULAR:
            OutputDebugString(L"\tRT3: SPECULAR PASS\n");
            break;
        case eGBufferDataType::RT3_GLOSS:
            OutputDebugString(L"\tRT3: GLOSS PASS\n");
            break;
#elif KILLZONE_GBUFFER
        case eGBufferDataType::DEPTH:
            OutputDebugString(L"\tDEPTH PASS\n");
            break;
        case eGBufferDataType::RT0_LIGHT_ACCUMULATION:
            OutputDebugString(L"\tRT0: LIGHT ACCUMULATION PASS\n");
            break;
        //case eGBufferDataType::RT0_INTENSITY:
        //    OutputDebugString(L"\tRT0: INTENSITY PASS\n");
        //    break;
        case eGBufferDataType::RT1_NORMAL:
            OutputDebugString(L"\tRT1: NORMAL PASS\n");
            break;
        case eGBufferDataType::RT2_MOTION_VECTORS:
            OutputDebugString(L"\tRT2: MOTION VECTORS PASS\n");
            break;
        case eGBufferDataType::RT2_SPEC_INTENSITY:
            OutputDebugString(L"\tRT2: SPECULAR INTENSITY PASS\n");
            break;
        case eGBufferDataType::RT3_DIFFUSE_ALBEDO:
            OutputDebugString(L"\tRT3: DIFFUSE ALBEDO PASS\n");
            break;
        case eGBufferDataType::RT3_SUN_OCCLUSION:
            OutputDebugString(L"\tRT3: SUN OCCLUSION PASS\n");
            break;
        case eGBufferDataType::LIGHT_DENSITY:
            OutputDebugString(L"\tLIGHT DENSITY PASS\n");
            break;
        case eGBufferDataType::FALSE_POSITIVE_RATE:
            OutputDebugString(L"\tFALSE POSITIVE RATE PASS\n");
            break;
#endif
        case eGBufferDataType::COUNT:
            OutputDebugString(L"\tGBUFFER PASS\n");
            break;
        default:
            ASSERT(false);
            break;
        }
        break;
    case eRenderType::DEFERRED_THIN:
        OutputDebugString(L"DEFERRED THIN GBUFFER RENDERING PIPELINE\n");
        switch (m_CurrentThinGBufferType)
        {
        case eThinGBufferDataType::DEPTH:
            OutputDebugString(L"\tDEPTH PASS\n");
            break;
        case eThinGBufferDataType::RT0_LIGHT_ACCUMULATION:
            OutputDebugString(L"\tRT0: LIGHT ACCUMULATION PASS\n");
            break;
        case eThinGBufferDataType::RT1_NORMAL:
            OutputDebugString(L"\tRT1: NORMAL PASS\n");
            break;
        case eThinGBufferDataType::RT1_GLOSSINESS:
            OutputDebugString(L"\tRT1: GLOSSINESS PASS\n");
            break;
        case eThinGBufferDataType::RT2_DIFFUSE_ALBEDO:
            OutputDebugString(L"\tRT2: DIFFUSE ALBEDO PASS\n");
            break;
        case eThinGBufferDataType::RT2_SPEC_INTENSITY:
            OutputDebugString(L"\tRT2: SPECULAR INTENSITY PASS\n");
            break;
        case eThinGBufferDataType::LIGHT_DENSITY:
            OutputDebugString(L"\tLIGHT DENSITY PASS\n");
            break;
        case eThinGBufferDataType::FALSE_POSITIVE_RATE:
            OutputDebugString(L"\tFALSE POSITIVE RATE PASS\n");
            break;
        case eThinGBufferDataType::COUNT:
            OutputDebugString(L"\tGBUFFER PASS\n");
            break;
        default:
            ASSERT(false);
            break;
        }
        break;
    case eRenderType::COUNT:
        // intentional fallthrough
    default:
        ASSERT(false);
        break;
    }

    switch (m_CurrentLightType)
    {
    case eLightType::DEFAULT:
        OutputDebugString(L"\tDEFAULT LIGHTS PASS\n");
        break;
    case eLightType::TILED:
        OutputDebugString(L"\tTILED LIGHTS PASS\n");
        break;
    case eLightType::TILED_2_5:
        OutputDebugString(L"\tTILED 2.5D CULLING LIGHTS PASS\n");
        break;
    case eLightType::TILED_2_5_AABB:
        OutputDebugString(L"\tTILED 2.5D AABB CULLING LIGHTS PASS\n");
        break;
    case eLightType::TILED_DICE:
        OutputDebugString(L"\tDICE TILED LIGHTS PASS\n");
        break;
    case eLightType::TILED_DICE_2_5:
        OutputDebugString(L"\tDICE 2.5 CULLING TILED LIGHTS PASS\n");
        break;
    case eLightType::TILED_DICE_2_5_AABB:
        OutputDebugString(L"\tDICE 2.5 AABB CULLING TILED LIGHTS PASS\n");
        break;
    case eLightType::TILED_INTEL:
        OutputDebugString(L"\tINTEL TILED LIGHTS PASS\n");
        break;
    case eLightType::CLUSTERED:
        OutputDebugString(L"\tCLUSTERED LIGHTS PASS\n");
        break;
    case eLightType::COUNT:
        // Intentional fallthrough
    default:
        ASSERT(false);
        break;
    }

    m_CurrentRenderType = renderType;
}

void Sponza::SetNextBufferOutput()
{
    switch (m_CurrentRenderType)
    {
    case eRenderType::FORWARD:
        m_CurrentForwardType = static_cast<eForwardType>((static_cast<size_t>(m_CurrentForwardType) + 1) % (static_cast<size_t>(eForwardType::COUNT) + 1));
        switch (m_CurrentForwardType)
        {
        case eForwardType::WORLD_POS:
            OutputDebugString(L"\tWORLD POS PASS\n");
            break;
        case eForwardType::DIFFUSE_ALBEDO:
            OutputDebugString(L"\tDIFFUSE ALBEDO PASS\n");
            break;
        case eForwardType::GLOSS:
            OutputDebugString(L"\tGLOSSINESS PASS\n");
            break;
        case eForwardType::NORMAL:
            OutputDebugString(L"\tNORMAL PASS\n");
            break;
        case eForwardType::SPECULAR_INTENSITY:
            OutputDebugString(L"\tSPECULAR INTENSITY PASS\n");
            break;
        case eForwardType::LIGHT_DENSITY:
            OutputDebugString(L"\tLIGHT DENSITY PASS\n");
            break;
        case eForwardType::FALSE_POSITIVE_RATE:
            OutputDebugString(L"\tFALSE POSITIVE RATE PASS\n");
            break;
        case eForwardType::COUNT:
            OutputDebugString(L"\tCOLOR PASS\n");
            break;
        default:
            break;
        }
        break;
    case eRenderType::DEFERRED:
        m_CurrentGBufferType = static_cast<eGBufferDataType>((static_cast<size_t>(m_CurrentGBufferType) + 1) % (static_cast<size_t>(eGBufferDataType::COUNT) + 1));
        switch (m_CurrentGBufferType)
        {
#if SIMPLE_GBUFFER
        case eGBufferDataType::RT0_WORLD_POS:
            OutputDebugString(L"\tRT0: WORLD POS PASS\n");
            break;
        case eGBufferDataType::RT1_NORMAL:
            OutputDebugString(L"\tRT1: NORMAL PASS\n");
            break;
        case eGBufferDataType::RT1_DEPTH:
            OutputDebugString(L"\tRT1: DEPTH PASS\n");
            break;
        case eGBufferDataType::RT2_ALBEDO:
            OutputDebugString(L"\tRT2: ALBEDO PASS\n");
            break;
        case eGBufferDataType::RT3_SPECULAR:
            OutputDebugString(L"\tRT3: SPECULAR PASS\n");
            break;
        case eGBufferDataType::RT3_GLOSS:
            OutputDebugString(L"\tRT3: GLOSS PASS\n");
            break;
#elif KILLZONE_GBUFFER
        case eGBufferDataType::DEPTH:
            OutputDebugString(L"\tDEPTH PASS\n");
            break;
        case eGBufferDataType::RT0_LIGHT_ACCUMULATION:
            OutputDebugString(L"\tRT0: LIGHT ACCUMULATION PASS\n");
            break;
        case eGBufferDataType::RT0_INTENSITY:
            OutputDebugString(L"\tRT0: INTENSITY PASS\n");
            break;
        case eGBufferDataType::RT1_NORMAL:
            OutputDebugString(L"\tRT1: NORMAL PASS\n");
            break;
        case eGBufferDataType::RT2_MOTION_VECTORS:
            OutputDebugString(L"\tRT2: MOTION VECTORS PASS\n");
            break;
        case eGBufferDataType::RT2_SPEC_INTENSITY:
            OutputDebugString(L"\tRT2: SPECULAR INTENSITY PASS\n");
            break;
        case eGBufferDataType::RT3_DIFFUSE_ALBEDO:
            OutputDebugString(L"\tRT3: DIFFUSE ALBEDO PASS\n");
            break;
        case eGBufferDataType::RT3_SUN_OCCLUSION:
            OutputDebugString(L"\tRT3: SUN OCCLUSION PASS\n");
            break;
#endif
        case eGBufferDataType::LIGHT_DENSITY:
            OutputDebugString(L"\tLIGHT DENSITY PASS\n");
            break;
        case eGBufferDataType::FALSE_POSITIVE_RATE:
            OutputDebugString(L"\tFALSE POSITIVE RATE PASS\n");
            break;
        case eGBufferDataType::COUNT:
            OutputDebugString(L"\tGBUFFER PASS\n");
            break;
        default:
            ASSERT(false);
            break;
        }
        break;
    case eRenderType::DEFERRED_THIN:
        m_CurrentThinGBufferType = static_cast<eThinGBufferDataType>((static_cast<size_t>(m_CurrentThinGBufferType) + 1) % (static_cast<size_t>(eThinGBufferDataType::COUNT) + 1));
        switch (m_CurrentThinGBufferType)
        {
        case eThinGBufferDataType::DEPTH:
            OutputDebugString(L"\tDEPTH PASS\n");
            break;
        case eThinGBufferDataType::RT0_LIGHT_ACCUMULATION:
            OutputDebugString(L"\tRT0: LIGHT ACCUMULATION PASS\n");
            break;
        case eThinGBufferDataType::RT1_NORMAL:
            OutputDebugString(L"\tRT1: NORMAL PASS\n");
            break;
        case eThinGBufferDataType::RT1_GLOSSINESS:
            OutputDebugString(L"\tRT1: GLOSSINESS PASS\n");
            break;
        case eThinGBufferDataType::RT2_DIFFUSE_ALBEDO:
            OutputDebugString(L"\tRT2: DIFFUSE ALBEDO PASS\n");
            break;
        case eThinGBufferDataType::RT2_SPEC_INTENSITY:
            OutputDebugString(L"\tRT2: SPECULAR INTENSITY PASS\n");
            break;
        case eThinGBufferDataType::LIGHT_DENSITY:
            OutputDebugString(L"\tLIGHT DENSITY PASS\n");
            break;
        case eThinGBufferDataType::FALSE_POSITIVE_RATE:
            OutputDebugString(L"\tFALSE POSITIVE RATE PASS\n");
            break;
        case eThinGBufferDataType::COUNT:
            OutputDebugString(L"\tGBUFFER PASS\n");
            break;
        default:
            ASSERT(false);
            break;
        }
        break;
    case eRenderType::COUNT:
        break;
    default:
        break;
    }
}

void Sponza::SetPreviousBufferOutput()
{
    switch (m_CurrentRenderType)
    {
    case eRenderType::FORWARD:
        m_CurrentForwardType = static_cast<eForwardType>((static_cast<size_t>(m_CurrentForwardType) + static_cast<size_t>(eForwardType::COUNT)) % (static_cast<size_t>(eForwardType::COUNT) + 1));
        switch (m_CurrentForwardType)
        {
        case eForwardType::WORLD_POS:
            OutputDebugString(L"\tWORLD POS PASS\n");
            break;
        case eForwardType::DIFFUSE_ALBEDO:
            OutputDebugString(L"\tDIFFUSE ALBEDO PASS\n");
            break;
        case eForwardType::GLOSS:
            OutputDebugString(L"\tGLOSSINESS PASS\n");
            break;
        case eForwardType::NORMAL:
            OutputDebugString(L"\tNORMAL PASS\n");
            break;
        case eForwardType::SPECULAR_INTENSITY:
            OutputDebugString(L"\tSPECULAR INTENSITY PASS\n");
            break;
        case eForwardType::LIGHT_DENSITY:
            OutputDebugString(L"\tLIGHT DENSITY PASS\n");
            break;
        case eForwardType::FALSE_POSITIVE_RATE:
            OutputDebugString(L"\tFALSE POSITIVE RATE PASS\n");
            break;
        case eForwardType::COUNT:
            OutputDebugString(L"\tCOLOR PASS\n");
            break;
        default:
            break;
        }
        break;
    case eRenderType::DEFERRED:
        m_CurrentGBufferType = static_cast<eGBufferDataType>((static_cast<size_t>(m_CurrentGBufferType) + static_cast<size_t>(eGBufferDataType::COUNT)) % (static_cast<size_t>(eGBufferDataType::COUNT) + 1));
        switch (m_CurrentGBufferType)
        {
#if SIMPLE_GBUFFER
        case eGBufferDataType::RT0_WORLD_POS:
            OutputDebugString(L"\tRT0: WORLD POS PASS\n");
            break;
        case eGBufferDataType::RT1_NORMAL:
            OutputDebugString(L"\tRT1: NORMAL PASS\n");
            break;
        case eGBufferDataType::RT1_DEPTH:
            OutputDebugString(L"\tRT1: DEPTH PASS\n");
            break;
        case eGBufferDataType::RT2_ALBEDO:
            OutputDebugString(L"\tRT2: ALBEDO PASS\n");
            break;
        case eGBufferDataType::RT3_SPECULAR:
            OutputDebugString(L"\tRT3: SPECULAR PASS\n");
            break;
        case eGBufferDataType::RT3_GLOSS:
            OutputDebugString(L"\tRT3: GLOSS PASS\n");
            break;
#elif KILLZONE_GBUFFER
        case eGBufferDataType::DEPTH:
            OutputDebugString(L"\tDEPTH PASS\n");
            break;
        case eGBufferDataType::RT0_LIGHT_ACCUMULATION:
            OutputDebugString(L"\tRT0: LIGHT ACCUMULATION PASS\n");
            break;
        //case eGBufferDataType::RT0_INTENSITY:
        //    OutputDebugString(L"\tRT0: INTENSITY PASS\n");
        //    break;
        case eGBufferDataType::RT1_NORMAL:
            OutputDebugString(L"\tRT1: NORMAL PASS\n");
            break;
        case eGBufferDataType::RT2_MOTION_VECTORS:
            OutputDebugString(L"\tRT2: MOTION VECTORS PASS\n");
            break;
        case eGBufferDataType::RT2_SPEC_INTENSITY:
            OutputDebugString(L"\tRT2: SPECULAR INTENSITY PASS\n");
            break;
        case eGBufferDataType::RT3_DIFFUSE_ALBEDO:
            OutputDebugString(L"\tRT3: DIFFUSE ALBEDO PASS\n");
            break;
        case eGBufferDataType::RT3_SUN_OCCLUSION:
            OutputDebugString(L"\tRT3: SUN OCCLUSION PASS\n");
            break;
        case eGBufferDataType::LIGHT_DENSITY:
            OutputDebugString(L"\tLIGHT DENSITY PASS\n");
            break;
        case eGBufferDataType::FALSE_POSITIVE_RATE:
            OutputDebugString(L"\tFALSE POSITIVE RATE PASS\n");
            break;
#endif
        case eGBufferDataType::COUNT:
            OutputDebugString(L"\tGBUFFER PASS\n");
            break;
        default:
            ASSERT(false);
            break;
        }
        break;
    case eRenderType::DEFERRED_THIN:
        m_CurrentThinGBufferType = static_cast<eThinGBufferDataType>((static_cast<size_t>(m_CurrentThinGBufferType) + static_cast<size_t>(eThinGBufferDataType::COUNT)) % (static_cast<size_t>(eThinGBufferDataType::COUNT) + 1));
        switch (m_CurrentThinGBufferType)
        {
        case eThinGBufferDataType::DEPTH:
            OutputDebugString(L"\tDEPTH PASS\n");
            break;
        case eThinGBufferDataType::RT0_LIGHT_ACCUMULATION:
            OutputDebugString(L"\tRT0: LIGHT ACCUMULATION PASS\n");
            break;
        case eThinGBufferDataType::RT1_NORMAL:
            OutputDebugString(L"\tRT1: NORMAL PASS\n");
            break;
        case eThinGBufferDataType::RT1_GLOSSINESS:
            OutputDebugString(L"\tRT1: GLOSSINESS PASS\n");
            break;
        case eThinGBufferDataType::RT2_DIFFUSE_ALBEDO:
            OutputDebugString(L"\tRT2: DIFFUSE ALBEDO PASS\n");
            break;
        case eThinGBufferDataType::RT2_SPEC_INTENSITY:
            OutputDebugString(L"\tRT2: SPECULAR INTENSITY PASS\n");
            break;
        case eThinGBufferDataType::LIGHT_DENSITY:
            OutputDebugString(L"\tLIGHT DENSITY PASS\n");
            break;
        case eThinGBufferDataType::FALSE_POSITIVE_RATE:
            OutputDebugString(L"\tFALSE POSITIVE RATE PASS\n");
            break;
        case eThinGBufferDataType::COUNT:
            OutputDebugString(L"\tGBUFFER PASS\n");
            break;
        default:
            ASSERT(false);
            break;
        }
        break;
    case eRenderType::COUNT:
        break;
    default:
        break;
    }
}

void Sponza::SetNextLightType()
{
    m_CurrentLightType = static_cast<eLightType>((static_cast<size_t>(m_CurrentLightType) + 1) % (static_cast<size_t>(eLightType::COUNT)));
    switch (m_CurrentLightType)
    {
    case eLightType::DEFAULT:
        OutputDebugString(L"\tDEFAULT LIGHTS PASS\n");
        break;
    case eLightType::TILED:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        OutputDebugString(L"\tTILED LIGHTS PASS\n");
    }
    break;
    case eLightType::TILED_2_5:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        OutputDebugString(L"\tTILED 2.5D CULLING LIGHTS PASS\n");
    }
    break;
    case eLightType::TILED_2_5_AABB:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        OutputDebugString(L"\tTILED 2.5D AABB CULLING LIGHTS PASS\n");
    }
    break;
    case eLightType::TILED_DICE:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
        OutputDebugString(L"\tDICE TILED LIGHTS PASS\n");
        break;
    case eLightType::TILED_DICE_2_5:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
        OutputDebugString(L"\tDICE 2.5 CULLING TILED LIGHTS PASS\n");
        break;
    case eLightType::TILED_DICE_2_5_AABB:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
        OutputDebugString(L"\tDICE 2.5 AABB CULLING TILED LIGHTS PASS\n");
        break;
    case eLightType::TILED_INTEL:
        {
            uint32_t DestCount = 2;
            uint32_t SourceCounts[] = { 1, 1 };

            D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
            {
                Lighting::m_LightGrid.GetSRV(),         // 16
                Lighting::m_LightGridBitMask.GetSRV(),  // 17
                //g_aSceneGBuffers[0].GetSRV(),
                //g_aSceneGBuffers[1].GetSRV(),
            };

            g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        OutputDebugString(L"\tINTEL TILED LIGHTS PASS\n");
        break;
    case eLightType::CLUSTERED:
    {
        uint32_t DestCount = 2;
        //uint32_t DestCount = 1;
        uint32_t SourceCounts[] = { 1, 1, };
        //uint32_t SourceCounts[] = { 1, };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightCluster.GetSRV(),         // 16
            Lighting::m_LightClusterBitMask.GetSRV(),  // 17
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        OutputDebugString(L"\tCLUSTERED LIGHTS PASS\n");
    }
    break;
    case eLightType::COUNT:
        // Intentional fallthrough
    default:
        ASSERT(false);
        break;
    }
}

void Sponza::SetPreviousLightType()
{
    m_CurrentLightType = static_cast<eLightType>((static_cast<size_t>(m_CurrentLightType) + static_cast<size_t>(eLightType::COUNT) - 1) % (static_cast<size_t>(eLightType::COUNT)));
    switch (m_CurrentLightType)
    {
    case eLightType::DEFAULT:
        OutputDebugString(L"\tDEFAULT LIGHTS PASS\n");
        break;
    case eLightType::TILED:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        OutputDebugString(L"\tTILED LIGHTS PASS\n");
    }
    break;
    case eLightType::TILED_2_5:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        OutputDebugString(L"\tTILED 2.5D CULLING LIGHTS PASS\n");
    }
    break;
    case eLightType::TILED_2_5_AABB:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        OutputDebugString(L"\tTILED 2.5D AABB CULLING LIGHTS PASS\n");
    }
    break;
    case eLightType::TILED_DICE:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
        OutputDebugString(L"\tDICE TILED LIGHTS PASS\n");
        break;
    case eLightType::TILED_DICE_2_5:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
        OutputDebugString(L"\tDICE 2.5 CULLING TILED LIGHTS PASS\n");
        break;
    case eLightType::TILED_DICE_2_5_AABB:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
        OutputDebugString(L"\tDICE 2.5 AABB CULLING TILED LIGHTS PASS\n");
        break;
    case eLightType::TILED_INTEL:
    {
        uint32_t DestCount = 2;
        uint32_t SourceCounts[] = { 1, 1 };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightGrid.GetSRV(),         // 16
            Lighting::m_LightGridBitMask.GetSRV(),  // 17
            //g_aSceneGBuffers[0].GetSRV(),
            //g_aSceneGBuffers[1].GetSRV(),
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
        OutputDebugString(L"\tINTEL TILED LIGHTS PASS\n");
        break;
    case eLightType::CLUSTERED:
    {
        uint32_t DestCount = 2;
        //uint32_t DestCount = 1;
        uint32_t SourceCounts[] = { 1, 1, };
        //uint32_t SourceCounts[] = { 1, };

        D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
        {
            Lighting::m_LightCluster.GetSRV(),         // 16
            Lighting::m_LightClusterBitMask.GetSRV(),  // 17
        };

        g_Device->CopyDescriptors(1, &(Renderer::m_CommonTextures + 6 * Renderer::s_TextureHeap.GetDescriptorSize()), &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        OutputDebugString(L"\tCLUSTERED LIGHTS PASS\n");
    }
    break;
    case eLightType::COUNT:
        // Intentional fallthrough
    default:
        ASSERT(false);
        break;
    }
}

void Sponza::ToggleLightUpdate()
{
    if (!Lighting::m_bUpdateLightsToggle)
    {
        Lighting::ResetLights();
    }
    Lighting::m_bUpdateLightsToggle = !Lighting::m_bUpdateLightsToggle;
}

void Sponza::Cleanup( void )
{
    m_Model.Clear();
    Lighting::Shutdown();
    TextureManager::Shutdown();
}

void Sponza::RenderDeferredObjects(GraphicsContext& gfxContext, const Camera& camera, const Vector3& viewerPos)
{
    struct CommonConstants
    {
        UINT MaterialIdx;
        Matrix4 ModelToProjection;
        Matrix4 InvViewProj;
        Matrix4 InvProj;
        Matrix4 modelToShadow;
        XMFLOAT4 ViewerPos;
        uint32_t ViewportSize[2];
    } commonConstants;

    struct VSConstants
    {
        Matrix4 modelToProjection;
    } vsConstants;
    vsConstants.modelToProjection = camera.GetViewProjMatrix();
    commonConstants.ModelToProjection = camera.GetViewProjMatrix();

    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewProjMatrix()), camera.GetViewProjMatrix());
    XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetProjMatrix()), camera.GetProjMatrix());

    commonConstants.InvViewProj = Matrix4(
        Vector4(invViewProj.r[0]),
        Vector4(invViewProj.r[1]),
        Vector4(invViewProj.r[2]),
        Vector4(invViewProj.r[3])
    );
    commonConstants.InvProj = Matrix4(
        Vector4(invProj.r[0]),
        Vector4(invProj.r[1]),
        Vector4(invProj.r[2]),
        Vector4(invProj.r[3])
    );

    commonConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMStoreFloat4(&commonConstants.ViewerPos, viewerPos);
    commonConstants.ViewportSize[0] = g_SceneColorBuffer.GetWidth();
    commonConstants.ViewportSize[1] = g_SceneColorBuffer.GetHeight();

    //gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    gfxContext.SetDynamicConstantBufferView(Renderer::kMeshConstants, sizeof(vsConstants), &vsConstants);
    gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);

    gfxContext.SetDescriptorTable(Renderer::kGBufferSRVs, m_GBufferSRVs);
    gfxContext.DrawIndexed(NUM_GBUFFER_INDICES, 0, 0);
}

void Sponza::RenderDeferredObjectsThinGBuffer(GraphicsContext& gfxContext, const Camera& camera, const Vector3& viewerPos)
{
    struct CommonConstants
    {
        UINT MaterialIdx;
        Matrix4 ModelToProjection;
        Matrix4 InvViewProj;
        Matrix4 InvProj;
#if (Z_RECONSTRUCTION || SPHEREMAP_TRANSFORM)
        Matrix4 InvView;
#endif
        Matrix4 modelToShadow;
        XMFLOAT4 ViewerPos;
        uint32_t ViewportSize[2];
    } commonConstants;

    struct VSConstants
    {
        Matrix4 modelToProjection;
    } vsConstants;
    vsConstants.modelToProjection = camera.GetViewProjMatrix();
    commonConstants.ModelToProjection = camera.GetViewProjMatrix();

    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewProjMatrix()), camera.GetViewProjMatrix());
    XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetProjMatrix()), camera.GetProjMatrix());

    commonConstants.InvViewProj = Matrix4(
        Vector4(invViewProj.r[0]),
        Vector4(invViewProj.r[1]),
        Vector4(invViewProj.r[2]),
        Vector4(invViewProj.r[3])
    );
    commonConstants.InvProj = Matrix4(
        Vector4(invProj.r[0]),
        Vector4(invProj.r[1]),
        Vector4(invProj.r[2]),
        Vector4(invProj.r[3])
    );
#if (Z_RECONSTRUCTION || SPHEREMAP_TRANSFORM)
    XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewMatrix()), camera.GetViewMatrix());

    commonConstants.InvView = Matrix4(
        Vector4(invView.r[0]),
        Vector4(invView.r[1]),
        Vector4(invView.r[2]),
        Vector4(invView.r[3])
    );
#endif
    commonConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMStoreFloat4(&commonConstants.ViewerPos, viewerPos);
    commonConstants.ViewportSize[0] = g_SceneColorBuffer.GetWidth();
    commonConstants.ViewportSize[1] = g_SceneColorBuffer.GetHeight();

    //gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    gfxContext.SetDynamicConstantBufferView(Renderer::kMeshConstants, sizeof(vsConstants), &vsConstants);
    gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);

    gfxContext.SetDescriptorTable(Renderer::kGBufferSRVs, m_ThinGBufferSRVs);
    gfxContext.DrawIndexed(NUM_GBUFFER_INDICES, 0, 0);
}

void Sponza::RenderDeferredClusteredObjects(GraphicsContext& gfxContext, const Camera& camera, const Vector3& viewerPos)
{
    struct CommonConstants
    {
        UINT MaterialIdx;
        Matrix4 ModelToProjection;
        Matrix4 InvViewProj;
        Matrix4 InvProj;
        Matrix4 modelToShadow;
        XMFLOAT4 ViewerPos;
        UINT ViewportSize[2];
        float NearZ;
        float FarZ;
    } commonConstants;

    struct VSConstants
    {
        Matrix4 modelToProjection;
    } vsConstants;
    vsConstants.modelToProjection = camera.GetViewProjMatrix();
    commonConstants.ModelToProjection = camera.GetViewProjMatrix();

    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewProjMatrix()), camera.GetViewProjMatrix());
    XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetProjMatrix()), camera.GetProjMatrix());

    commonConstants.InvViewProj = Matrix4(
        Vector4(invViewProj.r[0]),
        Vector4(invViewProj.r[1]),
        Vector4(invViewProj.r[2]),
        Vector4(invViewProj.r[3])
    );
    commonConstants.InvProj = Matrix4(
        Vector4(invProj.r[0]),
        Vector4(invProj.r[1]),
        Vector4(invProj.r[2]),
        Vector4(invProj.r[3])
    );

    commonConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMStoreFloat4(&commonConstants.ViewerPos, viewerPos);
    commonConstants.ViewportSize[0] = g_SceneColorBuffer.GetWidth();
    commonConstants.ViewportSize[1] = g_SceneColorBuffer.GetHeight();
    commonConstants.NearZ = camera.GetNearClip();
    commonConstants.FarZ = camera.GetFarClip();

    //gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    gfxContext.SetDynamicConstantBufferView(Renderer::kMeshConstants, sizeof(vsConstants), &vsConstants);
    gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);

    gfxContext.SetDescriptorTable(Renderer::kGBufferSRVs, m_GBufferSRVs);
    gfxContext.DrawIndexed(NUM_GBUFFER_INDICES, 0, 0);
}

void Sponza::RenderDeferredClusteredObjectsThinGBuffer(GraphicsContext& gfxContext, const Camera& camera, const Vector3& viewerPos)
{
    struct CommonConstants
    {
        UINT MaterialIdx;
        Matrix4 ModelToProjection;
        Matrix4 InvViewProj;
        Matrix4 InvProj;
#if (Z_RECONSTRUCTION || SPHEREMAP_TRANSFORM)
        Matrix4 InvView;
#endif
        Matrix4 modelToShadow;
        XMFLOAT4 ViewerPos;
        UINT ViewportSize[2];
        float NearZ;
        float FarZ;
    } commonConstants;

    struct VSConstants
    {
        Matrix4 modelToProjection;
    } vsConstants;
    vsConstants.modelToProjection = camera.GetViewProjMatrix();
    commonConstants.ModelToProjection = camera.GetViewProjMatrix();

    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewProjMatrix()), camera.GetViewProjMatrix());
    XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetProjMatrix()), camera.GetProjMatrix());

    commonConstants.InvViewProj = Matrix4(
        Vector4(invViewProj.r[0]),
        Vector4(invViewProj.r[1]),
        Vector4(invViewProj.r[2]),
        Vector4(invViewProj.r[3])
    );
    commonConstants.InvProj = Matrix4(
        Vector4(invProj.r[0]),
        Vector4(invProj.r[1]),
        Vector4(invProj.r[2]),
        Vector4(invProj.r[3])
    );
#if (Z_RECONSTRUCTION || SPHEREMAP_TRANSFORM)
    XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewMatrix()), camera.GetViewMatrix());

    commonConstants.InvView = Matrix4(
        Vector4(invView.r[0]),
        Vector4(invView.r[1]),
        Vector4(invView.r[2]),
        Vector4(invView.r[3])
    );
#endif
    commonConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMStoreFloat4(&commonConstants.ViewerPos, viewerPos);
    commonConstants.ViewportSize[0] = g_SceneColorBuffer.GetWidth();
    commonConstants.ViewportSize[1] = g_SceneColorBuffer.GetHeight();
    commonConstants.NearZ = camera.GetNearClip();
    commonConstants.FarZ = camera.GetFarClip();

    //gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    gfxContext.SetDynamicConstantBufferView(Renderer::kMeshConstants, sizeof(vsConstants), &vsConstants);
    gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);

    gfxContext.SetDescriptorTable(Renderer::kGBufferSRVs, m_ThinGBufferSRVs);
    gfxContext.DrawIndexed(NUM_GBUFFER_INDICES, 0, 0);
}

void Sponza::RenderObjects( GraphicsContext& gfxContext, const Camera& camera, const Vector3& viewerPos, eObjectFilter Filter )
{
    struct VSConstants
    {
        Matrix4 modelToProjection;
        Matrix4 modelToView;
        //Matrix4 modelToShadow;
        //XMFLOAT3 viewerPos;
    } vsConstants;
    vsConstants.modelToProjection = camera.GetViewProjMatrix();
    vsConstants.modelToView = camera.GetViewMatrix();
    //vsConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    //XMStoreFloat3(&vsConstants.viewerPos, viewerPos);

    struct CommonConstants
    {
        UINT MaterialIdx;
        Matrix4 ModelToProjection;
        Matrix4 InvViewProj;
        Matrix4 InvProj;
        Matrix4 modelToShadow;
        XMFLOAT3 ViewerPos;
    } commonConstants;
    commonConstants.ModelToProjection = camera.GetViewProjMatrix();
    commonConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMStoreFloat3(&commonConstants.ViewerPos, viewerPos);

    gfxContext.SetDynamicConstantBufferView(Renderer::kMeshConstants, sizeof(vsConstants), &vsConstants);
    gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);

    //__declspec(align(16)) uint32_t materialIdx = 0xFFFFFFFFul;
    commonConstants.MaterialIdx = 0xFFFFFFFFul;

    uint32_t VertexStride = m_Model.GetVertexStride();

    for (uint32_t meshIndex = 0; meshIndex < m_Model.GetMeshCount(); meshIndex++)
    {
        const ModelH3D::Mesh& mesh = m_Model.GetMesh(meshIndex);

        uint32_t indexCount = mesh.indexCount;
        uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
        uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

        if (mesh.materialIndex != commonConstants.MaterialIdx)
        {
            if ( m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kCutout) ||
                !m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kOpaque) )
                continue;

            commonConstants.MaterialIdx = mesh.materialIndex;
            gfxContext.SetDescriptorTable(Renderer::kMaterialSRVs, m_Model.GetSRVs(commonConstants.MaterialIdx));

            //gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(uint32_t), &materialIdx);
            gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);
        }

        gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
    }
}

void Sponza::RenderObjectsThinGBuffer(GraphicsContext& gfxContext, const Camera& camera, const Vector3& viewerPos, eObjectFilter Filter)
{
    struct VSConstants
    {
        Matrix4 modelToProjection;
        Matrix4 modelToView;
        //Matrix4 modelToShadow;
        //XMFLOAT3 viewerPos;
    } vsConstants;
    vsConstants.modelToProjection = camera.GetViewProjMatrix();
    vsConstants.modelToView = camera.GetViewMatrix();
    //vsConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    //XMStoreFloat3(&vsConstants.viewerPos, viewerPos);

    struct CommonConstants
    {
        UINT MaterialIdx;
        Matrix4 ModelToProjection;
        Matrix4 InvViewProj;
        Matrix4 InvProj;
#if (Z_RECONSTRUCTION || SPHEREMAP_TRANSFORM)
        Matrix4 ModelToView;
        Matrix4 InvView;
#endif
        Matrix4 modelToShadow;
        XMFLOAT3 ViewerPos;
    } commonConstants;
    commonConstants.ModelToProjection = camera.GetViewProjMatrix();
    commonConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMStoreFloat3(&commonConstants.ViewerPos, viewerPos);

#if (Z_RECONSTRUCTION || SPHEREMAP_TRANSFORM)
    commonConstants.ModelToView = camera.GetViewMatrix();
    XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewMatrix()), camera.GetViewMatrix());

    commonConstants.InvView = Matrix4(
        Vector4(invView.r[0]),
        Vector4(invView.r[1]),
        Vector4(invView.r[2]),
        Vector4(invView.r[3])
    );
#endif

    gfxContext.SetDynamicConstantBufferView(Renderer::kMeshConstants, sizeof(vsConstants), &vsConstants);
    gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);

    //__declspec(align(16)) uint32_t materialIdx = 0xFFFFFFFFul;
    commonConstants.MaterialIdx = 0xFFFFFFFFul;

    uint32_t VertexStride = m_Model.GetVertexStride();

    for (uint32_t meshIndex = 0; meshIndex < m_Model.GetMeshCount(); meshIndex++)
    {
        const ModelH3D::Mesh& mesh = m_Model.GetMesh(meshIndex);

        uint32_t indexCount = mesh.indexCount;
        uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
        uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

        if (mesh.materialIndex != commonConstants.MaterialIdx)
        {
            if (m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kCutout) ||
                !m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kOpaque))
                continue;

            commonConstants.MaterialIdx = mesh.materialIndex;
            gfxContext.SetDescriptorTable(Renderer::kMaterialSRVs, m_Model.GetSRVs(commonConstants.MaterialIdx));

            //gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(uint32_t), &materialIdx);
            gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);
        }

        gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
    }
}

void Sponza::RenderObjects(GraphicsContext& gfxContext, const Matrix4& ViewProjMatrix, const Vector3& viewerPos, eObjectFilter Filter)
{
    struct VSConstants
    {
        Matrix4 modelToProjection;
        //Matrix4 modelToView;
        //Matrix4 modelToShadow;
        //XMFLOAT3 viewerPos;
    } vsConstants;
    vsConstants.modelToProjection = ViewProjMatrix;
    //vsConstants.modelToView = camera.GetViewMatrix();
    //vsConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    //XMStoreFloat3(&vsConstants.viewerPos, viewerPos);

    struct CommonConstants
    {
        UINT MaterialIdx;
        Matrix4 ModelToProjection;
        Matrix4 InvViewProj;
        //Matrix4 InvView;
        Matrix4 modelToShadow;
        XMFLOAT3 ViewerPos;
    } commonConstants;
    commonConstants.ModelToProjection = ViewProjMatrix;
    commonConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMStoreFloat3(&commonConstants.ViewerPos, viewerPos);

    gfxContext.SetDynamicConstantBufferView(Renderer::kMeshConstants, sizeof(vsConstants), &vsConstants);
    //gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);

    //__declspec(align(16)) uint32_t materialIdx = 0xFFFFFFFFul;
    commonConstants.MaterialIdx = 0xFFFFFFFFul;

    uint32_t VertexStride = m_Model.GetVertexStride();

    for (uint32_t meshIndex = 0; meshIndex < m_Model.GetMeshCount(); meshIndex++)
    {
        const ModelH3D::Mesh& mesh = m_Model.GetMesh(meshIndex);

        uint32_t indexCount = mesh.indexCount;
        uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
        uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

        //if (mesh.materialIndex != materialIdx)
        if (mesh.materialIndex != commonConstants.MaterialIdx)
        {
            if (m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kCutout) ||
                !m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kOpaque))
                continue;

            //materialIdx = mesh.materialIndex;
            commonConstants.MaterialIdx = mesh.materialIndex;
            //gfxContext.SetDescriptorTable(Renderer::kMaterialSRVs, m_Model.GetSRVs(materialIdx));
            gfxContext.SetDescriptorTable(Renderer::kMaterialSRVs, m_Model.GetSRVs(commonConstants.MaterialIdx));

            //gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(uint32_t), &materialIdx);
            gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);
        }

        gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
    }
}

void Sponza::RenderClusteredObjects(GraphicsContext& gfxContext, const Camera& camera, const Vector3& viewerPos, eObjectFilter Filter)
{
    struct VSConstants
    {
        Matrix4 modelToProjection;
        Matrix4 modelToView;
        //Matrix4 modelToShadow;
        //XMFLOAT3 viewerPos;
    } vsConstants;
    vsConstants.modelToProjection = camera.GetViewProjMatrix();
    vsConstants.modelToView = camera.GetViewMatrix();
    //vsConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    //XMStoreFloat3(&vsConstants.viewerPos, viewerPos);

    struct CommonConstants
    {
        UINT MaterialIdx;
        Matrix4 ModelToProjection;
        Matrix4 InvViewProj;
        Matrix4 InvProj;
        Matrix4 modelToShadow;
        XMFLOAT4 ViewerPos;
        UINT ViewportSize[2];
        float NearZ;
        float FarZ;
    } commonConstants;
    commonConstants.ModelToProjection = camera.GetViewProjMatrix();
    commonConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewProjMatrix()), camera.GetViewProjMatrix());
    XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetProjMatrix()), camera.GetProjMatrix());

    commonConstants.InvViewProj = Matrix4(
        Vector4(invViewProj.r[0]),
        Vector4(invViewProj.r[1]),
        Vector4(invViewProj.r[2]),
        Vector4(invViewProj.r[3])
    );
    commonConstants.InvProj = Matrix4(
        Vector4(invProj.r[0]),
        Vector4(invProj.r[1]),
        Vector4(invProj.r[2]),
        Vector4(invProj.r[3])
    );
    XMStoreFloat4(&commonConstants.ViewerPos, viewerPos);
    commonConstants.ViewportSize[0] = g_SceneColorBuffer.GetWidth();
    commonConstants.ViewportSize[1] = g_SceneColorBuffer.GetHeight();
    commonConstants.NearZ = camera.GetNearClip();
    commonConstants.FarZ = camera.GetFarClip();

    gfxContext.SetDynamicConstantBufferView(Renderer::kMeshConstants, sizeof(vsConstants), &vsConstants);
    gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);

    //__declspec(align(16)) uint32_t materialIdx = 0xFFFFFFFFul;
    commonConstants.MaterialIdx = 0xFFFFFFFFul;

    uint32_t VertexStride = m_Model.GetVertexStride();

    for (uint32_t meshIndex = 0; meshIndex < m_Model.GetMeshCount(); meshIndex++)
    {
        const ModelH3D::Mesh& mesh = m_Model.GetMesh(meshIndex);

        uint32_t indexCount = mesh.indexCount;
        uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
        uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

        if (mesh.materialIndex != commonConstants.MaterialIdx)
        {
            if (m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kCutout) ||
                !m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kOpaque))
                continue;

            commonConstants.MaterialIdx = mesh.materialIndex;
            gfxContext.SetDescriptorTable(Renderer::kMaterialSRVs, m_Model.GetSRVs(commonConstants.MaterialIdx));

            //gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(uint32_t), &materialIdx);
            gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);
        }

        gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
    }
}

void Sponza::RenderLightShadows(GraphicsContext& gfxContext, const Camera& camera)
{
    using namespace Lighting;

    ScopedTimer _prof(L"RenderLightShadows", gfxContext);

    for (size_t i = 0; i < MaxLights / 32; ++i)
    {
        if (m_LightShadowParity[i])
        {
            DWORD dwIndex = 0;

            while (_BitScanForward(&dwIndex, m_LightShadowParity[i]))
            {
                m_LightShadowTempBuffer.BeginRendering(gfxContext);
                {
                    gfxContext.SetPipelineState(m_ShadowPSO);
                    RenderObjects(gfxContext, m_LightShadowMatrix[dwIndex + i * 32], camera.GetPosition(), kOpaque);
                    gfxContext.SetPipelineState(m_CutoutShadowPSO);
                    RenderObjects(gfxContext, m_LightShadowMatrix[dwIndex + i * 32], camera.GetPosition(), kCutout);
                }

                gfxContext.TransitionResource(m_LightShadowTempBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
                gfxContext.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_COPY_DEST);

                gfxContext.CopySubresource(m_LightShadowArray, dwIndex + i * 32, m_LightShadowTempBuffer, 0);

                gfxContext.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                m_LightShadowParity[i] &= ~(1u << dwIndex);
            }
        }
    }
}

void Sponza::RenderScene(
    GraphicsContext& gfxContext,
    const Camera& camera,
    const D3D12_VIEWPORT& viewport,
    const D3D12_RECT& scissor,
    //const Math::OrientedBox& boundingBox,
    bool skipDiffusePass,
    bool skipShadowMap)
{
    Renderer::UpdateGlobalDescriptors();

    uint32_t FrameIndex = TemporalEffects::GetFrameIndexMod2();

    float costheta = cosf(m_SunOrientation);
    float sintheta = sinf(m_SunOrientation);
    float cosphi = cosf(m_SunInclination * 3.14159f * 0.5f);
    float sinphi = sinf(m_SunInclination * 3.14159f * 0.5f);
    m_SunDirection = Normalize(Vector3( costheta * cosphi, sinphi, sintheta * cosphi ));

    __declspec(align(16)) struct
    {
        Vector3 sunDirection;
        Vector3 sunLight;
        Vector3 ambientLight;
        float ShadowTexelSize[4];

        float InvTileDim[4];
        uint32_t TileCount[4];
        uint32_t FirstLightIndex[4];

		uint32_t FrameIndexMod2;
  //      float Scale[3];
  //      float Bias[3];
    } psConstants;

    psConstants.sunDirection = m_SunDirection;
    psConstants.sunLight = Vector3(1.0f, 1.0f, 1.0f) * m_SunLightIntensity;
    psConstants.ambientLight = Vector3(1.0f, 1.0f, 1.0f) * m_AmbientIntensity;
    psConstants.ShadowTexelSize[0] = 1.0f / g_ShadowBuffer.GetWidth();
    switch (m_CurrentLightType)
    {
    case eLightType::DEFAULT:
        // intentional fallthrough
    case eLightType::TILED:
        // intentional fallthrough
    case eLightType::TILED_2_5:
        // intentional fallthrough
    case eLightType::TILED_2_5_AABB:
        // intentional fallthrough
    case eLightType::TILED_DICE:
        // intentional fallthrough
    case eLightType::TILED_DICE_2_5:
        // intentional fallthrough
    case eLightType::TILED_DICE_2_5_AABB:
        // intentional fallthrough
    case eLightType::TILED_INTEL:
        psConstants.InvTileDim[0] = 1.0f / Lighting::LightGridDim;
        psConstants.InvTileDim[1] = 1.0f / Lighting::LightGridDim;
        psConstants.TileCount[0] = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), Lighting::LightGridDim);
        psConstants.TileCount[1] = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), Lighting::LightGridDim);
        psConstants.FirstLightIndex[0] = Lighting::m_FirstConeLight;
        psConstants.FirstLightIndex[1] = Lighting::m_FirstConeShadowedLight;
        break;
    case eLightType::CLUSTERED:
    {
        psConstants.InvTileDim[0] = 1.0f / Lighting::aLightClusterDimensions[static_cast<size_t>(Lighting::LightClusterType)][0];
        psConstants.InvTileDim[1] = 1.0f / Lighting::aLightClusterDimensions[static_cast<size_t>(Lighting::LightClusterType)][0];
        psConstants.InvTileDim[2] = 1.0f / Lighting::aLightClusterDimensions[static_cast<size_t>(Lighting::LightClusterType)][1];
        psConstants.TileCount[0] = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), Lighting::aLightClusterDimensions[static_cast<size_t>(Lighting::LightClusterType)][0]);
        psConstants.TileCount[1] = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), Lighting::aLightClusterDimensions[static_cast<size_t>(Lighting::LightClusterType)][0]);
        psConstants.TileCount[2] = Lighting::aLightClusterDimensions[static_cast<size_t>(Lighting::LightClusterType)][1];
        psConstants.TileCount[3] = psConstants.TileCount[0] * psConstants.TileCount[1] * psConstants.TileCount[2];
        psConstants.FirstLightIndex[0] = Lighting::m_FirstConeLight;
        psConstants.FirstLightIndex[1] = Lighting::m_FirstConeShadowedLight;
        //Vector3 scale = Vector3(
        //    static_cast<float>(Lighting::aLightClusterDimensions[static_cast<size_t>(Lighting::LightClusterType)][0]), 
        //    static_cast<float>(Lighting::aLightClusterDimensions[static_cast<size_t>(Lighting::LightClusterType)][0]), 
        //    static_cast<float>(Lighting::aLightClusterDimensions[static_cast<size_t>(Lighting::LightClusterType)][1])
        //) / (m_Model.GetBoundingBox().GetMax() - m_Model.GetBoundingBox().GetMin());
        //psConstants.Scale[0] = scale.GetX();
        //psConstants.Scale[1] = scale.GetY();
        //psConstants.Scale[2] = scale.GetZ();
        //Vector3 bias = -scale * m_Model.GetBoundingBox().GetMin();
        //psConstants.Bias[0] = bias.GetX();
        //psConstants.Bias[1] = bias.GetY();
        //psConstants.Bias[2] = bias.GetZ();
    }
    break;
    case eLightType::COUNT:
        // intentional fallthrough
    default:
        ASSERT(false);
        break;
    }
	psConstants.FrameIndexMod2 = FrameIndex;

    // Set the default state for command lists
    auto& pfnSetupGraphicsState = [&](void)
    {
        gfxContext.SetRootSignature(Renderer::m_RootSig);
        gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Renderer::s_TextureHeap.GetHeapPointer());
        gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        gfxContext.SetIndexBuffer(m_Model.GetIndexBuffer());
        gfxContext.SetVertexBuffer(0, m_Model.GetVertexBuffer());
    };

    pfnSetupGraphicsState();

    RenderLightShadows(gfxContext, camera);

    {
        ScopedTimer _prof(L"Z PrePass", gfxContext);

        gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);

        {
            ScopedTimer _prof2(L"Opaque", gfxContext);
            {
                gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
                gfxContext.ClearDepth(g_SceneDepthBuffer);
                gfxContext.SetPipelineState(m_DepthPSO);
                gfxContext.SetDepthStencilTarget(g_SceneDepthBuffer.GetDSV());
                gfxContext.SetViewportAndScissor(viewport, scissor);
            }
            RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), kOpaque );
        }

        {
            ScopedTimer _prof2(L"Cutout", gfxContext);
            {
                gfxContext.SetPipelineState(m_CutoutDepthPSO);
            }
            RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), kCutout );
        }
    }

    SSAO::Render(gfxContext, camera);

    if (!skipDiffusePass)
    {
        switch (m_CurrentLightType)
        {
        case eLightType::DEFAULT:
            break;
        case eLightType::TILED:
            // intentional fallthrough
        case eLightType::TILED_2_5:
            // intentional fallthrough
        case eLightType::TILED_2_5_AABB:
            Lighting::FillLightGrid(gfxContext, camera, m_CurrentLightType);
            //NestedTim::
            break;
        case eLightType::TILED_DICE:
            // intentional fallthrough
        case eLightType::TILED_DICE_2_5:
            // intentional fallthrough
        case eLightType::TILED_DICE_2_5_AABB:
            // intentional fallthrough
        case eLightType::TILED_INTEL:
            if (m_CurrentRenderType == eRenderType::FORWARD)
            {
                Lighting::FillLightGrid(gfxContext, camera, m_CurrentLightType);
            }
            break;
        case eLightType::CLUSTERED:
            Lighting::FillLightGrid(gfxContext, camera, m_CurrentLightType);
            //Lighting::FillLightCluster(gfxContext, camera, m_Model.GetBoundingBox());
            break;
        case eLightType::COUNT:
            // intentional fallthrough
        default:
            ASSERT(false);
            break;
        }

        if (!SSAO::DebugDraw)
        {
            ScopedTimer _prof(L"Main Render", gfxContext);
            {
                gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
                gfxContext.TransitionResource(g_SceneNormalBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
                for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
                {
                    gfxContext.TransitionResource(g_aSceneGBuffers[i], D3D12_RESOURCE_STATE_RENDER_TARGET, true);
                    gfxContext.ClearColor(g_aSceneGBuffers[i]);
                }
                for (size_t i = 0; i < static_cast<size_t>(eThinGBufferType::COUNT); ++i)
                {
                    gfxContext.TransitionResource(g_aSceneThinGBuffers[i], D3D12_RESOURCE_STATE_RENDER_TARGET, true);
                    gfxContext.ClearColor(g_aSceneThinGBuffers[i]);
                }
                gfxContext.ClearColor(g_SceneColorBuffer);
            }
        }
    }

    if (!skipShadowMap)
    {
        if (!SSAO::DebugDraw)
        {
            pfnSetupGraphicsState();
            {
                ScopedTimer _prof2(L"Render Shadow Map", gfxContext);

                m_SunShadow.UpdateMatrix(-m_SunDirection, Vector3(0, -500.0f, 0), Vector3(ShadowDimX, ShadowDimY, ShadowDimZ),
                    (uint32_t)g_ShadowBuffer.GetWidth(), (uint32_t)g_ShadowBuffer.GetHeight(), 16);

                g_ShadowBuffer.BeginRendering(gfxContext);
                gfxContext.SetPipelineState(m_ShadowPSO);
                RenderObjects(gfxContext, m_SunShadow.GetViewProjMatrix(), camera.GetPosition(), kOpaque);
                gfxContext.SetPipelineState(m_CutoutShadowPSO);
                RenderObjects(gfxContext, m_SunShadow.GetViewProjMatrix(), camera.GetPosition(), kCutout);
                g_ShadowBuffer.EndRendering(gfxContext);
            }
        }
    }

    if (!skipDiffusePass)
    {
        if (!SSAO::DebugDraw)
        {
            if (SSAO::AsyncCompute)
            {
                gfxContext.Flush();
                pfnSetupGraphicsState();

                // Make the 3D queue wait for the Compute queue to finish SSAO
                g_CommandManager.GetGraphicsQueue().StallForProducer(g_CommandManager.GetComputeQueue());
            }

            {
                //ScopedTimer _prof2(L"Render Color", gfxContext);
                ScopedTimer _prof2(
                    m_CurrentRenderType == eRenderType::FORWARD ? 
                        A_SZ_FORWARD_PROF_NAME[static_cast<size_t>(m_CurrentLightType)][static_cast<size_t>(m_CurrentForwardType)] :
                        m_CurrentRenderType == eRenderType::DEFERRED ? 
                        A_SZ_DEFERRED_PROF_NAME[static_cast<size_t>(m_CurrentLightType)][static_cast<size_t>(m_CurrentGBufferType)] :
                        A_SZ_DEFERRED_THIN_PROF_NAME[static_cast<size_t>(m_CurrentLightType)][static_cast<size_t>(m_CurrentThinGBufferType)],
                    gfxContext
                );

                {
                    ScopedTimer _opaque(
                       L"OPAQUE",
                        gfxContext
                    );
                    //gfxContext.SetPipelineState(m_ModelPSO);
                    //gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
                    //D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), g_SceneNormalBuffer.GetRTV() };
                    //gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                    //gfxContext.SetViewportAndScissor(viewport, scissor);
                    switch (m_CurrentRenderType)
                    {
                    case eRenderType::FORWARD:
                    {
                        ScopedTimer _prof3(L"Forward Opaque", gfxContext);
                        gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

                        gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);
                        gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);
                        gfxContext.SetPipelineState(m_aForwardPSOs[static_cast<size_t>(m_CurrentLightType)][static_cast<size_t>(m_CurrentForwardType)]);
                        gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
                        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), g_SceneNormalBuffer.GetRTV() };
                        gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                        gfxContext.SetViewportAndScissor(viewport, scissor);

                        if (m_CurrentLightType == eLightType::CLUSTERED)
                        {
                            RenderClusteredObjects(gfxContext, camera, camera.GetPosition(), Sponza::kOpaque);
                        }
                        else
                        {
                            RenderObjects(gfxContext, camera, camera.GetPosition(), Sponza::kOpaque);
                        }
                    }
                    break;
                    case eRenderType::DEFERRED:
                    {
                        {
                            ScopedTimer _prof3(L"Geometry Phase", gfxContext);
                            gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

                            gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);
                            gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);
                            std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
                            rtvs.reserve(static_cast<size_t>(eGBufferType::COUNT));
                            for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
                            {
                                rtvs.push_back(g_aSceneGBuffers[i].GetRTV());
                            }
                            gfxContext.SetPipelineState(m_GBufferPSO);
                            gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
                            gfxContext.SetRenderTargets(rtvs.size(), rtvs.data(), g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                            gfxContext.SetViewportAndScissor(viewport, scissor);
                            RenderObjects(gfxContext, camera, camera.GetPosition(), Sponza::kOpaque);
                            //RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kOpaque);
                        }

                        switch (m_CurrentLightType)
                        {
                        case eLightType::DEFAULT:
                            // intentional fallthrough
                        case eLightType::TILED:
                            // intentional fallthrough
                        case eLightType::TILED_2_5:
                            // intentional fallthrough
                        case eLightType::TILED_2_5_AABB:
                        {
                            ScopedTimer _prof4(L"Lighting Phase", gfxContext);
                            for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
                            {
                                gfxContext.TransitionResource(g_aSceneGBuffers[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
                            }

                            gfxContext.SetIndexBuffer(m_GBufferIndexBuffer);
                            gfxContext.SetVertexBuffer(0, m_GBufferVertexBuffer);
                            gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);
                            gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);
                            gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
                            gfxContext.SetPipelineState(m_aGBufferPSOs[static_cast<size_t>(m_CurrentLightType)][static_cast<size_t>(m_CurrentGBufferType)]);
                            gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
                            gfxContext.SetViewportAndScissor(viewport, scissor);
                            RenderDeferredObjects(gfxContext, camera, camera.GetPosition());
                        }
                        gfxContext.SetIndexBuffer(m_Model.GetIndexBuffer());
                        gfxContext.SetVertexBuffer(0, m_Model.GetVertexBuffer());
                        break;
                        case eLightType::TILED_DICE:
                            // intentional fallthrough
                        case eLightType::TILED_DICE_2_5:
                            // intentional fallthrough
                        case eLightType::TILED_DICE_2_5_AABB:
                            // intentional fallthrough
                        case eLightType::TILED_INTEL:
                        {
                            ScopedTimer _prof4(L"Lighting Phase", gfxContext);
                            Lighting::FillAndShadeLightGrid(gfxContext, camera, m_GBufferSRVs, m_CurrentLightType, m_CurrentGBufferType);
                            pfnSetupGraphicsState();
                        }
                        break;
                        case eLightType::CLUSTERED:
                        {
                            ScopedTimer _prof4(L"Lighting Phase", gfxContext);
                            for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
                            {
                                gfxContext.TransitionResource(g_aSceneGBuffers[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
                            }

                            gfxContext.SetIndexBuffer(m_GBufferIndexBuffer);
                            gfxContext.SetVertexBuffer(0, m_GBufferVertexBuffer);
                            gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);
                            gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);
                            gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
                            gfxContext.SetPipelineState(m_aGBufferPSOs[static_cast<size_t>(m_CurrentLightType)][static_cast<size_t>(m_CurrentGBufferType)]);
                            gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
                            gfxContext.SetViewportAndScissor(viewport, scissor);
                            RenderDeferredClusteredObjects(gfxContext, camera, camera.GetPosition());
                        }
                        gfxContext.SetIndexBuffer(m_Model.GetIndexBuffer());
                        gfxContext.SetVertexBuffer(0, m_Model.GetVertexBuffer());
                        break;
                        case eLightType::COUNT:
                            // intentional fallthrough
                        default:
                            ASSERT(false);
                            break;
                        }
                        
                    }
                    break;
                    case eRenderType::DEFERRED_THIN:
                    {
                        {
                            ScopedTimer _prof3(L"Geometry Phase", gfxContext);
                            gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

                            gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);
                            gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);
                            std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
                            rtvs.reserve(static_cast<size_t>(eThinGBufferType::COUNT));
                            for (size_t i = 0; i < static_cast<size_t>(eThinGBufferType::COUNT); ++i)
                            {
                                rtvs.push_back(g_aSceneThinGBuffers[i].GetRTV());
                            }
                            gfxContext.SetPipelineState(m_ThinGBufferPSO);
                            gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
                            gfxContext.SetRenderTargets(rtvs.size(), rtvs.data(), g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                            gfxContext.SetViewportAndScissor(viewport, scissor);
                            RenderObjectsThinGBuffer(gfxContext, camera, camera.GetPosition(), Sponza::kOpaque);
                            //RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kOpaque);
                        }

                        switch (m_CurrentLightType)
                        {
                        case eLightType::DEFAULT:
                            // intentional fallthrough
                        case eLightType::TILED:
                            // intentional fallthrough
                        case eLightType::TILED_2_5:
                            // intentional fallthrough
                        case eLightType::TILED_2_5_AABB:
                        {
                            ScopedTimer _prof4(L"Lighting Phase", gfxContext);
                            for (size_t i = 0; i < static_cast<size_t>(eThinGBufferType::COUNT); ++i)
                            {
                                gfxContext.TransitionResource(g_aSceneThinGBuffers[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
                            }

                            gfxContext.SetIndexBuffer(m_GBufferIndexBuffer);
                            gfxContext.SetVertexBuffer(0, m_GBufferVertexBuffer);
                            gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);
                            gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);
                            gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
                            gfxContext.SetPipelineState(m_aThinGBufferPSOs[static_cast<size_t>(m_CurrentLightType)][static_cast<size_t>(m_CurrentThinGBufferType)]);
                            gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
                            gfxContext.SetViewportAndScissor(viewport, scissor);
                            RenderDeferredObjectsThinGBuffer(gfxContext, camera, camera.GetPosition());
                        }
                        gfxContext.SetIndexBuffer(m_Model.GetIndexBuffer());
                        gfxContext.SetVertexBuffer(0, m_Model.GetVertexBuffer());
                        break;
                        case eLightType::TILED_DICE:
                            // intentional fallthrough
                        case eLightType::TILED_DICE_2_5:
                            // intentional fallthrough
                        case eLightType::TILED_DICE_2_5_AABB:
                            // intentional fallthrough
                        case eLightType::TILED_INTEL:
                        {
                            ScopedTimer _prof4(L"Lighting Phase", gfxContext);
                            Lighting::FillAndShadeLightGridThinGBuffer(gfxContext, camera, m_ThinGBufferSRVs, m_CurrentLightType, m_CurrentThinGBufferType);
                            pfnSetupGraphicsState();
                        }
                        break;
                        case eLightType::CLUSTERED:
                        {
                            ScopedTimer _prof4(L"Lighting Phase", gfxContext);
                            for (size_t i = 0; i < static_cast<size_t>(eThinGBufferType::COUNT); ++i)
                            {
                                gfxContext.TransitionResource(g_aSceneThinGBuffers[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
                            }

                            gfxContext.SetIndexBuffer(m_GBufferIndexBuffer);
                            gfxContext.SetVertexBuffer(0, m_GBufferVertexBuffer);
                            gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);
                            gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);
                            gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
                            gfxContext.SetPipelineState(m_aThinGBufferPSOs[static_cast<size_t>(m_CurrentLightType)][static_cast<size_t>(m_CurrentThinGBufferType)]);
                            gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
                            gfxContext.SetViewportAndScissor(viewport, scissor);
                            RenderDeferredClusteredObjectsThinGBuffer(gfxContext, camera, camera.GetPosition());
                        }
                        gfxContext.SetIndexBuffer(m_Model.GetIndexBuffer());
                        gfxContext.SetVertexBuffer(0, m_Model.GetVertexBuffer());
                        break;
                        case eLightType::COUNT:
                            // intentional fallthrough
                        default:
                            ASSERT(false);
                            break;
                        }

                    }
                    break;
                    case eRenderType::COUNT:
                        // intentional fallthrough
                    default:
                        ASSERT(false);
                        break;
                    }
                }

                //RenderObjects( gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kOpaque );

                ScopedTimer _prof5(L"Non-opaque Render", gfxContext);
                gfxContext.SetPipelineState(m_CutoutModelPSO);
                gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
                D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), g_SceneNormalBuffer.GetRTV() };
                gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                gfxContext.SetViewportAndScissor(viewport, scissor);
                RenderObjects( gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kCutout);
            }
        }
    }
}

void Sponza::Update(float deltaTime)
{
    Lighting::UpdateLights(deltaTime);
}
