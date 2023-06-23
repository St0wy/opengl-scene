#pragma once

#include <ogl/pipeline.hpp>
#include <variant>

namespace stw
{
struct MaterialBase
{
	Pipeline& pipeline;
};

struct MaterialNormalNoSpecular
{
	f32 specular;
	f32 shininess;
	std::size_t diffuseMapIndex;
	std::size_t normalMapIndex;
};

struct MaterialNoNormalNoSpecular
{
	f32 specular;
	f32 shininess;
	std::size_t diffuseMapIndex;
};

using Material = std::variant<MaterialNoNormalNoSpecular, MaterialNormalNoSpecular>;
}
