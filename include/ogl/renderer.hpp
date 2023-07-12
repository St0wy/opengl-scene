#pragma once

#include <array>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "bloom_framebuffer.hpp"
#include "glm/gtc/bitfield.hpp"
#include "material.hpp"
#include "material_manager.hpp"
#include "mesh.hpp"
#include "ogl/framebuffer.hpp"
#include "scene_graph.hpp"
#include "texture_manager.hpp"
#include "uniform_buffer.hpp"

namespace stw
{
class Pipeline;

class Model;

constexpr f32 MinLightIntensity = 5.0f;

struct DirectionalLight
{
	glm::vec3 direction;
	glm::vec3 color;
};

struct PointLight
{
	PointLight() = default;
	PointLight(glm::vec3 position, f32 linear, f32 quadratic, glm::vec3 color);
	glm::vec3 position{};
	f32 linear{};
	f32 quadratic{};
	f32 radius{};
	glm::vec3 color{};
};

struct ProcessMeshResult
{
	stw::Mesh mesh;
	std::size_t materialIndex;
};

class Renderer
{
public:
	static constexpr u32 MaxPointLights = 128;
	static constexpr glm::uvec2 ShadowMapSize = { 4096, 4096 };
	static constexpr u32 MipChainLength = 5;
	static constexpr f32 FilterRadius = 0.005f;

	Renderer() = default;
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	~Renderer();

	glm::vec3 viewPosition{ 0.0f };

	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;

	void Init(glm::uvec2 screenSize);

	void SetEnableMultisample(bool enableMultisample);
	void SetEnableDepthTest(bool enableDepthTest);
	void SetDepthFunc(GLenum depthFunction);
	void SetEnableCullFace(bool enableCullFace);
	[[maybe_unused]] void SetCullFace(GLenum cullFace);
	[[maybe_unused]] void SetFrontFace(GLenum frontFace);
	[[maybe_unused]] void SetClearColor(const glm::vec4& clearColor);
	void SetProjectionMatrix(const glm::mat4& projection);
	void SetViewMatrix(const glm::mat4& view);
	void SetViewport(glm::ivec2 pos, glm::uvec2 size);

	SceneGraph& GetSceneGraph();

	void SetDirectionalLight(const DirectionalLight& directionalLight);
	[[maybe_unused]] void RemoveDirectionalLight();

	[[maybe_unused]] void PushPointLight(const PointLight& pointLight);
	[[maybe_unused]] void PopPointLight();
	[[maybe_unused]] void SetPointLight(usize index, const PointLight& pointLight);

	void Clear(GLbitfield mask);

	void DrawScene();

	std::expected<std::vector<std::reference_wrapper<const stw::SceneGraphNode>>, std::string> LoadModel(
		const std::filesystem::path& path, Pipeline& pipeline);
	[[maybe_unused]] [[nodiscard]] TextureManager& GetTextureManager();

	void Delete();

private:
	bool m_EnableMultisample = false;
	bool m_EnableDepthTest = false;
	bool m_EnableCullFace = false;
	bool m_IsInitialized = false;
	GLenum m_DepthFunction = GL_LESS;
	GLenum m_CullFace = GL_BACK;
	GLenum m_FrontFace = GL_CCW;
	glm::uvec2 m_ViewportSize{};
	glm::vec4 m_ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	UniformBuffer m_MatricesUniformBuffer;
	glm::mat4 m_CameraViewMatrix{};
	glm::mat4 m_CameraProjectionMatrix{};

	TextureManager m_TextureManager;
	MaterialManager m_MaterialManager;
	std::vector<Mesh> m_Meshes;
	SceneGraph m_SceneGraph;

	Pipeline m_DepthPipeline;
	Framebuffer m_LightDepthMapFramebuffer;
	Framebuffer m_HdrFramebuffer;
	Pipeline m_HdrPipeline;
	Mesh m_RenderQuad{};

	BloomFramebuffer m_BloomFramebuffer;
	Pipeline m_DownsamplePipeline;
	Pipeline m_UpsamplePipeline;

	Framebuffer m_GBufferFramebuffer;
	Pipeline m_GBufferPipeline;
	Pipeline m_DebugLightsPipeline;
	Mesh m_DebugSphereLight{};
	Pipeline m_PointLightPipeline;
	Pipeline m_DirectionalLightPipeline;

	std::optional<DirectionalLight> m_DirectionalLight = {};

	u32 m_PointLightsCount = 0;
	std::array<PointLight, MaxPointLights> m_PointLights{};

	static void SetOpenGlCapability(bool enabled, GLenum capability, bool& field);

	static ProcessMeshResult ProcessMesh(const aiMesh* assimpMesh, std::size_t materialIndexOffset);
	void RenderShadowMap(const glm::mat4& lightViewProjMatrix);
	void RenderBloomToBloomFramebuffer(GLuint hdrTexture, float filterRadius);
	void RenderDownsample(GLuint hdrTexture);
	void RenderUpsamples(float filterRadius);
	void RenderGBuffer();
	void RenderLightsToHdrFramebuffer();
	void RenderDebugLights();
	void RenderPointLights();
	void RenderDirectionalLight(const glm::mat4& lightViewProjMatrix);
};
}// namespace stw
