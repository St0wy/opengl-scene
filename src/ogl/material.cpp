#include "ogl/material.hpp"

#include <spdlog/spdlog.h>

#include "utils.hpp"

void stw::BindMaterial(const Material& materialVariant)
{
	constexpr auto normalNoSpecular = [](const MaterialNormalNoSpecular& material)
	{
		auto& pipeline = material.pipeline;
		pipeline.SetFloat("material.shininess", material.shininess);
		pipeline.SetFloat("material.specular", material.specular);
		material.diffuseMap.Bind();
		material.normalMap.Bind();
	};

	constexpr auto noNormalNoSpecular = [](const MaterialNoNormalNoSpecular& material)
	{
		spdlog::warn("Not implemented... {} {}", __FILE__, __LINE__);
	};

	std::visit(Overloaded{normalNoSpecular, noNormalNoSpecular}, materialVariant);
}
