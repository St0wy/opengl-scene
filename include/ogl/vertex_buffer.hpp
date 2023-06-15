#pragma once

#include <span>
#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"

namespace stw
{
template <class T>
class VertexBuffer
{
public:
	VertexBuffer() = default;
	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer(VertexBuffer&& other) noexcept;
	~VertexBuffer();

	VertexBuffer& operator=(const VertexBuffer&) = delete;
	VertexBuffer& operator=(VertexBuffer&& other) noexcept;

	void Init(std::span<const T> data);
	void Init();
	void SetData(std::span<const T> data) const;
	void Bind() const;
	void UnBind() const;
	void Delete();

private:
	GLuint m_BufferId{};
	bool m_IsInitialized = false;
};

template <class T>
VertexBuffer<T>::VertexBuffer(VertexBuffer&& other) noexcept
	: m_BufferId(other.m_BufferId), m_IsInitialized(other.m_IsInitialized)
{
	other.m_BufferId = 0;
	other.m_IsInitialized = false;
}

template <class T>
VertexBuffer<T>::~VertexBuffer()
{
	if (m_IsInitialized)
	{
		spdlog::warn("Destructor called when vertex buffer is still initialized.");
	}
}

template <class T>
VertexBuffer<T>& VertexBuffer<T>::operator=(VertexBuffer&& other) noexcept
{
	if (this == &other)
		return *this;

	m_BufferId = other.m_BufferId;
	m_IsInitialized = other.m_IsInitialized;
	other.m_BufferId = 0;
	other.m_IsInitialized = false;

	return *this;
}

template <class T>
void VertexBuffer<T>::Init(std::span<const T> data)
{
	Init();
	SetData(data);
}

template <class T>
void VertexBuffer<T>::Init()
{
	GLCALL(glGenBuffers(1, &m_BufferId));
	m_IsInitialized = true;
}

template <class T>
void VertexBuffer<T>::SetData(std::span<const T> data) const
{
	assert(m_IsInitialized);
	Bind();
	GLCALL(glBufferData(GL_ARRAY_BUFFER, data.size_bytes(), data.data(), GL_STATIC_DRAW));
	UnBind();
}

template <class T>
void VertexBuffer<T>::Bind() const
{
	assert(m_IsInitialized);
	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_BufferId));
}

template <class T>
// ReSharper disable once CppMemberFunctionMayBeStatic
void VertexBuffer<T>::UnBind() const
{
	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

template <class T>
void VertexBuffer<T>::Delete()
{
	GLCALL(glDeleteBuffers(1, &m_BufferId));

	m_IsInitialized = false;
}
}
