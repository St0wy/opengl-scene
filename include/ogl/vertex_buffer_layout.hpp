#pragma once

#include <vector>
#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include "number_types.hpp"

namespace stw
{
struct VertexBufferElement
{
	GLenum type;
	GLint count;
	GLboolean normalized;

	static GLsizei GetSizeOfType(const GLenum type)
	{
		switch (type)
		{
		case GL_FLOAT:
			return sizeof(GLfloat);
		case GL_UNSIGNED_INT:
			return sizeof(GLuint);
		default:
			spdlog::error("Invalid type sent in {}, {}", __FILE__, __LINE__);
			return 0;
		}
	}
};

class VertexBufferLayout
{
public:
	VertexBufferLayout() = default;

	[[nodiscard]] GLsizei GetStride() const;
	[[nodiscard]] const std::vector<VertexBufferElement>& GetElements() const;
	[[nodiscard]] std::vector<VertexBufferElement>& GetElements();

	template <typename T>
	void Push(GLint count);

private:
	std::vector<VertexBufferElement> m_Elements{};
	GLsizei m_Stride{};
};

template <typename T>
void VertexBufferLayout::Push(GLint count)
{
	spdlog::error("Push of an unknown type in vertex buffer layout");
}

template <>
void VertexBufferLayout::Push<f32>(GLint count);

template <>
void VertexBufferLayout::Push<u32>(GLint count);
}