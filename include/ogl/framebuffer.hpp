#pragma once

#include <array>
#include <expected>
#include <GL/glew.h>
#include <glm/vec2.hpp>

#include "number_types.hpp"
#include "texture.hpp"

namespace stw
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

	[[nodiscard]] stw::AttachmentType GetAttachmentType() const;
};

struct FramebufferDescription
{
	static constexpr usize MaxColorAttachments = 16;
	usize colorAttachmentsCount = 0;
	std::array<FramebufferColorAttachment, MaxColorAttachments> colorAttachments{};
	std::optional<FramebufferDepthStencilAttachment> depthStencilAttachment{};
	glm::uvec2 framebufferSize{ 1280, 720 };
};

class Framebuffer
{
public:
	Framebuffer() = default;
	~Framebuffer();

	void Init(const FramebufferDescription& description);
	void Bind() const;
	[[nodiscard]] std::optional<GLuint> GetDepthStencilAttachment() const;
	[[nodiscard]] GLuint GetColorAttachment(usize index) const;
	void UnBind() const;
	void Delete();

private:
	FramebufferDescription m_Description;

	GLuint m_Fbo = 0;
	usize m_ColorAttachmentsCount = 0;
	std::array<GLuint, FramebufferDescription::MaxColorAttachments> m_ColorAttachments{};
	std::optional<GLuint> m_DepthStencilAttachment{};

	void HandleColorAttachments(const FramebufferDescription& description);
};

bool CheckFramebufferStatus();
}// namespace stw
