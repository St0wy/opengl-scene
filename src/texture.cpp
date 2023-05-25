//
// Created by stowy on 05/05/2023.
//

#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
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
	if (CHECK_GL_ERROR())
	{
		assert(false);
	}
	GLuint textureId = 0;
	glGenTextures(1, &textureId);
	if (CHECK_GL_ERROR())
	{
		assert(false);
	}

	int width;
	int height;
	int nbrComponents;
	const auto stringPath = path.string();
	unsigned char* data = stbi_load(stringPath.c_str(), &width, &height, &nbrComponents, 0);
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
	if (CHECK_GL_ERROR())
	{
		assert(false);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0, format, GL_UNSIGNED_BYTE, data);
	if (CHECK_GL_ERROR())
	{
		assert(false);
	}
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);

	return {Texture{textureId, type}};
}

const char* stw::ToString(const TextureType type)
{
	switch (type)
	{
	case TextureType::Diffuse:
		return "texture_diffuse";
	case TextureType::Specular:
		return "texture_specular";
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
	}

	return aiTextureType_NONE;
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
