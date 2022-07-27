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

#include "CompiledShaders/FillLightClusterCS_8_8.h"
#include "CompiledShaders/FillLightClusterCS_16_16.h"
#include "CompiledShaders/FillLightClusterCS_24_16.h"
#include "CompiledShaders/FillLightClusterCS_32_16.h"
#include "CompiledShaders/FillLightClusterCS_32_32.h"
#include "CompiledShaders/FillLightClusterCS_64_16.h"
#include "CompiledShaders/FillLightClusterCS_64_32.h"

#if KILLZONE_GBUFFER
#include "CompiledShaders/KillzoneLightGridCS_8.h"
#include "CompiledShaders/KillzoneLightGridCS_16.h"
#include "CompiledShaders/KillzoneLightGridCS_24.h"
#include "CompiledShaders/KillzoneLightGridCS_32.h"

#include "CompiledShaders/KillzoneLightCullingGridCS_8.h"
#include "CompiledShaders/KillzoneLightCullingGridCS_16.h"
#include "CompiledShaders/KillzoneLightCullingGridCS_24.h"
#include "CompiledShaders/KillzoneLightCullingGridCS_32.h"

#include "CompiledShaders/KillzoneLightAABBCullingGridCS_8.h"
#include "CompiledShaders/KillzoneLightAABBCullingGridCS_16.h"
#include "CompiledShaders/KillzoneLightAABBCullingGridCS_24.h"
#include "CompiledShaders/KillzoneLightAABBCullingGridCS_32.h"

#include "CompiledShaders/KillzoneIntelLightGridCS_8.h"
#include "CompiledShaders/KillzoneIntelLightGridCS_16.h"
#include "CompiledShaders/KillzoneIntelLightGridCS_24.h"
#include "CompiledShaders/KillzoneIntelLightGridCS_32.h"
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
    eClusterType LightClusterType = eClusterType::_32x32x16;

    RootSignature m_FillLightRootSig;
    RootSignature m_FillLightClusterSig;
#if KILLZONE_GBUFFER
    RootSignature m_KillzoneLightRootSig;
#endif

    ComputePSO m_FillLightGridCS_8(L"Fill Light Grid 8 CS");
    ComputePSO m_FillLightGridCS_16(L"Fill Light Grid 16 CS");
    ComputePSO m_FillLightGridCS_24(L"Fill Light Grid 24 CS");
    ComputePSO m_FillLightGridCS_32(L"Fill Light Grid 32 CS");

    ComputePSO m_FillLight2_5DGridCS_8(L"Fill Light 2.5D Culling Grid 8 CS");
    ComputePSO m_FillLight2_5DGridCS_16(L"Fill Light 2.5D Culling Grid 16 CS");
    ComputePSO m_FillLight2_5DGridCS_24(L"Fill Light 2.5D Culling Grid 24 CS");
    ComputePSO m_FillLight2_5DGridCS_32(L"Fill Light 2.5D Culling Grid 32 CS");

    ComputePSO m_FillLight2_5DAABBGridCS_8(L"Fill Light 2.5D AABB Culling Grid 8 CS");
    ComputePSO m_FillLight2_5DAABBGridCS_16(L"Fill Light 2.5D AABB Culling Grid 16 CS");
    ComputePSO m_FillLight2_5DAABBGridCS_24(L"Fill Light 2.5D AABB Culling Grid 24 CS");
    ComputePSO m_FillLight2_5DAABBGridCS_32(L"Fill Light 2.5D AABB Culling Grid 32 CS");

    ComputePSO m_FillLightClusterCS_8_8(L"Fill Light Cluster 8 x 8 x 8 CS");
    ComputePSO m_FillLightClusterCS_16_16(L"Fill Light Cluster 16 x 16 x 16 CS");
    ComputePSO m_FillLightClusterCS_24_16(L"Fill Light Cluster 24 x 24 x 16 CS");
    ComputePSO m_FillLightClusterCS_32_16(L"Fill Light Cluster 32 x 32 x 16 CS");
    ComputePSO m_FillLightClusterCS_32_32(L"Fill Light Cluster 32 x 32 x 32 CS");
    ComputePSO m_FillLightClusterCS_64_16(L"Fill Light Cluster 64 x 64 x 16 CS");
    ComputePSO m_FillLightClusterCS_64_32(L"Fill Light Cluster 64 x 64 x 32 CS");

#if KILLZONE_GBUFFER
    ComputePSO m_KillzoneLightGridCS_8(L"Killzone Light Grid 8 CS");
    ComputePSO m_KillzoneLightGridCS_16(L"Killzone Light Grid 16 CS");
    ComputePSO m_KillzoneLightGridCS_24(L"Killzone Light Grid 24 CS");
    ComputePSO m_KillzoneLightGridCS_32(L"Killzone Light Grid 32 CS");

    ComputePSO m_KillzoneLightCullingGridCS_8(L"Killzone Light 2.5 Culling Grid 8 CS");
    ComputePSO m_KillzoneLightCullingGridCS_16(L"Killzone Light 2.5 Culling Grid 16 CS");
    ComputePSO m_KillzoneLightCullingGridCS_24(L"Killzone Light 2.5 Culling Grid 24 CS");
    ComputePSO m_KillzoneLightCullingGridCS_32(L"Killzone Light 2.5 Culling Grid 32 CS");

    ComputePSO m_KillzoneLightAABBCullingGridCS_8(L"Killzone Light 2.5 AABB Culling Grid 8 CS");
    ComputePSO m_KillzoneLightAABBCullingGridCS_16(L"Killzone Light 2.5 AABB Culling Grid 16 CS");
    ComputePSO m_KillzoneLightAABBCullingGridCS_24(L"Killzone Light 2.5 AABB Culling Grid 24 CS");
    ComputePSO m_KillzoneLightAABBCullingGridCS_32(L"Killzone Light 2.5 AABB Culling Grid 32 CS");

    ComputePSO m_KillzoneIntelLightGridCS_8(L"Killzone Intel Light Grid 8 CS");
    ComputePSO m_KillzoneIntelLightGridCS_16(L"Killzone Intel Light Grid 16 CS");
    ComputePSO m_KillzoneIntelLightGridCS_24(L"Killzone Intel Light Grid 24 CS");
    ComputePSO m_KillzoneIntelLightGridCS_32(L"Killzone Intel Light Grid 32 CS");
#endif

    LightData m_LightData[MaxLights];
    StructuredBuffer m_LightBuffer;
    ByteAddressBuffer m_LightGrid;
    ByteAddressBuffer m_LightCluster;

    ByteAddressBuffer m_LightGridBitMask;
    ByteAddressBuffer m_LightClusterBitMask;
    uint32_t m_FirstConeLight;
    uint32_t m_FirstConeShadowedLight;

    enum {shadowDim = 512};
    ColorBuffer m_LightShadowArray;
    ShadowBuffer m_LightShadowTempBuffer;
    Matrix4 m_LightShadowMatrix[MaxLights];

    void InitializeResources(void);
    void CreateRandomLights(const Vector3 minBound, const Vector3 maxBound);
    void FillLightGrid(GraphicsContext& gfxContext, const Camera& camera);
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
    m_FillLightClusterSig[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2);
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
#endif

    m_FillLightGridCS_8.SetRootSignature(m_FillLightRootSig);
    m_FillLightGridCS_8.SetComputeShader(g_pFillLightGridCS_8, sizeof(g_pFillLightGridCS_8));
    m_FillLightGridCS_8.Finalize();

    m_FillLightGridCS_16.SetRootSignature(m_FillLightRootSig);
    m_FillLightGridCS_16.SetComputeShader(g_pFillLightGridCS_16, sizeof(g_pFillLightGridCS_16));
    m_FillLightGridCS_16.Finalize();

    m_FillLightGridCS_24.SetRootSignature(m_FillLightRootSig);
    m_FillLightGridCS_24.SetComputeShader(g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24));
    m_FillLightGridCS_24.Finalize();

    m_FillLightGridCS_32.SetRootSignature(m_FillLightRootSig);
    m_FillLightGridCS_32.SetComputeShader(g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32));
    m_FillLightGridCS_32.Finalize();

    m_FillLightClusterCS_8_8.SetRootSignature(m_FillLightClusterSig);
    m_FillLightClusterCS_8_8.SetComputeShader(g_pFillLightClusterCS_8_8, sizeof(g_pFillLightClusterCS_8_8));
    m_FillLightClusterCS_8_8.Finalize();

    m_FillLightClusterCS_16_16.SetRootSignature(m_FillLightClusterSig);
    m_FillLightClusterCS_16_16.SetComputeShader(g_pFillLightClusterCS_16_16, sizeof(g_pFillLightClusterCS_16_16));
    m_FillLightClusterCS_16_16.Finalize();

    m_FillLightClusterCS_24_16.SetRootSignature(m_FillLightClusterSig);
    m_FillLightClusterCS_24_16.SetComputeShader(g_pFillLightClusterCS_24_16, sizeof(g_pFillLightClusterCS_24_16));
    m_FillLightClusterCS_24_16.Finalize();

    m_FillLightClusterCS_32_16.SetRootSignature(m_FillLightClusterSig);
    m_FillLightClusterCS_32_16.SetComputeShader(g_pFillLightClusterCS_32_16, sizeof(g_pFillLightClusterCS_32_16));
    m_FillLightClusterCS_32_16.Finalize();

    m_FillLightClusterCS_32_32.SetRootSignature(m_FillLightClusterSig);
    m_FillLightClusterCS_32_32.SetComputeShader(g_pFillLightClusterCS_32_32, sizeof(g_pFillLightClusterCS_32_32));
    m_FillLightClusterCS_32_32.Finalize();

    m_FillLightClusterCS_64_16.SetRootSignature(m_FillLightClusterSig);
    m_FillLightClusterCS_64_16.SetComputeShader(g_pFillLightClusterCS_64_16, sizeof(g_pFillLightClusterCS_64_16));
    m_FillLightClusterCS_64_16.Finalize();
    
    m_FillLightClusterCS_64_32.SetRootSignature(m_FillLightClusterSig);
    m_FillLightClusterCS_64_32.SetComputeShader(g_pFillLightClusterCS_64_32, sizeof(g_pFillLightClusterCS_64_32));
    m_FillLightClusterCS_64_32.Finalize();

#if KILLZONE_GBUFFER
    {
        m_KillzoneLightGridCS_8.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneLightGridCS_8.SetComputeShader(g_pKillzoneLightGridCS_8, sizeof(g_pKillzoneLightGridCS_8));
        m_KillzoneLightGridCS_8.Finalize();

        m_KillzoneLightGridCS_16.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneLightGridCS_16.SetComputeShader(g_pKillzoneLightGridCS_16, sizeof(g_pKillzoneLightGridCS_16));
        m_KillzoneLightGridCS_16.Finalize();

        m_KillzoneLightGridCS_24.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneLightGridCS_24.SetComputeShader(g_pKillzoneLightGridCS_24, sizeof(g_pKillzoneLightGridCS_24));
        m_KillzoneLightGridCS_24.Finalize();

        m_KillzoneLightGridCS_32.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneLightGridCS_32.SetComputeShader(g_pKillzoneLightGridCS_32, sizeof(g_pKillzoneLightGridCS_32));
        m_KillzoneLightGridCS_32.Finalize();

        m_KillzoneIntelLightGridCS_8.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneIntelLightGridCS_8.SetComputeShader(g_pKillzoneIntelLightGridCS_8, sizeof(g_pKillzoneIntelLightGridCS_8));
        m_KillzoneIntelLightGridCS_8.Finalize();

        m_KillzoneIntelLightGridCS_16.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneIntelLightGridCS_16.SetComputeShader(g_pKillzoneIntelLightGridCS_16, sizeof(g_pKillzoneIntelLightGridCS_16));
        m_KillzoneIntelLightGridCS_16.Finalize();

        m_KillzoneIntelLightGridCS_24.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneIntelLightGridCS_24.SetComputeShader(g_pKillzoneIntelLightGridCS_24, sizeof(g_pKillzoneIntelLightGridCS_24));
        m_KillzoneIntelLightGridCS_24.Finalize();

        m_KillzoneIntelLightGridCS_32.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneIntelLightGridCS_32.SetComputeShader(g_pKillzoneIntelLightGridCS_32, sizeof(g_pKillzoneIntelLightGridCS_32));
        m_KillzoneIntelLightGridCS_32.Finalize();
    }

    {
        m_KillzoneLightCullingGridCS_8.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneLightCullingGridCS_8.SetComputeShader(g_pKillzoneLightCullingGridCS_8, sizeof(g_pKillzoneLightCullingGridCS_8));
        m_KillzoneLightCullingGridCS_8.Finalize();

        m_KillzoneLightCullingGridCS_16.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneLightCullingGridCS_16.SetComputeShader(g_pKillzoneLightCullingGridCS_16, sizeof(g_pKillzoneLightCullingGridCS_16));
        m_KillzoneLightCullingGridCS_16.Finalize();

        m_KillzoneLightCullingGridCS_24.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneLightCullingGridCS_24.SetComputeShader(g_pKillzoneLightCullingGridCS_24, sizeof(g_pKillzoneLightCullingGridCS_24));
        m_KillzoneLightCullingGridCS_24.Finalize();

        m_KillzoneLightCullingGridCS_32.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneLightCullingGridCS_32.SetComputeShader(g_pKillzoneLightCullingGridCS_32, sizeof(g_pKillzoneLightCullingGridCS_32));
        m_KillzoneLightCullingGridCS_32.Finalize();
    }

    {
        m_KillzoneLightAABBCullingGridCS_8.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneLightAABBCullingGridCS_8.SetComputeShader(g_pKillzoneLightAABBCullingGridCS_8, sizeof(g_pKillzoneLightAABBCullingGridCS_8));
        m_KillzoneLightAABBCullingGridCS_8.Finalize();

        m_KillzoneLightAABBCullingGridCS_16.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneLightAABBCullingGridCS_16.SetComputeShader(g_pKillzoneLightAABBCullingGridCS_16, sizeof(g_pKillzoneLightAABBCullingGridCS_16));
        m_KillzoneLightAABBCullingGridCS_16.Finalize();

        m_KillzoneLightAABBCullingGridCS_24.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneLightAABBCullingGridCS_24.SetComputeShader(g_pKillzoneLightAABBCullingGridCS_24, sizeof(g_pKillzoneLightAABBCullingGridCS_24));
        m_KillzoneLightAABBCullingGridCS_24.Finalize();

        m_KillzoneLightAABBCullingGridCS_32.SetRootSignature(m_KillzoneLightRootSig);
        m_KillzoneLightAABBCullingGridCS_32.SetComputeShader(g_pKillzoneLightAABBCullingGridCS_32, sizeof(g_pKillzoneLightAABBCullingGridCS_32));
        m_KillzoneLightAABBCullingGridCS_32.Finalize();
    }
#endif

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

    m_LightBuffer.Create(L"m_LightBuffer", MaxLights, sizeof(LightData));
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
        if (n < 32 * 1)
            type = 0;
        else if (n < 32 * 3)
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

    CommandContext::InitializeBuffer(m_LightBuffer, m_LightData, MaxLights * sizeof(LightData));
}

void Lighting::Shutdown(void)
{
    m_LightBuffer.Destroy();
    m_LightCluster.Destroy();
    m_LightGrid.Destroy();
    m_LightClusterBitMask.Destroy();
    m_LightGridBitMask.Destroy();
    m_LightShadowArray.Destroy();
    m_LightShadowTempBuffer.Destroy();
}

void Lighting::UpdateLights(float deltaTime)
{
    /*
    XMMATRIX rotationY = XMMatrixRotationY(deltaTime);
    XMMATRIX rotationX = XMMatrixRotationX(deltaTime);

    size_t i = 0;
    for (; i < MaxLights / 3; ++i)
    {
        XMVECTOR position = XMVector4Transform(XMVectorSet(m_LightData[i].pos[0], m_LightData[i].pos[1], m_LightData[i].pos[2], 1.0f), rotationY);
        m_LightData[i].pos[0] = XMVectorGetX(position);
        m_LightData[i].pos[1] = XMVectorGetY(position);
        m_LightData[i].pos[2] = XMVectorGetZ(position);
    }

    for (; i < (MaxLights / 3) * 2; ++i)
    {
        XMVECTOR position = XMVector4Transform(XMVectorSet(m_LightData[i].pos[0], m_LightData[i].pos[1], m_LightData[i].pos[2], 1.0f), rotationX);
        m_LightData[i].pos[0] = XMVectorGetX(position);
        m_LightData[i].pos[1] = XMVectorGetY(position);
        m_LightData[i].pos[2] = XMVectorGetZ(position);
    }

    CommandContext::InitializeBuffer(m_LightBuffer, m_LightData, 2 * (MaxLights / 3) * sizeof(LightData));
    */
    UNREFERENCED_PARAMETER(deltaTime);
}

void Lighting::FillLightGrid(GraphicsContext& gfxContext, const Camera& camera)
{
    ScopedTimer _prof(L"FillLightGrid", gfxContext);

    ComputeContext& Context = gfxContext.GetComputeContext();

    Context.SetRootSignature(m_FillLightRootSig);

    switch ((int)LightGridDim)
    {
    case  8: Context.SetPipelineState(m_FillLightGridCS_8 ); break;
    case 16: Context.SetPipelineState(m_FillLightGridCS_16); break;
    case 24: Context.SetPipelineState(m_FillLightGridCS_24); break;
    case 32: Context.SetPipelineState(m_FillLightGridCS_32); break;
    default: ASSERT(false); break;
    }

    ColorBuffer& LinearDepth = g_LinearDepth[ TemporalEffects::GetFrameIndexMod2() ];

    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 1, LinearDepth.GetSRV());
    //Context.SetDynamicDescriptor(1, 1, g_SceneDepthBuffer.GetDepthSRV());
    Context.SetDynamicDescriptor(2, 0, m_LightGrid.GetUAV());
    Context.SetDynamicDescriptor(2, 1, m_LightGridBitMask.GetUAV());

    // todo: assumes 1920x1080 resolution
    uint32_t tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), LightGridDim);
    uint32_t tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), LightGridDim);

    float FarClipDist = camera.GetFarClip();
    float NearClipDist = camera.GetNearClip();
    const float RcpZMagic = NearClipDist / (FarClipDist - NearClipDist);

    struct CSConstants
    {
        uint32_t ViewportWidth, ViewportHeight;
        float InvTileDim;
        float RcpZMagic;
        uint32_t TileCount;
        Matrix4 ViewProjMatrix;
    } csConstants;
    // todo: assumes 1920x1080 resolution
    csConstants.ViewportWidth = g_SceneColorBuffer.GetWidth();
    csConstants.ViewportHeight = g_SceneColorBuffer.GetHeight();
    csConstants.InvTileDim = 1.0f / LightGridDim;
    csConstants.RcpZMagic = RcpZMagic;
    csConstants.TileCount = tileCountX;
    csConstants.ViewProjMatrix = camera.GetViewProjMatrix();
    Context.SetDynamicConstantBufferView(0, sizeof(CSConstants), &csConstants);

    Context.Dispatch(tileCountX, tileCountY, 1);

    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void Lighting::FillLight2_5DGrid(GraphicsContext& gfxContext, const Camera& camera)
{
    ScopedTimer _prof(L"FillLight2_5DGrid", gfxContext);

    ComputeContext& Context = gfxContext.GetComputeContext();

    Context.SetRootSignature(m_FillLightRootSig);

    switch ((int)LightGridDim)
    {
    case  8: Context.SetPipelineState(m_FillLight2_5DGridCS_8); break;
    case 16: Context.SetPipelineState(m_FillLight2_5DGridCS_16); break;
    case 24: Context.SetPipelineState(m_FillLight2_5DGridCS_24); break;
    case 32: Context.SetPipelineState(m_FillLight2_5DGridCS_32); break;
    default: ASSERT(false); break;
    }

    ColorBuffer& LinearDepth = g_LinearDepth[TemporalEffects::GetFrameIndexMod2()];

    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 1, LinearDepth.GetSRV());
    //Context.SetDynamicDescriptor(1, 1, g_SceneDepthBuffer.GetDepthSRV());
    Context.SetDynamicDescriptor(2, 0, m_LightGrid.GetUAV());
    Context.SetDynamicDescriptor(2, 1, m_LightGridBitMask.GetUAV());

    // todo: assumes 1920x1080 resolution
    uint32_t tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), LightGridDim);
    uint32_t tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), LightGridDim);

    float FarClipDist = camera.GetFarClip();
    float NearClipDist = camera.GetNearClip();
    const float RcpZMagic = NearClipDist / (FarClipDist - NearClipDist);

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
    Matrix4 InvViewProj;
    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewProjMatrix()), camera.GetViewProjMatrix());
    csConstants.InvViewProj = Matrix4(
        Vector4(invViewProj.r[0]),
        Vector4(invViewProj.r[1]),
        Vector4(invViewProj.r[2]),
        Vector4(invViewProj.r[3])
    );
    Context.SetDynamicConstantBufferView(0, sizeof(CSConstants), &csConstants);

    Context.Dispatch(tileCountX, tileCountY, 1);

    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void Lighting::FillLight2_5DAABBGrid(GraphicsContext& gfxContext, const Camera& camera)
{
    ScopedTimer _prof(L"FillLight2_5DAABBGrid", gfxContext);

    ComputeContext& Context = gfxContext.GetComputeContext();

    Context.SetRootSignature(m_FillLightRootSig);

    switch ((int)LightGridDim)
    {
    case  8: Context.SetPipelineState(m_FillLight2_5DAABBGridCS_8); break;
    case 16: Context.SetPipelineState(m_FillLight2_5DAABBGridCS_16); break;
    case 24: Context.SetPipelineState(m_FillLight2_5DAABBGridCS_24); break;
    case 32: Context.SetPipelineState(m_FillLight2_5DAABBGridCS_32); break;
    default: ASSERT(false); break;
    }

    ColorBuffer& LinearDepth = g_LinearDepth[TemporalEffects::GetFrameIndexMod2()];

    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 1, LinearDepth.GetSRV());
    //Context.SetDynamicDescriptor(1, 1, g_SceneDepthBuffer.GetDepthSRV());
    Context.SetDynamicDescriptor(2, 0, m_LightGrid.GetUAV());
    Context.SetDynamicDescriptor(2, 1, m_LightGridBitMask.GetUAV());

    // todo: assumes 1920x1080 resolution
    uint32_t tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), LightGridDim);
    uint32_t tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), LightGridDim);

    float FarClipDist = camera.GetFarClip();
    float NearClipDist = camera.GetNearClip();
    const float RcpZMagic = NearClipDist / (FarClipDist - NearClipDist);

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
    Matrix4 InvViewProj;
    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(camera.GetViewProjMatrix()), camera.GetViewProjMatrix());
    csConstants.InvViewProj = Matrix4(
        Vector4(invViewProj.r[0]),
        Vector4(invViewProj.r[1]),
        Vector4(invViewProj.r[2]),
        Vector4(invViewProj.r[3])
    );
    Context.SetDynamicConstantBufferView(0, sizeof(CSConstants), &csConstants);

    Context.Dispatch(tileCountX, tileCountY, 1);

    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void Lighting::KillzoneDiceLightGrid(GraphicsContext& gfxContext, const Camera& camera, const DescriptorHandle& gBufferHandle)
{
#if KILLZONE_GBUFFER
    ScopedTimer _prof(L"KillzoneDiceLightGrid", gfxContext);

    ComputeContext& Context = gfxContext.GetComputeContext();

    Context.SetRootSignature(m_KillzoneLightRootSig);

    switch ((int)LightGridDim)
    {
    case  8: Context.SetPipelineState(m_KillzoneLightGridCS_8); break;
    case 16: Context.SetPipelineState(m_KillzoneLightGridCS_16); break;
    case 24: Context.SetPipelineState(m_KillzoneLightGridCS_24); break;
    case 32: Context.SetPipelineState(m_KillzoneLightGridCS_32); break;
    default: ASSERT(false); break;
    }

    ColorBuffer& LinearDepth = g_LinearDepth[TemporalEffects::GetFrameIndexMod2()];

    //g_aSceneGBuffers[static_cast<size_t>(eGBufferType::COUNT)]
    for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
    {
        Context.TransitionResource(g_aSceneGBuffers[i], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    //Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    //Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
    //gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, s_KillzoneTextureHeap.GetHeapPointer());
    //Context.SetDescriptorTable(1, resourcesHandle);
    //for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
    //{
    //    Context.SetDynamicDescriptor(1, i, g_aSceneGBuffers[i].GetSRV());
    //}
    Context.SetDynamicDescriptor(1, 8, LinearDepth.GetSRV());
    Context.SetDynamicDescriptor(1, 4, m_LightBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 5, m_LightShadowArray.GetSRV());
    Context.SetDescriptorTable(2, gBufferHandle);
    //Context.SetDynamicDescriptor(1, static_cast<size_t>(eGBufferType::COUNT) + 2, m_LightShadowArray.GetSRV());
    //Context.SetDynamicDescriptor(1, 1, g_SceneDepthBuffer.GetDepthSRV());
    Context.SetDynamicDescriptor(3, 0, g_SceneColorBuffer.GetUAV());

    // todo: assumes 1920x1080 resolution
    uint32_t tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), LightGridDim);
    uint32_t tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), LightGridDim);

    float FarClipDist = camera.GetFarClip();
    float NearClipDist = camera.GetNearClip();
    const float RcpZMagic = NearClipDist / (FarClipDist - NearClipDist);

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
    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    //Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    //Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#else
    UNREFERENCED_PARAMETER(gfxContext);
    UNREFERENCED_PARAMETER(camera);
#endif
}

void Lighting::KillzoneDiceLightCullingGrid(GraphicsContext& gfxContext, const Camera& camera, const DescriptorHandle& gBufferHandle)
{
#if KILLZONE_GBUFFER
    ScopedTimer _prof(L"KillzoneDiceLightCullingGrid", gfxContext);

    ComputeContext& Context = gfxContext.GetComputeContext();

    Context.SetRootSignature(m_KillzoneLightRootSig);

    switch ((int)LightGridDim)
    {
    case  8: Context.SetPipelineState(m_KillzoneLightCullingGridCS_8); break;
    case 16: Context.SetPipelineState(m_KillzoneLightCullingGridCS_16); break;
    case 24: Context.SetPipelineState(m_KillzoneLightCullingGridCS_24); break;
    case 32: Context.SetPipelineState(m_KillzoneLightCullingGridCS_32); break;
    default: ASSERT(false); break;
    }

    ColorBuffer& LinearDepth = g_LinearDepth[TemporalEffects::GetFrameIndexMod2()];

    //g_aSceneGBuffers[static_cast<size_t>(eGBufferType::COUNT)]
    for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
    {
        Context.TransitionResource(g_aSceneGBuffers[i], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    //Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    //Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
    //gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, s_KillzoneTextureHeap.GetHeapPointer());
    //Context.SetDescriptorTable(1, resourcesHandle);
    //for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
    //{
    //    Context.SetDynamicDescriptor(1, i, g_aSceneGBuffers[i].GetSRV());
    //}
    Context.SetDynamicDescriptor(1, 8, LinearDepth.GetSRV());
    Context.SetDynamicDescriptor(1, 4, m_LightBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 5, m_LightShadowArray.GetSRV());
    Context.SetDescriptorTable(2, gBufferHandle);
    //Context.SetDynamicDescriptor(1, static_cast<size_t>(eGBufferType::COUNT) + 2, m_LightShadowArray.GetSRV());
    //Context.SetDynamicDescriptor(1, 1, g_SceneDepthBuffer.GetDepthSRV());
    Context.SetDynamicDescriptor(3, 0, g_SceneColorBuffer.GetUAV());

    // todo: assumes 1920x1080 resolution
    uint32_t tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), LightGridDim);
    uint32_t tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), LightGridDim);

    float FarClipDist = camera.GetFarClip();
    float NearClipDist = camera.GetNearClip();
    const float RcpZMagic = NearClipDist / (FarClipDist - NearClipDist);

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
    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    //Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    //Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#else
    UNREFERENCED_PARAMETER(gfxContext);
    UNREFERENCED_PARAMETER(camera);
#endif
}

void Lighting::KillzoneDiceLightAABBCullingGrid(GraphicsContext& gfxContext, const Camera& camera, const DescriptorHandle& gBufferHandle)
{
#if KILLZONE_GBUFFER
    ScopedTimer _prof(L"KillzoneDiceLightAABBCullingGrid", gfxContext);

    ComputeContext& Context = gfxContext.GetComputeContext();

    Context.SetRootSignature(m_KillzoneLightRootSig);

    switch ((int)LightGridDim)
    {
    case  8: Context.SetPipelineState(m_KillzoneLightAABBCullingGridCS_8); break;
    case 16: Context.SetPipelineState(m_KillzoneLightAABBCullingGridCS_16); break;
    case 24: Context.SetPipelineState(m_KillzoneLightAABBCullingGridCS_24); break;
    case 32: Context.SetPipelineState(m_KillzoneLightAABBCullingGridCS_32); break;
    default: ASSERT(false); break;
    }

    ColorBuffer& LinearDepth = g_LinearDepth[TemporalEffects::GetFrameIndexMod2()];

    //g_aSceneGBuffers[static_cast<size_t>(eGBufferType::COUNT)]
    for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
    {
        Context.TransitionResource(g_aSceneGBuffers[i], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    //Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    //Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
    //gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, s_KillzoneTextureHeap.GetHeapPointer());
    //Context.SetDescriptorTable(1, resourcesHandle);
    //for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
    //{
    //    Context.SetDynamicDescriptor(1, i, g_aSceneGBuffers[i].GetSRV());
    //}
    Context.SetDynamicDescriptor(1, 8, LinearDepth.GetSRV());
    Context.SetDynamicDescriptor(1, 4, m_LightBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 5, m_LightShadowArray.GetSRV());
    Context.SetDescriptorTable(2, gBufferHandle);
    //Context.SetDynamicDescriptor(1, static_cast<size_t>(eGBufferType::COUNT) + 2, m_LightShadowArray.GetSRV());
    //Context.SetDynamicDescriptor(1, 1, g_SceneDepthBuffer.GetDepthSRV());
    Context.SetDynamicDescriptor(3, 0, g_SceneColorBuffer.GetUAV());

    // todo: assumes 1920x1080 resolution
    uint32_t tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), LightGridDim);
    uint32_t tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), LightGridDim);

    float FarClipDist = camera.GetFarClip();
    float NearClipDist = camera.GetNearClip();
    const float RcpZMagic = NearClipDist / (FarClipDist - NearClipDist);

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
    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    //Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    //Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#else
    UNREFERENCED_PARAMETER(gfxContext);
    UNREFERENCED_PARAMETER(camera);
#endif
}


void Lighting::KillzoneIntelLightGrid(GraphicsContext& gfxContext, const Camera& camera, const DescriptorHandle& gBufferHandle)
{
#if KILLZONE_GBUFFER
    ScopedTimer _prof(L"KillzoneIntelLightGrid", gfxContext);

    ComputeContext& Context = gfxContext.GetComputeContext();

    Context.SetRootSignature(m_KillzoneLightRootSig);

    switch ((int)LightGridDim)
    {
    case  8: Context.SetPipelineState(m_KillzoneIntelLightGridCS_8); break;
    case 16: Context.SetPipelineState(m_KillzoneIntelLightGridCS_16); break;
    case 24: Context.SetPipelineState(m_KillzoneIntelLightGridCS_24); break;
    case 32: Context.SetPipelineState(m_KillzoneIntelLightGridCS_32); break;
    default: ASSERT(false); break;
    }

    ColorBuffer& LinearDepth = g_LinearDepth[TemporalEffects::GetFrameIndexMod2()];

    //g_aSceneGBuffers[static_cast<size_t>(eGBufferType::COUNT)]
    for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
    {
        Context.TransitionResource(g_aSceneGBuffers[i], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    //Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    //Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
    //gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, s_KillzoneTextureHeap.GetHeapPointer());
    //Context.SetDescriptorTable(1, resourcesHandle);
    //for (size_t i = 0; i < static_cast<size_t>(eGBufferType::COUNT); ++i)
    //{
    //    Context.SetDynamicDescriptor(1, i, g_aSceneGBuffers[i].GetSRV());
    //}
    Context.SetDynamicDescriptor(1, 8, LinearDepth.GetSRV());
    Context.SetDynamicDescriptor(1, 4, m_LightBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 5, m_LightShadowArray.GetSRV());
    Context.SetDescriptorTable(2, gBufferHandle);
    //Context.SetDynamicDescriptor(1, static_cast<size_t>(eGBufferType::COUNT) + 2, m_LightShadowArray.GetSRV());
    //Context.SetDynamicDescriptor(1, 1, g_SceneDepthBuffer.GetDepthSRV());
    Context.SetDynamicDescriptor(3, 0, g_SceneColorBuffer.GetUAV());

    // todo: assumes 1920x1080 resolution
    uint32_t tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), LightGridDim);
    uint32_t tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), LightGridDim);

    float FarClipDist = camera.GetFarClip();
    float NearClipDist = camera.GetNearClip();
    const float RcpZMagic = NearClipDist / (FarClipDist - NearClipDist);

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
    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    //Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    //Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
#else
    UNREFERENCED_PARAMETER(gfxContext);
    UNREFERENCED_PARAMETER(camera);
#endif
}

void Lighting::FillLightCluster(GraphicsContext& gfxContext, const Math::Camera& camera, const Math::AxisAlignedBox& boundingBox)
{
    ScopedTimer _prof(L"FillLightCluster", gfxContext);

    ComputeContext& Context = gfxContext.GetComputeContext();

    Context.SetRootSignature(m_FillLightClusterSig);

    switch (LightClusterType)
    {
    case eClusterType::_8x8x8:
        Context.SetPipelineState(m_FillLightClusterCS_8_8);
        break;
    case eClusterType::_16x16x16:
        Context.SetPipelineState(m_FillLightClusterCS_16_16);
        break;
    case eClusterType::_24x24x16:
        Context.SetPipelineState(m_FillLightClusterCS_24_16);
        break;
    case eClusterType::_32x32x16:
        Context.SetPipelineState(m_FillLightClusterCS_32_16);
        break;
    case eClusterType::_32x32x32:
        Context.SetPipelineState(m_FillLightClusterCS_32_32);
        break;
    case eClusterType::_64x64x16:
        Context.SetPipelineState(m_FillLightClusterCS_64_16);
        break;
    case eClusterType::_64x64x32:
        Context.SetPipelineState(m_FillLightClusterCS_64_32);
        break;
    case eClusterType::COUNT:
        // intentional fallthrough
    default:
        ASSERT(false);
        break;
    }

    ColorBuffer& LinearDepth = g_LinearDepth[TemporalEffects::GetFrameIndexMod2()];

    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightCluster, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(m_LightClusterBitMask, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 1, LinearDepth.GetSRV());
    //Context.SetDynamicDescriptor(1, 1, g_SceneDepthBuffer.GetDepthSRV());
    Context.SetDynamicDescriptor(2, 0, m_LightCluster.GetUAV());
    Context.SetDynamicDescriptor(2, 1, m_LightClusterBitMask.GetUAV());

    // todo: assumes 1920x1080 resolution
    uint32_t tileCountX = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0]);
    uint32_t tileCountY = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0]);
    uint32_t tileCountZ = aLightClusterDimensions[static_cast<size_t>(LightClusterType)][1];

    float FarClipDist = camera.GetFarClip();
    float NearClipDist = camera.GetNearClip();
    const float RcpZMagic = NearClipDist / (FarClipDist - NearClipDist);

    struct CSConstants
    {
        uint32_t ViewportWidth, ViewportHeight;
        float InvTileDim;
        float RcpZMagic;
        uint32_t TileCount[2];
        Matrix4 ViewProjMatrix;
    } csConstants;
    // todo: assumes 1920x1080 resolution
    csConstants.ViewportWidth = g_SceneColorBuffer.GetWidth();
    csConstants.ViewportHeight = g_SceneColorBuffer.GetHeight();
    csConstants.InvTileDim = 1.0f / aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0];
    csConstants.RcpZMagic = RcpZMagic;
    csConstants.TileCount[0] = tileCountX;
    csConstants.TileCount[1] = tileCountY;
    csConstants.ViewProjMatrix = camera.GetViewProjMatrix();
    Context.SetDynamicConstantBufferView(0, sizeof(CSConstants), &csConstants);

    Context.Dispatch(tileCountX, tileCountY, tileCountZ);

    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightCluster, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightClusterBitMask, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void Lighting::FillLightClusterCpu(GraphicsContext& gfxContext, const Math::Camera& camera, const Math::AxisAlignedBox& boundingBox)
{
    ScopedTimer _prof(L"FillLightClusterCpu", gfxContext);

    ComputeContext& Context = gfxContext.GetComputeContext();

    uint32_t* pLights = nullptr;
    pLights = static_cast<uint32_t*>(malloc(
            aLightClusterDimensions[static_cast<size_t>(LightClusterType)][1] * 
            aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] * 
            aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] * 
            sizeof(uint32_t)
    ));
    memset(
        pLights,
        0,
        aLightClusterDimensions[static_cast<size_t>(LightClusterType)][1] *
        aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] *
        aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] *
        sizeof(uint32_t)
    );

    Vector3 scale = Vector3(
        static_cast<float>(aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0]), 
        static_cast<float>(aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0]), 
        static_cast<float>(aLightClusterDimensions[static_cast<size_t>(LightClusterType)][1])
    ) / ((boundingBox.GetMax() + 1.0f) - boundingBox.GetMin());
    //Vector3 scale = Vector3(static_cast<float>(aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0]), static_cast<float>(aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0]), static_cast<float>(aLightClusterDimensions[static_cast<size_t>(LightClusterType)][1]));
    Vector3 invScale = 1.0f / scale;

    for (uint32_t i = 0; i < MaxLights; ++i)
    {
        const LightData& lightData = m_LightData[i];

        const Vector3 position = Vector3(lightData.pos[0], lightData.pos[1], lightData.pos[2]) - boundingBox.GetMin();
        //const Vector3 position = Vector3(lightData.pos[0], lightData.pos[1], lightData.pos[2]);
        const Vector3 minPosition = (position - sqrtf(lightData.radiusSq)) * scale;
        const Vector3 maxPosition = (position + sqrtf(lightData.radiusSq)) * scale;

        // Cluster for the center of the light
        int32_t px = static_cast<int32_t>(floorf(position.GetX() * scale.GetX()));
        int32_t py = static_cast<int32_t>(floorf(position.GetY() * scale.GetY()));
        int32_t pz = static_cast<int32_t>(floorf(position.GetZ() * scale.GetZ()));

        // Cluster bounds for the light
        int32_t x0 = std::max(static_cast<int32_t>(floorf(minPosition.GetX())), 0);
        int32_t x1 = std::min(static_cast<int32_t>(floorf(maxPosition.GetX())), static_cast<int32_t>(aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0]));
        int32_t y0 = std::max(static_cast<int32_t>(floorf(minPosition.GetY())), 0);
        int32_t y1 = std::min(static_cast<int32_t>(floorf(maxPosition.GetY())), static_cast<int32_t>(aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0]));
        int32_t z0 = std::max(static_cast<int32_t>(floorf(minPosition.GetZ())), 0);
        int32_t z1 = std::min(static_cast<int32_t>(floorf(maxPosition.GetZ())), static_cast<int32_t>(aLightClusterDimensions[static_cast<size_t>(LightClusterType)][1]));

        uint32_t mask = (1 << i);
        //WCHAR szDebugMsg[128];
        //swprintf_s(szDebugMsg, L"x0, x1: %d, %d / y0, y1: %d, %d / z0, z1: %d, %d\n", x0, x1, y0, y1, z0, z1);
        //OutputDebugString(szDebugMsg);
        // Do AABB<->Sphere tests to figure out which clusters are actually intersected by the light
        for (int32_t z = z0; z < z1; ++z)
        {
            float dz = (pz == z) ? 0.0f : boundingBox.GetMin().GetZ() + static_cast<float>(pz < z ? z : z + 1) * invScale.GetZ() - lightData.pos[2];
            //float dz = (pz == z) ? 0.0f : static_cast<float>(pz < z ? z : z + 1) * invScale.GetZ() - lightData.pos[2];
            dz *= dz;

            for (int32_t y = y0; y < y1; ++y)
            {
                float dy = (py == y) ? 0.0f : boundingBox.GetMin().GetY() + static_cast<float>(py < y ? y : y + 1) * invScale.GetY() - lightData.pos[1];
                //float dy = (py == y) ? 0.0f : static_cast<float>(py < y ? y : y + 1) * invScale.GetY() - lightData.pos[1];
                dy *= dy;
                dy += dz;

                for (int32_t x = x0; x < x1; ++x)
                {
                    float dx = (px == x) ? 0.0f : boundingBox.GetMin().GetX() + static_cast<float>(px < x ? x : x + 1) * invScale.GetX() - lightData.pos[0];
                    //float dx = (px == x) ? 0.0f : static_cast<float>(px < x ? x : x + 1) * invScale.GetX() - lightData.pos[0];
                    dx *= dx;
                    dx += dy;
                    
                    if (dx < lightData.radiusSq)
                    {
                        pLights[z * aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] * aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] + y * aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] + x] |= mask;
                        //swprintf_s(
                        //    szDebugMsg,
                        //    L"%u/%u\t(%d, %d, %d)\t0x%08x\n",
                        //    z * aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] * aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] +
                        //    y * aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] +
                        //    x,
                        //    aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] * aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] * aLightClusterDimensions[static_cast<size_t>(LightClusterType)][1],
                        //    x,
                        //    y,
                        //    z,
                        //    pLights[z * aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] * aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] + y * aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] + x]
                        //    );
                        //OutputDebugString(szDebugMsg);
                    }
                }
            }
        }
    }

    CommandContext::InitializeBuffer(
        m_LightCluster, 
        pLights, 
        aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] * 
        aLightClusterDimensions[static_cast<size_t>(LightClusterType)][0] * 
        aLightClusterDimensions[static_cast<size_t>(LightClusterType)][1] * 
        sizeof(uint32_t)
    );

    free(pLights);
}
