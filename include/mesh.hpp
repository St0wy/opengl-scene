#pragma once

#include <vector>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>

#include "index_buffer.hpp"
#include "number_types.hpp"
#include "pipeline.hpp"
#include "texture.hpp"
#include "vertex_buffer.hpp"

namespace stw
{
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

class Mesh
{
public:
	Mesh() = default;
	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) noexcept;

	~Mesh();

	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) noexcept = delete;

	void Init(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<Texture> textures);
	void Delete();

	[[nodiscard]] GLuint Vao() const;

	void Draw(const Pipeline& pipeline) const;
	void DrawNoSpecular(const Pipeline& pipeline) const;
	void DrawInstanced(const Pipeline& pipeline, GLsizei count) const;
	void DrawNoSpecularInstanced(const Pipeline& pipeline, GLsizei count) const;
	void DrawMeshOnly() const;
	void DrawMeshOnlyInstanced(GLsizei count) const;

private:
	std::vector<Vertex> m_Vertices;
	std::vector<u32> m_Indices;
	std::vector<Texture> m_Textures;

	GLuint m_Vao{};

	VertexBuffer<Vertex> m_VertexBuffer{};
	IndexBuffer m_IndexBuffer{};

	void SetupMesh();
};
}
