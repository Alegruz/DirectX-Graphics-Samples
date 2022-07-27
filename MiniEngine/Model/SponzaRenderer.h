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
// Author(s):  James Stanard
//

#pragma once

#include <d3d12.h>

class GraphicsContext;
class ShadowCamera;
class ModelH3D;
class ExpVar;

namespace Math
{
    class Camera;
    class Vector3;
}

enum class eRenderType : UINT8
{
    FORWARD,
    DEFERRED,
    COUNT,
};

enum class eForwardType : UINT8
{
    WORLD_POS,
    DIFFUSE_ALBEDO,
    GLOSS,
    NORMAL,
    SPECULAR_INTENSITY,
    COUNT,
};

enum class eLightType : UINT8
{
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

namespace Sponza
{
    void Startup( Math::Camera& camera );
    void Cleanup( void );

    void RenderScene(
        GraphicsContext& gfxContext,
        const Math::Camera& camera,
        const D3D12_VIEWPORT& viewport,
        const D3D12_RECT& scissor,
        //const Math::OrientedBox& boundingBox,
        bool skipDiffusePass = false,
        bool skipShadowMap = false );
    void Update(float deltaTime);

    const ModelH3D& GetModel();

    void SetRenderType(eRenderType renderType) noexcept;
    void SetNextBufferOutput();
    void SetPreviousBufferOutput();
    void SetNextLightType();
    void SetPreviousLightType();

    extern Math::Vector3 m_SunDirection;
    extern ShadowCamera m_SunShadow;
    extern ExpVar m_AmbientIntensity;
    extern ExpVar m_SunLightIntensity;

}
