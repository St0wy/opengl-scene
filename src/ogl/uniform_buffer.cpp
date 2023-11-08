/**
 * @file uniform_buffer.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the UniformBuffer class.
 * @version 1.0
 * @date 04/05/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <GL/glew.h>
#include <spdlog/spdlog.h>

export module uniform_buffer;

import utils;

export
{
	namespace stw
	{
	class UniformBuffer
	{
	public:
		UniformBuffer() = default;
		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer(UniformBuffer&&) = delete;
		~UniformBuffer();

		UniformBuffer& operator=(const UniformBuffer&) = delete;
		UniformBuffer& operator=(UniformBuffer&&) = delete;

		void Init(GLuint bindingIndex);
		void Bind() const;
		void Allocate(GLsizeiptr size) const;
		void SetData(GLsizeiptr size, const GLvoid* data) const;
		void SetSubData(GLintptr offset, GLsizeiptr size, const GLvoid* data) const;
		void UnBind() const;
		void Delete();

	private:
		GLuint m_Ubo{};
		GLuint m_BindingIndex{};
	};

	stw::UniformBuffer::~UniformBuffer()
	{
		if (m_Ubo != 0)
		{
			spdlog::error("Destructor called on uniform buffer that is not deleted.");
		}
	}

	void stw::UniformBuffer::Init(const GLuint bindingIndex)
	{
		glGenBuffers(1, &m_Ubo);
		m_BindingIndex = bindingIndex;
	}

	void stw::UniformBuffer::Bind() const
	{
		if (m_Ubo == 0)
		{
			spdlog::error("Bind called on uniform buffer that is not initialized");
		}

		glBindBuffer(GL_UNIFORM_BUFFER, m_Ubo);
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
		glBindBufferRange(GL_UNIFORM_BUFFER, m_BindingIndex, m_Ubo, 0, size);
	}

	void stw::UniformBuffer::SetData(const GLsizeiptr size, const GLvoid* data) const
	{
		if (m_Ubo == 0)
		{
			spdlog::error("SetData called on uniform buffer that is not initialized");
		}

		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
	}

	void stw::UniformBuffer::SetSubData(const GLintptr offset, const GLsizeiptr size, const GLvoid* data) const
	{
		if (m_Ubo == 0)
		{
			spdlog::error("SetSubData called on uniform buffer that is not initialized");
		}
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	}

	void stw::UniformBuffer::UnBind() const
	{
		if (m_Ubo == 0)
		{
			spdlog::error("UnBind called on uniform buffer that is not initialized");
		}

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void stw::UniformBuffer::Delete()
	{
		glDeleteBuffers(1, &m_Ubo);
		m_Ubo = 0;
	}
	}// namespace stw
}