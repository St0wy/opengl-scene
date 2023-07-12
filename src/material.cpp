#include "material.hpp"

#include <spdlog/spdlog.h>

#include "utils.hpp"

void stw::BindMaterialForGBuffer(
	const stw::Material& materialVariant, stw::TextureManager& textureManager, stw::Pipeline& gBufferPipeline)
{

	const auto noNormalNoSpecular = [&textureManager, &gBufferPipeline](const MaterialDiffuse& material) {
		Pipeline& pipeline = gBufferPipeline;
		pipeline.Bind();
		GLCALL(glActiveTexture(GL_TEXTURE0));
		pipeline.SetInt("texture_diffuse", 0);
		textureManager.GetTexture(material.diffuseMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
	};

	const auto normalSpecular = [&textureManager, &gBufferPipeline](const MaterialDiffuseSpecularNormal& material) {
		Pipeline& pipeline = gBufferPipeline;
		pipeline.Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
		pipeline.SetInt("texture_diffuse", 0);
		textureManager.GetTexture(material.diffuseMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE1));
		pipeline.SetInt("texture_normal", 1);
		textureManager.GetTexture(material.normalMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE2));
		pipeline.SetInt("texture_specular", 2);
		textureManager.GetTexture(material.specularMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
	};

	const auto noNormalSpecular = [&textureManager, &gBufferPipeline](const MaterialDiffuseSpecular& material) {
		Pipeline& pipeline = gBufferPipeline;
		pipeline.Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
		pipeline.SetInt("texture_diffuse", 0);
		textureManager.GetTexture(material.diffuseMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE1));
		pipeline.SetInt("texture_specular", 1);
		textureManager.GetTexture(material.specularMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
	};

	constexpr auto invalid = [](const InvalidMaterial&) {
		spdlog::error("Invalid material... {} {}", __FILE__, __LINE__);
	};

	std::visit(Overloaded{ invalid, noNormalNoSpecular, normalSpecular, noNormalSpecular }, materialVariant);
}