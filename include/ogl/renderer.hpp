#pragma once

#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "material.hpp"
#include "uniform_buffer.hpp"
#include "glm/gtc/bitfield.hpp"

namespace stw
{
class Pipeline;
class Model;

class Renderer
{
public:
	Renderer() = default;
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	~Renderer();

	glm::vec3 viewPosition;

	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;

	void Init();

	void SetEnableMultisample(bool enableMultisample);
	void SetEnableDepthTest(bool enableDepthTest);
	void SetDepthFunc(GLenum depthFunction);
	void SetEnableCullFace(bool enableCullFace);
	void SetCullFace(GLenum cullFace);
	void SetFrontFace(GLenum frontFace);
	void SetClearColor(const glm::vec4& clearColor);
	void SetProjectionMatrix(const glm::mat4& projection) const;
	void SetViewMatrix(const glm::mat4& view) const;
	void SetViewport(glm::ivec2 pos, glm::uvec2 size) const;

	void Clear(GLbitfield mask);

	void Draw(const Model& model, Pipeline& pipeline, const Material& material, const glm::mat4& modelMatrix) const;

	void Delete();

private:
	bool m_EnableMultisample = false;
	bool m_EnableDepthTest = false;
	bool m_EnableCullFace = false;
	bool m_IsInitialized = false;
	GLenum m_DepthFunction = GL_LESS;
	GLenum m_CullFace = GL_BACK;
	GLenum m_FrontFace = GL_CCW;
	glm::vec4 m_ClearColor{0.0f, 0.0f, 0.0f, 1.0f};

	UniformBuffer m_MatricesUniformBuffer;

	static void SetOpenGlCapability(bool enabled, GLenum capability, bool& field);
};
}
