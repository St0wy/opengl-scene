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
	VertexArray(VertexArray&& other) noexcept;
	~VertexArray();
	VertexArray& operator=(const VertexArray&) = delete;
	VertexArray& operator=(VertexArray&&) noexcept ;

	void Init();

	template <typename T>
	void AddBuffer(const VertexBuffer<T>& vertexBuffer, const VertexBufferLayout& layout);
	void Bind() const;
	void UnBind() const;
	void Delete();

private:
	GLuint m_Vao{};
	GLuint m_CurrentIndex{};
};

template <typename T>
void VertexArray::AddBuffer(const VertexBuffer<T>& vertexBuffer, const VertexBufferLayout& layout)
{
	Bind();
	vertexBuffer.Bind();
	const auto& elements = layout.GetElements();

	std::size_t offset = 0;
	GLuint glIndex = 0;
	for (std::size_t i = 0; i < elements.size(); i++)
	{
		glIndex = static_cast<GLuint>(i) + m_CurrentIndex;
		const auto& element = elements[i];
		GLCALL(glEnableVertexAttribArray(glIndex));
		const auto offsetVoid = reinterpret_cast<void*>(offset); // NOLINT(performance-no-int-to-ptr)
		GLCALL(
			glVertexAttribPointer(glIndex, element.count, element.type, element.normalized, layout.GetStride(),
				offsetVoid ));
		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);

		if (element.divisor.has_value())
		{
			GLCALL(glVertexAttribDivisor(glIndex, element.divisor.value()));
		}
	}

	m_CurrentIndex = glIndex + 1;
}
}
