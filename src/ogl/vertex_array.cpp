#include "ogl/vertex_array.hpp"

stw::VertexArray::VertexArray(VertexArray&& other) noexcept : m_Vao(other.m_Vao), m_CurrentIndex(other.m_CurrentIndex)
{
	other.m_Vao = 0;
	other.m_CurrentIndex = 0;
}

stw::VertexArray::~VertexArray()
{
	if (m_Vao != 0)
	{
		spdlog::error("Destructor called on vertex array that is not deleted");
	}
}

void stw::VertexArray::Init()
{
	GLCALL(glGenVertexArrays(1, &m_Vao));
	GLCALL(glBindVertexArray(m_Vao));
}

void stw::VertexArray::Bind() const
{
	if (m_Vao == 0)
	{
		spdlog::error("Binding a vertex array that is not initialized");
	}

	GLCALL(glBindVertexArray(m_Vao));
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void stw::VertexArray::UnBind() const { GLCALL(glBindVertexArray(0)); }

void stw::VertexArray::Delete()
{
	GLCALL(glDeleteVertexArrays(1, &m_Vao));

	m_Vao = 0;
}
stw::VertexArray& stw::VertexArray::operator=(stw::VertexArray&& other) noexcept
{
	if (this == &other) return *this;

	m_Vao = other.m_Vao;
	m_CurrentIndex = other.m_CurrentIndex;
	other.m_Vao = 0;
	other.m_CurrentIndex = 0;

	return *this;
}
