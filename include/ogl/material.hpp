#pragma once

#include <variant>

#include "texture.hpp"
#include "ogl/pipeline.hpp"

namespace stw
{
struct InvalidMaterial{};

struct MaterialBase
{
	Pipeline& pipeline;
};

struct MaterialNormalNoSpecular : MaterialBase
{
	f32 specular;
	f32 shininess;
	Texture diffuseMap;
	Texture normalMap;
};

struct MaterialNoNormalNoSpecular : MaterialBase
{
	f32 specular;
	f32 shininess;
	Texture diffuseMap;
};

using Material = std::variant<InvalidMaterial, MaterialNoNormalNoSpecular, MaterialNormalNoSpecular>;

void BindMaterial(const Material& materialVariant);
}
