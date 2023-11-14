/**
 * @file material.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the material variant.
 * @version 1.0
 * @date 28/06/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <array>
#include <variant>

#include <GL/glew.h>
#include <spdlog/spdlog.h>

export module material;

import number_types;
import consts;
import utils;
import texture_manager;
import pipeline;

export namespace stw
{
struct InvalidMaterial
{
};

struct MaterialPbrNormal
{
	std::size_t baseColorMapIndex{};
	std::size_t normalMapIndex{};
	std::size_t ambientOcclusionMapIndex{};
	std::size_t roughnessMapIndex{};
	std::size_t metallicMapIndex{};
};

struct MaterialPbrNormalNoAo
{
	std::size_t baseColorMapIndex{};
	std::size_t normalMapIndex{};
	std::size_t roughnessMapIndex{};
	std::size_t metallicMapIndex{};
};

struct MaterialPbrNormalArm
{
	std::size_t baseColorMapIndex{};
	std::size_t normalMapIndex{};
	std::size_t armMapIndex{};
};

using Material = std::variant<InvalidMaterial, MaterialPbrNormal, MaterialPbrNormalNoAo, MaterialPbrNormalArm>;

constexpr u32 MaterialCount = std::variant_size_v<Material> - 1;

void BindMaterialForGBuffer(const Material& materialVariant,
	TextureManager& textureManager,
	const std::array<std::reference_wrapper<Pipeline>, MaterialCount>& gBufferPipelines);

void stw::BindMaterialForGBuffer(const Material& materialVariant,
	TextureManager& textureManager,
	const std::array<std::reference_wrapper<Pipeline>, MaterialCount>& gBufferPipelines)
{
	const auto pbrNormal = [&textureManager, &gBufferPipelines](const MaterialPbrNormal& material) {
		Pipeline& pipeline = gBufferPipelines[0];
		pipeline.Bind();

		// Base Color
		glActiveTexture(GL_TEXTURE0);
		textureManager.GetTexture(material.baseColorMapIndex).Bind();

		// Normal
		glActiveTexture(GL_TEXTURE1);
		textureManager.GetTexture(material.normalMapIndex).Bind();

		// Ambient Occlusion
		glActiveTexture(GL_TEXTURE2);
		textureManager.GetTexture(material.ambientOcclusionMapIndex).Bind();

		// Roughness
		glActiveTexture(GL_TEXTURE3);
		textureManager.GetTexture(material.roughnessMapIndex).Bind();

		// Metallic
		glActiveTexture(GL_TEXTURE4);
		textureManager.GetTexture(material.metallicMapIndex).Bind();

		glActiveTexture(GL_TEXTURE0);
	};

	const auto pbrNormalNoAo = [&textureManager, &gBufferPipelines](const MaterialPbrNormalNoAo& material) {
		Pipeline& pipeline = gBufferPipelines[1];
		pipeline.Bind();

		// Base Color
		glActiveTexture(GL_TEXTURE0);
		textureManager.GetTexture(material.baseColorMapIndex).Bind();

		// Normal
		glActiveTexture(GL_TEXTURE1);
		textureManager.GetTexture(material.normalMapIndex).Bind();

		// Roughness
		glActiveTexture(GL_TEXTURE2);
		textureManager.GetTexture(material.roughnessMapIndex).Bind();

		// Metallic
		glActiveTexture(GL_TEXTURE3);
		textureManager.GetTexture(material.metallicMapIndex).Bind();

		glActiveTexture(GL_TEXTURE0);
	};

	const auto pbrNormalArm = [&textureManager, &gBufferPipelines](const MaterialPbrNormalArm& material) {
		Pipeline& pipeline = gBufferPipelines[2];
		pipeline.Bind();

		// Base Color
		glActiveTexture(GL_TEXTURE0);
		textureManager.GetTexture(material.baseColorMapIndex).Bind();

		// Normal
		glActiveTexture(GL_TEXTURE1);
		textureManager.GetTexture(material.normalMapIndex).Bind();

		// ARM
		glActiveTexture(GL_TEXTURE2);
		textureManager.GetTexture(material.armMapIndex).Bind();

		glActiveTexture(GL_TEXTURE0);
	};

	constexpr auto invalid = [](const InvalidMaterial&) {
		spdlog::error("Invalid material... {} {}", __FILE__, __LINE__);
	};

	std::visit(Overloaded{ invalid, pbrNormal, pbrNormalNoAo, pbrNormalArm }, materialVariant);
}
}// namespace stw
