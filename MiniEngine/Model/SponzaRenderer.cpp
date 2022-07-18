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

// Tiled
#include "CompiledShaders/ModelViewerTiledPS.h"

// Clustered
#include "CompiledShaders/ModelViewerClusteredPS.h"

#include "CompiledShaders/ModelViewerDiffuseAlbedoPS.h"
#include "CompiledShaders/ModelViewerGlossPS.h"
#include "CompiledShaders/ModelViewerNormalPS.h"
#include "CompiledShaders/ModelViewerSpecularIntensityPS.h"
#include "CompiledShaders/ModelViewerWorldPosPS.h"

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
// Tiled
#include "CompiledShaders/KillzoneGBufferLightTiledPS.h"

// Clustered
#include "CompiledShaders/KillzoneGBufferLightClusteredPS.h"

#include "CompiledShaders/KillzoneDeferredPS.h"
#include "CompiledShaders/KillzoneGBufferDepthPS.h"
#include "CompiledShaders/KillzoneGBufferDiffuseAlbedoPS.h"
#include "CompiledShaders/KillzoneGBufferGlossPS.h"
#include "CompiledShaders/KillzoneGBufferLightAccumulationPS.h"
#include "CompiledShaders/KillzoneGBufferMotionVectorPS.h"
#include "CompiledShaders/KillzoneGBufferNormalPS.h"
#include "CompiledShaders/KillzoneGBufferSpecularIntensityPS.h"
#include "CompiledShaders/KillzoneGBufferSunOcclusionPS.h"
#endif

using namespace Math;
using namespace Graphics;

namespace Sponza
{
    void RenderLightShadows(GraphicsContext& gfxContext, const Camera& camera);

    enum eObjectFilter { kOpaque = 0x1, kCutout = 0x2, kTransparent = 0x4, kAll = 0xF, kNone = 0x0 };
    void RenderDeferredObjects(GraphicsContext& Context, const Camera& camera, const Vector3& viewerPos, eObjectFilter Filter = kAll);
    void RenderObjects( GraphicsContext& Context, const Matrix4& ViewProjMat, const Vector3& viewerPos, eObjectFilter Filter = kAll );

    GraphicsPSO m_DepthPSO = { (L"Sponza: Depth PSO") };
    GraphicsPSO m_CutoutDepthPSO = { (L"Sponza: Cutout Depth PSO") };
    GraphicsPSO m_CutoutModelPSO = { (L"Sponza: Cutout Color PSO") };
    GraphicsPSO m_ShadowPSO(L"Sponza: Shadow PSO");
    GraphicsPSO m_CutoutShadowPSO(L"Sponza: Cutout Shadow PSO");

    GraphicsPSO m_GBufferPSO = { (L"Sponza: GBuffer PSO") };

    GraphicsPSO m_aForwardPSOs[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eForwardType::COUNT) + 1] =
    {
        {
            { (L"Sponza: Tiled World Position PSO") },
            { (L"Sponza: Tiled Diffuse Albedo PSO") },
            { (L"Sponza: Tiled Glossiness PSO") },
            { (L"Sponza: Tiled Normal PSO") },
            { (L"Sponza: Tiled Specular Intensity PSO") },
            { (L"Sponza: Tiled Color PSO") },
        },
        {
            { (L"Sponza: Clustered World Position PSO") },
            { (L"Sponza: Clustered Diffuse Albedo PSO") },
            { (L"Sponza: Clustered Glossiness PSO") },
            { (L"Sponza: Clustered Normal PSO") },
            { (L"Sponza: Clustered Specular Intensity PSO") },
            { (L"Sponza: Clustered Color PSO") },
        },
    };

    GraphicsPSO m_aGBufferPSOs[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eGBufferDataType::COUNT) + 1] =
    {
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
            { (L"Sponza: Tiled GBuffer Light PSO") },
#endif
        },
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
            { (L"Sponza: Clustered GBuffer Light PSO") },
#endif
        },
    };

    std::pair<const unsigned char* const, size_t> m_aForwardPixelShaders[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eForwardType::COUNT) + 1] =
    {
        {
            { g_pModelViewerWorldPosPS,            sizeof(g_pModelViewerWorldPosPS)            },
            { g_pModelViewerDiffuseAlbedoPS,       sizeof(g_pModelViewerDiffuseAlbedoPS)       },
            { g_pModelViewerGlossPS,               sizeof(g_pModelViewerGlossPS)               },
            { g_pModelViewerNormalPS,              sizeof(g_pModelViewerNormalPS)              },
            { g_pModelViewerSpecularIntensityPS,   sizeof(g_pModelViewerSpecularIntensityPS)   },
            { g_pModelViewerTiledPS,               sizeof(g_pModelViewerTiledPS)               },
        },
        {
            { g_pModelViewerWorldPosPS,             sizeof(g_pModelViewerWorldPosPS)            },
            { g_pModelViewerDiffuseAlbedoPS,        sizeof(g_pModelViewerDiffuseAlbedoPS)       },
            { g_pModelViewerGlossPS,                sizeof(g_pModelViewerGlossPS)               },
            { g_pModelViewerNormalPS,               sizeof(g_pModelViewerNormalPS)              },
            { g_pModelViewerSpecularIntensityPS,    sizeof(g_pModelViewerSpecularIntensityPS)   },
            { g_pModelViewerClusteredPS,            sizeof(g_pModelViewerClusteredPS)           },
        },
    };

    std::pair<const unsigned char* const, size_t> m_aGBufferPixelShaders[static_cast<size_t>(eLightType::COUNT)][static_cast<size_t>(eGBufferDataType::COUNT) + 1] =
    {
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
            { g_pKillzoneGBufferLightTiledPS,          sizeof(g_pKillzoneGBufferLightTiledPS)         },
#endif
        },
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
            { g_pKillzoneGBufferLightClusteredPS,      sizeof(g_pKillzoneGBufferLightClusteredPS)     },
#endif
        },
    };

    DescriptorHandle m_GBufferSRVs;

    eRenderType m_CurrentRenderType = eRenderType::FORWARD;
    eForwardType m_CurrentForwardType = eForwardType::COUNT;
    eGBufferDataType m_CurrentGBufferType = eGBufferDataType::COUNT;
    eLightType m_CurrentLightType = eLightType::TILED;

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
#endif
    m_GBufferPSO.SetPixelShader(g_pKillzoneDeferredPS, sizeof(g_pKillzoneDeferredPS));
    m_GBufferPSO.Finalize();

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
    assert(renderType != eRenderType::COUNT);

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
        case eGBufferDataType::COUNT:
            OutputDebugString(L"\tGBUFFER PASS\n");
            break;
        default:
            assert(false);
            break;
        }
        break;
    case eRenderType::COUNT:
        // intentional fallthrough
    default:
        assert(false);
        break;
    }

    switch (m_CurrentLightType)
    {
    case eLightType::TILED:
        OutputDebugString(L"\tTIELD LIGHTS PASS\n");
        break;
    case eLightType::CLUSTERED:
        OutputDebugString(L"\tCLUSTERED LIGHTS PASS\n");
        break;
    case eLightType::COUNT:
        // Intentional fallthrough
    default:
        assert(false);
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
        case eGBufferDataType::COUNT:
            OutputDebugString(L"\tGBUFFER PASS\n");
            break;
        default:
            assert(false);
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
        case eGBufferDataType::COUNT:
            OutputDebugString(L"\tGBUFFER PASS\n");
            break;
        default:
            assert(false);
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
    case eLightType::TILED:
        OutputDebugString(L"\tTIELD LIGHTS PASS\n");
        break;
    case eLightType::CLUSTERED:
        OutputDebugString(L"\tCLUSTERED LIGHTS PASS\n");
        break;
    case eLightType::COUNT:
        // Intentional fallthrough
    default:
        assert(false);
        break;
    }
}

void Sponza::SetPreviousLightType()
{
    m_CurrentLightType = static_cast<eLightType>((static_cast<size_t>(m_CurrentLightType) + static_cast<size_t>(eLightType::COUNT) - 1) % (static_cast<size_t>(eLightType::COUNT)));
    switch (m_CurrentLightType)
    {
    case eLightType::TILED:
        OutputDebugString(L"\tTIELD LIGHTS PASS\n");
        break;
    case eLightType::CLUSTERED:
        OutputDebugString(L"\tCLUSTERED LIGHTS PASS\n");
        break;
    case eLightType::COUNT:
        // Intentional fallthrough
    default:
        assert(false);
        break;
    }
}

void Sponza::Cleanup( void )
{
    m_Model.Clear();
    Lighting::Shutdown();
    TextureManager::Shutdown();
}

void Sponza::RenderDeferredObjects(GraphicsContext& gfxContext, const Camera& camera, const Vector3& viewerPos, eObjectFilter Filter)
{
    //XMMATRIX ortho = XMMatrixTranspose(XMMatrixOrthographicLH(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight(), camera.GetNearClip(), camera.GetFarClip()));
    //Matrix4 orthoMatrix(
    //    Vector4(XMVectorGetX(ortho.r[0]), XMVectorGetY(ortho.r[0]), XMVectorGetZ(ortho.r[0]), XMVectorGetW(ortho.r[0])),
    //    Vector4(XMVectorGetX(ortho.r[1]), XMVectorGetY(ortho.r[1]), XMVectorGetZ(ortho.r[1]), XMVectorGetW(ortho.r[1])),
    //    Vector4(XMVectorGetX(ortho.r[2]), XMVectorGetY(ortho.r[2]), XMVectorGetZ(ortho.r[2]), XMVectorGetW(ortho.r[2])),
    //    Vector4(XMVectorGetX(ortho.r[3]), XMVectorGetY(ortho.r[3]), XMVectorGetZ(ortho.r[3]), XMVectorGetW(ortho.r[3]))
    //);
    //WCHAR szDebugMsg[64];
    //swprintf_s(szDebugMsg, L"(%f, %f, %f, %f)\n", XMVectorGetX(ortho.r[0]), XMVectorGetY(ortho.r[0]), XMVectorGetZ(ortho.r[0]), XMVectorGetW(ortho.r[0]));
    //OutputDebugString(szDebugMsg);
    //swprintf_s(szDebugMsg, L"(%f, %f, %f, %f)\n", XMVectorGetX(ortho.r[1]), XMVectorGetY(ortho.r[1]), XMVectorGetZ(ortho.r[1]), XMVectorGetW(ortho.r[1]));
    //OutputDebugString(szDebugMsg);
    //swprintf_s(szDebugMsg, L"(%f, %f, %f, %f)\n", XMVectorGetX(ortho.r[2]), XMVectorGetY(ortho.r[2]), XMVectorGetZ(ortho.r[2]), XMVectorGetW(ortho.r[2]));
    //OutputDebugString(szDebugMsg);
    //swprintf_s(szDebugMsg, L"(%f, %f, %f, %f)\n", XMVectorGetX(ortho.r[3]), XMVectorGetY(ortho.r[3]), XMVectorGetZ(ortho.r[3]), XMVectorGetW(ortho.r[3]));
    //OutputDebugString(szDebugMsg);
    struct CommonConstants
    {
        UINT MaterialIdx;
        Matrix4 ModelToProjection;
        Matrix4 InvViewProj;
        //Matrix4 InvView;
        Matrix4 modelToShadow;
        XMFLOAT3 ViewerPos;
    } commonConstants;

    struct VSConstants
    {
        Matrix4 modelToProjection;
    } vsConstants;
    vsConstants.modelToProjection = camera.GetViewProjMatrix();
    commonConstants.ModelToProjection = camera.GetViewProjMatrix();

    //Matrix4 matScreen = Matrix4(
    //    Vector4(2.0f / g_SceneColorBuffer.GetWidth(), 0, 0, 0),
    //    Vector4(0, -2.0f / g_SceneColorBuffer.GetHeight(), 0, 0),
    //    Vector4(0, 0, 1.0f, 0),
    //    Vector4(-1.0f, 1.0f, 0, 1.0f)
    //);
    //XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetProjMatrix()), camera.GetProjMatrix());
    XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewProjMatrix()), camera.GetViewProjMatrix());
    //XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewMatrix()), camera.GetViewMatrix());    

    //commonConstants.InvProj = Matrix4(
    //    Vector4(invProj.r[0]),
    //    Vector4(invProj.r[1]),
    //    Vector4(invProj.r[2]),
    //    Vector4(invProj.r[3])
    //);
    commonConstants.InvViewProj = Matrix4(
        Vector4(invProj.r[0]),
        Vector4(invProj.r[1]),
        Vector4(invProj.r[2]),
        Vector4(invProj.r[3])
    );
    //commonConstants.InvView = Matrix4(
    //    Vector4(invView.r[0]),
    //    Vector4(invView.r[1]),
    //    Vector4(invView.r[2]),
    //    Vector4(invView.r[3])
    //);
    commonConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMStoreFloat3(&commonConstants.ViewerPos, viewerPos);

    //gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    gfxContext.SetDynamicConstantBufferView(Renderer::kMeshConstants, sizeof(vsConstants), &vsConstants);
    gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(CommonConstants), &commonConstants);

    gfxContext.SetDescriptorTable(Renderer::kGBufferSRVs, m_GBufferSRVs);
    gfxContext.DrawIndexed(NUM_GBUFFER_INDICES, 0, 0);
}

void Sponza::RenderObjects( GraphicsContext& gfxContext, const Matrix4& ViewProjMat, const Vector3& viewerPos, eObjectFilter Filter )
{
    struct VSConstants
    {
        Matrix4 modelToProjection;
        //Matrix4 modelToShadow;
        //XMFLOAT3 viewerPos;
    } vsConstants;
    vsConstants.modelToProjection = ViewProjMat;
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
    commonConstants.ModelToProjection = ViewProjMat;
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

void Sponza::RenderLightShadows(GraphicsContext& gfxContext, const Camera& camera)
{
    using namespace Lighting;

    ScopedTimer _prof(L"RenderLightShadows", gfxContext);

    static uint32_t LightIndex = 0;
    if (LightIndex >= MaxLights)
        return;

    m_LightShadowTempBuffer.BeginRendering(gfxContext);
    {
        gfxContext.SetPipelineState(m_ShadowPSO);
        RenderObjects(gfxContext, m_LightShadowMatrix[LightIndex], camera.GetPosition(), kOpaque);
        gfxContext.SetPipelineState(m_CutoutShadowPSO);
        RenderObjects(gfxContext, m_LightShadowMatrix[LightIndex], camera.GetPosition(), kCutout);
    }
    //m_LightShadowTempBuffer.EndRendering(gfxContext);

    gfxContext.TransitionResource(m_LightShadowTempBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
    gfxContext.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_COPY_DEST);

    gfxContext.CopySubresource(m_LightShadowArray, LightIndex, m_LightShadowTempBuffer, 0);

    gfxContext.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    ++LightIndex;
}

void Sponza::RenderScene(
    GraphicsContext& gfxContext,
    const Camera& camera,
    const D3D12_VIEWPORT& viewport,
    const D3D12_RECT& scissor,
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
    } psConstants;

    psConstants.sunDirection = m_SunDirection;
    psConstants.sunLight = Vector3(1.0f, 1.0f, 1.0f) * m_SunLightIntensity;
    psConstants.ambientLight = Vector3(1.0f, 1.0f, 1.0f) * m_AmbientIntensity;
    psConstants.ShadowTexelSize[0] = 1.0f / g_ShadowBuffer.GetWidth();
    psConstants.InvTileDim[0] = 1.0f / Lighting::LightGridDim;
    psConstants.InvTileDim[1] = 1.0f / Lighting::LightGridDim;
    psConstants.TileCount[0] = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), Lighting::LightGridDim);
    psConstants.TileCount[1] = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), Lighting::LightGridDim);
    psConstants.FirstLightIndex[0] = Lighting::m_FirstConeLight;
    psConstants.FirstLightIndex[1] = Lighting::m_FirstConeShadowedLight;
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
        Lighting::FillLightGrid(gfxContext, camera);

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
                ScopedTimer _prof2(L"Render Color", gfxContext);

                gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

                gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);
                gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);

                {
                    //gfxContext.SetPipelineState(m_ModelPSO);
                    //gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
                    //D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), g_SceneNormalBuffer.GetRTV() };
                    //gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                    //gfxContext.SetViewportAndScissor(viewport, scissor);
                    switch (m_CurrentRenderType)
                    {
                    case eRenderType::FORWARD:
                    {
                        gfxContext.SetPipelineState(m_aForwardPSOs[static_cast<size_t>(m_CurrentLightType)][static_cast<size_t>(m_CurrentForwardType)]);
                        gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
                        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), g_SceneNormalBuffer.GetRTV() };
                        gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                    }
                    break;
                    case eRenderType::DEFERRED:
                    {
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
                        RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kOpaque);

                        for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
                        {
                            gfxContext.TransitionResource(g_aSceneGBuffers[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
                        }

                        gfxContext.SetIndexBuffer(m_GBufferIndexBuffer);
                        gfxContext.SetVertexBuffer(0, m_GBufferVertexBuffer);
                        gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);
                        gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);
                        gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
                    }
                    break;
                    case eRenderType::COUNT:
                        // intentional fallthrough
                    default:
                        assert(false);
                        break;
                    }

                    //gfxContext.SetRenderTargets(0, nullptr, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                    gfxContext.SetViewportAndScissor(viewport, scissor);
                }
                
                switch (m_CurrentRenderType)
                {
                case eRenderType::FORWARD:
                    RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kOpaque);
                    break;
                case eRenderType::DEFERRED:
                    gfxContext.SetPipelineState(m_aGBufferPSOs[static_cast<size_t>(m_CurrentLightType)][static_cast<size_t>(m_CurrentGBufferType)]);
                    gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
                    RenderDeferredObjects(gfxContext, camera, camera.GetPosition(), Sponza::kOpaque);
                    gfxContext.SetIndexBuffer(m_Model.GetIndexBuffer());
                    gfxContext.SetVertexBuffer(0, m_Model.GetVertexBuffer());
                    break;
                case eRenderType::COUNT:
                    // intentional fallthrough
                default:
                    assert(false);
                    break;
                }

                //RenderObjects( gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kOpaque );

                gfxContext.SetPipelineState(m_CutoutModelPSO);
                gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
                D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), g_SceneNormalBuffer.GetRTV() };
                gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                gfxContext.SetViewportAndScissor(viewport, scissor);
                RenderObjects( gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kCutout );
            }
        }
    }
}
