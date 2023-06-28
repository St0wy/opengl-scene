#pragma once

#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "material.hpp"
#include "uniform_buffer.hpp"
#include "glm/gtc/bitfield.hpp"
#include "texture_manager.hpp"
#include "mesh.hpp"
#include "material_manager.hpp"

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

	glm::vec3 viewPosition{ 0.0f };

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

	void Draw(Pipeline& pipeline, const glm::mat4& modelMatrix);

	std::optional<std::string> LoadModel(const std::filesystem::path& path, Pipeline& pipeline);
	[[nodiscard]] TextureManager& GetTextureManager();

	void Delete();

private:

	bool m_EnableMultisample = false;
	bool m_EnableDepthTest = false;
	bool m_EnableCullFace = false;
	bool m_IsInitialized = false;
	GLenum m_DepthFunction = GL_LESS;
	GLenum m_CullFace = GL_BACK;
	GLenum m_FrontFace = GL_CCW;
	glm::vec4 m_ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	UniformBuffer m_MatricesUniformBuffer;

	TextureManager m_TextureManager;
	MaterialManager m_MaterialManager;
	std::vector<Mesh> m_Meshes;

	static void SetOpenGlCapability(bool enabled, GLenum capability, bool& field);

	void ProcessNode(const aiNode* node, const aiScene* scene, std::size_t materialIndexOffset);
	static stw::Mesh ProcessMesh(aiMesh* assimpMesh, std::size_t materialIndexOffset);
};
}
