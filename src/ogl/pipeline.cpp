//
// Created by stowy on 04/05/2023.
//

#include "ogl/pipeline.hpp"

#include <glm/matrix.hpp>
#include <spdlog/spdlog.h>

#include "shader.hpp"
#include "utils.hpp"

constexpr std::size_t LogSize = 512;

void stw::Pipeline::Use() const
{
	ASSERT_MESSAGE(m_IsInitialized, "Pipeline should be initialized before using it.");
	glUseProgram(m_ProgramId);
}

void stw::Pipeline::SetBool(const std::string_view name, const bool value) const
{
	const auto location = glGetUniformLocation(m_ProgramId, name.data());
	if (location == -1)
	{
		spdlog::warn("Uniform \"{}\" does not exist.", name);
	}
	else
	{
		glUniform1i(location, static_cast<int>(value));
	}
}

void stw::Pipeline::SetInt(const std::string_view name, const int value) const
{
	const auto location = glGetUniformLocation(m_ProgramId, name.data());
	if (location == -1)
	{
		spdlog::warn("Uniform \"{}\" does not exist.", name);
	}
	else
	{
		glUniform1i(location, value);
	}
}

void stw::Pipeline::SetUnsignedInt(const std::string_view name, const u32 value) const
{
	const auto location = glGetUniformLocation(m_ProgramId, name.data());
	if (location == -1)
	{
		spdlog::warn("Uniform \"{}\" does not exist.", name);
	}
	else
	{
		glUniform1ui(location, value);
	}
}

void stw::Pipeline::SetFloat(const std::string_view name, const float value) const
{
	const auto location = glGetUniformLocation(m_ProgramId, name.data());
	if (location == -1)
	{
		spdlog::warn("Uniform \"{}\" does not exist.", name);
	}
	else
	{
		glUniform1f(location, value);
	}
}

void stw::Pipeline::SetVec3(const std::string_view name, glm::vec3 value) const
{
	const auto location = glGetUniformLocation(m_ProgramId, name.data());
	if (location == -1)
	{
		spdlog::warn("Uniform \"{}\" does not exist.", name);
	}
	else
	{
		glUniform3fv(location, 1, &value[0]);
	}
}

void stw::Pipeline::SetMat3(const std::string_view name, const glm::mat3& mat) const
{
	const auto location = glGetUniformLocation(m_ProgramId, name.data());
	if (location == -1)
	{
		spdlog::warn("Uniform \"{}\" does not exist.", name);
	}
	else
	{
		glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
	}
}

void stw::Pipeline::SetMat4(const std::string_view name, const glm::mat4& mat) const
{
	const auto location = glGetUniformLocation(m_ProgramId, name.data());
	if (location == -1)
	{
		spdlog::warn("Uniform \"{}\" does not exist.", name);
	}
	else
	{
		glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
	}
}

void stw::Pipeline::SetPointLightsCount(const u32 count)
{
	ASSERT_MESSAGE(count <= MaxPointLights, "Count is above max lights.");
	SetUnsignedInt("pointLightsCount", count);
	m_PointLightsCount = count;
}

void stw::Pipeline::SetDirectionalLightsCount(const u32 count)
{
	ASSERT_MESSAGE(count <= MaxDirectionalLights, "Count is above max lights.");
	SetUnsignedInt("directionalLightsCount", count);
	m_DirectionalLightsCount = count;
}

void stw::Pipeline::SetSpotLightsCount(const u32 count)
{
	ASSERT_MESSAGE(count <= MaxSpotLights, "Count is above max lights.");
	SetUnsignedInt("spotLightsCount", count);
	m_SpotLightsCount = count;
}

void stw::Pipeline::SetPointLight(std::string_view name,
	u32 index,
	const PointLight& pointLight,
	const glm::mat4& view) const
{
	ASSERT_MESSAGE(index < m_PointLightsCount, "Index should be bellow the light count.");

	const auto indexedName = fmt::format("{}[{}]", name, index);

	const auto viewSpaceLightPosition = glm::vec3(view * glm::vec4(pointLight.position, 1.0f));
	SetVec3(fmt::format("{}.position", indexedName), viewSpaceLightPosition);

	SetVec3(fmt::format("{}.ambient", indexedName), pointLight.ambient);
	SetVec3(fmt::format("{}.diffuse", indexedName), pointLight.diffuse);
	SetVec3(fmt::format("{}.specular", indexedName), pointLight.specular);

	SetFloat(fmt::format("{}.constant", indexedName), pointLight.constant);
	SetFloat(fmt::format("{}.linear", indexedName), pointLight.linear);
	SetFloat(fmt::format("{}.quadratic", indexedName), pointLight.quadratic);
}

void stw::Pipeline::SetDirectionalLight(std::string_view name,
	u32 index,
	const DirectionalLight& directionalLight) const
{
	ASSERT_MESSAGE(index < m_DirectionalLightsCount, "Index should be bellow the light count.");

	const auto indexedName = fmt::format("{}[{}]", name, index);
	SetVec3(fmt::format("{}.direction", indexedName), directionalLight.direction);
	SetVec3(fmt::format("{}.ambient", indexedName), directionalLight.ambient);
	SetVec3(fmt::format("{}.diffuse", indexedName), directionalLight.diffuse);
	SetVec3(fmt::format("{}.specular", indexedName), directionalLight.specular);
}

void stw::Pipeline::SetSpotLight(std::string_view name,
	u32 index,
	const SpotLight& spotLight,
	const glm::mat4& view) const
{
	ASSERT_MESSAGE(index < m_SpotLightsCount, "Index should be bellow the light count.");

	const auto indexedName = fmt::format("{}[{}]", name, index);

	const auto viewSpacePosition = glm::vec3(view * glm::vec4(spotLight.position, 1.0f));
	SetVec3(fmt::format("{}.position", indexedName), viewSpacePosition);
	const auto viewSpaceDirection = glm::vec3(view * glm::vec4(spotLight.direction, 1.0f));
	SetVec3(fmt::format("{}.direction", indexedName), viewSpaceDirection);
	SetVec3(fmt::format("{}.ambient", indexedName), spotLight.ambient);
	SetVec3(fmt::format("{}.diffuse", indexedName), spotLight.diffuse);
	SetVec3(fmt::format("{}.specular", indexedName), spotLight.specular);
	SetFloat(fmt::format("{}.constant", indexedName), spotLight.constant);
	SetFloat(fmt::format("{}.linear", indexedName), spotLight.linear);
	SetFloat(fmt::format("{}.quadratic", indexedName), spotLight.quadratic);
	SetFloat(fmt::format("{}.cutOff", indexedName), spotLight.cutOff);
	SetFloat(fmt::format("{}.outerCutOff", indexedName), spotLight.outerCutOff);
}

stw::Pipeline::~Pipeline()
{
	if (m_ProgramId != 0)
	{
		spdlog::warn("Destructor called on a pipeline that still has an ID.");
	}
}

void stw::Pipeline::InitFromPath(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
{
	const auto vertexResult = OpenFile(vertexPath);

	if (!vertexResult.has_value())
	{
		spdlog::error("Could not load vertex shader file {}", vertexPath.string());
		return;
	}

	const auto fragmentResult = OpenFile(fragmentPath);

	if (!fragmentResult.has_value())
	{
		spdlog::error("Could not load fragment shader file {}", fragmentPath.string());
		return;
	}

	InitFromSource(vertexResult.value(), fragmentResult.value());
}

void stw::Pipeline::InitFromPathSingleFile(const std::filesystem::path& shaderFile)
{
	const auto [vertex, fragment] = ShaderProgramSource::LoadFromFile(shaderFile);
	if (!vertex.has_value())
	{
		spdlog::error("Could not load vertex from shader file {}", shaderFile.string());
		return;
	}


	if (!fragment.has_value())
	{
		spdlog::error("Could not load fragment from shader file {}", shaderFile.string());
		return;
	}

	InitFromSource(vertex.value(), fragment.value());
}

void stw::Pipeline::InitFromSource(const std::string_view vertexSource, const std::string_view fragmentSource)
{
	const char* vertexSourcePtr = vertexSource.data();
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
		return;
	}

	const char* fragmentSourcePtr = fragmentSource.data();
	m_FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(m_FragmentShaderId, 1, &fragmentSourcePtr, nullptr);
	glCompileShader(m_FragmentShaderId);

	glGetShaderiv(m_FragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[LogSize];
		glGetShaderInfoLog(m_FragmentShaderId, LogSize, nullptr, infoLog);
		spdlog::error("Error while loading fragment shader. {}", infoLog);
		return;
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
		return;
	}

	glDeleteShader(m_FragmentShaderId);
	glDeleteShader(m_VertexShaderId);

	m_IsInitialized = true;
}

void stw::Pipeline::Delete()
{
	// Check if the pipeline was initialized
	if (m_ProgramId == 0)
	{
		spdlog::warn("Deleting program that has not a valid id.");
		return;
	}

	glDeleteProgram(m_ProgramId);
	m_ProgramId = 0;
}

GLuint stw::Pipeline::Id() const
{
	return m_ProgramId;
}