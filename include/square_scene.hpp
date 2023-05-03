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
	std::array<float, 9> VERTICES = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f, 0.5f, 0.0f,
	};

	void Begin() override
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
			spdlog::error("Error while loading vertex shader");
		}

		std::string fragmentSource = OpenFile("shaders/uniform.frag");
		const char* fragmentSourcePtr = fragmentSource.c_str();
		m_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(m_FragmentShader, 1, &fragmentSourcePtr, nullptr);
		glCompileShader(m_FragmentShader);

		glGetShaderiv(m_FragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			spdlog::error("Error while loading fragment shader");
		}

		m_ShaderProgram = glCreateProgram();
		glAttachShader(m_ShaderProgram, m_VertexShader);
		glAttachShader(m_ShaderProgram, m_FragmentShader);
		glLinkProgram(m_ShaderProgram);

		glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			spdlog::error("Error while linking shader program");
		}

		glGenVertexArrays(1, &m_Vao);
		glBindVertexArray(m_Vao);

		glGenBuffers(1, &m_Vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);

		glBufferData(GL_ARRAY_BUFFER, VERTICES.size(), VERTICES.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, VERTICES.size(), GL_FLOAT, GL_FALSE, VERTICES.size() * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

	}

	void End() override
	{
		glDeleteShader(m_FragmentShader);
		glDeleteShader(m_VertexShader);
		glDeleteProgram(m_ShaderProgram);
		glDeleteVertexArrays(1, &m_Vao);
	}

	void Update(float dt) override
	{
		glUseProgram(m_ShaderProgram);
		glBindVertexArray(m_Vao);
		// TODO: Regarder pourquoi le draw array fait tout crash
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
