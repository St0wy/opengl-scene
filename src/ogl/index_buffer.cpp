/**
 * @file index_buffer.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the IndexBuffer class.
 * @version 1.0
 * @date 09/11/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <span>

#include <glad/glad.h>
#include <spdlog/spdlog.h>

export module index_buffer;

import utils;
import number_types;

export namespace stw
{
/**
 * Wrapper arround an OpenGL index buffer (GL_ELEMENT_ARRAY_BUFFER).
 */
class IndexBuffer
{
public:
	IndexBuffer() = default;
	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer(IndexBuffer&& other) noexcept;
	~IndexBuffer();

	IndexBuffer& operator=(const IndexBuffer&) = delete;
	IndexBuffer& operator=(IndexBuffer&& other) noexcept;

	/**
	 * Creates the index buffer with the provided indices.
	 * @param indices Indices that will be in the buffer.
	 */
	void Init(std::span<GLuint> indices);
	void Bind() const;
	static void UnBind();
	void Delete();

	/**
	 * Gets the number of indices in the buffer.
	 */
	[[nodiscard]] u32 GetCount() const;

private:
	GLuint m_BufferId{};
	u32 m_Count{};
	bool m_IsInitialized = false;
};

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
	: m_BufferId(other.m_BufferId), m_Count(other.m_Count), m_IsInitialized(other.m_IsInitialized)
{
	other.m_BufferId = 0;
	other.m_Count = 0;
	other.m_IsInitialized = false;
}

IndexBuffer::~IndexBuffer()
{
	if (m_IsInitialized)
	{
		spdlog::warn("Destructor called when vertex buffer is still initialized.");
	}
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
{
	m_BufferId = other.m_BufferId;
	m_Count = other.m_Count;
	m_IsInitialized = other.m_IsInitialized;

	other.m_BufferId = 0;
	other.m_Count = 0;
	other.m_IsInitialized = false;

	return *this;
}

void IndexBuffer::Init(const std::span<u32> indices)
{
	m_Count = static_cast<u32>(indices.size());
	glGenBuffers(1, &m_BufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size_bytes()), indices.data(), GL_STATIC_DRAW);

	m_IsInitialized = true;
}

void IndexBuffer::Bind() const
{
	assert(m_IsInitialized);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
}

void IndexBuffer::UnBind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

void IndexBuffer::Delete()
{
	glDeleteBuffers(1, &m_BufferId);

	m_IsInitialized = false;
}

u32 IndexBuffer::GetCount() const { return m_Count; }
}// namespace stw
