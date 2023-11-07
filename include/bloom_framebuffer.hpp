/**
 * @file bloom_framebuffer.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Camera class.
 * @version 1.0
 * @date 10/07/2023
 *
 * @copyright SAE (c) 2023
 *
 */

#pragma once

#include <span>
#include <vector>
#include <GL/glew.h>
#include <glm/vec2.hpp>

#include "number_types.hpp"

namespace stw
{
struct BloomMip
{
	glm::vec2 size;
	glm::ivec2 intSize;
	GLuint texture;
};

class BloomFramebuffer
{
public:
	BloomFramebuffer() = default;
	~BloomFramebuffer();
	BloomFramebuffer(const BloomFramebuffer&) = delete;
	BloomFramebuffer(BloomFramebuffer&&) = delete;

	BloomFramebuffer& operator=(const BloomFramebuffer&) = delete;
	BloomFramebuffer& operator=(BloomFramebuffer&&) = delete;

	bool Init(glm::uvec2 screenSize, u32 mipChainLength);
	void Delete();
	void Bind() const;
	void UnBind() const;
	[[nodiscard]] std::span<const BloomMip> MipChain() const;

private:
	bool m_IsInitialized = false;
	GLuint m_FramebufferIndex = 0;
	std::vector<BloomMip> m_MipChain{};
};
}// namespace stw
