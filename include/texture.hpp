/**
 * @file texture.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Texture class.
 * @version 1.0
 * @date 28/06/2023
 *
 * @copyright SAE (c) 2023
 *
 */

#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <optional>

#include <assimp/material.h>
#include <GL/glew.h>

namespace stw
{
enum class TextureType
{
	BaseColor,
	Specular,
	Normal,
	Roughness,
	AmbientOcclusion,
	Metallic,
	CubeMap,
	DepthMap,
	RadianceMap,
};

enum class TextureSpace
{
	Srgb,
	Linear
};

const char* ToString(TextureType type);
aiTextureType ToAssimpTextureType(TextureType type);

class Texture
{
public:
	Texture() = default;
	Texture(const Texture&) = delete;
	Texture(Texture&& other) noexcept;
	~Texture();

	Texture& operator=(const Texture&) = delete;
	Texture& operator=(Texture&& other) noexcept;

	static constexpr std::size_t CubeMapTextureCount = 6;

	static std::expected<Texture, std::string> LoadFromPath(
		const std::filesystem::path& path, TextureType type, TextureSpace space);
	static std::expected<Texture, std::string> LoadRadianceMapFromPath(const std::filesystem::path& path);
	static std::expected<stw::Texture, std::string> LoadKtxFromPath(
		const std::filesystem::path& path, TextureType type);
	static std::expected<Texture, std::string> LoadCubeMap(
		const std::array<std::filesystem::path, CubeMapTextureCount>& paths);

	GLuint textureId = 0;
	TextureType textureType = TextureType::BaseColor;
	TextureSpace space = TextureSpace::Srgb;
	GLenum glFormat = GL_INVALID_ENUM;
	GLint internalFormat = -1;

	void Bind() const;
	void Init(TextureType type, TextureSpace textureSpace);
	void SetFormat(int nbComponents);
	void Specify(GLsizei width,
		GLsizei height,
		const GLvoid* data,
		std::optional<GLenum> optionalTarget = {},
		GLenum dataType = GL_UNSIGNED_BYTE) const;
	void GenerateMipmap() const;
	void SetMinFilter(GLint filter) const;
	void SetMagFilter(GLint filter) const;
	void SetWrapS(GLint wrap) const;
	void SetWrapT(GLint wrap) const;
	void SetWrapR(GLint wrap) const;
	void Delete();

private:
	Texture(GLuint textureId,
		TextureType textureType,
		TextureSpace textureSpace,
		GLenum glFormat,
		GLint internalFormat,
		GLenum glTextureTarget);
	Texture(GLuint textureId, GLenum glFormat, GLint internalFormat, GLenum glTextureTarget);
	Texture(GLuint textureId, GLenum glTextureTarget, TextureType textureType);
	GLenum m_GlTextureTarget = GL_INVALID_ENUM;
};
}// namespace stw
