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

struct MaterialDiffuse
{
	std::size_t diffuseMapIndex{};
};

struct MaterialDiffuseSpecular
{
	std::size_t diffuseMapIndex{};
	std::size_t specularMapIndex{};
};

struct MaterialDiffuseSpecularNormal
{
	std::size_t diffuseMapIndex{};
	std::size_t specularMapIndex{};
	std::size_t normalMapIndex{};
};

using Material = std::variant<InvalidMaterial, MaterialDiffuse, MaterialDiffuseSpecularNormal, MaterialDiffuseSpecular>;

void BindMaterialForGBuffer(const Material& materialVariant, TextureManager& textureManager, Pipeline& gBufferPipeline);
}// namespace stw
