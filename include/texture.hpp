//
// Created by stowy on 05/05/2023.
//

#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <assimp/material.h>
#include <GL/glew.h>

namespace stw
{
enum class TextureType
{
	Diffuse,
	Specular,
	Normal,
	CubeMap,
	DepthMap,
};

enum class TextureSpace
{
	Srgb,
	Linear
};

const char* ToString(TextureType type);
aiTextureType ToAssimpTextureType(TextureType type);

// This is the texture that was used in the light scene
// It is called smart because it handles the OpenGL logic to bind it to the shader
//class SmartTexture final
//{
//public:
//	SmartTexture() = default;
//	~SmartTexture();
//	SmartTexture(const SmartTexture& other) = delete;
//	SmartTexture(SmartTexture&& other) = default;
//	SmartTexture& operator=(const SmartTexture& other) = delete;
//	SmartTexture& operator=(SmartTexture&& other) noexcept = default;
//
//	void Init(std::string_view path,
//		std::string uniformName,
//		GLint uniformId,
//		Pipeline& pipeline,
//		GLint format = GL_RGB);
//	void Bind(Pipeline& pipeline) const;
//
//private:
//	GLuint m_TextureId{};
//	i32 m_Width{};
//	i32 m_Height{};
//	i32 m_ChannelsInFile{};
//	std::string m_UniformName{};
//	GLint m_UniformId{};
//};

struct Texture
{
	Texture() = default;
	Texture(const Texture&) = delete;
	Texture(Texture&& other) noexcept;
	~Texture();

	Texture& operator=(const Texture&) = delete;
	Texture& operator=(Texture&& other) noexcept;

	static constexpr std::size_t CubeMapTextureCount = 6;

	static std::expected<Texture, std::string> LoadFromPath(const std::filesystem::path& path,
		TextureType type,
		TextureSpace space = TextureSpace::Linear);
	static std::expected<Texture, std::string> LoadCubeMap(
		const std::array<std::filesystem::path, CubeMapTextureCount>& paths);

	GLuint textureId = 0;
	TextureType textureType = TextureType::Diffuse;
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
	GLenum m_GlTextureTarget = GL_INVALID_ENUM;
};
} // namespace stw
