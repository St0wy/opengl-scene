#pragma once

#include <span>
#include <GL/glew.h>

#include "number_types.hpp"

namespace stw
{
class IndexBuffer
{
public:
	IndexBuffer() = default;
	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer(IndexBuffer&& other) noexcept;
	~IndexBuffer();

	IndexBuffer& operator=(const IndexBuffer&) = delete;
	IndexBuffer& operator=(IndexBuffer&& other) noexcept;

	void Init(std::span<GLuint> data);
	void Bind() const;
	static void UnBind();
	void Delete();

	[[nodiscard]] u32 GetCount() const;

private:
	GLuint m_BufferId{};
	u32 m_Count{};
	bool m_IsInitialized = false;
};
}
