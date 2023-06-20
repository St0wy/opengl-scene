#include "ogl/framebuffer.hpp"

#include <spdlog/spdlog.h>

#include "utils.hpp"

stw::Framebuffer::~Framebuffer()
{
	if (m_Fbo == 0)
	{
		spdlog::error("Destructor called on framebuffer that is not deleted");
	}
}

void stw::Framebuffer::Init()
{
	GLCALL(glGenFramebuffers(1, &m_Fbo));
}

void stw::Framebuffer::Bind() const
{
	if (m_Fbo == 0)
	{
		spdlog::error("Binding framebuffer that is not initialized");
	}

	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo));
}

void stw::Framebuffer::UnBind() const
{
	if (m_Fbo == 0)
	{
		spdlog::error("Unbinding framebuffer that is not initialized");
	}

	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void stw::Framebuffer::Delete()
{
	GLCALL(glDeleteFramebuffers(1, &m_Fbo));
	m_Fbo = 0;
}
