#pragma once

#include <variant>
#include <ogl/pipeline.hpp>

#include "texture.hpp"

namespace stw
{
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

using Material = std::variant<MaterialNoNormalNoSpecular, MaterialNormalNoSpecular>;

void BindMaterial(const Material& materialVariant);
}
