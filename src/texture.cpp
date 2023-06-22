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


//void stw::SmartTexture::Bind(Pipeline& pipeline) const
//{
//	ASSERT_MESSAGE(m_TextureId != 0, "The texture should be initialized before being boud.");
//	const GLenum activeTexture = GetTextureFromId(m_UniformId);
//
//	glActiveTexture(activeTexture);
//	pipeline.SetInt(m_UniformName.c_str(), m_UniformId);
//	glBindTexture(GL_TEXTURE_2D, m_TextureId);
//}

std::expected<stw::Texture, std::string> stw::Texture::LoadFromPath(const std::filesystem::path& path,
	const TextureType type,
	const TextureSpace space)
{
	int width;
	int height;
	int nbrComponents;
	const auto stringPath = path.string();
	unsigned char* data = stbi_load(stringPath.c_str(), &width, &height, &nbrComponents, 0);

	if (!data)
	{
		stbi_image_free(data);
		return std::unexpected(fmt::format("Texture failed to load at path: {}\n{}",
			stringPath,
			stbi_failure_reason()));
	}

	Texture texture;
	texture.Init(type, space);
	texture.SetFormat(nbrComponents);
	texture.Bind();
	texture.Specify(width, height, data);
	texture.GenerateMipmap();

	texture.SetMinFilter(GL_LINEAR_MIPMAP_LINEAR);
	texture.SetMagFilter(GL_LINEAR);
	texture.SetWrapS(GL_REPEAT);
	texture.SetWrapT(GL_REPEAT);

	stbi_image_free(data);

	return {std::move(texture)};
}

std::expected<stw::Texture, std::string> stw::Texture::LoadCubeMap(
	const std::array<std::filesystem::path, CubeMapTextureCount>& paths)
{
	Texture texture;
	texture.Init(TextureType::CubeMap, TextureSpace::Srgb);
	texture.Bind();

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
			texture.Delete();
			return std::unexpected(fmt::format("Texture failed to load at path: {}", stringPath));
		}

		texture.SetFormat(nbrComponents);
		const auto target = static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
		texture.Specify(width, height, data, {target});
		stbi_image_free(data);
	}

	texture.SetMinFilter(GL_LINEAR);
	texture.SetMagFilter(GL_LINEAR);
	texture.SetWrapS(GL_CLAMP_TO_EDGE);
	texture.SetWrapT(GL_CLAMP_TO_EDGE);
	texture.SetWrapR(GL_CLAMP_TO_EDGE);

	return {std::move(texture)};
}

void stw::Texture::Init(const TextureType type, const stw::TextureSpace textureSpace)
{
	glGenTextures(1, &textureId);
	this->textureType = type;
	this->space = textureSpace;
	m_GlTextureTarget = GetGlTextureTarget(this->textureType);
}

void stw::Texture::SetFormat(const int nbComponents)
{
	switch (nbComponents)
	{
	case 1:
		glFormat = GL_RED;
		internalFormat = static_cast<GLint>(glFormat);

		break;
	case 2:
		glFormat = GL_RG;
		internalFormat = static_cast<GLint>(glFormat);
		break;
	case 3:
		glFormat = GL_RGB;
		internalFormat = static_cast<GLint>(glFormat);
		if (space == TextureSpace::Srgb)
		{
			internalFormat = GL_SRGB;
		}
		break;
	case 4:
		glFormat = GL_RGBA;
		internalFormat = static_cast<GLint>(glFormat);
		if (space == TextureSpace::Srgb)
		{
			internalFormat = GL_SRGB_ALPHA;
		}
		break;
	default:
		glFormat = GL_INVALID_ENUM;
		internalFormat = GL_INVALID_ENUM;
		break;
	}
}

void stw::Texture::Bind() const
{
	if (textureId == 0)
	{
		spdlog::error("Binding texture that is not initialized");
	}

	GLCALL(glBindTexture(m_GlTextureTarget, textureId));
}

void stw::Texture::Specify(const GLsizei width,
	const GLsizei height,
	const GLvoid* data,
	const std::optional<GLenum> optionalTarget,
	const GLenum dataType) const
{
	if (optionalTarget.has_value())
	{
		GLCALL(glTexImage2D(optionalTarget.value(), 0, internalFormat, width, height, 0, glFormat, dataType, data));
	}
	else
	{
		GLCALL(glTexImage2D(m_GlTextureTarget, 0, internalFormat, width, height, 0, glFormat, dataType, data));
	}
}

void stw::Texture::GenerateMipmap() const
{
	GLCALL(glGenerateMipmap(m_GlTextureTarget));
}

void stw::Texture::SetMinFilter(const GLint filter) const
{
	glTexParameteri(m_GlTextureTarget, GL_TEXTURE_MIN_FILTER, filter);
}

void stw::Texture::SetMagFilter(const GLint filter) const
{
	glTexParameteri(m_GlTextureTarget, GL_TEXTURE_MAG_FILTER, filter);
}

void stw::Texture::SetWrapS(const GLint wrap) const
{
	glTexParameteri(m_GlTextureTarget, GL_TEXTURE_WRAP_S, wrap);
}

void stw::Texture::SetWrapT(const GLint wrap) const
{
	glTexParameteri(m_GlTextureTarget, GL_TEXTURE_WRAP_T, wrap);
}

void stw::Texture::SetWrapR(const GLint wrap) const
{
	glTexParameteri(m_GlTextureTarget, GL_TEXTURE_WRAP_R, wrap);
}

void stw::Texture::Delete()
{
	glDeleteTextures(1, &textureId);
	textureId = 0;
}

const char* stw::ToString(const TextureType type)
{
	switch (type)
	{
	case TextureType::Diffuse:
		return "texture_diffuse";
	case TextureType::Specular:
		return "texture_specular";
	case TextureType::Normal:
		return "texture_normal";
	case TextureType::CubeMap:
		return "texture_cube_map";
	default:
		return "";
	}
}

aiTextureType stw::ToAssimpTextureType(const TextureType type)
{
	switch (type)
	{
	case TextureType::Diffuse:
		return aiTextureType_DIFFUSE;
	case TextureType::Specular:
		return aiTextureType_SPECULAR;
	case TextureType::Normal:
		return aiTextureType_NORMALS;
	default:
		return aiTextureType_NONE;
	}
}

stw::Texture::Texture(Texture&& other) noexcept
	: textureId(other.textureId),
	textureType(other.textureType),
	space(other.space),
	glFormat(other.glFormat),
	internalFormat(other.internalFormat),
	m_GlTextureTarget(other.m_GlTextureTarget)
{
	other.textureId = 0;
	other.glFormat = GL_INVALID_ENUM;
	other.internalFormat = -1;
	other.m_GlTextureTarget = GL_INVALID_ENUM;
}

stw::Texture::~Texture()
{
	if (textureId != 0)
	{
		spdlog::error("Destructor called on texture that wasn't deleted.");
	}
}

stw::Texture& stw::Texture::operator=(Texture&& other) noexcept
{
	if (this == &other)
		return *this;

	Delete();
	textureId = other.textureId;
	textureType = other.textureType;
	space = other.space;
	glFormat = other.glFormat;
	internalFormat = other.internalFormat;
	m_GlTextureTarget = other.m_GlTextureTarget;
	other.textureId = 0;
	other.glFormat = GL_INVALID_ENUM;
	other.internalFormat = -1;
	other.m_GlTextureTarget = GL_INVALID_ENUM;

	return *this;
}
