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
//             Alex Nankervis
//

#include "Common.hlsli"

cbuffer VSConstants : register(b0)
{
    float4x4 modelToProjection;
    //float4x4 modelToShadow;
    //float3 ViewerPos;
};

cbuffer StartVertex : register(b1)
{
    uint materialIdx;
    float4x4 ModelToProjection;
    float4x4 InvViewProj;
    float4x4 InvProj;
    float4x4 modelToShadow;
    float3 ViewerPos;
};

struct VSInput
{
    float3 position : POSITION;
    float2 texcoord0 : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_Position;
    //float3 worldPos : WorldPos;
    //float2 texCoord : TexCoord0;
    //float3 viewDir : TexCoord1;
    //float3 shadowCoord : TexCoord2;
#if ENABLE_TRIANGLE_ID
    uint vertexID : TexCoord3;
#endif
};

[RootSignature(Renderer_RootSig)]
//VSOutput main(VSInput vsInput, uint vertexID : SV_VertexID)
VSOutput main(uint vertexID : SV_VertexID)
{
    VSOutput vsOutput;
    
    //vsOutput.position = float4(vsInput.position, 1.0f);
    //float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    float2 uv = float2(((2 - vertexID) << 1) & 2, (2 - vertexID) & 2);
    vsOutput.position = float4(uv * float2(2, -2) + float2(-1, 1), 1, 1);

#if ENABLE_TRIANGLE_ID
    vsOutput.vertexID = materialIdx << 24 | (vertexID & 0xFFFF);
#endif

    return vsOutput;
}
