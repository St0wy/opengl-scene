//
// Created by stowy on 04/05/2023.
//

#include "pipeline.hpp"

#include <spdlog/spdlog.h>

#include "utils.hpp"

constexpr std::size_t LOG_SIZE = 512;

void stw::Pipeline::Use()
{
	glUseProgram(m_ProgramId);
}

void stw::Pipeline::SetBool(std::string_view name, bool value) const
{
	glUniform1i(glGetUniformLocation(m_ProgramId, name.data()), static_cast<int>(value));
}

void stw::Pipeline::SetInt(std::string_view name, int value) const
{
	glUniform1i(glGetUniformLocation(m_ProgramId, name.data()), value);
}

void stw::Pipeline::SetFloat(std::string_view name, float value) const
{
	glUniform1f(glGetUniformLocation(m_ProgramId, name.data()), value);
}

stw::Pipeline::~Pipeline()
{
	// Check if the pipeline was initialized
	if (m_ProgramId == 0) return;

	glDeleteProgram(m_ProgramId);
}

void stw::Pipeline::InitFromPath(std::string_view vertexPath, std::string_view fragmentPath)
{
	std::string vertexSource = OpenFile(vertexPath);
	const char* vertexSourcePtr = vertexSource.c_str();
	m_VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(m_VertexShaderId, 1, &vertexSourcePtr, nullptr);
	glCompileShader(m_VertexShaderId);

	GLint success;
	glGetShaderiv(m_VertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[LOG_SIZE];
		glGetShaderInfoLog(m_VertexShaderId, LOG_SIZE, nullptr, infoLog);
		spdlog::error("Error while loading vertex shader. {}", infoLog);
	}

	std::string fragmentSource = OpenFile(fragmentPath);
	const char* fragmentSourcePtr = fragmentSource.c_str();
	m_FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(m_FragmentShaderId, 1, &fragmentSourcePtr, nullptr);
	glCompileShader(m_FragmentShaderId);

	glGetShaderiv(m_FragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[LOG_SIZE];
		glGetShaderInfoLog(m_FragmentShaderId, LOG_SIZE, nullptr, infoLog);
		spdlog::error("Error while loading fragment shader. {}", infoLog);
	}

	m_ProgramId = glCreateProgram();
	glAttachShader(m_ProgramId, m_VertexShaderId);
	glAttachShader(m_ProgramId, m_FragmentShaderId);
	glLinkProgram(m_ProgramId);

	glGetProgramiv(m_ProgramId, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[LOG_SIZE];
		glGetProgramInfoLog(m_ProgramId, LOG_SIZE, nullptr, infoLog);
		spdlog::error("Error while linking shader program. {}", infoLog);
	}

	glDeleteShader(m_FragmentShaderId);
	glDeleteShader(m_VertexShaderId);
}
