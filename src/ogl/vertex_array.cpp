/**
 * @file vertex_array.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the VertexArray class.
 * @version 1.0
 * @date 09/11/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <glad/glad.h>
#include <spdlog/spdlog.h>

export module vertex_array;

import vertex_buffer;
import vertex_buffer_layout;

export namespace stw
{
class VertexArray
{
public:
	VertexArray() = default;
	VertexArray(const VertexArray&) = delete;
	VertexArray(VertexArray&& other) noexcept;
	~VertexArray();
	VertexArray& operator=(const VertexArray&) = delete;
	VertexArray& operator=(VertexArray&& other) noexcept;

	void Init();

	template<typename T>
	void AddBuffer(const VertexBuffer<T>& vertexBuffer, const VertexBufferLayout& layout);
	void Bind() const;
	void UnBind() const;
	void Delete();

private:
	GLuint m_Vao{};
	GLuint m_CurrentIndex{};
};

template<typename T>
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
		glEnableVertexAttribArray(glIndex);
		const auto offsetVoid = reinterpret_cast<void*>(offset);// NOLINT(performance-no-int-to-ptr)

		glVertexAttribPointer(glIndex, element.count, element.type, element.normalized, layout.GetStride(), offsetVoid);
		offset += static_cast<std::size_t>(element.count) * VertexBufferElement::GetSizeOfType(element.type);

		if (element.divisor.has_value())
		{
			glVertexAttribDivisor(glIndex, element.divisor.value());
		}
	}

	m_CurrentIndex = glIndex + 1;
}


VertexArray::VertexArray(VertexArray&& other) noexcept : m_Vao(other.m_Vao), m_CurrentIndex(other.m_CurrentIndex)
{
	other.m_Vao = 0;
	other.m_CurrentIndex = 0;
}

VertexArray::~VertexArray()
{
	if (m_Vao != 0)
	{
		spdlog::error("Destructor called on vertex array that is not deleted");
	}
}

void VertexArray::Init()
{
	glGenVertexArrays(1, &m_Vao);
	glBindVertexArray(m_Vao);
}

void VertexArray::Bind() const
{
	if (m_Vao == 0)
	{
		spdlog::error("Binding a vertex array that is not initialized");
	}

	glBindVertexArray(m_Vao);
}

void VertexArray::UnBind() const { glBindVertexArray(0); }

void VertexArray::Delete()
{
	glDeleteVertexArrays(1, &m_Vao);

	m_Vao = 0;
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
	if (this == &other)
	{
		return *this;
	}

	m_Vao = other.m_Vao;
	m_CurrentIndex = other.m_CurrentIndex;
	other.m_Vao = 0;
	other.m_CurrentIndex = 0;

	return *this;
}
}// namespace stw
