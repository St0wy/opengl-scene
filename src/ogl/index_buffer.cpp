/**
 * @file index_buffer.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the IndexBuffer class.
 * @version 1.0
 * @date 14/07/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <span>

#include <GL/glew.h>
#include <spdlog/spdlog.h>

export module index_buffer;

import utils;
import number_types;

export
{
	namespace stw
	{
	class IndexBuffer
	{
	public:
		IndexBuffer() = default;
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer(IndexBuffer&& other) noexcept;
		~IndexBuffer();

		IndexBuffer& operator=(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&& other) noexcept;

		void Init(std::span<GLuint> data);
		void Bind() const;
		static void UnBind();
		void Delete();

		[[nodiscard]] u32 GetCount() const;

	private:
		GLuint m_BufferId{};
		u32 m_Count{};
		bool m_IsInitialized = false;
	};

	stw::IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
		: m_BufferId(other.m_BufferId), m_Count(other.m_Count), m_IsInitialized(other.m_IsInitialized)
	{
		other.m_BufferId = 0;
		other.m_Count = 0;
		other.m_IsInitialized = false;
	}

	stw::IndexBuffer::~IndexBuffer()
	{
		if (m_IsInitialized)
		{
			spdlog::warn("Destructor called when vertex buffer is still initialized.");
		}
	}

	stw::IndexBuffer& stw::IndexBuffer::operator=(IndexBuffer&& other) noexcept
	{
		m_BufferId = other.m_BufferId;
		m_Count = other.m_Count;
		m_IsInitialized = other.m_IsInitialized;

		other.m_BufferId = 0;
		other.m_Count = 0;
		other.m_IsInitialized = false;

		return *this;
	}

	void stw::IndexBuffer::Init(const std::span<u32> data)
	{
		m_Count = static_cast<u32>(data.size());
		glGenBuffers(1, &m_BufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.size_bytes()), data.data(), GL_STATIC_DRAW);

		m_IsInitialized = true;
	}

	void stw::IndexBuffer::Bind() const
	{
		assert(m_IsInitialized);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
	}

	void stw::IndexBuffer::UnBind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

	void stw::IndexBuffer::Delete()
	{
		glDeleteBuffers(1, &m_BufferId);

		m_IsInitialized = false;
	}

	u32 stw::IndexBuffer::GetCount() const { return m_Count; }
	}// namespace stw
}