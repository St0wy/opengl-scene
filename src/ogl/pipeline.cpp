/**
 * @file pipeline.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Pipeline class.
 * @version 1.0
 * @date 09/11/2023
 *
 * @copyright SAE (c) 2023
 *
 */

// ReSharper disable CppMemberFunctionMayBeConst
module;

#include <array>
#include <filesystem>
#include <span>

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include "macros.hpp"

export module pipeline;

import consts;
import utils;
import number_types;

export namespace stw
{
constexpr std::size_t LogSize = 512;

class Pipeline
{
public:
	Pipeline() = default;
	Pipeline(const Pipeline& other) = delete;
	Pipeline(Pipeline&& other) = default;
	Pipeline& operator=(const Pipeline& other) = delete;
	Pipeline& operator=(Pipeline&& other) = default;
	~Pipeline();

	void InitFromPath(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
	void InitFromSource(std::string_view vertexSource, std::string_view fragmentSource);

	void Delete();

	[[nodiscard]] GLuint Id() const;

	void Bind();
	void UnBind();

	void SetBool(std::string_view name, bool value);
	void SetInt(std::string_view name, i32 value);
	void SetUnsignedInt(std::string_view name, u32 value);
	void SetFloat(std::string_view name, f32 value);
	void SetVec4(std::string_view name, const glm::vec4& value);
	void SetVec3(std::string_view name, const glm::vec3& value);
	void SetVec3V(std::string_view name, std::span<const glm::vec3> values);
	void SetVec2(std::string_view name, const glm::vec2& value);
	void SetMat3(std::string_view name, const glm::mat3& mat);
	void SetMat4(std::string_view name, const glm::mat4& mat);

	[[nodiscard]] usize GetTextureCount() const;

private:
	// TODO : Rework the cache to not use string views because the end up pointing to garbage memory...
	// std::unordered_map<std::string_view, GLint> m_UniformsLocation{};
	bool m_IsInitialized = false;

	GLuint m_ProgramId{};
	GLuint m_VertexShaderId{};
	GLuint m_FragmentShaderId{};
	usize m_TexturesCount{};

	[[nodiscard]] GLint GetUniformLocation(std::string_view name) const;
	[[nodiscard]] usize GetTextureCountFromOpenGl() const;
};

void Pipeline::Bind()
{
	ASSERT_MESSAGE(m_IsInitialized, "Pipeline should be initialized before using it.");
	glUseProgram(m_ProgramId);
}

void Pipeline::UnBind()
{
	ASSERT_MESSAGE(m_IsInitialized, "Pipeline should be initialized when unbinding.");
	glUseProgram(0);
}

void Pipeline::SetBool(const std::string_view name, const bool value)
{
	const auto location = GetUniformLocation(name);
	glUniform1i(location, static_cast<int>(value));
}

void Pipeline::SetInt(const std::string_view name, const int value)
{
	const auto location = GetUniformLocation(name);
	glUniform1i(location, value);
}

void Pipeline::SetUnsignedInt(const std::string_view name, const u32 value)
{
	const auto location = GetUniformLocation(name);
	glUniform1ui(location, value);
}

void Pipeline::SetFloat(const std::string_view name, const float value)
{
	const auto location = GetUniformLocation(name);
	glUniform1f(location, value);
}

void Pipeline::SetVec3(const std::string_view name, const glm::vec3& value)
{
	const auto location = GetUniformLocation(name);
	glUniform3f(location, value.x, value.y, value.z);
}

void Pipeline::SetMat3(const std::string_view name, const glm::mat3& mat)
{
	const auto location = GetUniformLocation(name);
	glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
}

void Pipeline::SetMat4(const std::string_view name, const glm::mat4& mat)
{
	const auto location = GetUniformLocation(name);
	glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
}

GLint Pipeline::GetUniformLocation(const std::string_view name) const
{
	const auto location = glGetUniformLocation(m_ProgramId, name.data());
	if (location == -1)
	{
		spdlog::warn("Uniform \"{}\" does not exist.", name);
	}

	return location;
}

Pipeline::~Pipeline()
{
	if (m_ProgramId != 0)
	{
		spdlog::error("Destructor called on a pipeline that still has an ID.");
	}
}

void Pipeline::InitFromPath(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
{
	// TODO : Cache already opened shaders
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

void Pipeline::InitFromSource(const std::string_view vertexSource, const std::string_view fragmentSource)
{
	const char* vertexSourcePtr = vertexSource.data();
	m_VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(m_VertexShaderId, 1, &vertexSourcePtr, nullptr);
	glCompileShader(m_VertexShaderId);

	GLint success = 0;
	glGetShaderiv(m_VertexShaderId, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		std::array<char, LogSize> infoLog{};
		glGetShaderInfoLog(m_VertexShaderId, LogSize, nullptr, infoLog.data());
		spdlog::error("Error while loading vertex shader.\n{}", infoLog.data());
		return;
	}

	const char* fragmentSourcePtr = fragmentSource.data();
	m_FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(m_FragmentShaderId, 1, &fragmentSourcePtr, nullptr);
	glCompileShader(m_FragmentShaderId);

	glGetShaderiv(m_FragmentShaderId, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		std::array<char, LogSize> infoLog{};
		glGetShaderInfoLog(m_FragmentShaderId, LogSize, nullptr, infoLog.data());
		spdlog::error("Error while loading fragment shader.\n{}", infoLog.data());
		return;
	}

	m_ProgramId = glCreateProgram();
	glAttachShader(m_ProgramId, m_VertexShaderId);
	glAttachShader(m_ProgramId, m_FragmentShaderId);
	glLinkProgram(m_ProgramId);

	glGetProgramiv(m_ProgramId, GL_LINK_STATUS, &success);
	if (success == 0)
	{
		std::array<char, LogSize> infoLog{};
		glGetProgramInfoLog(m_ProgramId, LogSize, nullptr, infoLog.data());
		spdlog::error("Error while linking shader program.\n{}", infoLog.data());
		glDeleteShader(m_FragmentShaderId);
		glDeleteShader(m_VertexShaderId);
		return;
	}

	glDeleteShader(m_FragmentShaderId);
	glDeleteShader(m_VertexShaderId);

	m_IsInitialized = true;

	m_TexturesCount = GetTextureCountFromOpenGl();
}

void Pipeline::Delete()
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

GLuint Pipeline::Id() const { return m_ProgramId; }

usize Pipeline::GetTextureCountFromOpenGl() const
{
	usize textureCount = 0;
	GLint numActiveUniforms = 0;
	glGetProgramInterfaceiv(m_ProgramId, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numActiveUniforms);

	std::vector<GLenum> properties;
	properties.push_back(GL_NAME_LENGTH);
	properties.push_back(GL_TYPE);
	properties.push_back(GL_ARRAY_SIZE);
	std::vector<GLint> values(properties.size());
	for (int attrib = 0; attrib < numActiveUniforms; ++attrib)
	{
		glGetProgramResourceiv(m_ProgramId,
			GL_UNIFORM,
			attrib,
			static_cast<GLsizei>(properties.size()),
			properties.data(),
			static_cast<GLsizei>(values.size()),
			nullptr,
			values.data());

		if (values[1] == GL_SAMPLER_2D)
		{
			textureCount++;
		}
	}

	return textureCount;
}

usize Pipeline::GetTextureCount() const { return m_TexturesCount; }

void Pipeline::SetVec2(const std::string_view name, const glm::vec2& value)
{
	const auto location = GetUniformLocation(name);

	glUniform2f(location, value.x, value.y);
}

void Pipeline::SetVec4(const std::string_view name, const glm::vec4& value)
{
	const auto location = GetUniformLocation(name);

	glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Pipeline::SetVec3V(const std::string_view name, const std::span<const glm::vec3> values)
{
	const auto location = GetUniformLocation(name);

	glUniform3fv(location, static_cast<GLsizei>(values.size()), value_ptr(values[0]));
}
}// namespace stw
