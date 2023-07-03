#include "material.hpp"

#include <spdlog/spdlog.h>

#include "utils.hpp"

void stw::BindMaterial(const Material& materialVariant, TextureManager& textureManager)
{
	const auto normalNoSpecular = [&textureManager](const MaterialNormalNoSpecular& material) {
		auto& pipeline = material.pipeline;
		pipeline.Bind();
		pipeline.SetFloat("material.shininess", material.shininess);
		pipeline.SetVec3("material.specular", material.specular);

		GLCALL(glActiveTexture(GL_TEXTURE0));
		pipeline.SetInt("material.texture_diffuse1", 0);
		textureManager.GetTexture(material.diffuseMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE1));
		pipeline.SetInt("material.texture_normal1", 1);
		textureManager.GetTexture(material.normalMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
		pipeline.UnBind();
	};

	const auto noNormalNoSpecular = [&textureManager](const MaterialNoNormalNoSpecular& material) {
		auto& pipeline = material.pipeline;
		pipeline.Bind();
		pipeline.SetFloat("material.shininess", material.shininess);
		pipeline.SetVec3("material.specular", material.specular);

		GLCALL(glActiveTexture(GL_TEXTURE0));
		pipeline.SetInt("material.texture_diffuse1", 0);
		textureManager.GetTexture(material.diffuseMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
		pipeline.UnBind();
	};

	const auto normalSpecular = [&textureManager](const MaterialNormalSpecular& material) {
		auto& pipeline = material.pipeline;
		pipeline.Bind();
		pipeline.SetFloat("material.shininess", material.shininess);

		GLCALL(glActiveTexture(GL_TEXTURE0));
		pipeline.SetInt("material.texture_diffuse1", 0);
		textureManager.GetTexture(material.diffuseMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE1));
		pipeline.SetInt("material.texture_normal1", 1);
		textureManager.GetTexture(material.normalMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE2));
		pipeline.SetInt("material.texture_specular1", 2);
		textureManager.GetTexture(material.specularMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
		pipeline.UnBind();
	};

	const auto noNormalSpecular = [&textureManager](const MaterialNoNormalSpecular& material) {
		auto& pipeline = material.pipeline;
		pipeline.Bind();
		pipeline.SetFloat("material.shininess", material.shininess);

		GLCALL(glActiveTexture(GL_TEXTURE0));
		pipeline.SetInt("material.texture_diffuse1", 0);
		textureManager.GetTexture(material.diffuseMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE1));
		pipeline.SetInt("material.texture_specular1", 1);
		textureManager.GetTexture(material.specularMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
	};

	constexpr auto invalid = [](const InvalidMaterial& material) {
		spdlog::error("Invalid material... {} {}", __FILE__, __LINE__);
	};

	std::visit(
		Overloaded{ invalid, normalNoSpecular, noNormalNoSpecular, normalSpecular, noNormalSpecular }, materialVariant);
}
