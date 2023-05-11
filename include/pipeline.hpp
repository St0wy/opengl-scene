//
// Created by stowy on 04/05/2023.
//

#pragma once

#include <string_view>
#include <GL/glew.h>

#include "number_types.hpp"
#include "glm/fwd.hpp"

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

	void InitFromPath(std::string_view vertexPath, std::string_view fragmentPath);
	void Use() const;
	void SetBool(std::string_view name, bool value) const;
	void SetInt(std::string_view name, i32 value) const;
	void SetFloat(std::string_view name, f32 value) const;
	void SetVec3(std::string_view name, glm::vec3 value) const;
	void SetMat3(std::string_view name, const glm::mat3& mat) const;
	void SetMat4(std::string_view name, const glm::mat4& mat) const;

private:
	GLuint m_ProgramId;
	GLuint m_VertexShaderId;
	GLuint m_FragmentShaderId;
};
}
