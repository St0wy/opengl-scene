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

module;

#include <array>
#include <filesystem>
#include <fstream>
#include <optional>
#include <random>
#include <concepts>

#include <assimp/matrix4x4.h>
#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <spdlog/spdlog.h>

export module utils;

import number_types;
import consts;

export
{
	namespace stw
	{
	template<class T, class U>
	concept Derived = std::is_base_of_v<U, T>;

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

	template<typename T>
	constexpr T Lerp(T a, T b, T f)
	{
		return a + f * (b - a);
	}

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
		GLenum err = GL_NO_ERROR;
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

	std::array<glm::vec3, SsaoKernelSize> GenerateSsaoKernel()
	{
		std::random_device rd;
		std::default_random_engine generator(rd());
		std::uniform_real_distribution<f32> randomFloats(0.0f, 1.0f);
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
}
