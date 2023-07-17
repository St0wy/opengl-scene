//
// Created by stowy on 13/07/2023.
//

#pragma once

#include "number_types.hpp"

namespace stw
{
static constexpr u32 MaxPointLights = 128;
static constexpr u32 ShadowMapSize = 4096;
static constexpr u32 MipChainLength = 5;
static constexpr f32 FilterRadius = 0.005f;
static constexpr usize ShadowMapNumCascades = 4;
static constexpr usize SsaoKernelSize = 64;
static constexpr usize SsaoRandomTextureSize = 16;
static constexpr u32 SkyboxResolution = 4096;
static constexpr u32 IrradianceMapResolution = 32;
static constexpr u32 PrefilterMapResolution = 128;
static constexpr u32 BrdfLutResolution = 512;
static constexpr usize InvalidId = static_cast<usize>(-1);

static constexpr f32 DefaultYaw = -90.0f;
static constexpr f32 DefaultPitch = 0.0f;
static constexpr f32 DefaultSpeed = 2.5f;
static constexpr f32 DefaultSensitivity = 0.1f;
static constexpr f32 DefaultFovY = 45.0f;
static constexpr f32 MinFovY = 1.0f;
static constexpr f32 MaxFovY = 120.0f;
static constexpr f32 MaxPitchAngle = 89.0f;
static constexpr f32 DefaultAspectRatio = 16.0f / 9.0f;
static constexpr f32 NearPlane = 0.1f;
static constexpr f32 FarPlane = 1'000.0f;
}// namespace stw
