#include "ogl/renderer.hpp"

#include <exception>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <queue>
#include <spdlog/spdlog.h>
#include <unordered_set>

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
	m_HdrPipeline.InitFromPath("shaders/hdr/quad.vert", "shaders/hdr/hdr.frag");
	m_HdrPipeline.Bind();
	m_HdrPipeline.SetInt("hdrBuffer", 0);
	m_HdrPipeline.SetInt("bloomBuffer", 1);
	m_HdrPipeline.UnBind();

	m_DownsamplePipeline.InitFromPath("shaders/hdr/quad.vert", "shaders/bloom/downsample.frag");
	m_DownsamplePipeline.Bind();
	m_DownsamplePipeline.SetInt("srcTexture", 0);
	m_DownsamplePipeline.UnBind();

	m_UpsamplePipeline.InitFromPath("shaders/hdr/quad.vert", "shaders/bloom/upsample.frag");
	m_UpsamplePipeline.Bind();
	m_UpsamplePipeline.SetInt("srcTexture", 0);
	m_UpsamplePipeline.UnBind();

	{
		FramebufferDepthStencilAttachment depthStencilAttachment{};
		depthStencilAttachment.isRenderbufferObject = false;
		depthStencilAttachment.hasStencil = false;
		FramebufferDescription framebufferDescription{};
		framebufferDescription.depthStencilAttachment = depthStencilAttachment;
		framebufferDescription.framebufferSize = ShadowMapSize;
		m_DepthMapFramebuffer.Init(framebufferDescription);
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

	const bool success = m_BloomFramebuffer.Init(screenSize, MipChainLength);

	if (!success)
	{
		spdlog::error("Could not successfully initialize bloom framebuffer");
		assert(false);
	}

	m_RenderQuad = Mesh::CreateQuad();

	SetViewport({ 0, 0 }, screenSize);
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

void stw::Renderer::SetProjectionMatrix(const glm::mat4& projection)
{
	assert(m_IsInitialized);
	m_CameraProjectionMatrix = projection;
	m_MatricesUniformBuffer.Bind();
	m_MatricesUniformBuffer.SetSubData(0, sizeof(glm::mat4), value_ptr(projection));
	m_MatricesUniformBuffer.UnBind();
}

void stw::Renderer::SetViewMatrix(const glm::mat4& view)
{
	assert(m_IsInitialized);
	m_CameraViewMatrix = view;
	m_MatricesUniformBuffer.Bind();
	m_MatricesUniformBuffer.SetSubData(sizeof(glm::mat4), sizeof(glm::mat4), value_ptr(view));
	m_MatricesUniformBuffer.UnBind();
}

void stw::Renderer::SetViewport(const glm::ivec2 pos, const glm::uvec2 size)
{
	m_ViewportSize = size;
	GLCALL(glViewport(pos.x, pos.y, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y)));
	m_HdrFramebuffer.Delete();
	// WET principles...
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
	framebufferDescription.framebufferSize = size;
	m_HdrFramebuffer.Init(framebufferDescription);
}

void stw::Renderer::Clear(const GLbitfield mask)// NOLINT(readability-convert-member-functions-to-static)
{
	GLCALL(glClear(mask));
}

void stw::Renderer::DrawScene()
{
	constexpr float lightNearPlane = 1.0f;
	constexpr float lightFarPlane = 15.0f;
	constexpr glm::vec3 lightPosition = glm::vec3{ 0.0f, 6.0f, 0.0f };
	const glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, lightNearPlane, lightFarPlane);
	const glm::mat4 lightView =
		glm::lookAt(lightPosition, lightPosition + m_DirectionalLight.value().direction, glm::vec3{ 0.0f, 1.0f, 0.0f });
	const glm::mat4 lightViewProjMatrix = lightProjection * lightView;

	if (m_DirectionalLight.has_value())
	{
		RenderShadowMap(lightViewProjMatrix);
	}

	GLCALL(glViewport(0, 0, m_ViewportSize.x, m_ViewportSize.y));

	RenderGeometryToHdrFramebuffer(lightViewProjMatrix);

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
	GLCALL(glEnable(GL_DEPTH_TEST));
}

void stw::Renderer::RenderGeometryToHdrFramebuffer(const glm::mat4& lightSpaceMatrix)
{
	m_HdrFramebuffer.Bind();
	GLCALL(glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a));
	GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	m_SceneGraph.ForEach(
		[this, &lightSpaceMatrix](SceneGraphElementIndex elementIndex, std::span<const glm::mat4> transformMatrices) {
			m_MatricesUniformBuffer.Bind();
			auto& material = m_MaterialManager[elementIndex.materialId];

			BindMaterial(material, m_TextureManager);
			auto pipelineResult = GetPipelineFromMaterial(material);

			if (pipelineResult)
			{
				Pipeline& pipeline = pipelineResult.value();
				pipeline.Bind();
				BindLights(pipeline);
				pipeline.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

				// We assume that the shadow map is the last texture
				auto shadowMapTextureId = static_cast<i32>(pipeline.GetTextureCount() - 1);
				auto textureEnum = GetTextureFromId(shadowMapTextureId);
				GLCALL(glActiveTexture(textureEnum));
				GLCALL(glBindTexture(GL_TEXTURE_2D, m_DepthMapFramebuffer.GetDepthStencilAttachment().value()));
			}

			auto& mesh = m_Meshes[elementIndex.meshId];
			mesh.Bind(transformMatrices);

			const auto size = static_cast<GLsizei>(mesh.GetIndicesSize());
			GLCALL(glDrawElementsInstanced(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr, transformMatrices.size()));

			mesh.UnBind();
			m_MatricesUniformBuffer.UnBind();
		});
	m_HdrFramebuffer.UnBind();
}

void stw::Renderer::RenderShadowMap(const glm::mat4& lightSpaceMatrix)
{
	m_DepthPipeline.Bind();
	m_DepthPipeline.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

	GLCALL(glViewport(0, 0, ShadowMapSize.x, ShadowMapSize.y));
	m_DepthMapFramebuffer.Bind();
	Clear(GL_DEPTH_BUFFER_BIT);
	// Render meshes on light depth buffer
	m_SceneGraph.ForEach([this](SceneGraphElementIndex elementIndex, std::span<const glm::mat4> transformMatrices) {
		m_MatricesUniformBuffer.Bind();
		auto& mesh = m_Meshes[elementIndex.meshId];
		mesh.Bind(transformMatrices);

		const auto indicesSize = static_cast<GLsizei>(mesh.GetIndicesSize());
		GLCALL(glDrawElementsInstanced(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, nullptr, transformMatrices.size()));

		mesh.UnBind();
		m_MatricesUniformBuffer.UnBind();
	});

	m_DepthMapFramebuffer.UnBind();
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
	m_DepthMapFramebuffer.Delete();
	m_HdrFramebuffer.Delete();
	m_HdrPipeline.Delete();
	m_RenderQuad.Delete();

	m_BloomFramebuffer.Delete();
	m_DownsamplePipeline.Delete();
	m_UpsamplePipeline.Delete();
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

std::optional<std::string> stw::Renderer::LoadModel(const std::filesystem::path& path, Pipeline& pipeline)
{
	Assimp::Importer importer;
	constexpr u32 assimpImportFlags = aiProcessPreset_TargetRealtime_Fast;

	const auto pathString = path.string();

	Timer timer;
	timer.Start();

	const aiScene* assimpScene = importer.ReadFile(pathString.c_str(), assimpImportFlags);

	if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
	{
		return { importer.GetErrorString() };
	}

	spdlog::info("Imported model {} in {:0.0f} ms", pathString, timer.GetElapsedTime().GetInMilliseconds());

	auto workingDirectory = path.parent_path();
	const std::size_t materialIndexOffset =
		m_MaterialManager.LoadMaterialsFromAssimpScene(assimpScene, workingDirectory, m_TextureManager, pipeline);

	m_Meshes.reserve(m_Meshes.size() + assimpScene->mNumMeshes);

	std::queue<const aiNode*> assimpNodes;
	assimpNodes.push(assimpScene->mRootNode);
	const std::span<aiMesh*> assimpSceneMeshes{ assimpScene->mMeshes, assimpScene->mNumMeshes };
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
			m_SceneGraph.AddElementToRoot(m_Meshes.size() - 1, meshMaterialIndex, transformMatrix);
		}

		const std::span<aiNode*> nodeChildren{ currentAssimpNode->mChildren, currentAssimpNode->mNumChildren };
		for (const aiNode* child : nodeChildren)
		{
			assimpNodes.push(child);
		}
	}

	spdlog::info("Converted model {} in {:0.0f} ms", pathString, timer.GetElapsedTime().GetInMilliseconds());

	return {};
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
	if (m_PointLightsCount == MaxPointLights)
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

[[maybe_unused]] void stw::Renderer::PushSpotLight(const stw::SpotLight& spotLight)
{
	if (m_SpotLightsCount == MaxSpotLights)
	{
		spdlog::warn("Pushing one too many spot light");
		return;
	}

	m_SpotLights.at(m_SpotLightsCount) = spotLight;
	m_SpotLightsCount++;
}

[[maybe_unused]] void stw::Renderer::PopSpotLight()
{
	if (m_SpotLightsCount == 0)
	{
		spdlog::warn("Popping on too many spot light");
	}

	m_SpotLightsCount--;
}

[[maybe_unused]] void stw::Renderer::SetSpotLight(usize index, const stw::SpotLight& spotLight)
{
	if (index >= m_SpotLightsCount)
	{
		spdlog::error("Invalid spot light index");
	}

	m_SpotLights.at(index) = spotLight;
}

void stw::Renderer::BindLights(stw::Pipeline& pipeline)
{
	if (m_DirectionalLight.has_value())
	{
		auto& directionalLight = m_DirectionalLight.value();
		pipeline.SetVec3("directionalLight.direction", directionalLight.direction);
		pipeline.SetVec3("directionalLight.ambient", directionalLight.ambient);
		pipeline.SetVec3("directionalLight.diffuse", directionalLight.diffuse);
		pipeline.SetVec3("directionalLight.specular", directionalLight.specular);
	}

	pipeline.SetUnsignedInt("pointLightsCount", m_PointLightsCount);
	for (usize i = 0; i < m_PointLightsCount; i++)
	{
		const auto indexedName = fmt::format("pointLights[{}]", i);

		const auto& pointLight = m_PointLights.at(i);
		pipeline.SetVec3(fmt::format("{}.position", indexedName), pointLight.position);
		pipeline.SetVec3(fmt::format("{}.ambient", indexedName), pointLight.ambient);
		pipeline.SetVec3(fmt::format("{}.diffuse", indexedName), pointLight.diffuse);
		pipeline.SetVec3(fmt::format("{}.specular", indexedName), pointLight.specular);
		pipeline.SetFloat(fmt::format("{}.constant", indexedName), pointLight.constant);
		pipeline.SetFloat(fmt::format("{}.linear", indexedName), pointLight.linear);
		pipeline.SetFloat(fmt::format("{}.quadratic", indexedName), pointLight.quadratic);
	}

	pipeline.SetUnsignedInt("spotLightsCount", m_SpotLightsCount);
	for (usize i = 0; i < m_SpotLightsCount; i++)
	{
		const auto indexedName = fmt::format("spotLights[{}]", i);

		const auto& spotLight = m_SpotLights.at(i);
		pipeline.SetVec3(fmt::format("{}.position", indexedName), spotLight.position);
		pipeline.SetVec3(fmt::format("{}.direction", indexedName), spotLight.direction);
		pipeline.SetVec3(fmt::format("{}.ambient", indexedName), spotLight.ambient);
		pipeline.SetVec3(fmt::format("{}.diffuse", indexedName), spotLight.diffuse);
		pipeline.SetVec3(fmt::format("{}.specular", indexedName), spotLight.specular);
		pipeline.SetFloat(fmt::format("{}.constant", indexedName), spotLight.constant);
		pipeline.SetFloat(fmt::format("{}.linear", indexedName), spotLight.linear);
		pipeline.SetFloat(fmt::format("{}.quadratic", indexedName), spotLight.quadratic);
		pipeline.SetFloat(fmt::format("{}.cutOff", indexedName), spotLight.cutOff);
		pipeline.SetFloat(fmt::format("{}.outerCutOff", indexedName), spotLight.outerCutOff);
	}
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
