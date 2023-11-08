/**
 * @file texture_manager.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the TextureManager class.
 * @version 1.0
 * @date 28/06/2023
 *
 * @copyright SAE (c) 2023
 *
 */
module;

#include <filesystem>
#include <optional>
#include <unordered_map>
#include <vector>
#include <expected>

#include <spdlog/spdlog.h>

export module texture_manager;

import number_types;
import consts;
import utils;
import texture;

export
{
	namespace stw
	{
	class TextureManager
	{
	public:
		std::optional<std::size_t> LoadTextureFromPath(
			const std::filesystem::path& path, stw::TextureType type, TextureSpace space);
		[[nodiscard]] Texture& GetTexture(std::size_t index);
		[[nodiscard]] const Texture& GetTexture(std::size_t index) const;

		void Delete();

	private:
		std::vector<Texture> m_Textures;
		std::unordered_map<std::filesystem::path, std::size_t> m_TexturesCache;
	};

	stw::Texture& stw::TextureManager::GetTexture(std::size_t index) { return m_Textures[index]; }

	const stw::Texture& stw::TextureManager::GetTexture(std::size_t index) const { return m_Textures[index]; }

	std::optional<std::size_t> stw::TextureManager::LoadTextureFromPath(
		const std::filesystem::path& path, stw::TextureType type, TextureSpace space)
	{
		spdlog::debug("Loading texture {}...", path.string());

		if (const auto result = m_TexturesCache.find(path); result != m_TexturesCache.end())
		{
			return { result->second };
		}

		std::expected<stw::Texture, std::string> textureResult;
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

	void stw::TextureManager::Delete()
	{
		for (auto& texture : m_Textures)
		{
			texture.Delete();
		}
	}
	}// namespace stw
}