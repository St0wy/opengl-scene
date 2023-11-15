/**
 * @file utils.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains some utility stuff for this project.
 * @version 1.0
 * @date 03/05/2023
 *
 * @copyright SAE (c) 2023
 *
 */

#pragma once

#include <array>
#include <cassert>
#include <filesystem>
#include <type_traits>
#include <optional>

#include <assimp/matrix4x4.h>
#include <GL/glew.h>
#include <glm/mat4x4.hpp>

#include "consts.hpp"
#include "number_types.hpp"

#define CHECK_GL_ERROR() stw::CheckGlError(__FILE__, __LINE__)

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

// Any callable type with a return type R and arguments Args
template<typename F, typename R, typename... Args>
concept Callable = std::same_as<std::invoke_result_t<F, Args...>, R>;

// Anything that can be invoked and doesn't return anything with no parameters
template<typename T>
concept Runnable = Callable<T, void>;

// Anything that can be invoked and returns something with no parameters
template<typename T, typename R>
concept Supplier = Callable<T, R>;

// Anything that can be invoked and returns nothing with parameters
template<typename T, typename... Args>
concept Consumer = Callable<T, void, Args...>;

// Anything that can be invoked and returns a bool with parameters
template<typename T, typename... Args>
concept Predicate = Callable<T, bool, Args...>;

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

std::array<f32, ShadowMapNumCascades> ComputeCascades();

std::array<glm::vec3, SsaoKernelSize> GenerateSsaoKernel();
std::array<glm::vec3, SsaoRandomTextureSize> GenerateSsaoRandomTexture();

template<typename T>
constexpr T Lerp(T a, T b, T f)
{
	return a + f * (b - a);
}

void ClearGlErrors();

bool CheckGlError(std::string_view file, u32 line);
}// namespace stw
