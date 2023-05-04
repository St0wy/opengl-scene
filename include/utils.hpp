//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <string_view>
#include <cassert>

#define assertm(expression, message) assert(((void)message, expression));

namespace stw
{
std::string OpenFile(std::string_view filename);

template<typename T>
T MapRange(T value, T a, T b, T c, T d)
{
	// first map value from (a..b) to (0..1)
	value = (value - a) / (b - a);
	// then map it from (0..1) to (c..d) and return it
	return c + value * (d - c);
}

bool CheckGlError(std::string_view file, uint32_t line);
#define CHECK_GL_ERROR() stw::CheckGlError(__FILE__, __LINE__)
}



