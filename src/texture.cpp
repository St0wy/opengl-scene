//
// Created by stowy on 05/05/2023.
//

#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <array>
#include <stb_image.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"


void stw::SmartTexture::Bind(const Pipeline& pipeline) const
{
	ASSERT_MESSAGE(m_TextureId != 0, "The texture should be initialized before being boud.");
	const GLenum activeTexture = GetTextureFromId(m_UniformId);

	glActiveTexture(activeTexture);
	pipeline.SetInt(m_UniformName.c_str(), m_UniformId);
	glBindTexture(GL_TEXTURE_2D, m_TextureId);
}

std::expected<stw::Texture, std::string> stw::Texture::LoadFromPath(const std::filesystem::path& path,
	const TextureType type)
{
	GLuint textureId = 0;
	glGenTextures(1, &textureId);

	int width;
	int height;
	int nbrComponents;
	const auto stringPath = path.string();
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(stringPath.c_str(), &width, &height, &nbrComponents, 0);
	stbi_set_flip_vertically_on_load(false);
	if (!data)
	{
		stbi_image_free(data);
		glDeleteTextures(1, &textureId);
		return std::unexpected(fmt::format("Texture failed to load at path: {}", stringPath));
	}

	GLenum format;
	switch (nbrComponents)
	{
	case 1:
		format = GL_RED;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		format = GL_INVALID_ENUM;
		break;
	}

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

	stbi_set_flip_vertically_on_load(true);

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
		glTexImage2D(target, 0, GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	stbi_set_flip_vertically_on_load(false);

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
	const Pipeline& pipeline,
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

	pipeline.Use();
	pipeline.SetInt(m_UniformName.c_str(), m_UniformId);
}
