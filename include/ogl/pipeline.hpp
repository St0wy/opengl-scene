//
// Created by stowy on 04/05/2023.
//

#pragma once

#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <GL/glew.h>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

#include "number_types.hpp"

namespace stw
{
struct DirectionalLight
{
	glm::vec3 direction;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

struct PointLight
{
	glm::vec3 position;
	f32 constant;
	f32 linear;
	f32 quadratic;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

struct SpotLight
{
	glm::vec3 position;
	glm::vec3 direction;
	f32 cutOff;
	f32 outerCutOff;
	f32 constant;
	f32 linear;
	f32 quadratic;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

class Pipeline
{
public:
	static constexpr u32 MaxPointLights = 32;
	static constexpr u32 MaxDirectionalLights = 8;
	static constexpr u32 MaxSpotLights = 32;

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

	void Bind() const;
	void UnBind() const;

	void SetBool(std::string_view name, bool value);
	void SetInt(std::string_view name, i32 value);
	void SetUnsignedInt(std::string_view name, u32 value);
	void SetFloat(std::string_view name, f32 value);
	void SetVec3(std::string_view name, glm::vec3 value);
	void SetMat3(std::string_view name, const glm::mat3& mat);
	void SetMat4(std::string_view name, const glm::mat4& mat);

	void SetPointLightsCount(u32 count);
	void SetDirectionalLightsCount(u32 count);
	void SetSpotLightsCount(u32 count);

	void SetPointLight(std::string_view name, u32 index, const PointLight& pointLight, const glm::mat4& view);
	void SetDirectionalLight(std::string_view name, u32 index, const DirectionalLight& directionalLight);
	void SetSpotLight(std::string_view name, u32 index, const SpotLight& spotLight, const glm::mat4& view);

private:
	std::unordered_map<std::string_view, GLint> m_UniformsLocation{};
	bool m_IsInitialized = false;

	GLuint m_ProgramId{};
	GLuint m_VertexShaderId{};
	GLuint m_FragmentShaderId{};

	u32 m_DirectionalLightsCount = 0;
	u32 m_PointLightsCount = 0;
	u32 m_SpotLightsCount = 0;

	GLint GetUniformLocation(const std::string_view name);

};
}
