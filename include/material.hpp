/**
 * @file material.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the material variant.
 * @version 1.0
 * @date 28/06/2023
 *
 * @copyright SAE (c) 2023
 *
 */

#pragma once

#include <array>
#include <variant>

#include "texture_manager.hpp"
#include "ogl/pipeline.hpp"

namespace stw
{
struct InvalidMaterial
{
};

struct MaterialPbrNormal
{
	std::size_t baseColorMapIndex{};
	std::size_t normalMapIndex{};
	std::size_t ambientOcclusionMapIndex{};
	std::size_t roughnessMapIndex{};
	std::size_t metallicMapIndex{};
};

struct MaterialPbrNormalNoAo
{
	std::size_t baseColorMapIndex{};
	std::size_t normalMapIndex{};
	std::size_t roughnessMapIndex{};
	std::size_t metallicMapIndex{};
};

struct MaterialPbrNormalArm
{
	std::size_t baseColorMapIndex{};
	std::size_t normalMapIndex{};
	std::size_t armMapIndex{};
};

using Material = std::variant<InvalidMaterial, MaterialPbrNormal, MaterialPbrNormalNoAo, MaterialPbrNormalArm>;

static constexpr u32 MaterialCount = std::variant_size_v<Material> - 1;

void BindMaterialForGBuffer(const Material& materialVariant,
	TextureManager& textureManager,
	const std::array<std::reference_wrapper<Pipeline>, MaterialCount>& gBufferPipelines);
}// namespace stw
