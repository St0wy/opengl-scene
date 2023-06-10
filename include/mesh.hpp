#pragma once

#include <vector>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>

#include "number_types.hpp"
#include "pipeline.hpp"
#include "texture.hpp"

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
	Mesh(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<Texture> textures);
	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) noexcept;

	~Mesh();

	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) noexcept = delete;

	[[nodiscard]] GLuint Vao() const;

	void Draw(const Pipeline& pipeline) const;
	void DrawNoSpecular(const Pipeline& pipeline) const;
	void DrawInstanced(const Pipeline& pipeline, GLsizei count) const;
	void DrawNoSpecularInstanced(const Pipeline& pipeline, GLsizei count) const;
	void DrawMeshOnly(const Pipeline& pipeline) const;
	void DrawMeshOnlyInstanced(const Pipeline& pipeline, GLsizei count) const;

private:
	std::vector<Vertex> m_Vertices;
	std::vector<u32> m_Indices;
	std::vector<Texture> m_Textures;

	GLuint m_Vao{};
	GLuint m_Vbo{};
	GLuint m_Ebo{};

	void SetupMesh();
};
}
