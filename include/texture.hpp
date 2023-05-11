//
// Created by stowy on 05/05/2023.
//

#pragma once

#include <GL/glew.h>
#include <cstdint>
#include <string_view>

#include "pipeline.hpp"

namespace stw
{
class Texture final
{
public:
	Texture() = default;
	~Texture();
	Texture(const Texture& other) = delete;
	Texture(Texture&& other) = default;
	Texture& operator=(const Texture& other) = delete;
	Texture& operator=(Texture&& other) noexcept = default;

	void Init(std::string_view path,
		std::string_view textureUniformName,
		GLint uniformValue,
		Pipeline* pipeline,
		GLint format = GL_RGB);
	void Bind(GLenum activeTexture = GL_TEXTURE0) const;

private:
	GLuint m_TextureId{};
	i32 m_Width{};
	i32 m_Height{};
	i32 m_ChannelsInFile{};
	Pipeline* m_Pipeline{};
};
} // namespace stw
