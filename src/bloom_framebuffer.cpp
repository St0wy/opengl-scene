//
// Created by stowy on 10/07/2023.
//
#include "bloom_framebuffer.hpp"

#include <array>
#include <spdlog/spdlog.h>

#include "utils.hpp"

stw::BloomFramebuffer::~BloomFramebuffer()
{
	if (m_IsInitialized)
	{
		spdlog::error("Destructor called on bloom framebuffer that is still initialized");
	}
}
bool stw::BloomFramebuffer::Init(glm::uvec2 screenSize, u32 mipChainLength)
{
	assert(!m_IsInitialized);

	GLCALL(glGenFramebuffers(1, &m_FramebufferIndex));
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferIndex));

	glm::vec2 mipSize{ screenSize };
	glm::ivec2 mipIntSize{ screenSize };

	for (u32 i = 0; i < mipChainLength; i++)
	{
		BloomMip bloomMip{};

		mipSize *= 0.5f;
		mipIntSize /= 2;

		bloomMip.size = mipSize;
		bloomMip.intSize = mipIntSize;

		GLCALL(glGenTextures(1, &bloomMip.texture));
		GLCALL(glBindTexture(GL_TEXTURE_2D, bloomMip.texture));

		//		GLCALL(glTexImage2D(
		//			GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, mipIntSize.x, mipIntSize.y, 0, GL_RGB, GL_FLOAT, nullptr));

		GLCALL(glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB16F, mipIntSize.x, mipIntSize.y, 0, GL_RGB, GL_FLOAT, nullptr));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

		m_MipChain.push_back(bloomMip);
	}

	GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_MipChain[0].texture, 0));

	constexpr std::array<GLuint, 1> attachments = { GL_COLOR_ATTACHMENT0 };
	GLCALL(glDrawBuffers(1, attachments.data()));

	GLCALL(const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		spdlog::error("Bloom framebuffer error, status : {0:#x}", status);
		GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		return false;
	}

	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	m_IsInitialized = true;
	return true;
}

void stw::BloomFramebuffer::Delete()
{
	assert(m_IsInitialized);

	for (BloomMip& bloomMip : m_MipChain)
	{
		GLCALL(glDeleteTextures(1, &bloomMip.texture));
		bloomMip.texture = 0;
	}

	GLCALL(glDeleteFramebuffers(1, &m_FramebufferIndex));
	m_FramebufferIndex = 0;
	m_IsInitialized = false;
}

void stw::BloomFramebuffer::Bind() const { GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferIndex)); }

std::span<const stw::BloomMip> stw::BloomFramebuffer::MipChain() const { return m_MipChain; }

void stw::BloomFramebuffer::UnBind() const
{
	assert(m_IsInitialized);
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}
