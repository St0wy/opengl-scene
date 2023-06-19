#pragma once

#include <vector>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>

#include "number_types.hpp"
#include "texture.hpp"
#include "ogl/index_buffer.hpp"
#include "ogl/pipeline.hpp"
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

	void Init(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<Texture> textures);
	void Delete();

	void Draw(Pipeline& pipeline, const glm::mat4&) const;
	void DrawNoSpecular(Pipeline& pipeline, const glm::mat4& modelMatrix) const;

	void DrawInstanced(Pipeline& pipeline, std::span<const glm::mat4> modelMatrices) const;
	void DrawNoSpecularInstanced(Pipeline& pipeline, std::span<const glm::mat4> modelMatrices) const;
	void DrawMeshOnlyInstanced(std::span<const glm::mat4> modelMatrices) const;

private:
	std::vector<Vertex> m_Vertices{};
	std::vector<u32> m_Indices{};
	std::vector<Texture> m_Textures{};

	VertexArray m_VertexArray{};
	VertexBuffer<Vertex> m_VertexBuffer{};
	VertexBuffer<glm::mat4> m_ModelMatrixBuffer{};
	IndexBuffer m_IndexBuffer{};

	bool m_IsInitialized = false;

	void SetupMesh();
};
}
