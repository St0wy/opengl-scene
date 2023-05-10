//
// Created by stowy on 04/05/2023.
//

#pragma once

#include <string_view>
#include <GL/glew.h>

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
	void SetInt(std::string_view name, int value) const;
	void SetFloat(std::string_view name, float value) const;

private:
	GLuint m_ProgramId;
	GLuint m_VertexShaderId;
	GLuint m_FragmentShaderId;
};
}
