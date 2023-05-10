//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <array>
#include <string_view>
#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include "pipeline.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "utils.hpp"

namespace stw
{

class TextureScene final : public Scene
{
public:
	static constexpr std::array Vertices{ 0.5f, 0.5f, 0.0f,  // top right
										  0.5f, -0.5f, 0.0f,  // bottom right
										  -0.5f, -0.5f, 0.0f,  // bottom left
										  -0.5f, 0.5f, 0.0f   // top left
	};

	static constexpr std::array TexCoords{ 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, };

	static constexpr std::array<uint32_t, 6> Indices{ 0, 1, 3, 1, 2, 3, };

	void Begin() override
	{

		m_Pipeline.InitFromPath("shaders/texture.vert", "shaders/texture.frag");

		glGenVertexArrays(1, &m_Vao);
		glGenBuffers(1, &m_VboVertices);
		glGenBuffers(1, &m_VboTexCoords);
		glGenBuffers(1, &m_Ebo);
		glBindVertexArray(m_Vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_VboVertices);
		glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(float), Vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, m_VboTexCoords);
		glBufferData(GL_ARRAY_BUFFER, TexCoords.size() * sizeof(float), TexCoords.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(float), Indices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_BoxTexture.Init("./data/container.jpg", "texture2", 0, &m_Pipeline);
		m_FaceTexture.Init("./data/face.png", "texture1", 1, &m_Pipeline, GL_RGBA);
	}

	void End() override
	{
		glDeleteVertexArrays(1, &m_Vao);
		glDeleteBuffers(1, &m_VboVertices);
	}

	void Update(const float deltaTime) override
	{
		m_Time += deltaTime * 2.0f;
		const float value = MapRange(std::cos(m_Time), -1.0f, 1.0f, 0.0f, 1.0f);

		m_Pipeline.Use();

		m_BoxTexture.Bind(GL_TEXTURE0);
		m_FaceTexture.Bind(GL_TEXTURE1);
		m_Pipeline.SetFloat("value", value);

		glBindVertexArray(m_Vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		assert(!CHECK_GL_ERROR());
	}

private:
	GLuint m_Vao{};
	GLuint m_VboVertices{};
	GLuint m_VboTexCoords{};
	GLuint m_Ebo{};
	Texture m_BoxTexture;
	Texture m_FaceTexture;
	Pipeline m_Pipeline{};
	float m_Time{};
};
}
