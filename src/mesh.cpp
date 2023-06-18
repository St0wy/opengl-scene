#include "mesh.hpp"

#include <span>
#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"
#include "scenes/instancing_scene.hpp"

stw::Mesh::Mesh(Mesh&& other) noexcept
	: m_Vertices(std::move(other.m_Vertices)),
	m_Indices(std::move(other.m_Indices)),
	m_Textures(std::move(other.m_Textures)),
	m_VertexArray(std::move(other.m_VertexArray)),
	m_VertexBuffer(std::move(other.m_VertexBuffer)),
	m_ModelMatrixBuffer(std::move(other.m_ModelMatrixBuffer)),
	m_IndexBuffer(std::move(other.m_IndexBuffer)),
	m_IsInitialized(other.m_IsInitialized)
{
	other.m_IsInitialized = false;
}

stw::Mesh::~Mesh()
{
	if (m_IsInitialized)
	{
		spdlog::error("Destructor called on a mesh that is still initialized");
	}
}

stw::Mesh& stw::Mesh::operator=(Mesh&& other) noexcept
{
	if (this == &other)
		return *this;

	m_Vertices = std::move(other.m_Vertices);
	m_Indices = std::move(other.m_Indices);
	m_Textures = std::move(other.m_Textures);
	m_VertexArray = std::move(other.m_VertexArray);
	m_VertexBuffer = std::move(other.m_VertexBuffer);
	m_ModelMatrixBuffer = std::move(other.m_ModelMatrixBuffer);
	m_IndexBuffer = std::move(other.m_IndexBuffer);
	m_IsInitialized = other.m_IsInitialized;
	other.m_IsInitialized = false;

	return *this;
}

void stw::Mesh::Init(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<Texture> textures)
{
	m_Vertices = std::move(vertices);
	m_Indices = std::move(indices);
	m_Textures = std::move(textures);
	SetupMesh();

	m_IsInitialized = true;
}

void stw::Mesh::Delete()
{
	if (!m_IsInitialized)
	{
		spdlog::error("Delete called on a mesh that is not initialized");
	}

	m_VertexBuffer.Delete();
	m_IndexBuffer.Delete();
	m_VertexArray.Delete();
	m_ModelMatrixBuffer.Delete();

	m_IsInitialized = false;
}

void stw::Mesh::Draw(Pipeline& pipeline, const glm::mat4& modelMatrix) const
{
	DrawInstanced(pipeline, {&modelMatrix, 1});
}

void stw::Mesh::DrawNoSpecular(stw::Pipeline& pipeline, const glm::mat4& modelMatrix) const
{
	std::array matrices{modelMatrix};
	DrawNoSpecularInstanced(pipeline, matrices);
}

void stw::Mesh::DrawInstanced(Pipeline& pipeline, const std::span<const glm::mat4> modelMatrices) const
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

	DrawMeshOnlyInstanced(modelMatrices);

	glActiveTexture(GL_TEXTURE0);
}

void stw::Mesh::DrawNoSpecularInstanced(Pipeline& pipeline, const std::span<const glm::mat4> modelMatrices) const
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

	DrawMeshOnlyInstanced(modelMatrices);

	glActiveTexture(GL_TEXTURE0);
}

void stw::Mesh::DrawMeshOnlyInstanced(const std::span<const glm::mat4> modelMatrices) const
{
	m_VertexArray.Bind();

	m_ModelMatrixBuffer.SetData(modelMatrices);
	const auto size = static_cast<GLsizei>(m_Indices.size());
	const auto elemCount = static_cast<GLsizei>(modelMatrices.size());
	glDrawElementsInstanced(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr, elemCount);

	m_VertexArray.UnBind();
}

void stw::Mesh::SetupMesh()
{
	m_VertexArray.Init();

	m_VertexBuffer.Init(m_Vertices);
	m_IndexBuffer.Init(m_Indices);
	m_ModelMatrixBuffer.Init();

	VertexBufferLayout vertexLayout;
	vertexLayout.Push<float>(3);
	vertexLayout.Push<float>(3);
	vertexLayout.Push<float>(2);

	m_VertexArray.AddBuffer(m_VertexBuffer, vertexLayout);

	VertexBufferLayout modelMatrixLayout;
	modelMatrixLayout.Push<float>(4, 1);
	modelMatrixLayout.Push<float>(4, 1);
	modelMatrixLayout.Push<float>(4, 1);
	modelMatrixLayout.Push<float>(4, 1);

	m_VertexArray.AddBuffer(m_ModelMatrixBuffer, modelMatrixLayout);
}
