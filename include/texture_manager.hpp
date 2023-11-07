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
#pragma once

#include <filesystem>
#include <optional>
#include <unordered_map>
#include <vector>

#include "texture.hpp"

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
}
