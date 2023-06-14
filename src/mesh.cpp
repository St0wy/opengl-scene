#include "mesh.hpp"

#include <array>
#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"

stw::Mesh::Mesh(Mesh&& other) noexcept
	: m_Vertices(std::move(other.m_Vertices)),
	m_Indices(std::move(other.m_Indices)),
	m_Textures(std::move(other.m_Textures)),
	m_Vao(other.m_Vao),
	m_VertexBuffer(std::move(other.m_VertexBuffer)),
	m_IndexBuffer(std::move(other.m_IndexBuffer))
{
	other.m_Vao = 0;
}

stw::Mesh::~Mesh()
{
	if (m_Vao != 0)
	{
		spdlog::warn("Destructor called on mesh with Vao not equal to 0");
	}
}

void stw::Mesh::Init(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<Texture> textures)
{
	m_Vertices = std::move(vertices);
	m_Indices = std::move(indices);
	m_Textures = std::move(textures);
	SetupMesh();
}

void stw::Mesh::Delete()
{
	if (m_Vao == 0)
	{
		spdlog::warn("Deleting mesh with no Vao");
		return;
	}

	GLCALL(glDeleteVertexArrays(1, &m_Vao));

	m_VertexBuffer.Delete();
	m_IndexBuffer.Delete();
	m_Vao = 0;
}

GLuint stw::Mesh::Vao() const
{
	return m_Vao;
}

void stw::Mesh::Draw(const Pipeline& pipeline) const
{
	u32 diffuseTextureCount = 0;
	u32 specularTextureCount = 0;
	for (std::size_t i = 0; i < m_Textures.size(); i++)
	{
		const auto id = GetTextureFromId(static_cast<i32>(i));
		glActiveTexture(id);

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
		default:
			break;
		}

		pipeline.SetInt(fmt::format("material.{}{}", ToString(m_Textures[i].textureType), number), static_cast<i32>(i));
		glBindTexture(GL_TEXTURE_2D, m_Textures[i].textureId);
	}
	glActiveTexture(GL_TEXTURE0);

	DrawMeshOnly();

	glActiveTexture(GL_TEXTURE0);
}

void stw::Mesh::DrawNoSpecular(const Pipeline& pipeline) const
{
	u32 diffuseTextureCount = 0;
	for (std::size_t i = 0; i < m_Textures.size(); i++)
	{
		const auto id = GetTextureFromId(static_cast<i32>(i));
		glActiveTexture(id);

		u32 number;
		switch (m_Textures[i].textureType)
		{
		case TextureType::Diffuse:
			diffuseTextureCount++;
			number = diffuseTextureCount;
			break;
		default:
			break;
		}

		pipeline.SetInt(fmt::format("material.texture_diffuse{}", number), static_cast<i32>(i));
		glBindTexture(GL_TEXTURE_2D, m_Textures[i].textureId);
	}
	glActiveTexture(GL_TEXTURE0);

	DrawMeshOnly();

	glActiveTexture(GL_TEXTURE0);
}

void stw::Mesh::DrawInstanced(const Pipeline& pipeline, const GLsizei count) const
{
	u32 diffuseTextureCount = 0;
	u32 specularTextureCount = 0;
	for (std::size_t i = 0; i < m_Textures.size(); i++)
	{
		const auto id = GetTextureFromId(static_cast<i32>(i));
		glActiveTexture(id);

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
		default:
			break;
		}

		pipeline.SetInt(fmt::format("material.{}{}", ToString(m_Textures[i].textureType), number), static_cast<i32>(i));
		glBindTexture(GL_TEXTURE_2D, m_Textures[i].textureId);
	}
	glActiveTexture(GL_TEXTURE0);

	DrawMeshOnlyInstanced(count);

	glActiveTexture(GL_TEXTURE0);
}

void stw::Mesh::DrawNoSpecularInstanced(const Pipeline& pipeline, const GLsizei count) const
{
	u32 diffuseTextureCount = 0;
	for (std::size_t i = 0; i < m_Textures.size(); i++)
	{
		const auto id = GetTextureFromId(static_cast<i32>(i));
		glActiveTexture(id);

		u32 number;
		switch (m_Textures[i].textureType)
		{
		case TextureType::Diffuse:
			diffuseTextureCount++;
			number = diffuseTextureCount;
			break;
		default:
			break;
		}

		pipeline.SetInt(fmt::format("material.texture_diffuse{}", number), static_cast<i32>(i));
		glBindTexture(GL_TEXTURE_2D, m_Textures[i].textureId);
	}
	glActiveTexture(GL_TEXTURE0);

	DrawMeshOnlyInstanced(count);

	glActiveTexture(GL_TEXTURE0);
}

void stw::Mesh::DrawMeshOnly() const
{
	glBindVertexArray(m_Vao);

	const auto size = static_cast<GLsizei>(m_Indices.size());
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void stw::Mesh::DrawMeshOnlyInstanced(const GLsizei count) const
{
	glBindVertexArray(m_Vao);

	const auto size = static_cast<GLsizei>(m_Indices.size());
	glDrawElementsInstanced(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr, count);
	glBindVertexArray(0);
}

void stw::Mesh::SetupMesh()
{
	glGenVertexArrays(1, &m_Vao);
	glBindVertexArray(m_Vao);

	m_VertexBuffer.Init(m_Vertices);
	m_IndexBuffer.Init(m_Indices);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

	// Vertex normals
	glEnableVertexAttribArray(1);
	auto offset = reinterpret_cast<void*>(offsetof(Vertex, normal)); // NOLINT(performance-no-int-to-ptr)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset);

	// Vertex texture coords
	glEnableVertexAttribArray(2);
	offset = reinterpret_cast<void*>(offsetof(Vertex, texCoords)); // NOLINT(performance-no-int-to-ptr)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset);
	glBindVertexArray(0);
}
