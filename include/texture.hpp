//
// Created by stowy on 05/05/2023.
//

#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <GL/glew.h>

#include "pipeline.hpp"
#include <assimp/material.h>

namespace stw
{
enum class TextureType
{
	Diffuse,
	Specular,
};

const char* ToString(TextureType type);
aiTextureType ToAssimpTextureType(TextureType type);

// This is the texture that was used in the light scene
// It is called smart because it handle the OpenGL logic to bind it to the shader
class SmartTexture final
{
public:
	SmartTexture() = default;
	~SmartTexture();
	SmartTexture(const SmartTexture& other) = delete;
	SmartTexture(SmartTexture&& other) = default;
	SmartTexture& operator=(const SmartTexture& other) = delete;
	SmartTexture& operator=(SmartTexture&& other) noexcept = default;

	void Init(std::string_view path,
		std::string uniformName,
		GLint uniformId,
		const Pipeline& pipeline,
		GLint format = GL_RGB);
	void Bind(const Pipeline& pipeline) const;

private:
	GLuint m_TextureId{};
	i32 m_Width{};
	i32 m_Height{};
	i32 m_ChannelsInFile{};
	std::string m_UniformName{};
	GLint m_UniformId{};
};

struct Texture
{
	static std::expected<Texture, std::string> LoadFromPath(const std::filesystem::path& path, TextureType type);
	GLuint textureId;
	TextureType textureType;
};
} // namespace stw
