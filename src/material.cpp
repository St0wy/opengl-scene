#include "material.hpp"

#include <spdlog/spdlog.h>

#include "utils.hpp"

void stw::BindMaterialForGBuffer(
	const stw::Material& materialVariant, stw::TextureManager& textureManager, stw::Pipeline& gBufferPipeline)
{
	const auto pbrNormal = [&textureManager, &gBufferPipeline](const MaterialPbrNormal& material) {
		Pipeline const& pipeline = gBufferPipeline;
		pipeline.Bind();

		// Base Color
		GLCALL(glActiveTexture(GL_TEXTURE0));
		textureManager.GetTexture(material.baseColorMapIndex).Bind();

		// Normal
		GLCALL(glActiveTexture(GL_TEXTURE1));
		textureManager.GetTexture(material.normalMapIndex).Bind();

		// Ambient Occlusion
		GLCALL(glActiveTexture(GL_TEXTURE2));
		textureManager.GetTexture(material.ambientOcclusionMapIndex).Bind();

		// Roughness
		GLCALL(glActiveTexture(GL_TEXTURE3));
		textureManager.GetTexture(material.roughnessMapIndex).Bind();

		// Metallic
		GLCALL(glActiveTexture(GL_TEXTURE4));
		textureManager.GetTexture(material.metallicMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
	};

	constexpr auto invalid = [](const InvalidMaterial&) {
		spdlog::error("Invalid material... {} {}", __FILE__, __LINE__);
	};

	std::visit(Overloaded{ invalid, pbrNormal }, materialVariant);
}