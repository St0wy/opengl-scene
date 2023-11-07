/**
 * @file shader.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains various shaders utility structs.
 * @version 1.0
 * @date 04/05/2023
 * 
 * @copyright SAE (c) 2023
 *
 */

#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace stw
{
enum class ShaderType : std::int8_t
{
	None = -1,
	Vertex = 0,
	Fragment = 1,
};

struct ShaderProgramSource
{
	/**
	 * \brief Represents the number of different possible shaders.
	 * Should be updated when new shaders are added.
	 */
	static constexpr std::size_t ShaderTypeCount = 2;

	std::optional<std::string> vertex;
	std::optional<std::string> fragment;

	static ShaderProgramSource LoadFromFile(const std::filesystem::path& path);

	std::optional<std::string> operator[](ShaderType type);
};
}
