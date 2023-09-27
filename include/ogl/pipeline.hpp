//
// Created by stowy on 04/05/2023.
//

#pragma once

#include <filesystem>
#include <unordered_map>
#include <GL/glew.h>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <span>

#include "number_types.hpp"

namespace stw
{
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
	void InitFromPathSingleFile(const std::filesystem::path& shaderFile);
	void InitFromSource(std::string_view vertexSource, std::string_view fragmentSource);

	void Delete();

	[[nodiscard]] GLuint Id() const;

	void Bind();
	void UnBind();

	void SetBool(std::string_view name, bool value);
	void SetInt(std::string_view name, i32 value);
	void SetUnsignedInt(std::string_view name, u32 value);
	void SetFloat(std::string_view name, f32 value);
	void SetVec4(std::string_view name, glm::vec4 value);
	void SetVec3(std::string_view name, glm::vec3 value);
	void SetVec3V(std::string_view name, std::span<const glm::vec3> values);
	void SetVec2(std::string_view name, glm::vec2 value);
	void SetMat3(std::string_view name, const glm::mat3& mat);
	void SetMat4(std::string_view name, const glm::mat4& mat);

	[[nodiscard]] usize GetTextureCount() const;
private:
	// TODO : Rework the cache to not use string views because the end up pointing to garbage memory...
	//std::unordered_map<std::string_view, GLint> m_UniformsLocation{};
	bool m_IsInitialized = false;

	GLuint m_ProgramId{};
	GLuint m_VertexShaderId{};
	GLuint m_FragmentShaderId{};
	usize m_TexturesCount{};

	GLint GetUniformLocation(std::string_view name) const;
	[[nodiscard]] usize GetTextureCountFromOpenGl() const;
};
}
