//
// Created by stowy on 03/05/2023.
//

#include "utils.hpp"

#include <filesystem>
#include <fstream>
#include <optional>
#include <random>
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

GLenum GetGlTextureTarget(const TextureType type)
{
	switch (type)
	{
	case TextureType::RadianceMap:
	case TextureType::Roughness:
	case TextureType::AmbientOcclusion:
	case TextureType::Metallic:
	case TextureType::BaseColor:
	case TextureType::Specular:
	case TextureType::Normal:
		return GL_TEXTURE_2D;
	case TextureType::CubeMap:
		return GL_TEXTURE_CUBE_MAP;
	case TextureType::DepthMap:
		return GL_DEPTH_COMPONENT;
	}

	spdlog::warn("Invalid texture type {} {}", __FILE__, __LINE__);
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
	//		const f32 far = NearPlane * std::pow(FarPlane / NearPlane, static_cast<f32>(i + 1) / ShadowMapNumCascades);
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
		glm::vec3 sample(randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator));
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
