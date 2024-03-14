/**
 * @file framebuffer.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Framebuffer class.
 * @version 1.0
 * @date 09/11/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <array>
#include <expected>

#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <SDL_assert.h>
#include <spdlog/spdlog.h>

export module framebuffer;

import number_types;
import consts;
import utils;
import texture;


export namespace stw
{
struct AttachmentType
{
	GLint internalFormat{};
	GLenum format{};
	GLenum type{};
};

struct FramebufferColorAttachment
{
	enum class Format : u8
	{
		Red,
		Rg,
		Rgb,
		Rgba,
	};

	enum class Size : u8
	{
		Eight,
		Sixteen,
		ThirtyTwo,
	};

	enum class Type : u8
	{
		Unsigned,
		Int,
		Float,
	};

	Format format = Format::Rgb;
	Size size = Size::Eight;
	Type type = Type::Unsigned;
	bool isRenderbufferObject = false;

	[[nodiscard]] std::expected<AttachmentType, std::string> GetAttachmentType() const;
};

struct FramebufferDepthStencilAttachment
{
	bool hasStencil = true;
	bool isRenderbufferObject = false;

	[[nodiscard]] AttachmentType GetAttachmentType() const;
};

struct FramebufferDescription
{
	static constexpr usize MaxColorAttachments = 16;
	static constexpr glm::uvec2 DefaultFramebufferSize{ 1280, 720 };

	usize colorAttachmentsCount = 0;
	std::array<FramebufferColorAttachment, MaxColorAttachments> colorAttachments{};
	std::optional<FramebufferDepthStencilAttachment> depthStencilAttachment{};
	glm::uvec2 framebufferSize = DefaultFramebufferSize;
};

// TODO : Rule of five
class Framebuffer
{
public:
	Framebuffer() = default;
	Framebuffer(const Framebuffer& other) = delete;
	Framebuffer(Framebuffer&&) = delete;
	~Framebuffer();

	Framebuffer& operator=(const Framebuffer& other) = delete;
	Framebuffer& operator=(Framebuffer&&) = delete;

	void Init(const FramebufferDescription& description);
	void Bind() const;
	void BindRead() const;
	void BindWrite() const;
	[[nodiscard]] std::optional<GLuint> GetDepthStencilAttachment() const;
	[[nodiscard]] GLuint GetColorAttachment(usize index) const;
	void UnBind() const;
	void Delete();
	void Resize(glm::uvec2 newSize);

private:
	FramebufferDescription m_Description;

	GLuint m_Fbo = 0;
	usize m_ColorAttachmentsCount = 0;
	std::array<GLuint, FramebufferDescription::MaxColorAttachments> m_ColorAttachments{};
	std::optional<GLuint> m_DepthStencilAttachment{};

	void HandleColorAttachments(const FramebufferDescription& description);
};

bool CheckFramebufferStatus();

Framebuffer::~Framebuffer()
{
	if (m_Fbo != 0)
	{
		spdlog::error("Destructor called on framebuffer that is not deleted");
	}
}

void Framebuffer::Init(const FramebufferDescription& description)
{
	assert(m_Fbo == 0);

	m_Description = description;

	glCreateFramebuffers(1, &m_Fbo);
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
			glGenRenderbuffers(1, &index);
			m_DepthStencilAttachment = index;

			glBindRenderbuffer(GL_RENDERBUFFER, m_DepthStencilAttachment.value());
			glRenderbufferStorage(GL_RENDERBUFFER, attachmentType.internalFormat, width, height);
			const auto attachmentEnum =
				depthAttachmentInfo.hasStencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
			glFramebufferRenderbuffer(
				GL_FRAMEBUFFER, attachmentEnum, GL_RENDERBUFFER, m_DepthStencilAttachment.value());
		}
		else
		{
			GLuint index = 0;
			glCreateTextures(GL_TEXTURE_2D, 1, &index);
			m_DepthStencilAttachment = index;

			glBindTexture(GL_TEXTURE_2D, m_DepthStencilAttachment.value());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexImage2D(GL_TEXTURE_2D,
				0,
				attachmentType.internalFormat,
				width,
				height,
				0,
				attachmentType.format,
				attachmentType.type,
				nullptr);

			const auto attachmentEnum =
				depthAttachmentInfo.hasStencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentEnum, GL_TEXTURE_2D, m_DepthStencilAttachment.value(), 0);
		}
	}

	CheckFramebufferStatus();
	UnBind();
}

void Framebuffer::HandleColorAttachments(const FramebufferDescription& description)
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
			glGenRenderbuffers(1, &colorAttachmentId);
			glBindRenderbuffer(GL_RENDERBUFFER, colorAttachmentId);
			glRenderbufferStorage(GL_RENDERBUFFER, attachmentType.internalFormat, width, height);

			glFramebufferRenderbuffer(
				GL_FRAMEBUFFER, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i), GL_RENDERBUFFER, colorAttachmentId);
		}
		else
		{
			glGenTextures(1, &colorAttachmentId);
			glBindTexture(GL_TEXTURE_2D, colorAttachmentId);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		glTexStorage2D(GL_TEXTURE_2D, 1, attachmentType.internalFormat, width, height);

		glFramebufferTexture2D(
			GL_FRAMEBUFFER, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i), GL_TEXTURE_2D, colorAttachmentId, 0);
	}

	if (m_ColorAttachmentsCount == 0)
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	else
	{
		// If this assert fails, add more attachments bellow and increase the value accordingly
		constexpr usize colorAttachmentsInArray = 16;
		static_assert(FramebufferDescription::MaxColorAttachments == colorAttachmentsInArray);
		static constexpr std::array<GLenum, FramebufferDescription::MaxColorAttachments> AvailableColorAttachments = {
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
		glDrawBuffers(static_cast<GLsizei>(m_ColorAttachmentsCount), AvailableColorAttachments.data());
	}
}

void Framebuffer::Bind() const
{
	if (m_Fbo == 0)
	{
		spdlog::error("Binding framebuffer that is not initialized");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
}

void Framebuffer::UnBind() const
{
	if (m_Fbo == 0)
	{
		spdlog::error("Unbinding framebuffer that is not initialized");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Delete()
{
	glDeleteFramebuffers(1, &m_Fbo);
	m_Fbo = 0;
}

std::optional<GLuint> Framebuffer::GetDepthStencilAttachment() const { return m_DepthStencilAttachment; }

GLuint Framebuffer::GetColorAttachment(const usize index) const { return m_ColorAttachments.at(index); }

void Framebuffer::Resize(const glm::uvec2 newSize)
{
	auto descriptionCopy = m_Description;
	descriptionCopy.framebufferSize = newSize;
	Delete();
	Init(descriptionCopy);
}

void Framebuffer::BindRead() const { glBindFramebuffer(GL_READ_FRAMEBUFFER, m_Fbo); }

void Framebuffer::BindWrite() const { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Fbo); }

std::expected<AttachmentType, std::string> FramebufferColorAttachment::GetAttachmentType() const
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

AttachmentType FramebufferDepthStencilAttachment::GetAttachmentType() const
{
	if (hasStencil)
	{
		return { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 };
	}

	return { GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT };
}

bool CheckFramebufferStatus()
{
	switch (const auto framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER))
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
}// namespace stw
