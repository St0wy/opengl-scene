/**
 * @file bloom_framebuffer.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Camera class.
 * @version 1.0
 * @date 10/07/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include "glm/detail/_noise.hpp"


#include <array>
#include <span>
#include <vector>

#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <spdlog/spdlog.h>

export module bloom_framebuffer;

import number_types;
import consts;
import utils;

export
{
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

		bool Init(const glm::uvec2& screenSize, u32 mipChainLength);
		void Delete();
		void Bind() const;
		void UnBind() const;
		[[nodiscard]] std::span<const BloomMip> MipChain() const;

	private:
		bool m_IsInitialized = false;
		GLuint m_FramebufferIndex = 0;
		std::vector<BloomMip> m_MipChain{};
	};

	BloomFramebuffer::~BloomFramebuffer()
	{
		if (m_IsInitialized)
		{
			spdlog::error("Destructor called on bloom framebuffer that is still initialized");
		}
	}
	bool BloomFramebuffer::Init(const glm::uvec2& screenSize, const u32 mipChainLength)
	{
		assert(!m_IsInitialized);

		glGenFramebuffers(1, &m_FramebufferIndex);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferIndex);

		glm::vec2 mipSize{ screenSize };
		glm::ivec2 mipIntSize{ screenSize };

		for (u32 i = 0; i < mipChainLength; i++)
		{
			BloomMip bloomMip{};

			mipSize *= 0.5f;
			mipIntSize /= 2;

			bloomMip.size = mipSize;
			bloomMip.intSize = mipIntSize;

			glGenTextures(1, &bloomMip.texture);
			glBindTexture(GL_TEXTURE_2D, bloomMip.texture);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, mipIntSize.x, mipIntSize.y, 0, GL_RGB, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			m_MipChain.push_back(bloomMip);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_MipChain[0].texture, 0);

		constexpr std::array<GLuint, 1> attachments = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments.data());

		const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			spdlog::error("Bloom framebuffer error, status : {0:#x}", status);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_IsInitialized = true;
		return true;
	}

	void BloomFramebuffer::Delete()
	{
		assert(m_IsInitialized);

		for (BloomMip& bloomMip : m_MipChain)
		{
			glDeleteTextures(1, &bloomMip.texture);
			bloomMip.texture = 0;
		}

		glDeleteFramebuffers(1, &m_FramebufferIndex);
		m_FramebufferIndex = 0;
		m_IsInitialized = false;
	}

	void BloomFramebuffer::Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferIndex); }

	std::span<const BloomMip> BloomFramebuffer::MipChain() const { return m_MipChain; }

	void BloomFramebuffer::UnBind() const
	{
		assert(m_IsInitialized);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	}// namespace stw
}