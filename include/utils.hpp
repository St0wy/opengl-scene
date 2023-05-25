//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <cassert>
#include <string_view>
#include <GL/glew.h>

#include "number_types.hpp"

#define ASSERT_MESSAGE(expression, message) assert(((void)(message), expression))

namespace stw
{
template <class T, class U>concept Derived = std::is_base_of_v<U, T>;

std::string OpenFile(std::string_view filename);

GLenum GetTextureFromId(i32 id);

template <typename T>
T MapRange(T value, T a, T b, T c, T d)
{
	// first map value from (a..b) to (0..1)
	value = (value - a) / (b - a);
	// then map it from (0..1) to (c..d) and return it
	return c + value * (d - c);
}

bool CheckGlError(std::string_view file, u32 line);
#define CHECK_GL_ERROR() stw::CheckGlError(__FILE__, __LINE__)
} // namespace stw
