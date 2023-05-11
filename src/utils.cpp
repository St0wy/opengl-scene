//
// Created by stowy on 03/05/2023.
//

#include "utils.hpp"

#include <fstream>
#include <GL/glew.h>
#include <spdlog/spdlog.h>

namespace stw
{
std::string OpenFile(const std::string_view filename)
{
	std::ifstream ifs(filename.data());
	std::string content((std::istreambuf_iterator(ifs)), (std::istreambuf_iterator<char>()));

	return content;
}

GLenum GetTextureFromId(const i32 id)
{
	switch (id)
	{
	case 0:
		return GL_TEXTURE0;
	case 1:
		return GL_TEXTURE1;
	case 2:
		return GL_TEXTURE2;
	case 3:
		return GL_TEXTURE3;
	case 4:
		return GL_TEXTURE4;
	case 5:
		return GL_TEXTURE5;
	case 6:
		return GL_TEXTURE6;
	case 7:
		return GL_TEXTURE7;
	case 8:
		return GL_TEXTURE8;
	case 9:
		return GL_TEXTURE9;
	case 10:
		return GL_TEXTURE10;
	case 11:
		return GL_TEXTURE11;
	case 12:
		return GL_TEXTURE12;
	case 13:
		return GL_TEXTURE13;
	case 14:
		return GL_TEXTURE14;
	case 15:
		return GL_TEXTURE15;
	case 16:
		return GL_TEXTURE16;
	case 17:
		return GL_TEXTURE17;
	case 18:
		return GL_TEXTURE18;
	case 19:
		return GL_TEXTURE19;
	default:
		return GL_INVALID_ENUM;
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

		case GL_INVALID_VALUE:
			spdlog::error("File: {} Line: {} OpenGL: GL_INVALID_VALUE", file, line);

		case GL_INVALID_OPERATION:
			spdlog::error("File: {} Line: {} OpenGL: GL_INVALID_OPERATION", file, line);

		case GL_STACK_OVERFLOW:
			spdlog::error("File: {} Line: {} OpenGL: GL_STACK_OVERFLOW", file, line);

		case GL_STACK_UNDERFLOW:
			spdlog::error("File: {} Line: {} OpenGL: GL_STACK_UNDERFLOW", file, line);

		case GL_OUT_OF_MEMORY:
			spdlog::error("File: {} Line: {} OpenGL: GL_OUT_OF_MEMORY", file, line);

		case GL_INVALID_FRAMEBUFFER_OPERATION:
			spdlog::error("File: {} Line: {} OpenGL: GL_INVALID_FRAMEBUFFER_OPERATION", file, line);

		case GL_CONTEXT_LOST:
			spdlog::error("File: {} Line: {} OpenGL: GL_CONTEXT_LOST", file, line);

		case GL_TABLE_TOO_LARGE:
			spdlog::error("File: {} Line: {} OpenGL: GL_TABLE_TOO_LARGE", file, line);

		default: 
		}
	}

	return hadErrors;
}
} // namespace stw
