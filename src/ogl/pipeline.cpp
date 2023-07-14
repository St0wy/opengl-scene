//
// Created by stowy on 04/05/2023.
//

#include "ogl/pipeline.hpp"

#include <array>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
#include <spdlog/spdlog.h>

#include "ogl/shader.hpp"
#include "utils.hpp"

constexpr std::size_t LogSize = 512;

void stw::Pipeline::Bind() const
{
	ASSERT_MESSAGE(m_IsInitialized, "Pipeline should be initialized before using it.");
	GLCALL(glUseProgram(m_ProgramId));
}

void stw::Pipeline::UnBind() const
{
	ASSERT_MESSAGE(m_IsInitialized, "Pipeline should be initialized when unbinding.");
	GLCALL(glUseProgram(0));
}

void stw::Pipeline::SetBool(const std::string_view name, const bool value)
{
	const auto location = GetUniformLocation(name);
	GLCALL(glUniform1i(location, static_cast<int>(value)));
}

void stw::Pipeline::SetInt(const std::string_view name, const int value)
{
	const auto location = GetUniformLocation(name);
	GLCALL(glUniform1i(location, value));
}

void stw::Pipeline::SetUnsignedInt(const std::string_view name, const u32 value)
{
	const auto location = GetUniformLocation(name);
	GLCALL(glUniform1ui(location, value));
}

void stw::Pipeline::SetFloat(const std::string_view name, const float value)
{
	const auto location = GetUniformLocation(name);
	GLCALL(glUniform1f(location, value));
}

void stw::Pipeline::SetVec3(const std::string_view name, const glm::vec3 value)
{
	const auto location = GetUniformLocation(name);
	GLCALL(glUniform3f(location, value.x, value.y, value.z));
}

void stw::Pipeline::SetMat3(const std::string_view name, const glm::mat3& mat)
{
	const auto location = GetUniformLocation(name);
	GLCALL(glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]));
}

void stw::Pipeline::SetMat4(const std::string_view name, const glm::mat4& mat)
{
	const auto location = GetUniformLocation(name);
	GLCALL(glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]));
}

GLint stw::Pipeline::GetUniformLocation(const std::string_view name)
{
	if (const auto search = m_UniformsLocation.find(name); search != m_UniformsLocation.end())
	{
		return search->second;
	}

	GLCALL(const auto location = glGetUniformLocation(m_ProgramId, name.data()));
	if (location == -1)
	{
		spdlog::warn("Uniform \"{}\" does not exist.", name);
	}

	m_UniformsLocation[name] = location;
	return location;
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
	GLCALL(glShaderSource(m_VertexShaderId, 1, &vertexSourcePtr, nullptr));
	GLCALL(glCompileShader(m_VertexShaderId));

	GLint success = 0;
	GLCALL(glGetShaderiv(m_VertexShaderId, GL_COMPILE_STATUS, &success));
	if (!success)
	{
		std::array<char, LogSize> infoLog{};
		glGetShaderInfoLog(m_VertexShaderId, LogSize, nullptr, infoLog.data());
		spdlog::error("Error while loading vertex shader.\n{}", infoLog.data());
		return;
	}

	const char* fragmentSourcePtr = fragmentSource.data();
	m_FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	GLCALL(glShaderSource(m_FragmentShaderId, 1, &fragmentSourcePtr, nullptr));
	GLCALL(glCompileShader(m_FragmentShaderId));

	GLCALL(glGetShaderiv(m_FragmentShaderId, GL_COMPILE_STATUS, &success));
	if (!success)
	{
		std::array<char, LogSize> infoLog{};
		GLCALL(glGetShaderInfoLog(m_FragmentShaderId, LogSize, nullptr, infoLog.data()));
		spdlog::error("Error while loading fragment shader.\n{}", infoLog.data());
		return;
	}

	m_ProgramId = glCreateProgram();
	GLCALL(glAttachShader(m_ProgramId, m_VertexShaderId));
	GLCALL(glAttachShader(m_ProgramId, m_FragmentShaderId));
	GLCALL(glLinkProgram(m_ProgramId));

	GLCALL(glGetProgramiv(m_ProgramId, GL_LINK_STATUS, &success));
	if (!success)
	{
		std::array<char, LogSize> infoLog{};
		GLCALL(glGetProgramInfoLog(m_ProgramId, LogSize, nullptr, infoLog.data()));
		spdlog::error("Error while linking shader program.\n{}", infoLog.data());
		GLCALL(glDeleteShader(m_FragmentShaderId));
		GLCALL(glDeleteShader(m_VertexShaderId));
		return;
	}

	GLCALL(glDeleteShader(m_FragmentShaderId));
	GLCALL(glDeleteShader(m_VertexShaderId));

	m_IsInitialized = true;

	m_TexturesCount = GetTextureCountFromOpengl();
}

void stw::Pipeline::Delete()
{
	// Check if the pipeline was initialized
	if (m_ProgramId == 0)
	{
		spdlog::warn("Deleting program that has not a valid id.");
		return;
	}

	GLCALL(glDeleteProgram(m_ProgramId));
	m_ProgramId = 0;
}

GLuint stw::Pipeline::Id() const { return m_ProgramId; }

usize stw::Pipeline::GetTextureCountFromOpengl() const
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
			&properties[0],
			static_cast<GLsizei>(values.size()),
			nullptr,
			&values[0]);

		if (values[1] == GL_SAMPLER_2D)
		{
			textureCount++;
		}
	}

	return textureCount;
}

usize stw::Pipeline::GetTextureCount() const { return m_TexturesCount; }

void stw::Pipeline::SetVec2(std::string_view name, glm::vec2 value)
{
	const auto location = GetUniformLocation(name);

	GLCALL(glUniform2f(location, value.x, value.y));
}

void stw::Pipeline::SetVec4(std::string_view name, glm::vec4 value)
{
	const auto location = GetUniformLocation(name);

	GLCALL(glUniform4f(location, value.x, value.y, value.z, value.w));
}

void stw::Pipeline::SetVec3V(std::string_view name, std::span<const glm::vec3> values)
{
	const auto location = GetUniformLocation(name);

	GLCALL(glUniform3fv(location, values.size(), glm::value_ptr(values[0])));
}
