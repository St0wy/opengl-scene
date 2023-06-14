#pragma once

#include "ogl/vertex_buffer.hpp"
#include "ogl/vertex_buffer_layout.hpp"

namespace stw
{
class VertexArray
{
public:
	VertexArray() = default;
	VertexArray(const VertexArray&) = delete;
	VertexArray(VertexArray&&) = default;
	~VertexArray();
	VertexArray& operator=(const VertexArray&) = delete;
	VertexArray& operator=(VertexArray&&) = default;

	void Init();
	template <typename T>
	void AddBuffer(const VertexBuffer<T>& vertexBuffer, const VertexBufferLayout& layout);
	void Bind() const;
	void UnBind() const;
	void Delete();

private:
	GLuint m_Vao{};
};

template <typename T>
void VertexArray::AddBuffer(const VertexBuffer<T>& vertexBuffer, const VertexBufferLayout& layout)
{
	Bind();
	vertexBuffer.Bind();
	const auto& elements = layout.GetElements();

	u64 offset = 0;
	for (std::size_t i = 0; i < elements.size(); i++)
	{
		const auto glI = static_cast<GLuint>(i);
		const auto& element = elements[i];
		GLCALL(glEnableVertexAttribArray(glI));
		const auto offsetVoid = reinterpret_cast<void*>(offset); // NOLINT(performance-no-int-to-ptr)
		GLCALL(
			glVertexAttribPointer(glI, element.count, element.type, element.normalized, layout.GetStride(), offsetVoid
			));
		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
	}
}
}
