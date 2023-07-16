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
		const auto baseColorCount = material->GetTextureCount(aiTextureType_BASE_COLOR);
		const auto normalCount = material->GetTextureCount(aiTextureType_NORMALS);
		const auto roughnessCount = material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS);
		const auto ambientCount = material->GetTextureCount(aiTextureType_AMBIENT);
		const auto ambientOcclusionCount = material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION);
		const auto metallicCount = material->GetTextureCount(aiTextureType_METALNESS);


		if ((diffuseCount > 0 || baseColorCount > 0) > 0 && normalCount > 0 && roughnessCount > 0 && metallicCount > 0)
		{
			if (ambientCount > 0 || ambientOcclusionCount > 0)
			{
				LoadPbrNormal(material, workingDirectory, textureManager);
			}
			else
			{
				LoadPbrNormalNoAo(material, workingDirectory, textureManager);
			}
		}
	}

	return startIndex;
}

stw::Material& stw::MaterialManager::operator[](std::size_t index) { return m_Materials[index]; }

const stw::Material& stw::MaterialManager::operator[](std::size_t index) const { return m_Materials[index]; }

void stw::MaterialManager::LoadPbrNormal(
	const aiMaterial* material, const std::filesystem::path& workingDirectory, stw::TextureManager& textureManager)
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
		textureManager.LoadTextureFromPath(normalPath, TextureType::Normal, stw::TextureSpace::Linear).value();

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
		textureManager.LoadTextureFromPath(baseColorPath, TextureType::BaseColor, stw::TextureSpace::Srgb).value();

	result = material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Could not get roughness map");
		return;
	}
	const std::filesystem::path roughnessPath = workingDirectory / relativePath.C_Str();
	const std::size_t roughnessIndex =
		textureManager.LoadTextureFromPath(roughnessPath, TextureType::Roughness, stw::TextureSpace::Linear).value();

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
		textureManager
			.LoadTextureFromPath(ambientOcclusionPath, TextureType::AmbientOcclusion, stw::TextureSpace::Linear)
			.value();

	result = material->GetTexture(aiTextureType_METALNESS, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Could not get metallic map");
		return;
	}
	const std::filesystem::path metallicPath = workingDirectory / relativePath.C_Str();
	const std::size_t metallicIndex =
		textureManager.LoadTextureFromPath(metallicPath, TextureType::Metallic, stw::TextureSpace::Linear).value();

	m_Materials.emplace_back(
		MaterialPbrNormal{ baseColorIndex, normalIndex, ambientOcclusionIndex, roughnessIndex, metallicIndex });
}
void stw::MaterialManager::LoadPbrNormalNoAo(
	const aiMaterial* material, const std::filesystem::path& workingDirectory, stw::TextureManager& textureManager)
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
		textureManager.LoadTextureFromPath(normalPath, TextureType::Normal, stw::TextureSpace::Linear).value();

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
		textureManager.LoadTextureFromPath(baseColorPath, TextureType::BaseColor, stw::TextureSpace::Srgb).value();

	result = material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Could not get roughness map");
		return;
	}
	const std::filesystem::path roughnessPath = workingDirectory / relativePath.C_Str();
	const std::size_t roughnessIndex =
		textureManager.LoadTextureFromPath(roughnessPath, TextureType::Roughness, stw::TextureSpace::Linear).value();

	result = material->GetTexture(aiTextureType_METALNESS, 0, &relativePath);
	if (result != aiReturn_SUCCESS)
	{
		spdlog::error("Could not get metallic map");
		return;
	}
	const std::filesystem::path metallicPath = workingDirectory / relativePath.C_Str();
	const std::size_t metallicIndex =
		textureManager.LoadTextureFromPath(metallicPath, TextureType::Metallic, stw::TextureSpace::Linear).value();

	m_Materials.emplace_back(MaterialPbrNormalNoAo{ baseColorIndex, normalIndex, roughnessIndex, metallicIndex });
}
