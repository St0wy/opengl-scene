#include "ogl/vertex_buffer_layout.hpp"

namespace stw
{
GLsizei VertexBufferLayout::GetStride() const
{
	return m_Stride;
}

const std::vector<VertexBufferElement>& VertexBufferLayout::GetElements() const
{
	return m_Elements;
}

std::vector<VertexBufferElement>& VertexBufferLayout::GetElements()
{
	return m_Elements;
}

template <>
void VertexBufferLayout::Push<f32>(const GLint count)
{
	m_Elements.push_back({GL_FLOAT, count, GL_FALSE});
	m_Stride += VertexBufferElement::GetSizeOfType(GL_FLOAT);
}

template <>
void VertexBufferLayout::Push<u32>(const GLint count)
{
	m_Elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
	m_Stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
}
}
