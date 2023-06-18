#pragma once

#include <optional>
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
	std::optional<GLuint> divisor{};

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

	template <typename T>
	void Push(GLint count, GLuint divisor);

private:
	std::vector<VertexBufferElement> m_Elements{};
	GLsizei m_Stride{};
};

template <typename T>
void VertexBufferLayout::Push(GLint)
{
	spdlog::error("Push of an unknown type in vertex buffer layout");
}

template <typename T>
void VertexBufferLayout::Push(GLint, GLuint)
{
	spdlog::error("Push of an unknown type in vertex buffer layout with divisor");
}

template <>
void VertexBufferLayout::Push<f32>(GLint count);

template <>
void VertexBufferLayout::Push<u32>(GLint count);

template <>
void VertexBufferLayout::Push<f32>(GLint count, GLuint divisor);

template <>
void VertexBufferLayout::Push<u32>(GLint count, GLuint divisor);
}
