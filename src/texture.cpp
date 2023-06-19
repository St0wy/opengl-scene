//
// Created by stowy on 05/05/2023.
//

#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#ifndef NDEBUG
#define STBI_FAILURE_USERMSG
#endif
#include <array>
#include <stb_image/stb_image.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"


void stw::SmartTexture::Bind(Pipeline& pipeline) const
{
	ASSERT_MESSAGE(m_TextureId != 0, "The texture should be initialized before being boud.");
	const GLenum activeTexture = GetTextureFromId(m_UniformId);

	glActiveTexture(activeTexture);
	pipeline.SetInt(m_UniformName.c_str(), m_UniformId);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
}

std::expected<stw::Texture, std::string> stw::Texture::LoadFromPath(const std::filesystem::path& path,
	const TextureType type,
	const TextureFormat format)
{
	GLuint textureId = 0;
	glGenTextures(1, &textureId);

	int width;
	int height;
	int nbrComponents;
	const auto stringPath = path.string();
	unsigned char* data = stbi_load(stringPath.c_str(), &width, &height, &nbrComponents, 0);

	if (!data)
	{
		stbi_image_free(data);
		glDeleteTextures(1, &textureId);
		return std::unexpected(fmt::format("Texture failed to load at path: {}\n{}",
			stringPath,
			stbi_failure_reason()));
	}

	GLenum glFormat;
	GLint internalFormat = 0;
	switch (nbrComponents)
	{
	case 1:
		glFormat = GL_RED;
		internalFormat = static_cast<GLint>(glFormat);

		break;
	case 3:
		glFormat = GL_RGB;
		internalFormat = static_cast<GLint>(glFormat);
		if (format == TextureFormat::Srgb)
		{
			internalFormat = GL_SRGB;
		}
		break;
	case 4:
		glFormat = GL_RGBA;
		internalFormat = static_cast<GLint>(glFormat);
		if (format == TextureFormat::Srgb)
		{
			internalFormat = GL_SRGB_ALPHA;
		}
		break;
	default:
		glFormat = GL_INVALID_ENUM;
		internalFormat = GL_INVALID_ENUM;
		break;
	}

	GLCALL(glBindTexture(GL_TEXTURE_2D, textureId));
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, GL_UNSIGNED_BYTE, data));
	GLCALL(glGenerateMipmap(GL_TEXTURE_2D));

	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	stbi_image_free(data);

	return {Texture{textureId, type}};
}

std::expected<stw::Texture, std::string> stw::Texture::LoadCubeMap(
	const std::array<std::filesystem::path, CubeMapTextureCount>& paths)
{
	GLuint textureId = 0;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
	int width;
	int height;
	int nbrComponents;

	for (std::size_t i = 0; i < paths.size(); i++)
	{
		const auto stringPath = paths[i].string();
		unsigned char* data = stbi_load(stringPath.c_str(), &width, &height, &nbrComponents, 0);
		if (!data)
		{
			stbi_image_free(data);
			glDeleteTextures(1, &textureId);
			return std::unexpected(fmt::format("Texture failed to load at path: {}", stringPath));
		}

		const auto target = static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
		// TODO: Handle SRGB
		glTexImage2D(target, 0, GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return {Texture{textureId, TextureType::CubeMap}};
}

const char* stw::ToString(const TextureType type)
{
	switch (type)
	{
	case TextureType::Diffuse:
		return "texture_diffuse";
	case TextureType::Specular:
		return "texture_specular";
	case TextureType::CubeMap:
		return "texture_cube_map";
	}

	return "";
}

aiTextureType stw::ToAssimpTextureType(const TextureType type)
{
	switch (type)
	{
	case TextureType::Diffuse:
		return aiTextureType_DIFFUSE;
	case TextureType::Specular:
		return aiTextureType_SPECULAR;
	default:
		return aiTextureType_NONE;
	}
}

stw::SmartTexture::~SmartTexture()
{
	if (m_TextureId == 0)
		return;

	glDeleteTextures(1, &m_TextureId);
}

void stw::SmartTexture::Init(const std::string_view path,
	std::string uniformName,
	const GLint uniformId,
	Pipeline& pipeline,
	const GLint format)
{
	m_UniformId = uniformId;
	m_UniformName = std::move(uniformName);

	glGenTextures(1, &m_TextureId);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.data(), &m_Width, &m_Height, &m_ChannelsInFile, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		spdlog::error("Failed to load texture ");
		assert(false);
	}
	stbi_image_free(data);

	pipeline.Bind();
	pipeline.SetInt(m_UniformName.c_str(), m_UniformId);
}
