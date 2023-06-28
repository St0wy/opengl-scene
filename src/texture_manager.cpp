//
// Created by stowy on 28/06/2023.
//

#include "texture_manager.hpp"

stw::Texture& stw::TextureManager::GetTexture(std::size_t index)
{
	return m_Textures[index];
}

const stw::Texture& stw::TextureManager::GetTexture(std::size_t index) const
{
	return m_Textures[index];
}

std::optional<std::size_t>
stw::TextureManager::LoadTextureFromPath(const std::filesystem::path& path, stw::TextureType type)
{
	if (auto result = m_TexturesCache.find(path); result != m_TexturesCache.end())
	{
		return { result->second };
	}

	auto textureResult = Texture::LoadFromPath(path, type);

	if (!textureResult.has_value())
		return {};

	m_Textures.push_back(std::move(textureResult.value()));
	return { m_Textures.size() - 1 };
}

void stw::TextureManager::Delete()
{
	for (auto& texture : m_Textures)
	{
		texture.Delete();
	}
}
