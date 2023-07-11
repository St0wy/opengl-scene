#include "mesh.hpp"

#include <GL/glew.h>
#include <span>
#include <spdlog/spdlog.h>

#include "utils.hpp"

stw::Mesh::Mesh(Mesh&& other) noexcept
	: m_Vertices(std::move(other.m_Vertices)), m_Indices(std::move(other.m_Indices)),
	  m_VertexArray(std::move(other.m_VertexArray)), m_VertexBuffer(std::move(other.m_VertexBuffer)),
	  m_ModelMatrixBuffer(std::move(other.m_ModelMatrixBuffer)), m_IndexBuffer(std::move(other.m_IndexBuffer)),
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
	if (this == &other) return *this;

	m_Vertices = std::move(other.m_Vertices);
	m_Indices = std::move(other.m_Indices);
	m_VertexArray = std::move(other.m_VertexArray);
	m_VertexBuffer = std::move(other.m_VertexBuffer);
	m_ModelMatrixBuffer = std::move(other.m_ModelMatrixBuffer);
	m_IndexBuffer = std::move(other.m_IndexBuffer);
	m_IsInitialized = other.m_IsInitialized;
	other.m_IsInitialized = false;

	return *this;
}

void stw::Mesh::Init(std::vector<Vertex> vertices, std::vector<u32> indices)
{
	m_Vertices = std::move(vertices);
	m_Indices = std::move(indices);
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

std::size_t stw::Mesh::GetIndicesSize() const { return m_Indices.size(); }

void stw::Mesh::Bind(const std::span<const glm::mat4> modelMatrices) const
{
	m_VertexArray.Bind();
	m_ModelMatrixBuffer.SetData(modelMatrices);
}

void stw::Mesh::UnBind() const
{
	m_VertexArray.UnBind();

	GLCALL(glActiveTexture(GL_TEXTURE0));
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
	vertexLayout.Push<float>(3);

	m_VertexArray.AddBuffer(m_VertexBuffer, vertexLayout);

	VertexBufferLayout modelMatrixLayout;
	modelMatrixLayout.Push<float>(4, 1);
	modelMatrixLayout.Push<float>(4, 1);
	modelMatrixLayout.Push<float>(4, 1);
	modelMatrixLayout.Push<float>(4, 1);

	m_VertexArray.AddBuffer(m_ModelMatrixBuffer, modelMatrixLayout);
}
stw::Mesh stw::Mesh::CreateQuad()
{
	std::vector<Vertex> vertices = {
		Vertex{ glm::vec3{ -1.0f, 1.0f, 0.0f }, glm::vec3{}, glm::vec2{ 0.0f, 1.0f }, glm::vec3{} },
		Vertex{ glm::vec3{ -1.0f, -1.0f, 0.0f }, glm::vec3{}, glm::vec2{ 0.0f, 0.0f }, glm::vec3{} },
		Vertex{ glm::vec3{ 1.0f, 1.0f, 0.0f }, glm::vec3{}, glm::vec2{ 1.0f, 1.0f }, glm::vec3{} },
		Vertex{ glm::vec3{ 1.0f, -1.0f, 0.0f }, glm::vec3{}, glm::vec2{ 1.0f, 0.0f }, glm::vec3{} },
	};
	std::vector<u32> indices = { 0, 1, 2, 1, 3, 2 };

	Mesh mesh;
	mesh.Init(std::move(vertices), std::move(indices));

	return mesh;
}

const stw::VertexArray& stw::Mesh::GetVertexArray() const { return m_VertexArray; }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-avoid-magic-numbers"
stw::Mesh stw::Mesh::CreateCube()
{
	constexpr f32 size = 1.0f;
	std::vector<Vertex> vertices = {
		{ glm::vec3(-size, -size, -size), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3{} },
		{ glm::vec3(-size, -size, size), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3{} },
		{ glm::vec3(-size, size, -size), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3{} },
		{ glm::vec3(-size, size, size), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3{} },
		{ glm::vec3(size, -size, -size), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3{} },
		{ glm::vec3(size, -size, size), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec3{} },
		{ glm::vec3(size, size, -size), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec3{} },
		{ glm::vec3(size, size, size), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec3{} }
	};

	// clang-format off
	std::vector<u32> indices = {
		0, 1, 2, 2, 1, 3, // Left face
		5, 4, 6, 5, 6, 7, // Right face
		2, 3, 6, 6, 3, 7, // Top face
		0, 2, 4, 4, 2, 6, // Front face
		1, 5, 3, 3, 5, 7, // Back face
		0, 4, 1, 1, 4, 5  // Bottom face
	};
	// clang-format on

	Mesh mesh;
	mesh.Init(std::move(vertices), std::move(indices));

	return mesh;
}
#pragma clang diagnostic pop