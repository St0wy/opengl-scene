#include "mesh.hpp"

#include <array>
#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"

stw::Mesh::Mesh(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<Texture> textures)
	: m_Vertices(std::move(vertices)), m_Indices(std::move(indices)), m_Textures(std::move(textures))
{
	SetupMesh();
}

stw::Mesh::Mesh(Mesh&& other) noexcept
	: m_Vertices(std::move(other.m_Vertices)),
	m_Indices(std::move(other.m_Indices)),
	m_Textures(std::move(other.m_Textures)),
	m_Vao(other.m_Vao),
	m_Vbo(other.m_Vbo),
	m_Ebo(other.m_Ebo)
{
	other.m_Vao = 0;
	other.m_Vbo = 0;
	other.m_Ebo = 0;
}

stw::Mesh::~Mesh()
{
	if (m_Vao == 0)
	{
		return;
	}

	glDeleteVertexArrays(1, &m_Vao);

	const std::array buffers = {m_Vbo, m_Ebo};
	glDeleteBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
}

void stw::Mesh::Draw(const Pipeline& pipeline) const
{
	u32 diffuseTextureCount = 0;
	u32 specularTextureCount = 0;
	for (std::size_t i = 0; i < m_Textures.size(); i++)
	{
		const auto id = GetTextureFromId(static_cast<i32>(i));
		glActiveTexture(id);
		if (CHECK_GL_ERROR())
		{
			assert(false);
		}

		u32 number;
		switch (m_Textures[i].textureType)
		{
		case TextureType::Diffuse:
			diffuseTextureCount++;
			number = diffuseTextureCount;
			break;
		case TextureType::Specular:
			specularTextureCount++;
			number = specularTextureCount;
			break;
		}

		pipeline.SetInt(fmt::format("material.{}{}", ToString(m_Textures[i].textureType), number), static_cast<i32>(i));
		glBindTexture(GL_TEXTURE_2D, m_Textures[i].textureId);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(m_Vao);

	const auto size = static_cast<GLsizei>(m_Indices.size());
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

void stw::Mesh::SetupMesh()
{
	glGenVertexArrays(1, &m_Vao);
	glGenBuffers(1, &m_Vbo);
	glGenBuffers(1, &m_Ebo);

	glBindVertexArray(m_Vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);

	auto size = static_cast<GLsizeiptr>(m_Vertices.size() * sizeof(Vertex));
	glBufferData(GL_ARRAY_BUFFER, size, m_Vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
	size = static_cast<GLsizeiptr>(m_Indices.size() * sizeof(u32));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, m_Indices.data(), GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

	// vertex normals
	glEnableVertexAttribArray(1);
	auto offset = reinterpret_cast<void*>(offsetof(Vertex, normal)); // NOLINT(performance-no-int-to-ptr)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset);

	// vertex texture coords
	glEnableVertexAttribArray(2);
	offset = reinterpret_cast<void*>(offsetof(Vertex, texCoords)); // NOLINT(performance-no-int-to-ptr)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset);
	glBindVertexArray(0);
}
