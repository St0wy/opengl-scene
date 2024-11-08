/**
 * @file material_manager.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains MaterialManager class.
 * @version 1.0
 * @date 28/06/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <filesystem>
#include <span>
#include <vector>

#include <assimp/scene.h>
#include <spdlog/spdlog.h>

export module material_manager;

import number_types;
import texture;
import texture_manager;
import material;

export namespace stw
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
		const aiMaterial* material, const std::filesystem::path& workingDirectory, TextureManager& textureManager);
	void LoadPbrNormalNoAo(
		const aiMaterial* material, const std::filesystem::path& workingDirectory, TextureManager& textureManager);
	void LoadPbrNormalArm(
		const aiMaterial* material, const std::filesystem::path& workingDirectory, TextureManager& textureManager);

	std::vector<Material> m_Materials;
};

std::vector<std::size_t> MaterialManager::LoadMaterialsFromAssimpScene(
	const aiScene* assimpScene, const std::filesystem::path& workingDirectory, TextureManager& textureManager)
{
	const std::span assimpMaterials{ assimpScene->mMaterials, assimpScene->mNumMaterials };

	std::vector<std::size_t> assimpMaterialIndicesLoaded;
	assimpMaterialIndicesLoaded.reserve(assimpMaterials.size());

	for (usize i = 0; i < assimpMaterials.size(); i++)
	{
		const aiMaterial* material = assimpMaterials[i];
		const auto diffuseCount = material->GetTextureCount(aiTextureType_DIFFUSE);
		const auto baseColorCount = material->GetTextureCount(aiTextureType_BASE_COLOR);
		const auto normalCount = material->GetTextureCount(aiTextureType_NORMALS);
		const auto roughnessCount = material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS);
		const auto ambientCount = material->GetTextureCount(aiTextureType_AMBIENT);
		const auto ambientOcclusionCount = material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION);
		const auto metallicCount = material->GetTextureCount(aiTextureType_METALNESS);
		const auto unknownCount = material->GetTextureCount(aiTextureType_UNKNOWN);

		if ((diffuseCount > 0 || baseColorCount > 0) && normalCount > 0 && roughnessCount > 0 && metallicCount > 0)
		{
			if (unknownCount == 0)
			{
				if (ambientCount > 0 || ambientOcclusionCount > 0)
				{
					LoadPbrNormal(material, workingDirectory, textureManager);
					assimpMaterialIndicesLoaded.push_back(i);
				}
				else
				{
					LoadPbrNormalNoAo(material, workingDirectory, textureManager);
					assimpMaterialIndicesLoaded.push_back(i);
				}
			}
			else
			{
				LoadPbrNormalArm(material, workingDirectory, textureManager);
				assimpMaterialIndicesLoaded.push_back(i);
			}
		}
		else
		{
			spdlog::debug("Unhandled material");
		}
	}

	return assimpMaterialIndicesLoaded;
}

Material& MaterialManager::operator[](const std::size_t index) { return m_Materials[index]; }

const Material& MaterialManager::operator[](const std::size_t index) const { return m_Materials[index]; }

void MaterialManager::LoadPbrNormal(
	const aiMaterial* material, const std::filesystem::path& workingDirectory, TextureManager& textureManager)
{
	aiString relativePath;
	aiReturn result = material->GetTexture(aiTextureType_NORMALS, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Could not get normal map");
		return;
	}

	const std::filesystem::path normalPath = workingDirectory / relativePath.C_Str();
	const std::size_t normalIndex =
		textureManager.LoadTextureFromPath(normalPath, TextureType::Normal, TextureSpace::Linear).value();

	result = material->GetTexture(aiTextureType_DIFFUSE, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		result = material->GetTexture(aiTextureType_BASE_COLOR, 0, &relativePath);
		if (result != aiReturn_SUCCESS)
		{
			spdlog::error("Could not get diffuse / base color map");
			return;
		}
	}
	const std::filesystem::path baseColorPath = workingDirectory / relativePath.C_Str();
	const std::size_t baseColorIndex =
		textureManager.LoadTextureFromPath(baseColorPath, TextureType::BaseColor, TextureSpace::Srgb).value();

	result = material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Could not get roughness map");
		return;
	}
	const std::filesystem::path roughnessPath = workingDirectory / relativePath.C_Str();
	const std::size_t roughnessIndex =
		textureManager.LoadTextureFromPath(roughnessPath, TextureType::Roughness, TextureSpace::Linear).value();

	result = material->GetTexture(aiTextureType_AMBIENT, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		result = material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &relativePath);
		if (result != aiReturn_SUCCESS)
		{
			spdlog::error("Could not get ambient occlusion map");
			return;
		}
	}
	const std::filesystem::path ambientOcclusionPath = workingDirectory / relativePath.C_Str();
	const std::size_t ambientOcclusionIndex =
		textureManager.LoadTextureFromPath(ambientOcclusionPath, TextureType::AmbientOcclusion, TextureSpace::Linear)
			.value();

	result = material->GetTexture(aiTextureType_METALNESS, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Could not get metallic map");
		return;
	}
	const std::filesystem::path metallicPath = workingDirectory / relativePath.C_Str();
	const std::size_t metallicIndex =
		textureManager.LoadTextureFromPath(metallicPath, TextureType::Metallic, TextureSpace::Linear).value();

	m_Materials.emplace_back(
		MaterialPbrNormal{ baseColorIndex, normalIndex, ambientOcclusionIndex, roughnessIndex, metallicIndex });
}
void MaterialManager::LoadPbrNormalNoAo(
	const aiMaterial* material, const std::filesystem::path& workingDirectory, TextureManager& textureManager)
{

	aiString relativePath;
	aiReturn result = material->GetTexture(aiTextureType_NORMALS, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Could not get normal map");
		return;
	}

	const std::filesystem::path normalPath = workingDirectory / relativePath.C_Str();
	const std::size_t normalIndex =
		textureManager.LoadTextureFromPath(normalPath, TextureType::Normal, TextureSpace::Linear).value();

	result = material->GetTexture(aiTextureType_DIFFUSE, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		result = material->GetTexture(aiTextureType_BASE_COLOR, 0, &relativePath);
		if (result != aiReturn_SUCCESS)
		{
			spdlog::error("Could not get diffuse / base color map");
			return;
		}
	}
	const std::filesystem::path baseColorPath = workingDirectory / relativePath.C_Str();
	const std::size_t baseColorIndex =
		textureManager.LoadTextureFromPath(baseColorPath, TextureType::BaseColor, TextureSpace::Srgb).value();

	result = material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Could not get roughness map");
		return;
	}
	const std::filesystem::path roughnessPath = workingDirectory / relativePath.C_Str();
	const std::size_t roughnessIndex =
		textureManager.LoadTextureFromPath(roughnessPath, TextureType::Roughness, TextureSpace::Linear).value();

	result = material->GetTexture(aiTextureType_METALNESS, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Could not get metallic map");
		return;
	}
	const std::filesystem::path metallicPath = workingDirectory / relativePath.C_Str();
	const std::size_t metallicIndex =
		textureManager.LoadTextureFromPath(metallicPath, TextureType::Metallic, TextureSpace::Linear).value();

	m_Materials.emplace_back(MaterialPbrNormalNoAo{ baseColorIndex, normalIndex, roughnessIndex, metallicIndex });
}

void MaterialManager::LoadPbrNormalArm(
	const aiMaterial* material, const std::filesystem::path& workingDirectory, TextureManager& textureManager)
{
	aiString relativePath;
	aiReturn result = material->GetTexture(aiTextureType_NORMALS, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Could not get normal map");
		return;
	}

	const std::filesystem::path normalPath = workingDirectory / relativePath.C_Str();
	const std::size_t normalIndex =
		textureManager.LoadTextureFromPath(normalPath, TextureType::Normal, TextureSpace::Linear).value();

	result = material->GetTexture(aiTextureType_DIFFUSE, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		result = material->GetTexture(aiTextureType_BASE_COLOR, 0, &relativePath);
		if (result != aiReturn_SUCCESS)
		{
			spdlog::error("Could not get diffuse / base color map");
			return;
		}
	}
	const std::filesystem::path baseColorPath = workingDirectory / relativePath.C_Str();
	const std::size_t baseColorIndex =
		textureManager.LoadTextureFromPath(baseColorPath, TextureType::BaseColor, TextureSpace::Srgb).value();

	// Load ARM (ambient, roughness, metallic) map
	result = material->GetTexture(aiTextureType_UNKNOWN, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Could not get ARM map");
		return;
	}
	const std::filesystem::path armPath = workingDirectory / relativePath.C_Str();
	const std::size_t armIndex =
		textureManager.LoadTextureFromPath(armPath, TextureType::Roughness, TextureSpace::Linear).value();

	m_Materials.emplace_back(MaterialPbrNormalArm{ baseColorIndex, normalIndex, armIndex });
}

std::size_t MaterialManager::Size() const { return m_Materials.size(); }
}// namespace stw
