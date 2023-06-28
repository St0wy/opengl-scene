#include "material.hpp"

#include <spdlog/spdlog.h>

#include "utils.hpp"

void stw::BindMaterial(const Material& materialVariant, TextureManager& textureManager)
{
	const auto normalNoSpecular = [&textureManager](const MaterialNormalNoSpecular& material)
	{
		auto& pipeline = material.pipeline;
		pipeline.SetFloat("material.shininess", material.shininess);
		pipeline.SetVec3("material.specular", material.specular);

		textureManager.GetTexture(material.normalMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
		pipeline.SetInt("material.texture_diffuse1", 0);
		textureManager.GetTexture(material.diffuseMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE1));
		pipeline.SetInt("material.texture_normal1", 1);
		textureManager.GetTexture(material.normalMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
	};

	constexpr auto noNormalNoSpecular = [](const MaterialNoNormalNoSpecular& material)
	{
		spdlog::warn("Not implemented... {} {}", __FILE__, __LINE__);
	};

	constexpr auto invalid = [](const InvalidMaterial& material)
	{
		spdlog::warn("Invalid material... {} {}", __FILE__, __LINE__);
	};

	std::visit(Overloaded{ invalid, normalNoSpecular, noNormalNoSpecular }, materialVariant);
}
