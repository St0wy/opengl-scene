#include "material.hpp"

#include <spdlog/spdlog.h>

#include "utils.hpp"

void stw::BindMaterial(const Material& materialVariant, TextureManager& textureManager)
{
	const auto normalNoSpecular = [&textureManager](const MaterialNormalNoSpecular& material) {
		Pipeline& pipeline = material.pipeline;
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
	};

	const auto noNormalNoSpecular = [&textureManager](const MaterialNoNormalNoSpecular& material) {
		Pipeline& pipeline = material.pipeline;
		pipeline.Bind();
		pipeline.SetFloat("material.shininess", material.shininess);
		pipeline.SetVec3("material.specular", material.specular);

		GLCALL(glActiveTexture(GL_TEXTURE0));
		pipeline.SetInt("material.texture_diffuse1", 0);
		textureManager.GetTexture(material.diffuseMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
	};

	const auto normalSpecular = [&textureManager](const MaterialNormalSpecular& material) {
		Pipeline& pipeline = material.pipeline;
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
	};

	const auto noNormalSpecular = [&textureManager](const MaterialNoNormalSpecular& material) {
		Pipeline& pipeline = material.pipeline;
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

std::optional<std::reference_wrapper<stw::Pipeline>> stw::GetPipelineFromMaterial(const Material& materialVariant)
{
	constexpr auto normalNoSpecular =
		[](const MaterialNormalNoSpecular& material) -> std::optional<std::reference_wrapper<stw::Pipeline>> {
		return material.pipeline;
	};

	constexpr auto noNormalNoSpecular =
		[](const MaterialNoNormalNoSpecular& material) -> std::optional<std::reference_wrapper<stw::Pipeline>> {
		return material.pipeline;
	};

	constexpr auto normalSpecular =
		[](const MaterialNormalSpecular& material) -> std::optional<std::reference_wrapper<stw::Pipeline>> {
		return material.pipeline;
	};

	constexpr auto noNormalSpecular =
		[](const MaterialNoNormalSpecular& material) -> std::optional<std::reference_wrapper<stw::Pipeline>> {
		return material.pipeline;
	};

	constexpr auto invalid =
		[](const InvalidMaterial& material) -> std::optional<std::reference_wrapper<stw::Pipeline>> {
		spdlog::error("Invalid material when getting pipeline... {} {}", __FILE__, __LINE__);
		return std::nullopt;
	};

	return std::visit(
		Overloaded{ invalid, normalNoSpecular, noNormalNoSpecular, normalSpecular, noNormalSpecular }, materialVariant);
}
