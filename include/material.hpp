#pragma once

#include <array>
#include <variant>

#include "ogl/pipeline.hpp"
#include "texture.hpp"
#include "texture_manager.hpp"

namespace stw
{
static constexpr u32 MaterialCount = 3;

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

void BindMaterialForGBuffer(const Material& materialVariant,
	stw::TextureManager& textureManager,
	const std::array<std::reference_wrapper<stw::Pipeline>, MaterialCount>& gBufferPipelines);
}// namespace stw
