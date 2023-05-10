//
// Created by stowy on 03/05/2023.
//

#include "utils.hpp"

#include <GL/glew.h>
#include <fstream>
#include <spdlog/spdlog.h>

namespace stw
{
std::string OpenFile(const std::string_view filename)
{
    std::ifstream ifs(filename.data());
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

    return content;
}

bool CheckGlError(std::string_view file, std::uint32_t line)
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
            break;
        }
    }

    return hadErrors;
}
} // namespace stw
