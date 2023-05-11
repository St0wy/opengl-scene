//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <GL/glew.h>
#include <array>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <string_view>

#include "pipeline.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "utils.hpp"

namespace stw
{

class TextureScene final : public Scene
{
public:
	static constexpr std::array Vertices{
	0.5f,
	0.5f,
	0.0f, // top right
	0.5f,
	-0.5f,
	0.0f, // bottom right
	-0.5f,
	-0.5f,
	0.0f, // bottom left
	-0.5f,
	0.5f,
	0.0f // top left
	};

	static constexpr std::array TexCoords{
	1.0f,
	1.0f,
	1.0f,
	0.0f,
	0.0f,
	0.0f,
	0.0f,
	1.0f,
	};

	static constexpr std::array<u32, 6> Indices{
	0,
	1,
	3,
	1,
	2,
	3,
	};

	void Begin() override
	{
		m_Pipeline.InitFromPath("shaders/texture.vert", "shaders/texture.frag");

		glGenVertexArrays(1, &m_Vao);
		glGenBuffers(1, &m_VboVertices);
		glGenBuffers(1, &m_VboTexCoords);
		glGenBuffers(1, &m_Ebo);
		glBindVertexArray(m_Vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_VboVertices);
		glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(f32), Vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), nullptr);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, m_VboTexCoords);
		glBufferData(GL_ARRAY_BUFFER, TexCoords.size() * sizeof(f32), TexCoords.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), nullptr);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(f32), Indices.data(), GL_STATIC_DRAW);

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

	void Update(const f32 deltaTime) override
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		m_Time += deltaTime * 2.0f;
		const f32 value = MapRange(std::cos(m_Time), -1.0f, 1.0f, 0.0f, 1.0f);

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
	f32 m_Time{};
};
} // namespace stw
