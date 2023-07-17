#include "material.hpp"

#include <spdlog/spdlog.h>

#include "utils.hpp"

void stw::BindMaterialForGBuffer(const stw::Material& materialVariant,
	stw::TextureManager& textureManager,
	const std::array<std::reference_wrapper<stw::Pipeline>, MaterialCount>& gBufferPipelines)
{
	const auto pbrNormal = [&textureManager, &gBufferPipelines](const MaterialPbrNormal& material) {
		Pipeline const& pipeline = gBufferPipelines[0];
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

	const auto pbrNormalNoAo = [&textureManager, &gBufferPipelines](const MaterialPbrNormalNoAo& material) {
		Pipeline const& pipeline = gBufferPipelines[1];
		pipeline.Bind();

		// Base Color
		GLCALL(glActiveTexture(GL_TEXTURE0));
		textureManager.GetTexture(material.baseColorMapIndex).Bind();

		// Normal
		GLCALL(glActiveTexture(GL_TEXTURE1));
		textureManager.GetTexture(material.normalMapIndex).Bind();

		// Roughness
		GLCALL(glActiveTexture(GL_TEXTURE2));
		textureManager.GetTexture(material.roughnessMapIndex).Bind();

		// Metallic
		GLCALL(glActiveTexture(GL_TEXTURE3));
		textureManager.GetTexture(material.metallicMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
	};

	const auto pbrNormalArm = [&textureManager, &gBufferPipelines](const MaterialPbrNormalArm& material) {
		Pipeline const& pipeline = gBufferPipelines[2];
		pipeline.Bind();

		// Base Color
		GLCALL(glActiveTexture(GL_TEXTURE0));
		textureManager.GetTexture(material.baseColorMapIndex).Bind();

		// Normal
		GLCALL(glActiveTexture(GL_TEXTURE1));
		textureManager.GetTexture(material.normalMapIndex).Bind();

		// ARM
		GLCALL(glActiveTexture(GL_TEXTURE2));
		textureManager.GetTexture(material.armMapIndex).Bind();

		GLCALL(glActiveTexture(GL_TEXTURE0));
	};

	constexpr auto invalid = [](const InvalidMaterial&) {
		spdlog::error("Invalid material... {} {}", __FILE__, __LINE__);
	};

	std::visit(Overloaded{ invalid, pbrNormal, pbrNormalNoAo, pbrNormalArm }, materialVariant);
}