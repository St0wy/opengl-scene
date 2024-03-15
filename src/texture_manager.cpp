/**
 * @file texture_manager.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the TextureManager class.
 * @version 1.0
 * @date 28/06/2023
 *
 * @copyright SAE (c) 2023
 *
 */
module;

#include <expected>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <vector>

#include <spdlog/spdlog.h>

export module texture_manager;

import texture;
import number_types;
import consts;
import utils;

export namespace stw
{
/**
 * Manages all textures in this renderer. When a material wants to reference a texture, it gets an ID that can be used
 * on this manager.
 */
class TextureManager
{
public:
	/**
	 * Loads a texture from the provided path.
	 * @param path Path to the texture.
	 * @param type What will this texture be used for ?
	 * @param space This is usually SRGB for Base Color and Linear for the rest.
	 * @return An index to the loaded texture or nothing if the loading failed.
	 */
	std::optional<std::size_t> LoadTextureFromPath(
		const std::filesystem::path& path, TextureType type, TextureSpace space);
	[[nodiscard]] Texture& GetTexture(std::size_t index);
	[[nodiscard]] const Texture& GetTexture(std::size_t index) const;

	void Delete();

private:
	std::vector<Texture> m_Textures;
	std::unordered_map<std::filesystem::path, std::size_t> m_TexturesCache;
};

Texture& TextureManager::GetTexture(const std::size_t index) { return m_Textures[index]; }

const Texture& TextureManager::GetTexture(const std::size_t index) const { return m_Textures[index]; }

std::optional<std::size_t> TextureManager::LoadTextureFromPath(
	const std::filesystem::path& path, const TextureType type, const TextureSpace space)
{
	spdlog::debug("Loading texture {}...", path.string());

	if (const auto result = m_TexturesCache.find(path); result != m_TexturesCache.end())
	{
		return { result->second };
	}

	std::expected<Texture, std::string> textureResult;
	const auto extension = path.extension();
	if (extension == ".ktx" || extension == ".ktx2")
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

void TextureManager::Delete()
{
	for (auto& texture : m_Textures)
	{
		texture.Delete();
	}
}
}// namespace stw
