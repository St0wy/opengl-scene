//
// Created by stowy on 28/06/2023.
//

#pragma once

#include <vector>
#include <assimp/scene.h>

#include "material.hpp"
#include "texture_manager.hpp"

namespace stw
{
class MaterialManager
{
public:
	std::size_t
	LoadMaterialsFromAssimpScene(const aiScene* assimpScene, const std::filesystem::path& workingDirectory, TextureManager& textureManager, Pipeline& pipeline);
	Material& operator[](std::size_t index);
	const Material& operator[](std::size_t index) const;
private:
	void
	LoadNoNormalNoSpecular(aiMaterial* material, const std::filesystem::path& workingDirectory, TextureManager& textureManager, Pipeline& pipeline);
	void
	LoadNormalNoSpecular(aiMaterial* material, const std::filesystem::path& workingDirectory, TextureManager& textureManager, Pipeline& pipeline);
	void
	LoadNoNormalSpecular(aiMaterial* material, const std::filesystem::path& workingDirectory, TextureManager& textureManager, Pipeline& pipeline);
	void
	LoadNormalSpecular(aiMaterial* material, const std::filesystem::path& workingDirectory, TextureManager& textureManager, Pipeline& pipeline);

	std::vector<Material> m_Materials;
};
}
