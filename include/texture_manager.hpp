//
// Created by stowy on 28/06/2023.
//
#pragma once

#include <vector>
#include <optional>
#include <filesystem>
#include <unordered_map>

#include "texture.hpp"

namespace stw
{
class TextureManager
{
public:
	std::optional<std::size_t> LoadTextureFromPath(const std::filesystem::path& path, TextureType type);
	[[nodiscard]] Texture& GetTexture(std::size_t index);
	[[nodiscard]] const Texture& GetTexture(std::size_t index) const;

	void Delete();
private:
	std::vector<Texture> m_Textures;
	std::unordered_map<std::filesystem::path, std::size_t> m_TexturesCache;
};
}
