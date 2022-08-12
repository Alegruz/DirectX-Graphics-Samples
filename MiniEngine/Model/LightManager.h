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

#pragma once

#include <cstdint>

#define NO_ENCODING (0)
#define BASELINE (0)
#define Z_RECONSTRUCTION (0)
#define SPHERICAL_COORDNATES (0)
#define SPHEREMAP_TRANSFORM (0)
#define OCTAHEDRON_NORMAL (0)
#define OCT24 (1)

class StructuredBuffer;
class ByteAddressBuffer;
class ColorBuffer;
class ShadowBuffer;
class GraphicsContext;
class IntVar;
class DescriptorHandle;

namespace Graphics
{
    enum class eLightType : uint8_t;
    enum class eGBufferDataType : uint8_t;
    enum class eThinGBufferDataType : uint8_t;
}

namespace Math
{
    class Vector3;
    class Matrix4;
    class Camera;
    class AxisAlignedBox;
}

namespace Lighting
{
    enum class eClusterType
    {
        _8x8x8,
        _16x16x16,
        _24x24x16,
        _32x32x16,
        _32x32x32,
        _64x64x16,
        _64x64x32,
        COUNT,
    };

    extern IntVar LightGridDim;
    extern eClusterType LightClusterType;
    extern uint32_t aLightClusterDimensions[static_cast<size_t>(eClusterType::COUNT)][2];

    enum { MaxLights = 192, MaxClusterLights = MaxLights, MaxPointLights = 0, MaxConeLights = 0 };

    //LightData m_LightData[MaxLights];
    extern StructuredBuffer m_LightBuffer;
    extern ByteAddressBuffer m_LightGrid;
    extern ByteAddressBuffer m_LightCluster;

    extern ByteAddressBuffer m_LightGridBitMask;
    extern ByteAddressBuffer m_LightClusterBitMask;
    extern std::uint32_t m_FirstConeLight;
    extern std::uint32_t m_FirstConeShadowedLight;

    extern ColorBuffer m_LightShadowArray;
    extern ShadowBuffer m_LightShadowTempBuffer;
    extern Math::Matrix4 m_LightShadowMatrix[MaxLights];

    extern uint32_t m_LightShadowParity[MaxLights / 32];

    extern bool m_bUpdateLightsToggle;

    void InitializeResources(void);
    void CreateRandomLights(const Math::Vector3 minBound, const Math::Vector3 maxBound);
    void FillLightGrid(GraphicsContext& gfxContext, const Math::Camera& camera, Graphics::eLightType lightType);
    void FillAndShadeLightGrid(GraphicsContext& gfxContext, const Math::Camera& camera, const DescriptorHandle& gBufferHandle, Graphics::eLightType lightType, Graphics::eGBufferDataType gbufferType);
    void FillAndShadeLightGridThinGBuffer(GraphicsContext& gfxContext, const Math::Camera& camera, const DescriptorHandle& gBufferHandle, Graphics::eLightType lightType, Graphics::eThinGBufferDataType gbufferType);
    //void KillzoneDiceLightGrid(GraphicsContext& gfxContext, const Math::Camera& camera, const DescriptorHandle& gBufferHandle);
    //void KillzoneDiceLightCullingGrid(GraphicsContext& gfxContext, const Math::Camera& camera, const DescriptorHandle& gBufferHandle);
    //void KillzoneDiceLightAABBCullingGrid(GraphicsContext& gfxContext, const Math::Camera& camera, const DescriptorHandle& gBufferHandle);
    //void KillzoneIntelLightGrid(GraphicsContext& gfxContext, const Math::Camera& camera, const DescriptorHandle& gBufferHandle);
    //void FillLightCluster(GraphicsContext& gfxContext, const Math::Camera& camera, const Math::AxisAlignedBox& boundingBox, Graphics::eLightType lightType, Graphics::eGBufferType gbufferType);
    void ResetLights(void);
    void Shutdown(void);
    void UpdateLights(float deltaTime);
}
