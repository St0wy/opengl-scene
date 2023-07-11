#include "ogl/framebuffer.hpp"

#include <SDL_assert.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"

stw::Framebuffer::~Framebuffer()
{
	if (m_Fbo != 0)
	{
		spdlog::error("Destructor called on framebuffer that is not deleted");
	}
}

void stw::Framebuffer::Init(const FramebufferDescription& description)
{
	assert(m_Fbo == 0);

	m_Description = description;

	GLCALL(glCreateFramebuffers(1, &m_Fbo));
	Bind();

	m_ColorAttachmentsCount = m_Description.colorAttachmentsCount;

	HandleColorAttachments(description);

	if (description.depthStencilAttachment.has_value())
	{
		const auto& depthAttachmentInfo = description.depthStencilAttachment.value();
		const auto attachmentType = depthAttachmentInfo.GetAttachmentType();

		const auto width = static_cast<GLsizei>(description.framebufferSize.x);
		const auto height = static_cast<GLsizei>(description.framebufferSize.y);

		if (depthAttachmentInfo.isRenderbufferObject)
		{
			GLuint index = 0;
			GLCALL(glGenRenderbuffers(1, &index));
			m_DepthStencilAttachment = index;

			GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, m_DepthStencilAttachment.value()));
			GLCALL(glRenderbufferStorage(GL_RENDERBUFFER, attachmentType.internalFormat, width, height));
			const auto attachmentEnum =
				depthAttachmentInfo.hasStencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
			GLCALL(glFramebufferRenderbuffer(
				GL_FRAMEBUFFER, attachmentEnum, GL_RENDERBUFFER, m_DepthStencilAttachment.value()));
		}
		else
		{
			GLuint index = 0;
			GLCALL(glCreateTextures(GL_TEXTURE_2D, 1, &index));
			m_DepthStencilAttachment = index;

			GLCALL(glBindTexture(GL_TEXTURE_2D, m_DepthStencilAttachment.value()));
			GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

			GLCALL(glTexImage2D(GL_TEXTURE_2D,
				0,
				attachmentType.internalFormat,
				width,
				height,
				0,
				attachmentType.format,
				attachmentType.type,
				nullptr));

			const auto attachmentEnum =
				depthAttachmentInfo.hasStencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
			GLCALL(glFramebufferTexture2D(
				GL_FRAMEBUFFER, attachmentEnum, GL_TEXTURE_2D, m_DepthStencilAttachment.value(), 0));
		}
	}

	CheckFramebufferStatus();
	UnBind();
}

void stw::Framebuffer::HandleColorAttachments(const stw::FramebufferDescription& description)
{
	for (usize i = 0; i < m_ColorAttachmentsCount; i++)
	{
		GLuint& colorAttachmentId = m_ColorAttachments.at(i);
		const FramebufferColorAttachment& colorAttachmentInfo = m_Description.colorAttachments.at(i);
		const auto attachmentTypeResult = colorAttachmentInfo.GetAttachmentType();
		if (!attachmentTypeResult)
		{
			spdlog::error("Error getting the framebuffer color attachment type : {}", attachmentTypeResult.error());
			assert(false);
		}

		const AttachmentType& attachmentType = attachmentTypeResult.value();
		const auto width = static_cast<GLsizei>(description.framebufferSize.x);
		const auto height = static_cast<GLsizei>(description.framebufferSize.y);

		if (colorAttachmentInfo.isRenderbufferObject)
		{
			GLCALL(glGenRenderbuffers(1, &colorAttachmentId));
			GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, colorAttachmentId));
			GLCALL(glRenderbufferStorage(GL_RENDERBUFFER, attachmentType.internalFormat, width, height));

			GLCALL(glFramebufferRenderbuffer(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, colorAttachmentId));
		}
		else
		{
			GLCALL(glGenTextures(1, &colorAttachmentId));
			GLCALL(glBindTexture(GL_TEXTURE_2D, colorAttachmentId));

			GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		}

		GLCALL(glTexStorage2D(GL_TEXTURE_2D, 1, attachmentType.internalFormat, width, height));
		GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorAttachmentId, 0));
	}

	if (m_ColorAttachmentsCount == 0)
	{
		GLCALL(glDrawBuffer(GL_NONE));
		GLCALL(glReadBuffer(GL_NONE));
	}
	else
	{
		// If this assert fails, add more attachments bellow and increase the value accordingly
		constexpr usize colorAttachmentsInArray = 16;
		static_assert(FramebufferDescription::MaxColorAttachments == colorAttachmentsInArray);
		static constexpr std::array<GLenum, FramebufferDescription::MaxColorAttachments> v = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3,
			GL_COLOR_ATTACHMENT4,
			GL_COLOR_ATTACHMENT5,
			GL_COLOR_ATTACHMENT6,
			GL_COLOR_ATTACHMENT7,
			GL_COLOR_ATTACHMENT8,
			GL_COLOR_ATTACHMENT9,
			GL_COLOR_ATTACHMENT10,
			GL_COLOR_ATTACHMENT11,
			GL_COLOR_ATTACHMENT12,
			GL_COLOR_ATTACHMENT13,
			GL_COLOR_ATTACHMENT14,
			GL_COLOR_ATTACHMENT15,
		};
		GLCALL(glDrawBuffers(static_cast<GLsizei>(m_ColorAttachmentsCount), v.data()));
	}
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
std::optional<GLuint> stw::Framebuffer::GetDepthStencilAttachment() const { return m_DepthStencilAttachment; }

GLuint stw::Framebuffer::GetColorAttachment(usize index) const { return m_ColorAttachments.at(index); }

void stw::Framebuffer::Resize(glm::uvec2 newSize)
{
	auto descriptionCopy = m_Description;
	descriptionCopy.framebufferSize = newSize;
	Delete();
	Init(descriptionCopy);
}

std::expected<stw::AttachmentType, std::string> stw::FramebufferColorAttachment::GetAttachmentType() const
{
	GLint glInternalFormat{};
	GLenum glFormat{};
	GLenum glType{};

	switch (type)
	{
	case Type::Unsigned:
		switch (size)
		{
		case Size::Eight:
			glType = GL_UNSIGNED_BYTE;
			switch (format)
			{
			case Format::Red:
				glInternalFormat = GL_R8UI;
				glFormat = GL_RED_INTEGER;
				break;
			case Format::Rg:
				glInternalFormat = GL_RG8UI;
				glFormat = GL_RG_INTEGER;
				break;
			case Format::Rgb:
				glInternalFormat = GL_RGB8UI;
				glFormat = GL_RGB_INTEGER;
				break;
			case Format::Rgba:
				glInternalFormat = GL_RGBA8UI;
				glFormat = GL_RGBA_INTEGER;
				break;
			default:
				return std::unexpected("Bad format");
			}
			break;
		case Size::Sixteen:
			glType = GL_UNSIGNED_SHORT;
			switch (format)
			{
			case Format::Red:
				glInternalFormat = GL_R16UI;
				glFormat = GL_RED_INTEGER;
				break;
			case Format::Rg:
				glInternalFormat = GL_RG16UI;
				glFormat = GL_RG_INTEGER;
				break;
			case Format::Rgb:
				glInternalFormat = GL_RGB16UI;
				glFormat = GL_RGB_INTEGER;
				break;
			case Format::Rgba:
				glInternalFormat = GL_RGBA16UI;
				glFormat = GL_RGBA_INTEGER;
				break;
			default:
				return std::unexpected("Bad format");
			}
			break;
		case Size::ThirtyTwo:
			glType = GL_UNSIGNED_INT;
			switch (format)
			{
			case Format::Red:
				glInternalFormat = GL_R32UI;
				glFormat = GL_RED_INTEGER;
				break;
			case Format::Rg:
				glInternalFormat = GL_RG32UI;
				glFormat = GL_RG_INTEGER;
				break;
			case Format::Rgb:
				glInternalFormat = GL_RGB32UI;
				glFormat = GL_RGB_INTEGER;
				break;
			case Format::Rgba:
				glInternalFormat = GL_RGBA32UI;
				glFormat = GL_RGBA_INTEGER;
				break;
			default:
				return std::unexpected("Bad format");
			}
			break;
		default:
			return std::unexpected("Bad size");
		}
		break;
	case Type::Int:
		switch (size)
		{
		case Size::Eight:
			glType = GL_BYTE;
			switch (format)
			{
			case Format::Red:
				glInternalFormat = GL_R8I;
				glFormat = GL_RED_INTEGER;
				break;
			case Format::Rg:
				glInternalFormat = GL_RG8I;
				glFormat = GL_RG_INTEGER;
				break;
			case Format::Rgb:
				glInternalFormat = GL_RGB8I;
				glFormat = GL_RGB_INTEGER;
				break;
			case Format::Rgba:
				glInternalFormat = GL_RGBA8I;
				glFormat = GL_RGBA_INTEGER;
				break;
			default:
				return std::unexpected("Bad format");
			}
			break;
		case Size::Sixteen:
			glType = GL_SHORT;
			switch (format)
			{
			case Format::Red:
				glInternalFormat = GL_R16I;
				glFormat = GL_RED_INTEGER;
				break;
			case Format::Rg:
				glInternalFormat = GL_RG16I;
				glFormat = GL_RG_INTEGER;
				break;
			case Format::Rgb:
				glInternalFormat = GL_RGB16I;
				glFormat = GL_RGB_INTEGER;
				break;
			case Format::Rgba:
				glInternalFormat = GL_RGBA16I;
				glFormat = GL_RGBA_INTEGER;
				break;
			default:
				return std::unexpected("Bad format");
			}
			break;
		case Size::ThirtyTwo:
			glType = GL_INT;
			switch (format)
			{
			case Format::Red:
				glInternalFormat = GL_R32I;
				glFormat = GL_RED_INTEGER;
				break;
			case Format::Rg:
				glInternalFormat = GL_RG32I;
				glFormat = GL_RG_INTEGER;
				break;
			case Format::Rgb:
				glInternalFormat = GL_RGB32I;
				glFormat = GL_RGB_INTEGER;
				break;
			case Format::Rgba:
				glInternalFormat = GL_RGBA32I;
				glFormat = GL_RGBA_INTEGER;
				break;
			default:
				return std::unexpected("Bad format");
			}
			break;
		default:
			return std::unexpected("Bad size");
		}
		break;
	case Type::Float:
		switch (size)
		{
		case Size::Eight:
			glType = GL_BYTE;
			switch (format)
			{
			case Format::Red:
				glInternalFormat = GL_R8;
				glFormat = GL_RED;
				break;
			case Format::Rg:
				glInternalFormat = GL_RG8;
				glFormat = GL_RG;
				break;
			case Format::Rgb:
				glInternalFormat = GL_RGB8;
				glFormat = GL_RGB;
				break;
			case Format::Rgba:
				glInternalFormat = GL_RGBA8;
				glFormat = GL_RGBA;
				break;
			default:
				return std::unexpected("Bad format");
			}
			break;
		case Size::Sixteen:
			glType = GL_HALF_FLOAT;
			switch (format)
			{
			case Format::Red:
				glInternalFormat = GL_R16F;
				glFormat = GL_RED;
				break;
			case Format::Rg:
				glInternalFormat = GL_RG16F;
				glFormat = GL_RG;
				break;
			case Format::Rgb:
				glInternalFormat = GL_RGB16F;
				glFormat = GL_RGB;
				break;
			case Format::Rgba:
				glInternalFormat = GL_RGBA16F;
				glFormat = GL_RGBA;
				break;
			default:
				return std::unexpected("Bad format");
			}
			break;
		case Size::ThirtyTwo:
			glType = GL_FLOAT;
			switch (format)
			{
			case Format::Red:
				glInternalFormat = GL_R32F;
				glFormat = GL_RED;
				break;
			case Format::Rg:
				glInternalFormat = GL_RG32I;
				glFormat = GL_RG;
				break;
			case Format::Rgb:
				glInternalFormat = GL_RGB32I;
				glFormat = GL_RGB;
				break;
			case Format::Rgba:
				glInternalFormat = GL_RGBA32I;
				glFormat = GL_RGBA;
				break;
			default:
				return std::unexpected("Bad format");
			}
			break;
		default:
			return std::unexpected("Bad size");
		}
		break;
	}

	return AttachmentType{ glInternalFormat, glFormat, glType };
}
stw::AttachmentType stw::FramebufferDepthStencilAttachment::GetAttachmentType() const
{
	if (hasStencil)
	{
		return { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 };
	}
	else
	{
		return { GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT };
	}
}

bool stw::CheckFramebufferStatus()
{
	const auto framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch (framebufferStatus)
	{
	case GL_FRAMEBUFFER_COMPLETE:
		spdlog::debug("Check Framebuffer: Complete");
		return true;
	case GL_FRAMEBUFFER_UNDEFINED:
		spdlog::error("Check Framebuffer: Undefined");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		spdlog::error("Check Framebuffer: Incomplete Attachment");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		spdlog::error("Check Framebuffer: Incomplete Missing Attachment");
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		spdlog::error("Check Framebuffer: Unsupported");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		spdlog::error("Check Framebuffer: Incomplete Multisample");
		break;
	default:
		spdlog::error("Check Framebuffer: Unknown Error with code {}", framebufferStatus);
		break;
	}
	return false;
}
