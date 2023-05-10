//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <string_view>
#include <string>
#include <array>
#include <GL/glew.h>
#include <spdlog/spdlog.h>
#include <fmt/core.h>

#include "scene.hpp"
#include "utils.hpp"
#include "camera.hpp"

namespace stw
{

class SquareScene : public stw::Scene
{
public:
	static constexpr std::array VERTICES{
		0.5f, 0.5f, 0.0f,  // top right
		0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f, 0.5f, 0.0f   // top left
	};

	static constexpr std::array COLORS{
		1.0f, 0.5f, 0.0f,  // top right
		0.5f, 1.0f, 0.0f,  // bottom right
		1.0f, 0.0f, 0.0f,  // bottom left
		0.0f, 1.0f, 0.0f   // top left
	};

	static constexpr std::array<uint32_t, 6> INDICES = {
		0, 1, 3,
		1, 2, 3,
	};

	static constexpr std::size_t LOG_SIZE = 512;

	void LoadShaders()
	{
		std::string vertexSource = OpenFile("shaders/uniform.vert");
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

		std::string fragmentSource = OpenFile("shaders/uniform.frag");
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
		glGenBuffers(1, &m_VboColor);
		glGenBuffers(1, &m_Ebo);
		glBindVertexArray(m_Vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_VboVertices);
		glBufferData(GL_ARRAY_BUFFER, VERTICES.size() * sizeof(float), VERTICES.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, m_VboColor);
		glBufferData(GL_ARRAY_BUFFER, COLORS.size() * sizeof(float), COLORS.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDICES.size() * sizeof(float), INDICES.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
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

		glUseProgram(m_ShaderProgram);
		glUniform1f(0, value);
		glBindVertexArray(m_Vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		if (CHECK_GL_ERROR())
		{
#ifdef DEBUG
			assertm(false, "OpenGL errors found")
#endif
		}
	}

private:
	GLuint m_VertexShader{};
	GLuint m_FragmentShader{};
	GLuint m_ShaderProgram{};
	GLuint m_Vao{};
	GLuint m_VboVertices{};
	GLuint m_VboColor{};
	GLuint m_Ebo{};
	float time;
	Camera m_Camera{};
};
}
