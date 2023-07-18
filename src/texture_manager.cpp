//
// Created by stowy on 28/06/2023.
//

#include "texture_manager.hpp"
#include <spdlog/spdlog.h>

stw::Texture& stw::TextureManager::GetTexture(std::size_t index) { return m_Textures[index]; }

const stw::Texture& stw::TextureManager::GetTexture(std::size_t index) const { return m_Textures[index]; }

std::optional<std::size_t> stw::TextureManager::LoadTextureFromPath(
	const std::filesystem::path& path, stw::TextureType type, TextureSpace space)
{
	spdlog::debug("Loading texture {}...", path.string());

	if (auto result = m_TexturesCache.find(path); result != m_TexturesCache.end())
	{
		return { result->second };
	}

	std::expected<stw::Texture, std::string> textureResult;
	if (path.extension() == ".ktx")
	{
		textureResult = Texture::LoadKtxFromPath(path, type);
	}
	else
	{
		textureResult = Texture::LoadFromPath(path, type, space);
	}


	if (!textureResult.has_value())
	{
		spdlog::error("Error while loading texture : {}", textureResult.error());
		return {};
	}

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
