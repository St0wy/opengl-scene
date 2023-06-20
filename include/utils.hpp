//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <cassert>
#include <filesystem>
#include <GL/glew.h>

#include "number_types.hpp"

#define CHECK_GL_ERROR() stw::CheckGlError(__FILE__, __LINE__)

#if defined(_MSC_VER) && !defined(NDEBUG)
#define ASSERTD(x) if(!(x)) __debugbreak()
#else
#define ASSERTD(x) (void)x
#endif

#ifndef NDEBUG
#define GLCALL(x) ClearGlErrors();\
	x;\
	ASSERTD(!CHECK_GL_ERROR())
#else
#define GLCALL(x) x
#endif

#define ASSERT_MESSAGE(expression, message) assert(((void)(message), expression))

namespace stw
{
enum class TextureType;
template <class T, class U>concept Derived = std::is_base_of_v<U, T>;

std::optional<std::string> OpenFile(const std::filesystem::path& filename);

GLenum GetTextureFromId(i32 id);
GLenum GetGlTextureType(TextureType type);

template <typename T>
constexpr T MapRange(T value, T a, T b, T c, T d)
{
	// first map value from (a..b) to (0..1)
	value = (value - a) / (b - a);
	// then map it from (0..1) to (c..d) and return it
	return c + value * (d - c);
}

void ClearGlErrors();

bool CheckGlError(std::string_view file, u32 line);
} // namespace stw
