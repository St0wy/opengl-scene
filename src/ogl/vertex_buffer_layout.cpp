/**
 * @file vertex_buffer_layout.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the abstraction for Vertex Buffer Layouts.
 * @version 1.0
 * @date 04/05/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <optional>
#include <vector>

#include <GL/glew.h>
#include <spdlog/spdlog.h>

export module vertex_buffer_layout;

import number_types;

export
{
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

		template<typename T>
		void Push(GLint count);

		template<typename T>
		void Push(GLint count, GLuint divisor);

	private:
		std::vector<VertexBufferElement> m_Elements{};
		GLsizei m_Stride{};
	};

	template<typename T>
	void VertexBufferLayout::Push(GLint)
	{
		spdlog::error("Push of an unknown type in vertex buffer layout");
	}

	template<typename T>
	void VertexBufferLayout::Push(GLint, GLuint)
	{
		spdlog::error("Push of an unknown type in vertex buffer layout with divisor");
	}

	template<>
	void VertexBufferLayout::Push<f32>(GLint count);

	template<>
	void VertexBufferLayout::Push<u32>(GLint count);

	template<>
	void VertexBufferLayout::Push<f32>(GLint count, GLuint divisor);

	template<>
	void VertexBufferLayout::Push<u32>(GLint count, GLuint divisor);

	GLsizei VertexBufferLayout::GetStride() const { return m_Stride; }

	const std::vector<VertexBufferElement>& VertexBufferLayout::GetElements() const { return m_Elements; }

	std::vector<VertexBufferElement>& VertexBufferLayout::GetElements() { return m_Elements; }

	template<>
	void VertexBufferLayout::Push<f32>(const GLint count)
	{
		m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
	}

	template<>
	void VertexBufferLayout::Push<u32>(const GLint count)
	{
		m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
	}

	template<>
	void VertexBufferLayout::Push<f32>(const GLint count, GLuint divisor)
	{
		m_Elements.push_back({ GL_FLOAT, count, GL_FALSE, { divisor } });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
	}

	template<>
	void VertexBufferLayout::Push<u32>(const GLint count, GLuint divisor)
	{
		m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE, { divisor } });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
	}
	}// namespace stw
}