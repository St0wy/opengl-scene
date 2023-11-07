/**
 * @file uniform_buffer.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the UniformBuffer class.
 * @version 1.0
 * @date 04/05/2023
 * 
 * @copyright SAE (c) 2023
 *
 */

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
