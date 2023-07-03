//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <assimp/matrix4x4.h>
#include <cassert>
#include <filesystem>
#include <GL/glew.h>
#include <glm/mat4x4.hpp>

#include "number_types.hpp"

#define CHECK_GL_ERROR() stw::CheckGlError(__FILE__, __LINE__)


#ifndef NDEBUG
#define GLCALL(x)    \
	ClearGlErrors(); \
	x;               \
	ASSERTD(!CHECK_GL_ERROR())

#ifdef _MSC_VER
#define ASSERTD(x) \
	if (!(x)) __debugbreak()
#elifdef __clang__
#define ASSERTD(x) \
	if (!(x)) __builtin_debugtrap()
#endif
#else
#define GLCALL(x) x
#endif

#define ASSERT_MESSAGE(expression, message) assert(((void)(message), expression))

namespace stw
{
enum class TextureType;
template<class T, class U>
concept Derived = std::is_base_of_v<U, T>;

// Utility for std::visit
template<class... Ts>
struct Overloaded : Ts...
{
	using Ts::operator()...;
};

template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

std::optional<std::string> OpenFile(const std::filesystem::path& filename);

GLenum GetTextureFromId(i32 id);
GLenum GetGlTextureTarget(TextureType type);

template<typename T>
constexpr T MapRange(T value, T a, T b, T c, T d)
{
	// first map value from (a..b) to (0..1)
	value = (value - a) / (b - a);
	// then map it from (0..1) to (c..d) and return it
	return c + value * (d - c);
}

constexpr glm::mat4 ConvertMatAssimpToGlm(const aiMatrix4x4& assimpMatrix)
{
	// clang-format off
	return glm::mat4{
		assimpMatrix.a1, assimpMatrix.a2, assimpMatrix.a3, assimpMatrix.a4,
		assimpMatrix.b1, assimpMatrix.b2, assimpMatrix.b3, assimpMatrix.b4,
		assimpMatrix.c1, assimpMatrix.c2, assimpMatrix.c3, assimpMatrix.c4,
		assimpMatrix.d1, assimpMatrix.d2, assimpMatrix.d3, assimpMatrix.d4,
	};
	// clang-format on
}

void ClearGlErrors();

bool CheckGlError(std::string_view file, u32 line);
}// namespace stw
