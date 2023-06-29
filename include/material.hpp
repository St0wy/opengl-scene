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
	glm::vec3 specular;
	f32 shininess;
	std::size_t diffuseMapIndex;
};

struct MaterialNormalSpecular : MaterialBase
{
	f32 shininess;
	std::size_t diffuseMapIndex;
	std::size_t specularMapIndex;
	std::size_t normalMapIndex;
};

using Material =
	std::variant<InvalidMaterial, MaterialNoNormalNoSpecular, MaterialNormalNoSpecular, MaterialNormalSpecular>;

void BindMaterial(const Material& materialVariant, TextureManager& textureManager);
}// namespace stw
