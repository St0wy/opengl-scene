//
// Created by stowy on 28/06/2023.
//

#pragma once

#include <assimp/scene.h>
#include <filesystem>
#include <vector>

#include "material.hpp"
#include "texture_manager.hpp"

namespace stw
{


class MaterialManager
{
public:
	std::vector<std::size_t> LoadMaterialsFromAssimpScene(
		const aiScene* assimpScene, const std::filesystem::path& workingDirectory, TextureManager& textureManager);
	Material& operator[](std::size_t index);
	const Material& operator[](std::size_t index) const;
	[[nodiscard]] std::size_t Size() const;

private:
	void LoadPbrNormal(
		const aiMaterial* material, const std::filesystem::path& workingDirectory, stw::TextureManager& textureManager);
	void LoadPbrNormalNoAo(
		const aiMaterial* material, const std::filesystem::path& workingDirectory, stw::TextureManager& textureManager);
	void LoadPbrNormalArm(
		const aiMaterial* material, const std::filesystem::path& workingDirectory, stw::TextureManager& textureManager);

	std::vector<Material> m_Materials;
};
}// namespace stw
