#include "ogl/renderer.hpp"

#include <exception>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <queue>
#include <spdlog/spdlog.h>
#include <xmemory>

#include "timer.hpp"
#include "utils.hpp"

stw::Renderer::~Renderer()
{
	if (m_IsInitialized)
	{
		spdlog::error("Destructor called on renderer that is still initialized");
	}
}

void stw::Renderer::Init(glm::uvec2 screenSize)
{
	m_IsInitialized = true;
	m_MatricesUniformBuffer.Init(0);
	m_MatricesUniformBuffer.Bind();
	constexpr GLsizeiptr matricesSize = 2 * sizeof(glm::mat4);
	m_MatricesUniformBuffer.Allocate(matricesSize);
	m_SceneGraph.Init();

	m_DepthPipeline.InitFromPath("shaders/shadow_map/depth.vert", "shaders/shadow_map/depth.frag");
	m_HdrPipeline.InitFromPath("shaders/quad.vert", "shaders/hdr/hdr.frag");
	m_HdrPipeline.Bind();
	m_HdrPipeline.SetInt("hdrBuffer", 0);
	m_HdrPipeline.SetInt("bloomBuffer", 1);
	m_HdrPipeline.UnBind();

	m_DownsamplePipeline.InitFromPath("shaders/quad.vert", "shaders/bloom/downsample.frag");
	m_DownsamplePipeline.Bind();
	m_DownsamplePipeline.SetInt("srcTexture", 0);
	m_DownsamplePipeline.UnBind();

	m_UpsamplePipeline.InitFromPath("shaders/quad.vert", "shaders/bloom/upsample.frag");
	m_UpsamplePipeline.Bind();
	m_UpsamplePipeline.SetInt("srcTexture", 0);
	m_UpsamplePipeline.UnBind();

	m_GBufferPipeline.InitFromPath("shaders/deferred/gbuffer.vert", "shaders/deferred/gbuffer.frag");

	m_PointLightPipeline.InitFromPath("shaders/deferred/light_pass.vert", "shaders/deferred/point_light_pass.frag");
	m_PointLightPipeline.Bind();
	m_PointLightPipeline.SetInt("gPosition", 0);
	m_PointLightPipeline.SetInt("gNormal", 1);
	m_PointLightPipeline.SetInt("gBaseColorSpecular", 2);
	m_PointLightPipeline.UnBind();

	m_DirectionalLightPipeline.InitFromPath("shaders/quad.vert", "shaders/deferred/directional_light_pass.frag");
	m_DirectionalLightPipeline.Bind();
	m_DirectionalLightPipeline.SetInt("gPosition", 0);
	m_DirectionalLightPipeline.SetInt("gNormal", 1);
	m_DirectionalLightPipeline.SetInt("gBaseColorSpecular", 2);
	m_DirectionalLightPipeline.SetInt("shadowMaps[0]", 4);
	m_DirectionalLightPipeline.SetInt("shadowMaps[1]", 5);
	m_DirectionalLightPipeline.SetInt("shadowMaps[2]", 6);
	m_DirectionalLightPipeline.SetInt("shadowMaps[3]", 7);
	m_DirectionalLightPipeline.UnBind();

	m_DebugLightsPipeline.InitFromPath("shaders/deferred/debug_light.vert", "shaders/deferred/debug_light.frag");
	m_DebugSphereLight = Mesh::CreateUvSphere(1.0f, 20, 20);

	{
		FramebufferDepthStencilAttachment depthStencilAttachment{};
		depthStencilAttachment.isRenderbufferObject = false;
		depthStencilAttachment.hasStencil = false;
		FramebufferDescription framebufferDescription{};
		framebufferDescription.depthStencilAttachment = depthStencilAttachment;
		framebufferDescription.framebufferSize = glm::uvec2{ ShadowMapSize, ShadowMapSize };

		for (Framebuffer& lightDepthMapFramebuffer : m_LightDepthMapFramebuffers)
		{
			lightDepthMapFramebuffer.Init(framebufferDescription);
		}
	}

	{
		FramebufferDepthStencilAttachment depthStencilAttachment{};
		depthStencilAttachment.isRenderbufferObject = true;
		depthStencilAttachment.hasStencil = false;

		FramebufferColorAttachment colorAttachment{};
		colorAttachment.format = FramebufferColorAttachment::Format::Rgb;
		colorAttachment.size = FramebufferColorAttachment::Size::Sixteen;
		colorAttachment.type = FramebufferColorAttachment::Type::Float;

		FramebufferDescription framebufferDescription{};
		framebufferDescription.depthStencilAttachment = depthStencilAttachment;
		framebufferDescription.colorAttachmentsCount = 1;
		framebufferDescription.colorAttachments[0] = colorAttachment;
		framebufferDescription.framebufferSize = screenSize;
		m_HdrFramebuffer.Init(framebufferDescription);
	}

	{
		FramebufferDepthStencilAttachment depthStencilAttachment{};
		depthStencilAttachment.isRenderbufferObject = true;
		depthStencilAttachment.hasStencil = false;

		FramebufferColorAttachment positionAttachment{};
		positionAttachment.format = FramebufferColorAttachment::Format::Rgba;
		positionAttachment.size = FramebufferColorAttachment::Size::Sixteen;
		positionAttachment.type = FramebufferColorAttachment::Type::Float;

		FramebufferColorAttachment normalAttachment{};
		normalAttachment.format = FramebufferColorAttachment::Format::Rgba;
		normalAttachment.size = FramebufferColorAttachment::Size::Sixteen;
		normalAttachment.type = FramebufferColorAttachment::Type::Float;

		FramebufferColorAttachment colorSpecularAttachment{};
		colorSpecularAttachment.format = FramebufferColorAttachment::Format::Rgba;
		colorSpecularAttachment.size = FramebufferColorAttachment::Size::Sixteen;
		colorSpecularAttachment.type = FramebufferColorAttachment::Type::Float;

		FramebufferDescription framebufferDescription{};
		framebufferDescription.depthStencilAttachment = depthStencilAttachment;
		framebufferDescription.colorAttachmentsCount = 3;
		framebufferDescription.colorAttachments[0] = positionAttachment;
		framebufferDescription.colorAttachments[1] = normalAttachment;
		framebufferDescription.colorAttachments[2] = colorSpecularAttachment;
		framebufferDescription.framebufferSize = screenSize;
		m_GBufferFramebuffer.Init(framebufferDescription);
	}

	const bool success = m_BloomFramebuffer.Init(screenSize, MipChainLength);

	if (!success)
	{
		spdlog::error("Could not successfully initialize bloom framebuffer");
		assert(false);
	}

	m_RenderQuad = Mesh::CreateQuad();

	SetViewport({ 0, 0 }, screenSize);

	m_Intervals = ComputeCascades();

	m_SsaoKernel = GenerateSsaoKernel();
	m_SsaoRandomTexture = GenerateSsaoRandomTexture();
	GLCALL(glGenTextures(1, &m_SsaoGlRandomTexture));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_SsaoGlRandomTexture));
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, m_SsaoRandomTexture.data()));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
}

void stw::Renderer::SetEnableMultisample(const bool enableMultisample)
{
	SetOpenGlCapability(enableMultisample, GL_MULTISAMPLE, m_EnableMultisample);
}

void stw::Renderer::SetEnableDepthTest(const bool enableDepthTest)
{
	SetOpenGlCapability(enableDepthTest, GL_DEPTH_TEST, m_EnableDepthTest);
}

void stw::Renderer::SetDepthFunc(const GLenum depthFunction)
{
	m_DepthFunction = depthFunction;
	GLCALL(glDepthFunc(depthFunction));
}

void stw::Renderer::SetEnableCullFace(const bool enableCullFace)
{
	SetOpenGlCapability(enableCullFace, GL_CULL_FACE, m_EnableCullFace);
}

[[maybe_unused]] void stw::Renderer::SetCullFace(const GLenum cullFace)
{
	m_CullFace = cullFace;
	GLCALL(glCullFace(cullFace));
}

[[maybe_unused]] void stw::Renderer::SetFrontFace(const GLenum frontFace)
{
	m_FrontFace = frontFace;
	GLCALL(glFrontFace(m_FrontFace));
}

[[maybe_unused]] void stw::Renderer::SetClearColor(const glm::vec4& clearColor)
{
	m_ClearColor = clearColor;
	GLCALL(glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a));
}

void stw::Renderer::UpdateProjectionMatrix()
{
	assert(m_IsInitialized);
	m_MatricesUniformBuffer.Bind();
	m_MatricesUniformBuffer.SetSubData(0, sizeof(glm::mat4), value_ptr(m_Camera.GetProjectionMatrix()));
	m_MatricesUniformBuffer.UnBind();
}

void stw::Renderer::UpdateViewMatrix()
{
	assert(m_IsInitialized);
	m_MatricesUniformBuffer.Bind();
	m_MatricesUniformBuffer.SetSubData(sizeof(glm::mat4), sizeof(glm::mat4), value_ptr(m_Camera.GetViewMatrix()));
	m_MatricesUniformBuffer.UnBind();
}

void stw::Renderer::SetViewport(const glm::ivec2 pos, const glm::uvec2 size)
{
	m_ViewportSize = size;
	GLCALL(glViewport(pos.x, pos.y, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y)));
	m_HdrFramebuffer.Delete();

	m_HdrFramebuffer.Resize(size);
	m_GBufferFramebuffer.Resize(size);
}

void stw::Renderer::Clear(const GLbitfield mask)// NOLINT(readability-convert-member-functions-to-static)
{
	GLCALL(glClear(mask));
}

void stw::Renderer::DrawScene()
{
	GLCALL(glDepthMask(GL_TRUE));
	GLCALL(glEnable(GL_DEPTH_TEST));
	GLCALL(glDisable(GL_BLEND));

	RenderGBuffer();

	RenderLightsToHdrFramebuffer();

	RenderDebugLights();

	RenderBloomToBloomFramebuffer(m_HdrFramebuffer.GetColorAttachment(0), FilterRadius);

	m_HdrPipeline.Bind();
	GLCALL(glDisable(GL_DEPTH_TEST));

	GLCALL(glActiveTexture(GL_TEXTURE0));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_HdrFramebuffer.GetColorAttachment(0)));

	GLCALL(glActiveTexture(GL_TEXTURE1));
	const GLuint bloomTexture = m_BloomFramebuffer.MipChain()[0].texture;
	GLCALL(glBindTexture(GL_TEXTURE_2D, bloomTexture));

	m_RenderQuad.GetVertexArray().Bind();
	GLCALL(glDrawElements(GL_TRIANGLES, m_RenderQuad.GetIndicesSize(), GL_UNSIGNED_INT, nullptr));
	m_RenderQuad.GetVertexArray().UnBind();
	GLCALL(glEnable(GL_DEPTH_TEST));
}

void stw::Renderer::RenderShadowMaps(const std::array<glm::mat4, ShadowMapNumCascades>& lightViewProjMatrices)
{
	for (usize i = 0; i < lightViewProjMatrices.size(); i++)
	{
		GLCALL(glEnable(GL_DEPTH_CLAMP));
		m_DepthPipeline.Bind();
		m_DepthPipeline.SetMat4("lightViewProjMatrix", lightViewProjMatrices.at(i));

		GLCALL(glViewport(0, 0, ShadowMapSize, ShadowMapSize));
		m_LightDepthMapFramebuffers.at(i).Bind();
		Clear(GL_DEPTH_BUFFER_BIT);
		// Render meshes on light depth buffer
		m_SceneGraph.ForEach([this](SceneGraphElementIndex elementIndex, std::span<const glm::mat4> transformMatrices) {
			m_MatricesUniformBuffer.Bind();
			auto& mesh = m_Meshes[elementIndex.meshId];
			mesh.Bind(transformMatrices);

			const auto indicesSize = static_cast<GLsizei>(mesh.GetIndicesSize());
			GLCALL(
				glDrawElementsInstanced(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, nullptr, transformMatrices.size()));

			mesh.UnBind();
			m_MatricesUniformBuffer.UnBind();
		});

		m_DepthPipeline.UnBind();
		m_LightDepthMapFramebuffers.at(i).UnBind();
		GLCALL(glDisable(GL_DEPTH_CLAMP));
	}
}

void stw::Renderer::Delete()
{
	m_IsInitialized = false;
	m_MatricesUniformBuffer.Delete();
	m_TextureManager.Delete();

	for (auto& mesh : m_Meshes)
	{
		mesh.Delete();
	}

	m_DepthPipeline.Delete();
	for (Framebuffer& lightDepthMapFramebuffer : m_LightDepthMapFramebuffers)
	{
		lightDepthMapFramebuffer.Delete();
	}

	m_HdrFramebuffer.Delete();
	m_HdrPipeline.Delete();
	m_RenderQuad.Delete();

	m_BloomFramebuffer.Delete();
	m_DownsamplePipeline.Delete();
	m_UpsamplePipeline.Delete();

	m_PointLightPipeline.Delete();
	m_GBufferPipeline.Delete();
	m_GBufferFramebuffer.Delete();
	m_DebugSphereLight.Delete();
	m_DebugLightsPipeline.Delete();
	m_DirectionalLightPipeline.Delete();
}

void stw::Renderer::SetOpenGlCapability(const bool enabled, const GLenum capability, bool& field)
{
	field = enabled;
	if (field)
	{
		GLCALL(glEnable(capability));
	}
	else
	{
		GLCALL(glDisable(capability));
	}
}

[[maybe_unused]] stw::TextureManager& stw::Renderer::GetTextureManager() { return m_TextureManager; }

std::expected<std::vector<std::reference_wrapper<const stw::SceneGraphNode>>, std::string> stw::Renderer::LoadModel(
	const std::filesystem::path& path)
{
	Assimp::Importer importer;
	constexpr u32 assimpImportFlags = aiProcessPreset_TargetRealtime_Fast;

	const auto pathString = path.string();

	Timer timer;
	timer.Start();

	const aiScene* assimpScene = importer.ReadFile(pathString.c_str(), assimpImportFlags);

	if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
	{
		return std::unexpected(importer.GetErrorString());
	}

	spdlog::info("Assimp imported the model {} in {:0.0f} ms", pathString, timer.GetElapsedTime().GetInMilliseconds());

	auto workingDirectory = path.parent_path();
	const std::size_t materialIndexOffset =
		m_MaterialManager.LoadMaterialsFromAssimpScene(assimpScene, workingDirectory, m_TextureManager);

	m_Meshes.reserve(m_Meshes.size() + assimpScene->mNumMeshes);

	std::queue<const aiNode*> assimpNodes;
	assimpNodes.push(assimpScene->mRootNode);
	const std::span<aiMesh*> assimpSceneMeshes{ assimpScene->mMeshes, assimpScene->mNumMeshes };

	std::vector<std::reference_wrapper<const SceneGraphNode>> addedNodes;
	while (!assimpNodes.empty())
	{
		const aiNode* currentAssimpNode = assimpNodes.front();
		assimpNodes.pop();
		const std::span<u32> nodeMeshIndices{ currentAssimpNode->mMeshes, currentAssimpNode->mNumMeshes };

		for (const u32 meshIndex : nodeMeshIndices)
		{
			const aiMesh* assimpMesh = assimpSceneMeshes[meshIndex];

			if (assimpMesh->mMaterialIndex == 0)
			{
				continue;
			}

			auto [mesh, meshMaterialIndex] = ProcessMesh(assimpMesh, materialIndexOffset);

			m_Meshes.push_back(std::move(mesh));

			// TODO: Take in account parents and children (right now there's only one level of nodes)
			const glm::mat4 transformMatrix = ConvertMatAssimpToGlm(currentAssimpNode->mTransformation);
			const SceneGraphNode& node =
				m_SceneGraph.AddElementToRoot(m_Meshes.size() - 1, meshMaterialIndex, transformMatrix);
			addedNodes.emplace_back(node);
		}

		const std::span<aiNode*> nodeChildren{ currentAssimpNode->mChildren, currentAssimpNode->mNumChildren };
		for (const aiNode* child : nodeChildren)
		{
			assimpNodes.push(child);
		}
	}

	spdlog::info("Converted model {} in {:0.0f} ms", pathString, timer.GetElapsedTime().GetInMilliseconds());

	return { std::move(addedNodes) };
}

stw::ProcessMeshResult stw::Renderer::ProcessMesh(const aiMesh* assimpMesh, std::size_t materialIndexOffset)
{
	std::vector<Vertex> vertices{};
	vertices.reserve(assimpMesh->mNumVertices);
	const std::span<aiVector3D> assimpMeshVertices{ assimpMesh->mVertices, assimpMesh->mNumVertices };
	const std::span<aiVector3D> assimpMeshNormals{ assimpMesh->mNormals, assimpMesh->mNumVertices };
	const std::span<aiVector3D> assimpMeshTangents{ assimpMesh->mTangents, assimpMesh->mNumVertices };

	for (std::size_t i = 0; i < assimpMesh->mNumVertices; ++i)
	{
		const aiVector3D meshVertex = assimpMeshVertices[i];
		const aiVector3D meshNormal = assimpMeshNormals[i];
		const aiVector3D meshTangent = assimpMeshTangents[i];

		glm::vec2 textureCoords(0.0f);
		if (assimpMesh->mTextureCoords[0])
		{
			const std::span<aiVector3D> assimpMeshTextureCoords{ assimpMesh->mTextureCoords[0],
				assimpMesh->mNumVertices };
			const auto meshTextureCoords = assimpMeshTextureCoords[i];
			textureCoords.x = meshTextureCoords.x;
			textureCoords.y = meshTextureCoords.y;
		}

		const Vertex vertex{ {
								 meshVertex.x,
								 meshVertex.y,
								 meshVertex.z,
							 },
			{
				meshNormal.x,
				meshNormal.y,
				meshNormal.z,
			},
			textureCoords,
			{ meshTangent.x, meshTangent.y, meshTangent.z } };
		vertices.push_back(vertex);
	}

	std::vector<u32> indices{};
	indices.reserve(static_cast<std::size_t>(assimpMesh->mNumFaces) * 3);
	const std::span<aiFace> assimpMeshFaces{ assimpMesh->mFaces, assimpMesh->mNumFaces };
	for (const aiFace& face : assimpMeshFaces)
	{
		const std::span<u32> faceIndices{ face.mIndices, face.mNumIndices };
		for (const u32 index : faceIndices)
		{

			indices.push_back(index);
		}
	}

	Mesh mesh;
	const std::size_t materialIndex = materialIndexOffset + assimpMesh->mMaterialIndex - 1;
	mesh.Init(std::move(vertices), std::move(indices));

	return { std::move(mesh), materialIndex };
}

void stw::Renderer::SetDirectionalLight(const stw::DirectionalLight& directionalLight)
{
	m_DirectionalLight.emplace(directionalLight);
}

[[maybe_unused]] void stw::Renderer::RemoveDirectionalLight() { m_DirectionalLight.reset(); }

[[maybe_unused]] void stw::Renderer::PushPointLight(const stw::PointLight& pointLight)
{
	if (m_PointLightsCount >= MaxPointLights)
	{
		spdlog::warn("Pushing one too many point light");
		return;
	}

	m_PointLights.at(m_PointLightsCount) = pointLight;
	m_PointLightsCount++;
}

[[maybe_unused]] void stw::Renderer::PopPointLight()
{
	if (m_PointLightsCount == 0)
	{
		spdlog::warn("Popping on too many point light");
	}

	m_PointLightsCount--;
}

[[maybe_unused]] void stw::Renderer::SetPointLight(usize index, const stw::PointLight& pointLight)
{
	if (index >= m_PointLightsCount)
	{
		spdlog::error("Invalid point light index");
	}

	m_PointLights.at(index) = pointLight;
}

void stw::Renderer::RenderBloomToBloomFramebuffer(GLuint hdrTexture, f32 filterRadius)
{
	m_BloomFramebuffer.Bind();
	RenderDownsample(hdrTexture);
	RenderUpsamples(filterRadius);
	m_BloomFramebuffer.UnBind();
	GLCALL(glViewport(0, 0, m_ViewportSize.x, m_ViewportSize.y));
}

void stw::Renderer::RenderDownsample(const GLuint hdrTexture)
{
	m_DownsamplePipeline.Bind();
	m_DownsamplePipeline.SetVec2("srcResolution", glm::vec2(m_ViewportSize));

	GLCALL(glActiveTexture(GL_TEXTURE0));
	GLCALL(glBindTexture(GL_TEXTURE_2D, hdrTexture));

	for (const BloomMip& bloomMip : m_BloomFramebuffer.MipChain())
	{
		GLCALL(glViewport(0, 0, bloomMip.size.x, bloomMip.size.y));
		GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomMip.texture, 0));

		// Render current mip
		m_RenderQuad.GetVertexArray().Bind();
		GLCALL(glDrawElements(GL_TRIANGLES, m_RenderQuad.GetIndicesSize(), GL_UNSIGNED_INT, nullptr));
		m_RenderQuad.GetVertexArray().UnBind();

		m_DownsamplePipeline.SetVec2("srcResolution", bloomMip.size);
		GLCALL(glBindTexture(GL_TEXTURE_2D, bloomMip.texture));
	}

	m_DownsamplePipeline.UnBind();
}

void stw::Renderer::RenderUpsamples(f32 filterRadius)
{
	m_UpsamplePipeline.Bind();
	m_UpsamplePipeline.SetFloat("filterRadius", filterRadius);

	// Enable additive blending
	GLCALL(glEnable(GL_BLEND));
	GLCALL(glBlendFunc(GL_ONE, GL_ONE));
	GLCALL(glBlendEquation(GL_FUNC_ADD));

	const auto mipChain = m_BloomFramebuffer.MipChain();

	for (usize i = mipChain.size() - 1; i > 0; i--)
	{
		const BloomMip& bloomMip = mipChain[i];
		const BloomMip& nextBloomMip = mipChain[i - 1];

		GLCALL(glActiveTexture(GL_TEXTURE0));
		GLCALL(glBindTexture(GL_TEXTURE_2D, bloomMip.texture));

		GLCALL(glViewport(0, 0, nextBloomMip.size.x, nextBloomMip.size.y));
		GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextBloomMip.texture, 0));

		m_RenderQuad.GetVertexArray().Bind();
		GLCALL(glDrawElements(GL_TRIANGLES, m_RenderQuad.GetIndicesSize(), GL_UNSIGNED_INT, nullptr));
		m_RenderQuad.GetVertexArray().UnBind();
	}

	GLCALL(glDisable(GL_BLEND));

	m_UpsamplePipeline.UnBind();
}

stw::SceneGraph& stw::Renderer::GetSceneGraph() { return m_SceneGraph; }

void stw::Renderer::RenderGBuffer()
{
	m_GBufferFramebuffer.Bind();
	GLCALL(glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a));
	GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	m_SceneGraph.ForEach([this](SceneGraphElementIndex elementIndex, std::span<const glm::mat4> transformMatrices) {
		m_MatricesUniformBuffer.Bind();
		auto& material = m_MaterialManager[elementIndex.materialId];

		BindMaterialForGBuffer(material, m_TextureManager, m_GBufferPipeline);

		auto& mesh = m_Meshes[elementIndex.meshId];
		mesh.Bind(transformMatrices);

		const auto size = static_cast<GLsizei>(mesh.GetIndicesSize());
		GLCALL(glDrawElementsInstanced(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr, transformMatrices.size()));

		mesh.UnBind();
		m_MatricesUniformBuffer.UnBind();
	});
	m_GBufferFramebuffer.UnBind();
}

void stw::Renderer::RenderLightsToHdrFramebuffer()
{
	const auto lightViewProjMatrix = GetLightViewProjMatrices();

	if (m_DirectionalLight.has_value())
	{
		RenderShadowMaps(lightViewProjMatrix.value());
	}

	GLCALL(glViewport(0, 0, m_ViewportSize.x, m_ViewportSize.y));

	m_HdrFramebuffer.Bind();
	GLCALL(glDepthMask(GL_FALSE));
	GLCALL(glDisable(GL_DEPTH_TEST));

	GLCALL(glEnable(GL_BLEND));
	GLCALL(glBlendEquation(GL_FUNC_ADD));
	GLCALL(glBlendFunc(GL_ONE, GL_ONE));

	GLCALL(glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a));
	GLCALL(glClear(GL_COLOR_BUFFER_BIT));

	GLCALL(glCullFace(GL_FRONT));
	RenderPointLights();
	GLCALL(glCullFace(GL_BACK));

	if (m_DirectionalLight.has_value())
	{
		RenderDirectionalLight(lightViewProjMatrix.value());
	}

	GLCALL(glDepthMask(GL_TRUE));
	GLCALL(glEnable(GL_DEPTH_TEST));
	GLCALL(glDisable(GL_BLEND));
	m_HdrFramebuffer.UnBind();

	// Copy depth stencil from gbuffer to hdr framebuffer
	m_GBufferFramebuffer.BindRead();
	m_HdrFramebuffer.BindWrite();
	GLCALL(glBlitFramebuffer(0,
		0,
		m_ViewportSize.x,
		m_ViewportSize.y,
		0,
		0,
		m_ViewportSize.x,
		m_ViewportSize.y,
		GL_DEPTH_BUFFER_BIT,
		GL_NEAREST));
	m_HdrFramebuffer.UnBind();
}

void stw::Renderer::RenderPointLights()
{
	m_PointLightPipeline.Bind();
	m_PointLightPipeline.SetVec3("viewPos", m_Camera.GetPosition());
	m_PointLightPipeline.SetVec2("screenSize", m_ViewportSize);

	// GetPosition
	GLCALL(glActiveTexture(GL_TEXTURE0));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(0)));

	// Normal
	GLCALL(glActiveTexture(GL_TEXTURE1));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(1)));

	// Base Color + Specular
	GLCALL(glActiveTexture(GL_TEXTURE2));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(2)));

	for (usize i = 0; i < m_PointLightsCount; i++)
	{
		const PointLight& pointLight = m_PointLights.at(i);
		m_PointLightPipeline.SetVec3("pointLight.position", pointLight.position);
		m_PointLightPipeline.SetFloat("pointLight.linear", pointLight.linear);
		m_PointLightPipeline.SetFloat("pointLight.quadratic", pointLight.quadratic);
		m_PointLightPipeline.SetVec3("pointLight.color", pointLight.color);

		// Render
		glm::mat4 sphereModel{ 1.0f };
		sphereModel = glm::translate(sphereModel, pointLight.position);
		sphereModel = glm::scale(sphereModel, glm::vec3{ pointLight.radius });

		m_PointLightPipeline.SetMat4("modelMatrix", sphereModel);
		m_MatricesUniformBuffer.Bind();

		m_DebugSphereLight.GetVertexArray().Bind();
		GLCALL(glDrawElements(GL_TRIANGLES, m_DebugSphereLight.GetIndicesSize(), GL_UNSIGNED_INT, nullptr));
		m_DebugSphereLight.GetVertexArray().UnBind();
		m_MatricesUniformBuffer.UnBind();
	}
	m_PointLightPipeline.UnBind();
}

void stw::Renderer::RenderDebugLights()
{
	GLCALL(glDepthMask(GL_TRUE));
	GLCALL(glEnable(GL_DEPTH_TEST));
	constexpr f32 debugLightScale = 0.4f;
	m_HdrFramebuffer.Bind();
	m_DebugLightsPipeline.Bind();

	for (usize i = 0; i < m_PointLightsCount; i++)
	{
		m_MatricesUniformBuffer.Bind();
		const auto& pointLight = m_PointLights.at(i);

		m_DebugLightsPipeline.SetVec3("lightColor", pointLight.color);

		glm::mat4 model{ 1.0f };
		model = glm::translate(model, pointLight.position);
		model = glm::scale(model, glm::vec3{ debugLightScale });

		m_DebugSphereLight.Bind({ &model, 1 });
		GLCALL(glDrawElementsInstanced(GL_TRIANGLES, m_DebugSphereLight.GetIndicesSize(), GL_UNSIGNED_INT, nullptr, 1));
		m_DebugSphereLight.UnBind();

		m_MatricesUniformBuffer.UnBind();
	}
}

void stw::Renderer::RenderDirectionalLight(const std::array<glm::mat4, ShadowMapNumCascades>& lightViewProjMatrices)
{
	m_DirectionalLightPipeline.Bind();
	m_DirectionalLightPipeline.SetVec3("viewPos", m_Camera.GetPosition());
	m_DirectionalLightPipeline.SetMat4("viewMatrix", m_Camera.GetViewMatrix());

	m_DirectionalLightPipeline.SetVec4(
		"csmFarDistances", glm::vec4{ m_Intervals[0], m_Intervals[1], m_Intervals[2], m_Intervals[3] });

	for (usize i = 0; i < lightViewProjMatrices.size(); i++)
	{
		m_DirectionalLightPipeline.SetMat4(fmt::format("lightViewProjMatrix[{}]", i), lightViewProjMatrices.at(i));
	}

	// GetPosition
	GLCALL(glActiveTexture(GL_TEXTURE0));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(0)));

	// Normal
	GLCALL(glActiveTexture(GL_TEXTURE1));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(1)));

	// Base Color + Specular
	GLCALL(glActiveTexture(GL_TEXTURE2));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(2)));

	// Shadow map
	for (usize i = 0; i < m_LightDepthMapFramebuffers.size(); i++)
	{
		GLCALL(glActiveTexture(GL_TEXTURE4 + i));
		GLCALL(glBindTexture(GL_TEXTURE_2D, m_LightDepthMapFramebuffers.at(i).GetDepthStencilAttachment().value()));
	}

	const DirectionalLight& directionalLight = m_DirectionalLight.value();

	m_DirectionalLightPipeline.SetVec3("directionalLight.direction", directionalLight.direction);
	m_DirectionalLightPipeline.SetVec3("directionalLight.color", directionalLight.color);

	m_RenderQuad.GetVertexArray().Bind();
	GLCALL(glDrawElements(GL_TRIANGLES, m_RenderQuad.GetIndicesSize(), GL_UNSIGNED_INT, nullptr));
	m_RenderQuad.GetVertexArray().UnBind();

	m_DirectionalLightPipeline.UnBind();
}

stw::Renderer::Renderer(stw::Camera& camera) : m_Camera(camera) {}

std::optional<std::array<glm::mat4, stw::ShadowMapNumCascades>> stw::Renderer::GetLightViewProjMatrices()
{
	if (!m_DirectionalLight)
	{
		return std::nullopt;
	}

	//	if (m_Camera.GetPosition() == m_OldCamViewPos)
	//	{
	//		return m_LightViewProjMatrices;
	//	}
	//	m_OldCamViewPos = m_Camera.GetPosition();

	std::array<glm::mat4, ShadowMapNumCascades> lightViewMatrices{};
	for (usize i = 0; i < m_Intervals.size(); i++)
	{
		const auto interval = m_Intervals.at(i);
		if (i == 0)
		{
			lightViewMatrices.at(i) = ComputeLightViewProjMatrix(NearPlane, interval);
		}
		else if (i < m_Intervals.size())
		{
			lightViewMatrices.at(i) = ComputeLightViewProjMatrix(m_Intervals.at(i - 1), interval);
		}
		else
		{
			lightViewMatrices.at(i) = ComputeLightViewProjMatrix(m_Intervals.at(i - 1), FarPlane);
		}
	}

	//	m_LightViewProjMatrices = lightViewMatrices;
	return lightViewMatrices;
}

glm::mat4 stw::Renderer::ComputeLightViewProjMatrix(f32 nearPlane, f32 farPlane)
{
	constexpr glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	const glm::mat4 proj =
		glm::perspective(glm::radians(m_Camera.GetFovY()), m_Camera.GetAspectRatio(), nearPlane, farPlane);

	const auto frustumCorners = ComputeFrustumCorners(proj, m_Camera.GetViewMatrix());

	//	const glm::vec3 target = m_Camera.GetPosition() + m_Camera.GetFront() * NearPlane;
	//	const glm::vec3 lightPosition = target - m_DirectionalLight.value().direction;
	//	const glm::mat4 lightView = glm::lookAt(lightPosition, target, up);

	glm::vec3 center{ 0.0f };
	for (const auto& v : frustumCorners)
	{
		center += glm::vec3(v);
	}
	center /= frustumCorners.size();

	const auto lightView = glm::lookAt(center - m_DirectionalLight.value().direction, center, up);

	f32 minX = (std::numeric_limits<float>::max)();
	f32 maxX = std::numeric_limits<float>::lowest();
	f32 minY = (std::numeric_limits<float>::max)();
	f32 maxY = std::numeric_limits<float>::lowest();
	f32 minZ = (std::numeric_limits<float>::max)();
	f32 maxZ = std::numeric_limits<float>::lowest();
	for (const auto& v : frustumCorners)
	{
		const auto trf = lightView * glm::vec4{ v, 1.0f };
		minX = (std::min)(minX, trf.x);
		maxX = (std::max)(maxX, trf.x);
		minY = (std::min)(minY, trf.y);
		maxY = (std::max)(maxY, trf.y);

		minZ = (std::min)(minZ, trf.z);
		maxZ = (std::max)(maxZ, trf.z);
	}

	// We set the size of the shadow frustum to be the size of the biggest diagonal in the camera frustum
	//	f32 diagLength = glm::distance(frustumCorners.front(), frustumCorners.back());
	//	spdlog::debug("DiagLength : {}", diagLength);

	//	const f32 midX = minX + (maxX - minX) / 2.0f;
	//	const f32 midY = minY + (maxY - minY) / 2.0f;
	//	const f32 midZ = minZ + (maxZ - minZ) / 2.0f;

	//	glm::vec3 minLightProj{ midX - diagLength, midY - diagLength, midZ - diagLength };
	//	glm::vec3 maxLightProj{ midX + diagLength, midY + diagLength, midZ + diagLength };

	glm::vec3 minLightProj{ minX, minY, minZ };
	glm::vec3 maxLightProj{ maxX, maxY, maxZ };

	constexpr float zMultiplier = 10.0f;
	if (minLightProj.z < 0)
	{
		minLightProj.z *= zMultiplier;
	}
	else
	{
		minLightProj.z /= zMultiplier;
	}

	if (maxLightProj.z < 0)
	{
		maxLightProj.z /= zMultiplier;
	}
	else
	{
		maxLightProj.z *= zMultiplier;
	}

	const glm::mat4 lightProjection =
		glm::ortho(minLightProj.x, maxLightProj.x, minLightProj.y, maxLightProj.y, minLightProj.z, maxLightProj.z);

	return lightProjection * lightView;
}

stw::PointLight::PointLight(glm::vec3 position, f32 linear, f32 quadratic, glm::vec3 color)
	: position(position), linear(linear), quadratic(quadratic), color(color)
{
	const f32 maxColor = std::fmax(std::fmax(color.r, color.g), color.b);
	radius =
		(-linear + std::sqrtf(linear * linear - 4.0f * quadratic * (1.0f - (256.0f / MinLightIntensity) * maxColor)))
		/ (2 * quadratic);
}
