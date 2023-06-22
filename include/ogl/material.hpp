#pragma once

#include <ogl/pipeline.hpp>

namespace stw
{
struct MaterialBase
{
	Pipeline& pipeline;
};

struct MaterialNormalNoSpecular
{
	float specular;
	float shininess;

};
}
