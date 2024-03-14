/**
 * @file shader.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains various shaders utility structs.
 * @version 1.0
 * @date 09/11/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

export module shader;

export namespace stw
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
};

ShaderProgramSource ShaderProgramSource::LoadFromFile(const std::filesystem::path& path)
{
	std::ifstream stream(path);

	std::stringstream vertexStream{};
	std::stringstream fragmentStream{};
	std::stringstream commonStream{};

	ShaderProgramSource shaderProgramSource{};

	auto currentType = ShaderType::None;

	std::string line;
	while (std::getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				currentType = ShaderType::Vertex;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				currentType = ShaderType::Fragment;
			}
			else
			{
				currentType = ShaderType::None;
			}
		}
		else
		{
			switch (currentType)
			{
			case ShaderType::None:
				commonStream << line << '\n';
				break;
			case ShaderType::Vertex:
				vertexStream << line << '\n';
				break;
			case ShaderType::Fragment:
				fragmentStream << line << '\n';
				break;
			}
		}
	}

	std::string vertexString = vertexStream.str();
	std::string fragmentString = fragmentStream.str();
	std::string commonString = commonStream.str();

	if (!vertexString.empty())
	{
		vertexString.insert(0, commonString);
		shaderProgramSource.vertex = { std::move(vertexString) };
	}

	if (!fragmentString.empty())
	{
		fragmentString.insert(0, commonString);
		shaderProgramSource.fragment = { std::move(fragmentString) };
	}

	return shaderProgramSource;
}
}// namespace stw
