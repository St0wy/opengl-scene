//
// Created by stowy on 03/05/2023.
//

#include "utils.hpp"

#include <filesystem>
#include <fstream>
#include <optional>
#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include "texture.hpp"

namespace stw
{
std::optional<std::string> OpenFile(const std::filesystem::path& filename)
{
	std::ifstream ifs(filename);
	if (!ifs.is_open())
	{
		return {};
	}

	std::string content((std::istreambuf_iterator(ifs)), (std::istreambuf_iterator<char>()));

	return content;
}

GLenum GetTextureFromId(const i32 id)
{
	if (id > 19)
	{
		return GL_INVALID_ENUM;
	}

	return static_cast<GLenum>(GL_TEXTURE0 + id);
}

GLenum GetGlTextureType(const TextureType type)
{
	switch (type)
	{
	case TextureType::Diffuse:
	case TextureType::Specular:
	case TextureType::Normal:
		return GL_TEXTURE_2D;
	case TextureType::CubeMap:
		return GL_TEXTURE_CUBE_MAP;
	}

	return GL_INVALID_ENUM;
}

void ClearGlErrors()
{
	constexpr int maxErrorCount = 100;
	int i = 0;
	while (glGetError() != GL_NO_ERROR)
	{
		i++;
		if (i >= maxErrorCount)
		{
			spdlog::warn("Clearing more than {} errors...", maxErrorCount);
		}
	}
}

bool CheckGlError(std::string_view file, u32 line)
{
	bool hadErrors = false;
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		hadErrors = true;
		// Process/log the error.
		switch (err)
		{
		case GL_INVALID_ENUM:
			spdlog::error("File: {} Line: {} OpenGL: GL_INVALID_ENUM", file, line);
			break;
		case GL_INVALID_VALUE:
			spdlog::error("File: {} Line: {} OpenGL: GL_INVALID_VALUE", file, line);
			break;
		case GL_INVALID_OPERATION:
			spdlog::error("File: {} Line: {} OpenGL: GL_INVALID_OPERATION", file, line);
			break;
		case GL_STACK_OVERFLOW:
			spdlog::error("File: {} Line: {} OpenGL: GL_STACK_OVERFLOW", file, line);
			break;
		case GL_STACK_UNDERFLOW:
			spdlog::error("File: {} Line: {} OpenGL: GL_STACK_UNDERFLOW", file, line);
			break;
		case GL_OUT_OF_MEMORY:
			spdlog::error("File: {} Line: {} OpenGL: GL_OUT_OF_MEMORY", file, line);
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			spdlog::error("File: {} Line: {} OpenGL: GL_INVALID_FRAMEBUFFER_OPERATION", file, line);
			break;
		case GL_CONTEXT_LOST:
			spdlog::error("File: {} Line: {} OpenGL: GL_CONTEXT_LOST", file, line);
			break;
		case GL_TABLE_TOO_LARGE:
			spdlog::error("File: {} Line: {} OpenGL: GL_TABLE_TOO_LARGE", file, line);
			break;
		default:
			spdlog::error("File: {} Line: {} Other OpenGL Error", file, line);
			break;
		}
	}

	return hadErrors;
}
} // namespace stw
