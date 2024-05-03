/**
 * @file utils.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains some utility stuff for this project.
 * @version 1.0
 * @date 03/05/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <array>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <optional>
#include <random>

#include <assimp/matrix4x4.h>
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <spdlog/spdlog.h>

export module utils;

import number_types;
import consts;

export namespace stw
{
template<class T, class U>
concept Derived = std::is_base_of_v<U, T>;

template<typename T>
concept Number = std::is_arithmetic_v<T>;

// Utility for std::visit
template<class... Ts>
struct Overloaded : Ts...
{
	using Ts::operator()...;
};

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

/**
 * Maps value from the range [a..b] to [c..d]
 * @param value Value to map.
 * @param a Min of start range
 * @param b Max of start range
 * @param c Min of end range
 * @param d Max of end range
 * @return The mapped value
 */
constexpr Number auto MapRange(Number auto value, Number auto a, Number auto b, Number auto c, Number auto d)
{
	// first map value from (a..b) to (0..1)
	value = (value - a) / (b - a);
	// then map it from (0..1) to (c..d) and return it
	return c + value * (d - c);
}

/**
 * Converts an Assimp matrix4 to a glm matrix4.
 */
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

/**
 * Computes the linear interpolation between a and b.
 * @param f If 0, will return a, if 1, will return b.
 * @param a Start of the lerp range.
 * @param b End of the lerp range.
 * @return The linearly interpolated number.
 */
constexpr Number auto Lerp(Number auto f, Number auto a, Number auto b) { return a + f * (b - a); }

/**
 * Reads a file as a string. May fail if the file doesn't exist.
 * @param path Path of the file to read.
 * @return Content of the file as a string.
 */
std::optional<std::string> ReadFileAsString(const std::filesystem::path& path)
{
	std::ifstream ifs(path);
	if (!ifs.is_open())
	{
		return {};
	}

	std::string content((std::istreambuf_iterator(ifs)), (std::istreambuf_iterator<char>()));

	return content;
}

/**
 * Converts the id to an OpenGL texture enum (GL_TEXTURE). May return GL_INVALID_ENUM if id > 31.
 * @param id Texture ID to convert.
 * @return The OpenGL texture enum.
 */
GLenum GetTextureFromId(const i32 id)
{
	static constexpr i32 AssumedMaxNumberOfTexturesInOGL = 31;
	if (id > AssumedMaxNumberOfTexturesInOGL)
	{
		return GL_INVALID_ENUM;
	}

	return static_cast<GLenum>(GL_TEXTURE0 + id);
}

/**
 * Clears every error that may be returned by glGetError().
 */
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

/**
 * Computes the shadow cascade ranges.
 * @return An array will every shadow cascade values.
 */
std::array<f32, ShadowMapNumCascades> ComputeCascades()
{
	//	return { FarPlane / 50.0f, FarPlane / 25.0f, FarPlane / 10.0f, FarPlane / 2.0f };
	return { FarPlane / 100.0f, FarPlane / 25.0f, FarPlane / 10.0f, FarPlane / 2.0f };
	//	std::array<f32, ShadowMapNumCascades> intervals{};
	//	for (usize i = 0; i < intervals.size(); i++)
	//	{
	//		const f32 far = NearPlane * std::pow(FarPlane / NearPlane, static_cast<f32>(i + 1) /
	// ShadowMapNumCascades);
	//
	//		spdlog::debug("far : {}", far);
	//		intervals.at(i) = far;
	//	}
	//
	//	return intervals;
}

/**
 * Generates the kernel used in SSAO shader.
 */
std::array<glm::vec3, SsaoKernelSize> GenerateSsaoKernel()
{
	std::random_device rd;
	std::default_random_engine generator(rd());
	std::uniform_real_distribution randomFloats(0.0f, 1.0f);
	std::array<glm::vec3, SsaoKernelSize> ssaoKernel{};
	for (usize i = 0; i < SsaoKernelSize; i++)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		f32 scale = static_cast<f32>(i) / 64.0f;
		scale = Lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.at(i) = sample;
	}

	return ssaoKernel;
}

/**
 * Generates the texture used for random number generation in SSAO shader.
 */
std::array<glm::vec3, SsaoRandomTextureSize> GenerateSsaoRandomTexture()
{
	std::random_device rd;
	std::default_random_engine generator(rd());
	std::uniform_real_distribution randomFloats(0.0f, 1.0f);
	std::array<glm::vec3, SsaoRandomTextureSize> ssaoRandomTexture{};
	for (auto& randomSample : ssaoRandomTexture)
	{
		const glm::vec3 noise(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, 0.0f);
		randomSample = noise;
	}

	return ssaoRandomTexture;
}
}// namespace stw
