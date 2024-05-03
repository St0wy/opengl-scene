/**
 * @file mesh.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains Mesh class.
 * @version 1.0
 * @date 07/11/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <numbers>
#include <span>
#include <vector>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <spdlog/spdlog.h>

export module mesh;

import number_types;
import consts;
import utils;
import index_buffer;
import vertex_array;
import vertex_buffer;
import vertex_buffer_layout;

export namespace stw
{
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	// The tangent is used to render normal maps
	glm::vec3 tangent;
};

class Mesh
{
public:
	Mesh() = default;
	Mesh(const Mesh&) = delete;
	Mesh(Mesh&& other) noexcept;
	~Mesh();

	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&& other) noexcept;

	static Mesh CreateQuad();
	static Mesh CreateCube();
	static Mesh CreateInsideCube();
	static Mesh CreateUvSphere(f32 radius, u32 latitudes, u32 longitudes);

	void Init(std::vector<Vertex> vertices, std::vector<u32> indices);
	void Delete();

	[[nodiscard]] std::size_t GetIndicesSize() const;
	[[nodiscard]] const VertexArray& GetVertexArray() const;

	void Bind(std::span<const glm::mat4> modelMatrices) const;
	void UnBind() const;

private:
	std::vector<Vertex> m_Vertices{};
	std::vector<u32> m_Indices{};

	VertexArray m_VertexArray{};
	VertexBuffer<Vertex> m_VertexBuffer{};
	VertexBuffer<glm::mat4> m_ModelMatrixBuffer{};
	IndexBuffer m_IndexBuffer{};

	bool m_IsInitialized = false;

	void SetupMesh();
};

Mesh::Mesh(Mesh&& other) noexcept
	: m_Vertices(std::move(other.m_Vertices)), m_Indices(std::move(other.m_Indices)),
	  m_VertexArray(std::move(other.m_VertexArray)), m_VertexBuffer(std::move(other.m_VertexBuffer)),
	  m_ModelMatrixBuffer(std::move(other.m_ModelMatrixBuffer)), m_IndexBuffer(std::move(other.m_IndexBuffer)),
	  m_IsInitialized(other.m_IsInitialized)
{
	other.m_IsInitialized = false;
}

Mesh::~Mesh()
{
	if (m_IsInitialized)
	{
		spdlog::error("Destructor called on a mesh that is still initialized");
	}
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
	if (this == &other)
	{
		return *this;
	}

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

void Mesh::Init(std::vector<Vertex> vertices, std::vector<u32> indices)
{
	m_Vertices = std::move(vertices);
	m_Indices = std::move(indices);
	SetupMesh();

	m_IsInitialized = true;
}

void Mesh::Delete()
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

std::size_t Mesh::GetIndicesSize() const { return m_Indices.size(); }

void Mesh::Bind(const std::span<const glm::mat4> modelMatrices) const
{
	m_VertexArray.Bind();
	m_ModelMatrixBuffer.SetData(modelMatrices);
}

void Mesh::UnBind() const
{
	m_VertexArray.UnBind();

	glActiveTexture(GL_TEXTURE0);
}

void Mesh::SetupMesh()
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
Mesh Mesh::CreateQuad()
{
	std::vector vertices = {
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

const VertexArray& Mesh::GetVertexArray() const { return m_VertexArray; }

Mesh Mesh::CreateCube()
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
		0, 2, 4, 4, 2, 6, // GetFront face
		1, 5, 3, 3, 5, 7, // Back face
		0, 4, 1, 1, 4, 5  // Bottom face
	};
	// clang-format on

	Mesh mesh;
	mesh.Init(std::move(vertices), std::move(indices));

	return mesh;
}

Mesh Mesh::CreateInsideCube()
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
		1, 0, 2, 1, 2, 3, // Left face
		4, 5, 6, 6, 5, 7, // Right face
		3, 2, 6, 3, 6, 7, // Top face
		2, 0, 4, 2, 4, 6, // GetFront face
		5, 1, 3, 5, 3, 7, // Back face
		4, 0, 1, 4, 1, 5  // Bottom face
	};
	// clang-format on

	Mesh mesh;
	mesh.Init(std::move(vertices), std::move(indices));

	return mesh;
}

Mesh Mesh::CreateUvSphere(f32 radius, u32 latitudes, u32 longitudes)
{
	constexpr u32 minLatitudes = 3;
	constexpr u32 minLongitudes = 2;
	latitudes = std::max(latitudes, minLatitudes);
	longitudes = std::max(longitudes, minLongitudes);

	std::vector<Vertex> vertices;
	std::vector<u32> indices;

	const f32 inverseRadius = 1.0f / radius;
	const f32 deltaLatitude = std::numbers::pi_v<float> / static_cast<float>(latitudes);
	const f32 deltaLongitude = 2.0f * std::numbers::pi_v<float> / static_cast<float>(longitudes);

	for (i32 i = 0; i <= static_cast<i32>(latitudes); i++)
	{
		const f32 floatI = static_cast<f32>(i);
		const f32 latitudeAngle = std::numbers::pi_v<float> / 2.0f - floatI * deltaLatitude;
		const float xy = radius * std::cos(latitudeAngle);
		const float z = radius * std::sin(latitudeAngle);

		for (i32 j = 0; j <= static_cast<i32>(longitudes); j++)
		{
			const f32 floatJ = static_cast<float>(j);
			const f32 longitudeAngle = floatJ * deltaLongitude;

			Vertex vertex{};
			vertex.position.x = xy * std::cos(longitudeAngle);
			vertex.position.y = xy * std::sin(longitudeAngle);
			vertex.position.z = z;

			vertex.texCoords.x = floatJ / static_cast<float>(longitudes);
			vertex.texCoords.y = floatI / static_cast<float>(latitudes);

			vertex.normal = vertex.position * inverseRadius;
			vertices.push_back(vertex);
		}
	}

	for (i32 i = 0; i < static_cast<i32>(latitudes); i++)
	{
		u32 k1 = i * (longitudes + 1);
		u32 k2 = k1 + longitudes + 1;

		for (i32 j = 0; j < static_cast<i32>(longitudes); ++j)
		{
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			if (i != (latitudes - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
			++k1;
			++k2;
		}
	}

	Mesh mesh;
	mesh.Init(std::move(vertices), std::move(indices));

	return mesh;
}
}// namespace stw
