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

#include "CompiledShaders/DeferredPS.h"
#include "CompiledShaders/DepthViewerVS.h"
#include "CompiledShaders/DepthViewerPS.h"
#include "CompiledShaders/ModelViewerVS.h"
#include "CompiledShaders/ModelViewerPS.h"
#include "CompiledShaders/GBufferAlbedoPS.h"
#include "CompiledShaders/GBufferDepthPS.h"
#include "CompiledShaders/GBufferLightVS.h"
#include "CompiledShaders/GBufferLightPS.h"
#include "CompiledShaders/GBufferNormalPS.h"
#include "CompiledShaders/GBufferSpecularPS.h"

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
    GraphicsPSO m_ModelPSO = { (L"Sponza: Color PSO") };
    GraphicsPSO m_CutoutModelPSO = { (L"Sponza: Cutout Color PSO") };
    GraphicsPSO m_ShadowPSO(L"Sponza: Shadow PSO");
    GraphicsPSO m_CutoutShadowPSO(L"Sponza: Cutout Shadow PSO");

    GraphicsPSO m_GBufferPSO = { (L"Sponza: GBuffer PSO") };
    GraphicsPSO m_GBufferLightPSO = { (L"Sponza: GBuffer Light PSO") };
    GraphicsPSO m_GBufferDepthPSO = { (L"Sponza: GBuffer Depth PSO") };
    GraphicsPSO m_GBufferNormalPSO = { (L"Sponza: GBuffer Normal PSO") };
    GraphicsPSO m_GBufferAlbedoPSO = { (L"Sponza: GBuffer Albedo PSO") };
    GraphicsPSO m_GBufferSpecularPSO = { (L"Sponza: GBuffer Specular PSO") };

    std::vector<std::vector<GraphicsPSO*>> m_apMainPSOs = 
    { 
        { &m_ModelPSO }, 
        { &m_GBufferPSO, &m_GBufferDepthPSO, &m_GBufferNormalPSO, &m_GBufferAlbedoPSO, &m_GBufferSpecularPSO },
    };
    DescriptorHandle m_GBufferSRVs;

    eRenderType m_CurrentRenderType = eRenderType::FORWARD;
    eGBufferType m_CurrentBufferType = eGBufferType::ALBEDO;
    //eGBufferType m_CurrentBufferType = eGBufferType::GBUFFER;

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
    constexpr const size_t NUM_GBUFFER_VERTICES = 4;
    constexpr const size_t NUM_GBUFFER_INDICES = 4;
    unsigned char* m_pGBufferVertexData;
    unsigned char* m_pGBufferIndexData;

    GBufferVertex QuadVerts[] =
    {
        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f } },
        { { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
        { {  1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } }
    };

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
    DXGI_FORMAT GBufferFormat = g_aSceneGBuffers[0].GetFormat();
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

    D3D12_INPUT_ELEMENT_DESC gBufferVertElem[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
    m_ModelPSO = m_DepthPSO;
    m_ModelPSO.SetBlendState(BlendDisable);
    m_ModelPSO.SetDepthStencilState(DepthStateTestEqual);
    m_ModelPSO.SetRenderTargetFormats(2, formats, DepthFormat);
    //m_ModelPSO.SetRenderTargetFormats(0, nullptr, DepthFormat);
    m_ModelPSO.SetVertexShader( g_pModelViewerVS, sizeof(g_pModelViewerVS) );
    m_ModelPSO.SetPixelShader( g_pModelViewerPS, sizeof(g_pModelViewerPS) );
    m_ModelPSO.Finalize();

    std::vector<DXGI_FORMAT> gbufferFormats;
    gbufferFormats.reserve(GBUFFER_COUNT + 1);
    for (size_t i = 0; i < GBUFFER_COUNT; ++i)
    {
        gbufferFormats.push_back(GBufferFormat);
    }
    gbufferFormats.push_back(NormalFormat);

    m_GBufferPSO = m_ModelPSO;
    m_GBufferPSO.SetRenderTargetFormats(GBUFFER_COUNT + 1, gbufferFormats.data(), DepthFormat);
    m_GBufferPSO.SetPixelShader(g_pDeferredPS, sizeof(g_pDeferredPS));
    m_GBufferPSO.Finalize();

    m_GBufferLightPSO = m_ModelPSO;
    m_GBufferLightPSO.SetRenderTargetFormat(ColorFormat, DepthFormat);
    m_GBufferLightPSO.SetVertexShader(g_pGBufferLightVS, sizeof(g_pGBufferLightVS));
    m_GBufferLightPSO.SetPixelShader(g_pGBufferLightPS, sizeof(g_pGBufferLightPS));
    m_GBufferLightPSO.Finalize();

    m_GBufferDepthPSO = m_ModelPSO;
    m_GBufferDepthPSO.SetRenderTargetFormats(1, &GBufferFormat, DepthFormat);
    m_GBufferDepthPSO.SetPixelShader(g_pGBufferDepthPS, sizeof(g_pGBufferDepthPS));
    m_GBufferDepthPSO.Finalize();

    m_GBufferNormalPSO = m_GBufferDepthPSO;
    m_GBufferNormalPSO.SetRenderTargetFormats(1, &NormalFormat, DepthFormat);
    m_GBufferNormalPSO.SetPixelShader(g_pGBufferNormalPS, sizeof(g_pGBufferNormalPS));
    m_GBufferNormalPSO.Finalize();

    m_GBufferAlbedoPSO = m_GBufferDepthPSO;
    m_GBufferAlbedoPSO.SetRenderTargetFormats(1, &GBufferFormat, DepthFormat);
    m_GBufferAlbedoPSO.SetPixelShader(g_pGBufferAlbedoPS, sizeof(g_pGBufferAlbedoPS));
    m_GBufferAlbedoPSO.Finalize();

    m_GBufferSpecularPSO = m_GBufferDepthPSO;
    m_GBufferSpecularPSO.SetRenderTargetFormats(1, &GBufferFormat, DepthFormat);
    m_GBufferSpecularPSO.SetPixelShader(g_pGBufferSpecularPS, sizeof(g_pGBufferSpecularPS));
    m_GBufferSpecularPSO.Finalize();

    m_CutoutModelPSO = m_ModelPSO;
    m_CutoutModelPSO.SetRasterizerState(RasterizerTwoSided);
    m_CutoutModelPSO.Finalize();

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

    m_GBufferSRVs = Renderer::s_TextureHeap.Alloc(4);

    uint32_t DestCount = 4;
    uint32_t SourceCounts[] = { 1, 1, 1, 1 };
    D3D12_CPU_DESCRIPTOR_HANDLE aGBuffers[] =
    {
        g_aSceneGBuffers[0].GetSRV(),
        g_aSceneGBuffers[1].GetSRV(),
        g_aSceneGBuffers[2].GetSRV(),
        g_SceneNormalBuffer.GetSRV(),
    };

    Graphics::g_Device->CopyDescriptors(
        1, 
        &m_GBufferSRVs, 
        &DestCount,
        DestCount, 
        aGBuffers, 
        SourceCounts, 
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );

    UploadBuffer geomBuffer;

    float left = -(g_aSceneGBuffers[0].GetWidth() / 2.0f);
    float right = left + g_aSceneGBuffers[0].GetWidth();
    float top = (g_aSceneGBuffers[0].GetHeight() / 2.0f);
    float bottom = top - g_aSceneGBuffers[0].GetHeight();

    uint32_t totalBinarySize = NUM_GBUFFER_VERTICES * sizeof(GBufferVertex) + NUM_GBUFFER_INDICES * sizeof(WORD);

    geomBuffer.Create(L"GBuffer Geometry Upload Buffer", totalBinarySize);
    uint8_t* uploadMem = (uint8_t*)geomBuffer.Map();

    m_pGBufferVertexData = uploadMem;
    m_pGBufferIndexData = m_pGBufferVertexData + NUM_GBUFFER_VERTICES * sizeof(GBufferVertex);

    WORD* pIndices = reinterpret_cast<WORD*>(m_pGBufferIndexData);

    for (WORD i = 0; i < NUM_GBUFFER_INDICES; ++i)
    {
        pIndices[i] = i;
    }

    static_assert(sizeof(QuadVerts) == NUM_GBUFFER_VERTICES * sizeof(GBufferVertex));
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
        break;
    case eRenderType::DEFERRED:
        OutputDebugString(L"DEFERRED RENDERING PIPELINE\n");
        switch (m_CurrentBufferType)
        {
        case eGBufferType::GBUFFER:
            OutputDebugString(L"\tGBUFFER PASS\n");
            break;
        case eGBufferType::DEPTH:
            OutputDebugString(L"\tDEPTH PASS\n");
            break;
        case eGBufferType::NORMAL:
            OutputDebugString(L"\tNORMAL PASS\n");
            break;
        case eGBufferType::ALBEDO:
            OutputDebugString(L"\tALBEDO PASS\n");
            break;
        case eGBufferType::SPECULAR:
            OutputDebugString(L"\tSPECULAR PASS\n");
            break;
        case eGBufferType::COUNT:
            // intentional fallthrough
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

    m_CurrentRenderType = renderType;
}

void Sponza::SetNextGBufferOutput()
{
    m_CurrentBufferType = static_cast<eGBufferType>((static_cast<size_t>(m_CurrentBufferType) + 1) % static_cast<size_t>(eGBufferType::COUNT));
    switch (m_CurrentBufferType)
    {
    case eGBufferType::GBUFFER:
        OutputDebugString(L"\tGBUFFER PASS\n");
        break;
    case eGBufferType::DEPTH:
        OutputDebugString(L"\tDEPTH PASS\n");
        break;
    case eGBufferType::NORMAL:
        OutputDebugString(L"\tNORMAL PASS\n");
        break;
    case eGBufferType::ALBEDO:
        OutputDebugString(L"\tALBEDO PASS\n");
        break;
    case eGBufferType::SPECULAR:
        OutputDebugString(L"\tSPECULAR PASS\n");
        break;
    case eGBufferType::COUNT:
        // intentional fallthrough
    default:
        assert(false);
        break;
    }
}

void Sponza::SetPreviousGBufferOutput()
{
    m_CurrentBufferType = static_cast<eGBufferType>((static_cast<size_t>(m_CurrentBufferType) + static_cast<size_t>(eGBufferType::COUNT) - 1) % static_cast<size_t>(eGBufferType::COUNT));
    switch (m_CurrentBufferType)
    {
    case eGBufferType::GBUFFER:
        OutputDebugString(L"\tGBUFFER PASS\n");
        break;
    case eGBufferType::DEPTH:
        OutputDebugString(L"\tDEPTH PASS\n");
        break;
    case eGBufferType::NORMAL:
        OutputDebugString(L"\tNORMAL PASS\n");
        break;
    case eGBufferType::ALBEDO:
        OutputDebugString(L"\tALBEDO PASS\n");
        break;
    case eGBufferType::SPECULAR:
        OutputDebugString(L"\tSPECULAR PASS\n");
        break;
    case eGBufferType::COUNT:
        // intentional fallthrough
    default:
        assert(false);
        break;
    }
}

void Sponza::Cleanup( void )
{
    m_GBufferGeometryBuffer.Destroy();

    delete[] m_pGBufferVertexData;
    delete[] m_pGBufferIndexData;

    m_pGBufferVertexData = nullptr;
    m_pGBufferIndexData = nullptr;

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
    struct VSConstants
    {
        Matrix4 modelToProjection;
        Matrix4 modelToShadow;
        XMFLOAT3 viewerPos;
    } vsConstants;
    vsConstants.modelToProjection = camera.GetViewProjMatrix();
    vsConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMStoreFloat3(&vsConstants.viewerPos, viewerPos);

    //gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    gfxContext.SetDynamicConstantBufferView(Renderer::kMeshConstants, sizeof(vsConstants), &vsConstants);

    gfxContext.SetDescriptorTable(Renderer::kGBufferSRVs, m_GBufferSRVs);

    __declspec(align(16)) uint32_t materialIdx = 0xFFFFFFFFul;

    uint32_t VertexStride = m_Model.GetVertexStride();

    for (uint32_t meshIndex = 0; meshIndex < m_Model.GetMeshCount(); meshIndex++)
    {
        const ModelH3D::Mesh& mesh = m_Model.GetMesh(meshIndex);

        uint32_t indexCount = mesh.indexCount;
        uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
        uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

        if (mesh.materialIndex != materialIdx)
        {
            if (m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kCutout) ||
                !m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kOpaque))
                continue;

            materialIdx = mesh.materialIndex;
            gfxContext.SetDescriptorTable(Renderer::kMaterialSRVs, m_Model.GetSRVs(materialIdx));

            gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(uint32_t), &materialIdx);
        }

        gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
    }

    //gfxContext.DrawIndexed(NUM_GBUFFER_INDICES, 0, 0);
    //gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Sponza::RenderObjects( GraphicsContext& gfxContext, const Matrix4& ViewProjMat, const Vector3& viewerPos, eObjectFilter Filter )
{
    struct VSConstants
    {
        Matrix4 modelToProjection;
        Matrix4 modelToShadow;
        XMFLOAT3 viewerPos;
    } vsConstants;
    vsConstants.modelToProjection = ViewProjMat;
    vsConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMStoreFloat3(&vsConstants.viewerPos, viewerPos);

    gfxContext.SetDynamicConstantBufferView(Renderer::kMeshConstants, sizeof(vsConstants), &vsConstants);

    __declspec(align(16)) uint32_t materialIdx = 0xFFFFFFFFul;

    uint32_t VertexStride = m_Model.GetVertexStride();

    for (uint32_t meshIndex = 0; meshIndex < m_Model.GetMeshCount(); meshIndex++)
    {
        const ModelH3D::Mesh& mesh = m_Model.GetMesh(meshIndex);

        uint32_t indexCount = mesh.indexCount;
        uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
        uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

        if (mesh.materialIndex != materialIdx)
        {
            if (m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kCutout) ||
                !m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kOpaque))
                continue;

            materialIdx = mesh.materialIndex;
            gfxContext.SetDescriptorTable(Renderer::kMaterialSRVs, m_Model.GetSRVs(materialIdx));

            gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(uint32_t), &materialIdx);
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
                for (size_t i = 0; i < GBUFFER_COUNT; ++i)
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

                gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);

                {
                    switch (m_CurrentRenderType)
                    {
                    case eRenderType::FORWARD:
                    {
                        gfxContext.SetPipelineState(*m_apMainPSOs[static_cast<size_t>(eRenderType::FORWARD)][0]);
                        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), g_SceneNormalBuffer.GetRTV() };
                        gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                    }
                        break;
                    case eRenderType::DEFERRED:
                    {
                        if (m_CurrentBufferType != eGBufferType::GBUFFER)
                        {
                            gfxContext.SetPipelineState(*m_apMainPSOs[static_cast<size_t>(eRenderType::DEFERRED)][static_cast<size_t>(m_CurrentBufferType)]);
                            D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), };
                            gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                        }
                        else
                        {
                            std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
                            rtvs.reserve(GBUFFER_COUNT + 1);
                            for (size_t i = 0; i < GBUFFER_COUNT; ++i)
                            {
                                rtvs.push_back(g_aSceneGBuffers[i].GetRTV());
                            }
                            rtvs.push_back(g_SceneNormalBuffer.GetRTV());
                            gfxContext.SetPipelineState(*m_apMainPSOs[static_cast<size_t>(eRenderType::DEFERRED)][static_cast<size_t>(eGBufferType::GBUFFER)]);
                            gfxContext.SetRenderTargets(rtvs.size(), rtvs.data(), g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                            gfxContext.SetViewportAndScissor(viewport, scissor);
                            RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kOpaque);

                            for (size_t i = 0; i < GBUFFER_COUNT; ++i)
                            {
                                gfxContext.TransitionResource(g_aSceneGBuffers[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
                            }
                            //gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, true);

                            uint32_t DestCount = 4;
                            uint32_t SourceCounts[] = { 1, 1, 1, 1, };

                            D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
                            {
                                g_aSceneGBuffers[0].GetSRV(),
                                g_aSceneGBuffers[1].GetSRV(),
                                g_aSceneGBuffers[2].GetSRV(),
                                g_SceneNormalBuffer.GetSRV(),
                            };

                            //DescriptorHandle dest = Renderer::m_CommonTextures + 8 * Renderer::s_TextureHeap.GetDescriptorSize();

                            //g_Device->CopyDescriptors(1, &dest, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                            //SourceTextures[0] = g_aSceneGBuffers[2].GetSRV();
                            //SourceTextures[1] = g_SceneNormalBuffer.GetSRV();

                            DescriptorHandle dest = m_GBufferSRVs;

                            g_Device->CopyDescriptors(
                                1, 
                                &dest, 
                                &DestCount, 
                                DestCount, 
                                SourceTextures,
                                SourceCounts,
                                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                            );


                            //gfxContext.SetIndexBuffer(m_GBufferIndexBuffer);
                            //gfxContext.SetVertexBuffer(0, m_GBufferVertexBuffer);
                            gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);
                            gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);

                            gfxContext.SetPipelineState(m_GBufferLightPSO);
                            gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV());
                            //gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
                        }
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
                    if (m_CurrentBufferType == eGBufferType::GBUFFER)
                    {
                        RenderDeferredObjects(gfxContext, camera, camera.GetPosition(), Sponza::kOpaque);
                        //gfxContext.SetIndexBuffer(m_Model.GetIndexBuffer());
                        //gfxContext.SetVertexBuffer(0, m_Model.GetVertexBuffer());
                    }
                    else
                    {
                        RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kOpaque);
                    }
                    break;
                case eRenderType::COUNT:
                    // intentional fallthrough
                default:
                    assert(false);
                    break;
                }
            }
            gfxContext.SetPipelineState(m_CutoutModelPSO);
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), g_SceneNormalBuffer.GetRTV() };
            gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
            gfxContext.SetViewportAndScissor(viewport, scissor);
            RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kCutout);
        }
    }
}
