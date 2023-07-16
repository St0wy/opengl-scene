#pragma once

#include <variant>
#include <array>

#include "ogl/pipeline.hpp"
#include "texture.hpp"
#include "texture_manager.hpp"

namespace stw
{
static constexpr u32 MaterialCount = 2;

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

using Material = std::variant<InvalidMaterial, MaterialPbrNormal, MaterialPbrNormalNoAo>;

void BindMaterialForGBuffer(
	const std::variant<InvalidMaterial, MaterialPbrNormal, MaterialPbrNormalNoAo>& materialVariant,
	stw::TextureManager& textureManager,
	const std::array<std::reference_wrapper<stw::Pipeline>, MaterialCount>& gBufferPipelines);
}// namespace stw
