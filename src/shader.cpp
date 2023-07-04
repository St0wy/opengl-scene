#include "ogl/shader.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace stw
{
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
		shaderProgramSource.vertex = {std::move(vertexString)};
	}

	if (!fragmentString.empty())
	{
		fragmentString.insert(0, commonString);
		shaderProgramSource.fragment = {std::move(fragmentString)};
	}

	return shaderProgramSource;
}

std::optional<std::string> ShaderProgramSource::operator[](const ShaderType type)
{
	switch (type)
	{
	case ShaderType::Vertex:
		return vertex;
	case ShaderType::Fragment:
		return fragment;
	default:
		return {};
	}
}
}
