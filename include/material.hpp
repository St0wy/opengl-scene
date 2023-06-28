#pragma once

#include <variant>

#include "texture.hpp"
#include "ogl/pipeline.hpp"
#include "texture_manager.hpp"

namespace stw
{
struct InvalidMaterial
{
};

struct MaterialBase
{
	Pipeline& pipeline;
};

struct MaterialNormalNoSpecular : MaterialBase
{
	glm::vec3 specular;
	f32 shininess;
	std::size_t diffuseMapIndex;
	std::size_t normalMapIndex;
};

struct MaterialNoNormalNoSpecular : MaterialBase
{
	f32 specular;
	f32 shininess;
	std::size_t diffuseMapIndex;
};

using Material = std::variant<InvalidMaterial, MaterialNoNormalNoSpecular, MaterialNormalNoSpecular>;

void BindMaterial(const Material& materialVariant, TextureManager& textureManager);
}
