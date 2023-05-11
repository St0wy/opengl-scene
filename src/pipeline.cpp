//
// Created by stowy on 04/05/2023.
//

#include "pipeline.hpp"

#include <glm/matrix.hpp>
#include <spdlog/spdlog.h>

#include "utils.hpp"

constexpr std::size_t LogSize = 512;

void stw::Pipeline::Use() const
{
	glUseProgram(m_ProgramId);
}

void stw::Pipeline::SetBool(const std::string_view name, const bool value) const
{
	glUniform1i(glGetUniformLocation(m_ProgramId, name.data()), static_cast<int>(value));
}

void stw::Pipeline::SetInt(const std::string_view name, const int value) const
{
	glUniform1i(glGetUniformLocation(m_ProgramId, name.data()), value);
}

void stw::Pipeline::SetFloat(const std::string_view name, const float value) const
{
	glUniform1f(glGetUniformLocation(m_ProgramId, name.data()), value);
}

void stw::Pipeline::SetVec3(const std::string_view name, glm::vec3 value) const
{
	glUniform3fv(glGetUniformLocation(m_ProgramId, name.data()), 1, &value[0]);
}

void stw::Pipeline::SetMat3(const std::string_view name, const glm::mat3& mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(m_ProgramId, name.data()), 1, GL_FALSE, &mat[0][0]);
}

void stw::Pipeline::SetMat4(const std::string_view name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(m_ProgramId, name.data()), 1, GL_FALSE, &mat[0][0]);
}

stw::Pipeline::~Pipeline()
{
	// Check if the pipeline was initialized
	if (m_ProgramId == 0)
		return;

	glDeleteProgram(m_ProgramId);
}

void stw::Pipeline::InitFromPath(const std::string_view vertexPath, const std::string_view fragmentPath)
{
	const std::string vertexSource = OpenFile(vertexPath);
	const char* vertexSourcePtr = vertexSource.c_str();
	m_VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(m_VertexShaderId, 1, &vertexSourcePtr, nullptr);
	glCompileShader(m_VertexShaderId);

	GLint success;
	glGetShaderiv(m_VertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[LogSize];
		glGetShaderInfoLog(m_VertexShaderId, LogSize, nullptr, infoLog);
		spdlog::error("Error while loading vertex shader. {}", infoLog);
	}

	const std::string fragmentSource = OpenFile(fragmentPath);
	const char* fragmentSourcePtr = fragmentSource.c_str();
	m_FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(m_FragmentShaderId, 1, &fragmentSourcePtr, nullptr);
	glCompileShader(m_FragmentShaderId);

	glGetShaderiv(m_FragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[LogSize];
		glGetShaderInfoLog(m_FragmentShaderId, LogSize, nullptr, infoLog);
		spdlog::error("Error while loading fragment shader. {}", infoLog);
	}

	m_ProgramId = glCreateProgram();
	glAttachShader(m_ProgramId, m_VertexShaderId);
	glAttachShader(m_ProgramId, m_FragmentShaderId);
	glLinkProgram(m_ProgramId);

	glGetProgramiv(m_ProgramId, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[LogSize];
		glGetProgramInfoLog(m_ProgramId, LogSize, nullptr, infoLog);
		spdlog::error("Error while linking shader program. {}", infoLog);
	}

	glDeleteShader(m_FragmentShaderId);
	glDeleteShader(m_VertexShaderId);
}
