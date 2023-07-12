//
// Created by stowy on 28/06/2023.
//

#include "material_manager.hpp"

#include <span>
#include <spdlog/spdlog.h>

std::size_t stw::MaterialManager::LoadMaterialsFromAssimpScene(
	const aiScene* assimpScene, const std::filesystem::path& workingDirectory, TextureManager& textureManager)
{
	const std::size_t startIndex = m_Materials.size();
	const std::span<aiMaterial*> assimpMaterials{ assimpScene->mMaterials, assimpScene->mNumMaterials };
	for (const aiMaterial* material : assimpMaterials)
	{
		const auto diffuseCount = material->GetTextureCount(aiTextureType_DIFFUSE);
		const auto specularCount = material->GetTextureCount(aiTextureType_SPECULAR);
		const auto normalCount = material->GetTextureCount(aiTextureType_NORMALS);

		if (diffuseCount == 0)
		{
			continue;
		}

		if (specularCount == 0 && normalCount == 0)
		{
			LoadDiffuse(material, workingDirectory, textureManager);
		}
		else if (specularCount > 0 && normalCount == 0)
		{
			LoadDiffuseSpecular(material, workingDirectory, textureManager);
		}
		else
		{
			LoadDiffuseSpecularNormal(material, workingDirectory, textureManager);
		}
	}

	return startIndex;
}

void stw::MaterialManager::LoadDiffuse(
	const aiMaterial* material, const std::filesystem::path& workingDirectory, stw::TextureManager& textureManager)
{
	aiColor3D specular;
	if (material->Get(AI_MATKEY_COLOR_SPECULAR, specular) != AI_SUCCESS)
	{
		spdlog::warn("Could not read specular from material");
		specular = aiColor3D{ 0.5f, 0.5f, 0.5f };
	}

	aiString relativePath;
	auto result = material->GetTexture(aiTextureType_DIFFUSE, 0, &relativePath);

	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Error when loading texture");
	}

	const std::filesystem::path diffusePath = workingDirectory / relativePath.C_Str();

	auto loadTextureResult = textureManager.LoadTextureFromPath(diffusePath, TextureType::Diffuse);

	if (!loadTextureResult.has_value())
	{
		spdlog::error("Could not load texture : {}", diffusePath.string());
	}

	const std::size_t diffuseIndex = loadTextureResult.value();

	m_Materials.emplace_back(MaterialDiffuse{
		diffuseIndex,
	});
}

void stw::MaterialManager::LoadDiffuseSpecular(
	const aiMaterial* material, const std::filesystem::path& workingDirectory, stw::TextureManager& textureManager)
{
	aiString relativePath;

	material->GetTexture(aiTextureType_DIFFUSE, 0, &relativePath);
	const std::filesystem::path diffusePath = workingDirectory / relativePath.C_Str();
	const std::size_t diffuseIndex = textureManager.LoadTextureFromPath(diffusePath, TextureType::Diffuse).value();

	material->GetTexture(aiTextureType_SPECULAR, 0, &relativePath);
	const std::filesystem::path specularPath = workingDirectory / relativePath.C_Str();
	const std::size_t specularIndex = textureManager.LoadTextureFromPath(specularPath, TextureType::Specular).value();

	m_Materials.emplace_back(MaterialDiffuseSpecular{ specularIndex, diffuseIndex });
}

void stw::MaterialManager::LoadDiffuseSpecularNormal(
	const aiMaterial* material, const std::filesystem::path& workingDirectory, stw::TextureManager& textureManager)
{
	aiString relativePath;
	material->GetTexture(aiTextureType_NORMALS, 0, &relativePath);
	const std::filesystem::path normalPath = workingDirectory / relativePath.C_Str();
	const std::size_t normalIndex = textureManager.LoadTextureFromPath(normalPath, TextureType::Normal).value();

	material->GetTexture(aiTextureType_DIFFUSE, 0, &relativePath);
	const std::filesystem::path diffusePath = workingDirectory / relativePath.C_Str();
	const std::size_t diffuseIndex = textureManager.LoadTextureFromPath(diffusePath, TextureType::Diffuse).value();

	material->GetTexture(aiTextureType_SPECULAR, 0, &relativePath);
	const std::filesystem::path specularPath = workingDirectory / relativePath.C_Str();
	const std::size_t specularIndex = textureManager.LoadTextureFromPath(specularPath, TextureType::Specular).value();

	m_Materials.emplace_back(MaterialDiffuseSpecularNormal{ diffuseIndex, specularIndex, normalIndex });
}

stw::Material& stw::MaterialManager::operator[](std::size_t index) { return m_Materials[index]; }

const stw::Material& stw::MaterialManager::operator[](std::size_t index) const { return m_Materials[index]; }
