#pragma once

#include <GL/glew.h>

namespace stw
{
class UniformBuffer
{
public:
	UniformBuffer() = default;
	UniformBuffer(const UniformBuffer&) = delete;
	UniformBuffer(UniformBuffer&&) = delete;
	~UniformBuffer();

	UniformBuffer& operator=(const UniformBuffer&) = delete;
	UniformBuffer& operator=(UniformBuffer&&) = delete;

	void Init(GLuint bindingIndex);
	void Bind() const;
	void Allocate(GLsizeiptr size) const;
	void SetData(GLsizeiptr size, const GLvoid* data) const;
	void SetSubData(GLintptr offset, GLsizeiptr size, const GLvoid* data) const;
	void UnBind() const;
	void Delete();

private:
	GLuint m_Ubo{};
	GLuint m_BindingIndex{};
};
}
