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
