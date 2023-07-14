#pragma once

#include <variant>

#include "ogl/pipeline.hpp"
#include "texture.hpp"
#include "texture_manager.hpp"

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

using Material = std::variant<InvalidMaterial, MaterialPbrNormal>;

void BindMaterialForGBuffer(const Material& materialVariant, TextureManager& textureManager, Pipeline& gBufferPipeline);
}// namespace stw
