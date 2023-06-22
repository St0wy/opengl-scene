#include "ogl/uniform_buffer.hpp"

#include <spdlog/spdlog.h>

#include "utils.hpp"

stw::UniformBuffer::~UniformBuffer()
{
	if (m_Ubo != 0)
	{
		spdlog::error("Destructor called on uniform buffer that is not deleted.");
	}
}

void stw::UniformBuffer::Init(const GLuint bindingIndex)
{
	GLCALL(glGenBuffers(1, &m_Ubo));
	m_BindingIndex = bindingIndex;
}

void stw::UniformBuffer::Bind() const
{
	if (m_Ubo == 0)
	{
		spdlog::error("Bind called on uniform buffer that is not initialized");
	}

	GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, m_Ubo));
}

void stw::UniformBuffer::Allocate(const GLsizeiptr size) const
{
	if (m_Ubo == 0)
	{
		spdlog::error("Allocate called on uniform buffer that is not initialized");
	}
	Bind();
	SetData(size, nullptr);
	UnBind();
	GLCALL(glBindBufferRange(GL_UNIFORM_BUFFER, m_BindingIndex, m_Ubo, 0, size));
}

void stw::UniformBuffer::SetData(const GLsizeiptr size, const GLvoid* data) const
{
	if (m_Ubo == 0)
	{
		spdlog::error("SetData called on uniform buffer that is not initialized");
	}

	GLCALL(glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW));
}

void stw::UniformBuffer::SetSubData(const GLintptr offset, const GLsizeiptr size, const GLvoid* data) const
{
	if (m_Ubo == 0)
	{
		spdlog::error("SetSubData called on uniform buffer that is not initialized");
	}
	GLCALL(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data));
}

void stw::UniformBuffer::UnBind() const
{
	if (m_Ubo == 0)
	{
		spdlog::error("UnBind called on uniform buffer that is not initialized");
	}

	GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void stw::UniformBuffer::Delete()
{
	GLCALL(glDeleteBuffers(1, &m_Ubo));
	m_Ubo = 0;
}
