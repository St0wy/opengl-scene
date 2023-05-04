//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <string_view>
#include <string>
#include <array>
#include <GL/glew.h>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "scene.hpp"
#include "utils.hpp"

namespace stw
{

class TextureScene : public stw::Scene
{
public:
	static constexpr std::array VERTICES{
		0.5f, 0.5f, 0.0f,  // top right
		0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f, 0.5f, 0.0f   // top left
	};

	static constexpr std::array TEX_COORDS{
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
	};

	static constexpr std::array<uint32_t, 6> INDICES{
		0, 1, 3,
		1, 2, 3,
	};

	static constexpr std::size_t LOG_SIZE = 512;

	void LoadShaders()
	{
		std::string vertexSource = OpenFile("shaders/texture.vert");
		const char* vertexSourcePtr = vertexSource.c_str();
		m_VertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(m_VertexShader, 1, &vertexSourcePtr, nullptr);
		glCompileShader(m_VertexShader);

		GLint success;
		glGetShaderiv(m_VertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			char infoLog[LOG_SIZE];
			glGetShaderInfoLog(m_VertexShader, LOG_SIZE, nullptr, infoLog);
			spdlog::error("Error while loading vertex shader. {}", infoLog);
		}

		std::string fragmentSource = OpenFile("shaders/texture.frag");
		const char* fragmentSourcePtr = fragmentSource.c_str();
		m_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(m_FragmentShader, 1, &fragmentSourcePtr, nullptr);
		glCompileShader(m_FragmentShader);

		glGetShaderiv(m_FragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			char infoLog[LOG_SIZE];
			glGetShaderInfoLog(m_FragmentShader, LOG_SIZE, nullptr, infoLog);
			spdlog::error("Error while loading fragment shader. {}", infoLog);
		}

		m_ShaderProgram = glCreateProgram();
		glAttachShader(m_ShaderProgram, m_VertexShader);
		glAttachShader(m_ShaderProgram, m_FragmentShader);
		glLinkProgram(m_ShaderProgram);

		glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			char infoLog[LOG_SIZE];
			glGetProgramInfoLog(m_ShaderProgram, LOG_SIZE, nullptr, infoLog);
			spdlog::error("Error while linking shader program. {}", infoLog);
		}

		glDeleteShader(m_FragmentShader);
		glDeleteShader(m_VertexShader);
	}

	void Begin() override
	{
		LoadShaders();

		glGenVertexArrays(1, &m_Vao);
		glGenBuffers(1, &m_VboVertices);
		glGenBuffers(1, &m_VboTexCoords);
		glGenBuffers(1, &m_Ebo);
		glBindVertexArray(m_Vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_VboVertices);
		glBufferData(GL_ARRAY_BUFFER, VERTICES.size() * sizeof(float), VERTICES.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, m_VboTexCoords);
		glBufferData(GL_ARRAY_BUFFER, TEX_COORDS.size() * sizeof(float), TEX_COORDS.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDICES.size() * sizeof(float), INDICES.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);


		// Handle texture
		glGenTextures(1, &m_Texture);
		glBindTexture(GL_TEXTURE_2D, m_Texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int32_t width, height, nbrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load("./data/container.jpg", &width, &height, &nbrChannels, 0);

		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			spdlog::error("Failed to load texture ");
		}
		stbi_image_free(data);

		glUseProgram(m_ShaderProgram);
		glUniform1i(glGetUniformLocation(m_ShaderProgram, "ourTexture"), 0);
	}

	void End() override
	{
		glDeleteVertexArrays(1, &m_Vao);
		glDeleteBuffers(1, &m_VboVertices);
		glDeleteProgram(m_ShaderProgram);
	}

	void Update(float deltaTime) override
	{
		time += deltaTime * 2.0f;
		float value = MapRange(std::cos(time), -1.0f, 1.0f, 0.0f, 1.0f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_Texture);

		glUseProgram(m_ShaderProgram);
		glUniform1f(glGetUniformLocation(m_ShaderProgram, "value"), value);

		glBindVertexArray(m_Vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		assert(!CHECK_GL_ERROR());
	}

private:
	GLuint m_VertexShader{};
	GLuint m_FragmentShader{};
	GLuint m_ShaderProgram{};
	GLuint m_Vao{};
	GLuint m_VboVertices{};
	GLuint m_VboTexCoords{};
	GLuint m_Ebo{};
	GLuint m_Texture{};
	float time;
};
}
