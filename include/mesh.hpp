/**
 * @file mesh.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains Mesh class.
 * @version 1.0
 * @date 07/11/2023
 *
 * @copyright SAE (c) 2023
 *
 */

#pragma once

#include <span>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>

#include "number_types.hpp"
#include "ogl/index_buffer.hpp"
#include "ogl/vertex_array.hpp"
#include "ogl/vertex_buffer.hpp"

namespace stw
{
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 tangent;
};

class Mesh
{
public:
	Mesh() = default;
	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) noexcept;
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
}// namespace stw
