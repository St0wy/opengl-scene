/**
* @file consts.hpp
* @author Fabian Huber (fabian.hbr@protonmail.ch)
* @brief Contains the various constants of the project.
* @version 1.0
* @date 13/07/2023
*
* @copyright SAE (c) 2023
*
*/

module;

export module consts;

import number_types;

namespace stw
{
export constexpr u32 MaxPointLights = 128;
export constexpr u32 ShadowMapSize = 4096;
export constexpr u32 MipChainLength = 5;
export constexpr f32 FilterRadius = 0.005f;
export constexpr usize ShadowMapNumCascades = 4;
export constexpr usize SsaoKernelSize = 64;
export constexpr usize SsaoRandomTextureSize = 16;
export constexpr u32 SkyboxResolution = 4096;
export constexpr u32 IrradianceMapResolution = 32;
export constexpr u32 PrefilterMapResolution = 128;
export constexpr u32 BrdfLutResolution = 512;
export constexpr usize InvalidId = static_cast<usize>(-1);

export constexpr f32 DefaultYaw = -90.0f;
export constexpr f32 DefaultPitch = 0.0f;
export constexpr f32 DefaultSpeed = 2.5f;
export constexpr f32 DefaultSensitivity = 0.1f;
export constexpr f32 DefaultFovY = 45.0f;
export constexpr f32 MinFovY = 1.0f;
export constexpr f32 MaxFovY = 120.0f;
export constexpr f32 MaxPitchAngle = 89.0f;
export constexpr f32 DefaultAspectRatio = 16.0f / 9.0f;
export constexpr f32 NearPlane = 0.1f;
export constexpr f32 FarPlane = 1'000.0f;
}// namespace stw
