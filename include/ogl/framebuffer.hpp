#pragma once

#include <GL/glew.h>

#include "texture.hpp"

namespace stw
{
class Framebuffer
{
public:
	Framebuffer() = default;
	~Framebuffer();

	void Init();
	void Bind() const;
	void UnBind() const;
	void Delete();
private:
	GLuint m_Fbo = 0;
	Texture m_Texture;
};
}
