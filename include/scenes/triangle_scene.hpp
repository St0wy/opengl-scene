//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <string_view>
#include <string>
#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include "scene.hpp"
#include "utils.hpp"

namespace stw
{

class TriangleScene : public stw::Scene
{
public:
	void Begin() override
	{
		std::string vertexSource = OpenFile("shaders/triangle.vert");
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

		std::string fragmentSource = OpenFile("shaders/triangle.frag");
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

		glCreateVertexArrays(1, &m_Vao);
	}

	void End() override
	{
		glDeleteShader(m_FragmentShader);
		glDeleteShader(m_VertexShader);
		glDeleteProgram(m_ShaderProgram);
		glDeleteVertexArrays(1, &m_Vao);
	}

	void Update(f32 dt) override
	{
		glUseProgram(m_ShaderProgram);
		glBindVertexArray(m_Vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void OnResize(i32 windowWidth, i32 windowHeight) override
	{
		glViewport(0, 0, windowWidth, windowHeight);
	}

private:
	GLuint m_VertexShader{};
	GLuint m_FragmentShader{};
	GLuint m_ShaderProgram{};
	GLuint m_Vao{};
};
}
