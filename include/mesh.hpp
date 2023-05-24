#pragma once

#include <string>
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
	void Draw(Pipeline& pipeline);

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
