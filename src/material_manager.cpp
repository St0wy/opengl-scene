//
// Created by stowy on 28/06/2023.
//

#include "material_manager.hpp"

#include <spdlog/spdlog.h>

std::size_t stw::MaterialManager::LoadMaterialsFromAssimpScene(const aiScene* assimpScene,
	const std::filesystem::path& workingDirectory,
	TextureManager& textureManager,
	Pipeline& pipeline)
{
	const std::size_t startIndex = m_Materials.size();
	for (std::size_t i = 0; i < assimpScene->mNumMaterials; i++)
	{
		aiMaterial* material = assimpScene->mMaterials[i];
		const auto diffuseCount = material->GetTextureCount(aiTextureType_DIFFUSE);
		const auto specularCount = material->GetTextureCount(aiTextureType_SPECULAR);
		const auto normalCount = material->GetTextureCount(aiTextureType_NORMALS);

		if (diffuseCount == 0) continue;

		if (specularCount == 0 && normalCount == 0)
		{
			LoadNoNormalNoSpecular(material, workingDirectory, textureManager, pipeline);
		}
		else if (specularCount > 0 && normalCount == 0)
		{
			LoadNoNormalSpecular(material, workingDirectory, textureManager, pipeline);
		}
		else if (specularCount == 0)
		{
			LoadNormalNoSpecular(material, workingDirectory, textureManager, pipeline);
		}
		else
		{
			LoadNormalSpecular(material, workingDirectory, textureManager, pipeline);
		}
	}

	return startIndex;
}

void stw::MaterialManager::LoadNormalNoSpecular(const aiMaterial* material,
	const std::filesystem::path& workingDirectory,
	TextureManager& textureManager,
	Pipeline& pipeline)
{
	f32 shininess = 0.0f;
	if (material->Get(AI_MATKEY_SHININESS, shininess) != AI_SUCCESS)
	{
		spdlog::error("Could not read shininess from material");
		return;
	}

	aiColor3D specular;
	if (material->Get(AI_MATKEY_COLOR_SPECULAR, specular) != AI_SUCCESS)
	{
		spdlog::error("Could not read specular from material");
		return;
	}

	aiString relativePath;
	material->GetTexture(aiTextureType_NORMALS, 0, &relativePath);
	std::filesystem::path normalPath = workingDirectory / relativePath.C_Str();
	std::size_t normalIndex = textureManager.LoadTextureFromPath(normalPath, TextureType::Normal).value();

	material->GetTexture(aiTextureType_DIFFUSE, 0, &relativePath);
	std::filesystem::path diffusePath = workingDirectory / relativePath.C_Str();
	std::size_t diffuseIndex = textureManager.LoadTextureFromPath(diffusePath, TextureType::Diffuse).value();

	m_Materials.emplace_back(MaterialNormalNoSpecular{
		{ pipeline }, { specular.r, specular.g, specular.b }, shininess, diffuseIndex, normalIndex });
}

void stw::MaterialManager::LoadNoNormalNoSpecular(aiMaterial* material,
	const std::filesystem::path& workingDirectory,
	stw::TextureManager& textureManager,
	stw::Pipeline& pipeline)
{
	f32 shininess = 0.0f;
	if (material->Get(AI_MATKEY_SHININESS, shininess) != AI_SUCCESS)
	{
		spdlog::error("Could not read shininess from material");
		return;
	}

	aiColor3D specular;
	if (material->Get(AI_MATKEY_COLOR_SPECULAR, specular) != AI_SUCCESS)
	{
		spdlog::error("Could not read specular from material");
		return;
	}

	aiString relativePath;
	material->GetTexture(aiTextureType_DIFFUSE, 0, &relativePath);
	std::filesystem::path diffusePath = workingDirectory / relativePath.C_Str();
	std::size_t diffuseIndex = textureManager.LoadTextureFromPath(diffusePath, TextureType::Diffuse).value();

	m_Materials.emplace_back(MaterialNoNormalNoSpecular{
		{ pipeline },
		{ specular.r, specular.g, specular.b },
		shininess,
		diffuseIndex,
	});
}

void stw::MaterialManager::LoadNoNormalSpecular(aiMaterial* material,
	const std::filesystem::path& workingDirectory,
	stw::TextureManager& textureManager,
	stw::Pipeline& pipeline)
{
	f32 shininess = 0.0f;
	if (material->Get(AI_MATKEY_SHININESS, shininess) != AI_SUCCESS)
	{
		spdlog::error("Could not read shininess from material");
		return;
	}

	aiString relativePath;

	material->GetTexture(aiTextureType_DIFFUSE, 0, &relativePath);
	std::filesystem::path diffusePath = workingDirectory / relativePath.C_Str();
	std::size_t diffuseIndex = textureManager.LoadTextureFromPath(diffusePath, TextureType::Diffuse).value();

	material->GetTexture(aiTextureType_SPECULAR, 0, &relativePath);
	std::filesystem::path specularPath = workingDirectory / relativePath.C_Str();
	std::size_t specularIndex = textureManager.LoadTextureFromPath(specularPath, TextureType::Specular).value();

	m_Materials.emplace_back(MaterialNoNormalSpecular{ { pipeline }, shininess, diffuseIndex, specularIndex });
}

void stw::MaterialManager::LoadNormalSpecular(aiMaterial* material,
	const std::filesystem::path& workingDirectory,
	stw::TextureManager& textureManager,
	stw::Pipeline& pipeline)
{
	f32 shininess = 0.0f;
	if (material->Get(AI_MATKEY_SHININESS, shininess) != AI_SUCCESS)
	{
		spdlog::error("Could not read shininess from material");
		return;
	}

	aiString relativePath;
	material->GetTexture(aiTextureType_NORMALS, 0, &relativePath);
	std::filesystem::path normalPath = workingDirectory / relativePath.C_Str();
	std::size_t normalIndex = textureManager.LoadTextureFromPath(normalPath, TextureType::Normal).value();

	material->GetTexture(aiTextureType_DIFFUSE, 0, &relativePath);
	std::filesystem::path diffusePath = workingDirectory / relativePath.C_Str();
	std::size_t diffuseIndex = textureManager.LoadTextureFromPath(diffusePath, TextureType::Diffuse).value();

	material->GetTexture(aiTextureType_SPECULAR, 0, &relativePath);
	std::filesystem::path specularPath = workingDirectory / relativePath.C_Str();
	std::size_t specularIndex = textureManager.LoadTextureFromPath(specularPath, TextureType::Specular).value();

	m_Materials.emplace_back(
		MaterialNormalSpecular{ { pipeline }, shininess, diffuseIndex, specularIndex, normalIndex });
}

stw::Material& stw::MaterialManager::operator[](std::size_t index) { return m_Materials[index]; }

const stw::Material& stw::MaterialManager::operator[](std::size_t index) const { return m_Materials[index]; }
