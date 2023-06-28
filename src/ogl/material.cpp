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

	constexpr auto invalid = [](const InvalidMaterial& material)
	{
		spdlog::warn("Invalid material... {} {}", __FILE__, __LINE__);
	};

	std::visit(Overloaded{invalid, normalNoSpecular, noNormalNoSpecular}, materialVariant);
}
