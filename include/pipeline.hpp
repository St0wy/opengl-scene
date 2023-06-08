//
// Created by stowy on 04/05/2023.
//

#pragma once

#include <string_view>
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

// TODO : Rework uniform setters
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

	void InitFromPath(std::string_view vertexPath, std::string_view fragmentPath);
	void Use() const;
	void SetBool(std::string_view name, bool value) const;
	void SetInt(std::string_view name, i32 value) const;
	void SetUnsignedInt(std::string_view name, u32 value) const;
	void SetFloat(std::string_view name, f32 value) const;
	void SetVec3(std::string_view name, glm::vec3 value) const;
	void SetMat3(std::string_view name, const glm::mat3& mat) const;
	void SetMat4(std::string_view name, const glm::mat4& mat) const;

	void SetPointLightsCount(u32 count);
	void SetDirectionalLightsCount(u32 count);
	void SetSpotLightsCount(u32 count);

	void SetPointLight(std::string_view name, u32 index, const PointLight& pointLight, const glm::mat4& view) const;
	void SetDirectionalLight(std::string_view name, u32 index, const DirectionalLight& directionalLight) const;
	void SetSpotLight(std::string_view name, u32 index, const SpotLight& spotLight, const glm::mat4& view) const;

private:
	GLuint m_ProgramId;
	GLuint m_VertexShaderId;
	GLuint m_FragmentShaderId;

	u32 m_DirectionalLightsCount;
	u32 m_PointLightsCount;
	u32 m_SpotLightsCount;
};
}
