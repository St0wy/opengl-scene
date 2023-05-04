//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <string_view>
#include <string>
#include <array>
#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include "scene.hpp"
#include "utils.hpp"

namespace stw
{

class SquareScene : public stw::Scene
{
public:
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

		constexpr std::array<float, 9> VERTICES = {
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
			0.0f, 0.5f, 0.0f,
		};

		glGenVertexArrays(1, &m_Vao);
		glGenBuffers(1, &m_Vbo);
		glBindVertexArray(m_Vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(VERTICES.size()), VERTICES.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void End() override
	{
		glDeleteVertexArrays(1, &m_Vao);
		glDeleteBuffers(1, &m_Vbo);
		glDeleteProgram(m_ShaderProgram);
	}

	void Update(float dt) override
	{
		glUseProgram(m_ShaderProgram);
		glBindVertexArray(m_Vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

private:
	GLuint m_VertexShader{};
	GLuint m_FragmentShader{};
	GLuint m_ShaderProgram{};
	GLuint m_Vao{};
	GLuint m_Vbo{};
};
}
