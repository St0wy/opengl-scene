#include "ogl/renderer.hpp"

#include <queue>
#include <assimp/Importer.hpp>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include "timer.hpp"
#include "utils.hpp"
#include <assimp/postprocess.h>

stw::Renderer::~Renderer()
{
	if (m_IsInitialized)
	{
		spdlog::error("Destructor called on renderer that is still initialized");
	}
}

void stw::Renderer::Init(const glm::uvec2 screenSize)
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	m_IsInitialized = true;

	m_MatricesUniformBuffer.Init(0);
	m_MatricesUniformBuffer.Bind();
	constexpr GLsizeiptr matricesSize = 2 * sizeof(glm::mat4);
	m_MatricesUniformBuffer.Allocate(matricesSize);

	m_SceneGraph.Init();

	m_DebugSphereLight = Mesh::CreateUvSphere(1.0f, 20, 20);
	m_RenderQuad = Mesh::CreateQuad();

	InitPipelines();

	InitFramebuffers(screenSize);
	SetViewport({ 0, 0 }, screenSize);

	m_Intervals = ComputeCascades();
	InitSsao();
	InitSkybox();
}

void stw::Renderer::InitPipelines()
{
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

	m_GBufferPipeline.InitFromPath("shaders/pbr/gbuffer.vert", "shaders/pbr/gbuffer.frag");
	m_GBufferPipeline.Bind();
	m_GBufferPipeline.SetInt("texture_base_color", 0);
	m_GBufferPipeline.SetInt("texture_normal", 1);
	m_GBufferPipeline.SetInt("texture_ambient_occlusion", 2);
	m_GBufferPipeline.SetInt("texture_roughness", 3);
	m_GBufferPipeline.SetInt("texture_metallic", 4);
	m_GBufferPipeline.UnBind();

	m_GBufferNoAoPipeline.InitFromPath("shaders/pbr/gbuffer.vert", "shaders/pbr/gbuffer_no_ao.frag");
	m_GBufferNoAoPipeline.Bind();
	m_GBufferNoAoPipeline.SetInt("texture_base_color", 0);
	m_GBufferNoAoPipeline.SetInt("texture_normal", 1);
	m_GBufferNoAoPipeline.SetInt("texture_roughness", 2);
	m_GBufferNoAoPipeline.SetInt("texture_metallic", 3);
	m_GBufferNoAoPipeline.UnBind();

	m_GBufferArmPipeline.InitFromPath("shaders/pbr/gbuffer.vert", "shaders/pbr/gbuffer_arm.frag");
	m_GBufferArmPipeline.Bind();
	m_GBufferArmPipeline.SetInt("texture_base_color", 0);
	m_GBufferArmPipeline.SetInt("texture_normal", 1);
	m_GBufferArmPipeline.SetInt("texture_arm", 2);
	m_GBufferArmPipeline.UnBind();

	m_PointLightPipeline.InitFromPath("shaders/deferred/light_pass.vert", "shaders/pbr/point_light_pass.frag");
	m_PointLightPipeline.Bind();
	m_PointLightPipeline.SetInt("gPositionAmbientOcclusion", 0);
	m_PointLightPipeline.SetInt("gNormalRoughness", 1);
	m_PointLightPipeline.SetInt("gBaseColorMetallic", 2);
	m_PointLightPipeline.UnBind();

	m_AmbientIblPipeline.InitFromPath("shaders/quad.vert", "shaders/pbr/ibl_ambient.frag");
	m_AmbientIblPipeline.Bind();
	m_AmbientIblPipeline.SetInt("gPositionAmbientOcclusion", 0);
	m_AmbientIblPipeline.SetInt("gNormalRoughness", 1);
	m_AmbientIblPipeline.SetInt("gBaseColorMetallic", 2);
	m_AmbientIblPipeline.SetInt("gSsao", 3);
	m_AmbientIblPipeline.SetInt("irradianceMap", 4);
	m_AmbientIblPipeline.SetInt("prefilterMap", 5);
	m_AmbientIblPipeline.SetInt("brdfLut", 6);
	m_AmbientIblPipeline.UnBind();

	m_DirectionalLightPipeline.InitFromPath("shaders/quad.vert", "shaders/pbr/directional_light_pass.frag");
	m_DirectionalLightPipeline.Bind();
	m_DirectionalLightPipeline.SetInt("gPositionAmbientOcclusion", 0);
	m_DirectionalLightPipeline.SetInt("gNormalRoughness", 1);
	m_DirectionalLightPipeline.SetInt("gBaseColorMetallic", 2);
	m_DirectionalLightPipeline.SetInt("shadowMaps[0]", 3);
	m_DirectionalLightPipeline.SetInt("shadowMaps[1]", 4);
	m_DirectionalLightPipeline.SetInt("shadowMaps[2]", 5);
	m_DirectionalLightPipeline.SetInt("shadowMaps[3]", 6);
	m_DirectionalLightPipeline.UnBind();

	m_DebugLightsPipeline.InitFromPath("shaders/deferred/debug_light.vert", "shaders/deferred/debug_light.frag");

	m_SsaoPipeline.InitFromPath("shaders/quad.vert", "shaders/ssao/ssao.frag");
	m_SsaoPipeline.Bind();
	m_SsaoPipeline.SetInt("gPositionAmbientOcclusion", 0);
	m_SsaoPipeline.SetInt("gNormalRoughness", 1);
	m_SsaoPipeline.SetInt("texNoise", 2);
	m_SsaoPipeline.UnBind();

	m_SsaoBlurPipeline.InitFromPath("shaders/quad.vert", "shaders/ssao/blur.frag");
	m_SsaoBlurPipeline.Bind();
	m_SsaoBlurPipeline.SetInt("gSsao", 0);
	m_SsaoBlurPipeline.UnBind();

	m_EquirectangularToCubemapPipeline.InitFromPath("shaders/pbr/equirectangular.vert",
		"shaders/pbr/equirectangular.frag");
	m_EquirectangularToCubemapPipeline.Bind();
	m_EquirectangularToCubemapPipeline.SetInt("equirectangularMap", 0);
	m_EquirectangularToCubemapPipeline.UnBind();

	m_CubemapPipeline.InitFromPath("shaders/pbr/cubemap.vert", "shaders/pbr/cubemap.frag");
	m_CubemapPipeline.Bind();
	m_CubemapPipeline.SetInt("environmentMap", 0);
	m_CubemapPipeline.UnBind();

	m_IrradiancePipeline.InitFromPath("shaders/pbr/equirectangular.vert", "shaders/pbr/irradiance_convolution.frag");
	m_IrradiancePipeline.Bind();
	m_IrradiancePipeline.SetInt("environmentMap", 0);
	m_IrradiancePipeline.UnBind();

	m_PrefilterShader.InitFromPath("shaders/pbr/equirectangular.vert", "shaders/pbr/prefilter_convolution.frag");
	m_PrefilterShader.Bind();
	m_PrefilterShader.SetInt("environmentMap", 0);
	m_PrefilterShader.UnBind();

	m_BrdfPipeline.InitFromPath("shaders/quad.vert", "shaders/pbr/brdf.frag");
}

void stw::Renderer::InitFramebuffers(glm::uvec2 screenSize)
{
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

		FramebufferColorAttachment positionAmbientOcclusionAttachment{};
		positionAmbientOcclusionAttachment.format = FramebufferColorAttachment::Format::Rgba;
		positionAmbientOcclusionAttachment.size = FramebufferColorAttachment::Size::Sixteen;
		positionAmbientOcclusionAttachment.type = FramebufferColorAttachment::Type::Float;

		FramebufferColorAttachment normalRoughnessAttachment{};
		normalRoughnessAttachment.format = FramebufferColorAttachment::Format::Rgba;
		normalRoughnessAttachment.size = FramebufferColorAttachment::Size::Sixteen;
		normalRoughnessAttachment.type = FramebufferColorAttachment::Type::Float;

		FramebufferColorAttachment colorMetallicAttachment{};
		colorMetallicAttachment.format = FramebufferColorAttachment::Format::Rgba;
		colorMetallicAttachment.size = FramebufferColorAttachment::Size::Sixteen;
		colorMetallicAttachment.type = FramebufferColorAttachment::Type::Float;

		FramebufferDescription framebufferDescription{};
		framebufferDescription.depthStencilAttachment = depthStencilAttachment;
		framebufferDescription.colorAttachmentsCount = 3;
		framebufferDescription.colorAttachments[0] = positionAmbientOcclusionAttachment;
		framebufferDescription.colorAttachments[1] = normalRoughnessAttachment;
		framebufferDescription.colorAttachments[2] = colorMetallicAttachment;
		framebufferDescription.framebufferSize = screenSize;
		m_GBufferFramebuffer.Init(framebufferDescription);
	}
	{
		FramebufferColorAttachment colorAttachment{};
		colorAttachment.format = FramebufferColorAttachment::Format::Red;
		colorAttachment.size = FramebufferColorAttachment::Size::Eight;
		colorAttachment.type = FramebufferColorAttachment::Type::Float;

		FramebufferDescription framebufferDescription{};
		framebufferDescription.colorAttachmentsCount = 1;
		framebufferDescription.colorAttachments[0] = colorAttachment;
		framebufferDescription.framebufferSize = screenSize;
		m_SsaoFramebuffer.Init(framebufferDescription);
		m_SsaoBlurFramebuffer.Init(framebufferDescription);
	}
	{
		FramebufferDepthStencilAttachment depthStencilAttachment{};
		depthStencilAttachment.isRenderbufferObject = true;
		depthStencilAttachment.hasStencil = false;

		FramebufferDescription framebufferDescription{};
		framebufferDescription.depthStencilAttachment = depthStencilAttachment;
		framebufferDescription.framebufferSize = glm::uvec2{ SkyboxResolution, SkyboxResolution };

		m_SkyboxCaptureFramebuffer.Init(framebufferDescription);
		m_SkyboxCaptureFramebuffer.Bind();
		constexpr GLenum buf = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(static_cast<GLsizei>(1), &buf);
	}
	{
		FramebufferDepthStencilAttachment depthStencilAttachment{};
		depthStencilAttachment.isRenderbufferObject = true;
		depthStencilAttachment.hasStencil = false;

		FramebufferColorAttachment brdfLutAttachment{};
		brdfLutAttachment.size = FramebufferColorAttachment::Size::Sixteen;
		brdfLutAttachment.type = FramebufferColorAttachment::Type::Float;
		brdfLutAttachment.format = FramebufferColorAttachment::Format::Rg;

		FramebufferDescription framebufferDescription{};
		framebufferDescription.depthStencilAttachment = depthStencilAttachment;
		framebufferDescription.colorAttachmentsCount = 1;
		framebufferDescription.colorAttachments[0] = brdfLutAttachment;
		framebufferDescription.framebufferSize = glm::uvec2{ BrdfLutResolution };

		m_BrdfFramebuffer.Init(framebufferDescription);
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
	}

	const bool success = m_BloomFramebuffer.Init(screenSize, MipChainLength);

	if (!success)
	{
		spdlog::error("Could not successfully initialize bloom framebuffer");
		assert(false);
	}
}

void stw::Renderer::InitSsao()
{
	m_SsaoKernel = GenerateSsaoKernel();
	m_SsaoRandomTexture = GenerateSsaoRandomTexture();
	glGenTextures(1, &m_SsaoGlRandomTexture);
	glBindTexture(GL_TEXTURE_2D, m_SsaoGlRandomTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, m_SsaoRandomTexture.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void stw::Renderer::InitSkybox()
{
	glGenTextures(1, &m_EnvironmentCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvironmentCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		// note that we store each face with 16 bit floating point values
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			SkyboxResolution,
			SkyboxResolution,
			0,
			GL_RGB,
			GL_FLOAT,
			nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	m_CubemapMesh = Mesh::CreateInsideCube();

	const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	const std::array captureViews = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) };

	auto loadResult = Texture::LoadRadianceMapFromPath("data/kloofendal_overcast_4k.hdr");

	if (!loadResult)
	{
		spdlog::error(loadResult.error());
	}

	m_HdrTexture = std::move(loadResult.value());

	glActiveTexture(GL_TEXTURE0);
	m_HdrTexture.Bind();
	m_SkyboxCaptureFramebuffer.Bind();

	m_EquirectangularToCubemapPipeline.Bind();
	m_EquirectangularToCubemapPipeline.SetMat4("projection", captureProjection);

	glViewport(0, 0, SkyboxResolution, SkyboxResolution);
	for (usize i = 0; i < 6; i++)
	{
		m_EquirectangularToCubemapPipeline.SetMat4("view", captureViews.at(i));
		glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i),
			m_EnvironmentCubemap,
			0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_CubemapMesh.GetVertexArray().Bind();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_CubemapMesh.GetIndicesSize()), GL_UNSIGNED_INT, nullptr);
		m_CubemapMesh.GetVertexArray().UnBind();
	}
	m_EquirectangularToCubemapPipeline.UnBind();
	m_SkyboxCaptureFramebuffer.UnBind();

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvironmentCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glGenTextures(1, &m_IrradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);
	for (u32 i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			IrradianceMapResolution,
			IrradianceMapResolution,
			0,
			GL_RGB,
			GL_FLOAT,
			nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	m_SkyboxCaptureFramebuffer.Bind();
	m_SkyboxCaptureFramebuffer.Resize(glm::uvec2{ IrradianceMapResolution });
	m_SkyboxCaptureFramebuffer.Bind();
	constexpr GLenum buf = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &buf);

	m_IrradiancePipeline.Bind();
	m_IrradiancePipeline.SetMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvironmentCubemap);

	glViewport(0, 0, IrradianceMapResolution, IrradianceMapResolution);
	m_SkyboxCaptureFramebuffer.Bind();
	for (u32 i = 0; i < 6; ++i)
	{
		m_IrradiancePipeline.SetMat4("view", captureViews.at(i));
		glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			m_IrradianceMap,
			0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_CubemapMesh.GetVertexArray().Bind();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_CubemapMesh.GetIndicesSize()), GL_UNSIGNED_INT, nullptr);
		m_CubemapMesh.GetVertexArray().UnBind();
	}
	m_SkyboxCaptureFramebuffer.UnBind();
	m_IrradiancePipeline.UnBind();

	glGenTextures(1, &m_PrefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			PrefilterMapResolution,
			PrefilterMapResolution,
			0,
			GL_RGB,
			GL_FLOAT,
			nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	m_PrefilterShader.Bind();
	m_PrefilterShader.SetMat4("projection", captureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvironmentCubemap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	m_SkyboxCaptureFramebuffer.Bind();

	constexpr u32 maxMipLevels = 5;
	for (u32 mip = 0; mip < maxMipLevels; mip++)
	{
		const u32 mipSize = static_cast<u32>(PrefilterMapResolution * std::pow(0.5f, mip));
		m_SkyboxCaptureFramebuffer.Resize(glm::uvec2{ mipSize });
		m_SkyboxCaptureFramebuffer.Bind();
		constexpr GLenum col = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &col);

		glViewport(0, 0, static_cast<GLsizei>(mipSize), static_cast<GLsizei>(mipSize));

		const f32 roughness = static_cast<f32>(mip) / static_cast<f32>((maxMipLevels - 1));
		m_PrefilterShader.SetFloat("roughness", roughness);
		for (u32 i = 0; i < 6; ++i)
		{
			m_PrefilterShader.SetMat4("view", captureViews.at(i));
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				m_PrefilterMap,
				static_cast<GLint>(mip));

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_CubemapMesh.GetVertexArray().Bind();
			glDrawElements(GL_TRIANGLES,
				static_cast<GLsizei>(m_CubemapMesh.GetIndicesSize()),
				GL_UNSIGNED_INT,
				nullptr);
			m_CubemapMesh.GetVertexArray().UnBind();
		}
	}
	m_PrefilterShader.UnBind();
	m_SkyboxCaptureFramebuffer.UnBind();

	m_BrdfFramebuffer.Bind();
	glViewport(0, 0, BrdfLutResolution, BrdfLutResolution);
	m_BrdfPipeline.Bind();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_RenderQuad.GetVertexArray().Bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_RenderQuad.GetIndicesSize()), GL_UNSIGNED_INT, nullptr);
	m_RenderQuad.GetVertexArray().UnBind();

	m_BrdfPipeline.UnBind();
	m_BrdfFramebuffer.UnBind();

	glViewport(0, 0, static_cast<GLsizei>(m_ViewportSize.x), static_cast<GLsizei>(m_ViewportSize.y));
}

void stw::Renderer::DrawScene()
{
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	RenderGBuffer();

	RenderSsao();

	RenderLightsToHdrFramebuffer();

	RenderDebugLights();

	RenderCubemap();

	RenderBloomToBloomFramebuffer(m_HdrFramebuffer.GetColorAttachment(0), FilterRadius);

	m_HdrPipeline.Bind();
	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_HdrFramebuffer.GetColorAttachment(0));

	glActiveTexture(GL_TEXTURE1);
	const GLuint bloomTexture = m_BloomFramebuffer.MipChain()[0].texture;
	glBindTexture(GL_TEXTURE_2D, bloomTexture);

	m_RenderQuad.GetVertexArray().Bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_RenderQuad.GetIndicesSize()), GL_UNSIGNED_INT, nullptr);
	m_RenderQuad.GetVertexArray().UnBind();
	glEnable(GL_DEPTH_TEST);
}

void stw::Renderer::RenderGBuffer()
{
	m_GBufferFramebuffer.Bind();
	glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto renderLambda = [this](const SceneGraphElementIndex elementIndex,
		const std::span<const glm::mat4> transformMatrices) {
		m_MatricesUniformBuffer.Bind();
		const auto& material = m_MaterialManager[elementIndex.materialId];

		BindMaterialForGBuffer(material,
			m_TextureManager,
			{ m_GBufferPipeline, m_GBufferNoAoPipeline, m_GBufferArmPipeline });

		const auto& mesh = m_Meshes[elementIndex.meshId];
		mesh.Bind(transformMatrices);

		const auto size = static_cast<GLsizei>(mesh.GetIndicesSize());
		glDrawElementsInstanced(GL_TRIANGLES,
			size,
			GL_UNSIGNED_INT,
			nullptr,
			static_cast<GLsizei>(transformMatrices.size()));

		mesh.UnBind();
		m_MatricesUniformBuffer.UnBind();
	};

	m_SceneGraph.ForEach(renderLambda);
	m_GBufferFramebuffer.UnBind();
}

void stw::Renderer::RenderSsao()
{
	m_SsaoFramebuffer.Bind();
	Clear(GL_COLOR_BUFFER_BIT);

	m_SsaoPipeline.Bind();
	m_SsaoPipeline.SetVec2("screenSize", m_ViewportSize);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(0));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(1));
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_SsaoGlRandomTexture);

	m_SsaoPipeline.SetVec3V("samples", m_SsaoKernel);

	m_RenderQuad.GetVertexArray().Bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_RenderQuad.GetIndicesSize()), GL_UNSIGNED_INT, nullptr);
	m_RenderQuad.GetVertexArray().UnBind();

	m_SsaoPipeline.UnBind();
	m_SsaoFramebuffer.UnBind();

	m_SsaoBlurFramebuffer.Bind();
	m_SsaoBlurPipeline.Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_SsaoFramebuffer.GetColorAttachment(0));

	m_RenderQuad.GetVertexArray().Bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_RenderQuad.GetIndicesSize()), GL_UNSIGNED_INT, nullptr);
	m_RenderQuad.GetVertexArray().UnBind();

	m_SsaoBlurFramebuffer.UnBind();
	m_SsaoBlurPipeline.UnBind();
}

void stw::Renderer::RenderLightsToHdrFramebuffer()
{
	const auto lightViewProjMatrices = GetLightViewProjMatrices();

	if (m_DirectionalLight.has_value() && lightViewProjMatrices.has_value())
	{
		RenderShadowMaps(lightViewProjMatrices.value());
	}

	glViewport(0, 0, static_cast<GLsizei>(m_ViewportSize.x), static_cast<GLsizei>(m_ViewportSize.y));

	m_HdrFramebuffer.Bind();
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);

	RenderAmbient();

	glCullFace(GL_FRONT);
	RenderPointLights();
	glCullFace(GL_BACK);

	if (m_DirectionalLight.has_value() && lightViewProjMatrices.has_value())
	{
		RenderDirectionalLight(lightViewProjMatrices.value());
	}

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	m_HdrFramebuffer.UnBind();

	// Copy depth stencil from gbuffer to hdr framebuffer
	m_GBufferFramebuffer.BindRead();
	m_HdrFramebuffer.BindWrite();
	glBlitFramebuffer(0,
		0,
		static_cast<GLint>(m_ViewportSize.x),
		static_cast<GLint>(m_ViewportSize.y),
		0,
		0,
		static_cast<GLint>(m_ViewportSize.x),
		static_cast<GLint>(m_ViewportSize.y),
		GL_DEPTH_BUFFER_BIT,
		GL_NEAREST);
	m_HdrFramebuffer.UnBind();
}

void stw::Renderer::RenderShadowMaps(const std::array<glm::mat4, ShadowMapNumCascades>& lightViewProjMatrices)
{
	glCullFace(GL_FRONT);
	for (usize i = 0; i < lightViewProjMatrices.size(); i++)
	{
		glEnable(GL_DEPTH_CLAMP);
		m_DepthPipeline.Bind();
		m_DepthPipeline.SetMat4("lightViewProjMatrix", lightViewProjMatrices.at(i));

		glViewport(0, 0, ShadowMapSize, ShadowMapSize);
		m_LightDepthMapFramebuffers.at(i).Bind();
		m_MatricesUniformBuffer.Bind();

		Clear(GL_DEPTH_BUFFER_BIT);
		// Render meshes on light depth buffer
		m_SceneGraph.ForEach([this](SceneGraphElementIndex elementIndex, std::span<const glm::mat4> transformMatrices) {
			m_MatricesUniformBuffer.Bind();
			const auto& mesh = m_Meshes[elementIndex.meshId];
			mesh.Bind(transformMatrices);

			const auto indicesSize = static_cast<GLsizei>(mesh.GetIndicesSize());
			glDrawElementsInstanced(GL_TRIANGLES,
				indicesSize,
				GL_UNSIGNED_INT,
				nullptr,
				static_cast<GLsizei>(transformMatrices.size()));

			mesh.UnBind();
			m_MatricesUniformBuffer.UnBind();
		});

		m_MatricesUniformBuffer.UnBind();
		m_DepthPipeline.UnBind();
		m_LightDepthMapFramebuffers.at(i).UnBind();
		glDisable(GL_DEPTH_CLAMP);
	}
	glCullFace(GL_BACK);
}

void stw::Renderer::RenderAmbient()
{
	m_AmbientIblPipeline.Bind();

	// Position + Ambient Occlusion
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(0));

	// Normal + Roughness
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(1));

	// Base Color + Metallic
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(2));

	// SSAO
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_SsaoBlurFramebuffer.GetColorAttachment(0));

	// Irradiance Map
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);

	// Prefilter Map
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);

	// BRDF Lut
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, m_BrdfFramebuffer.GetColorAttachment(0));

	m_RenderQuad.GetVertexArray().Bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_RenderQuad.GetIndicesSize()), GL_UNSIGNED_INT, nullptr);
	m_RenderQuad.GetVertexArray().UnBind();

	m_AmbientIblPipeline.UnBind();
}

void stw::Renderer::RenderPointLights()
{
	m_PointLightPipeline.Bind();
	m_PointLightPipeline.SetVec2("screenSize", m_ViewportSize);

	// Position + Ambient Occlusion
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(0));

	// Normal + Roughness
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(1));

	// Base Color + Metallic
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(2));

	for (usize i = 0; i < m_PointLightsCount; i++)
	{
		const PointLight& pointLight = m_PointLights.at(i);
		m_PointLightPipeline.SetVec3("pointLight.position",
			glm::vec3(m_Camera->GetViewMatrix() * glm::vec4(pointLight.position, 1.0f)));
		m_PointLightPipeline.SetVec3("pointLight.color", pointLight.color);

		// Render
		glm::mat4 sphereModel{ 1.0f };
		sphereModel = glm::translate(sphereModel, pointLight.position);
		sphereModel = glm::scale(sphereModel, glm::vec3{ pointLight.radius });

		m_PointLightPipeline.SetMat4("modelMatrix", sphereModel);
		m_MatricesUniformBuffer.Bind();

		m_DebugSphereLight.GetVertexArray().Bind();
		glDrawElements(GL_TRIANGLES,
			static_cast<GLsizei>(m_DebugSphereLight.GetIndicesSize()),
			GL_UNSIGNED_INT,
			nullptr);
		m_DebugSphereLight.GetVertexArray().UnBind();
		m_MatricesUniformBuffer.UnBind();
	}
	m_PointLightPipeline.UnBind();
}

void stw::Renderer::RenderDirectionalLight(const std::array<glm::mat4, ShadowMapNumCascades>& lightViewProjMatrices)
{
	m_DirectionalLightPipeline.Bind();
	//	m_DirectionalLightPipeline.SetVec3("viewPos", m_Camera->GetPosition());
	m_MatricesUniformBuffer.Bind();

	m_DirectionalLightPipeline.SetVec4("csmFarDistances",
		glm::vec4{ m_Intervals[0], m_Intervals[1], m_Intervals[2], m_Intervals[3] });

	for (usize i = 0; i < lightViewProjMatrices.size(); i++)
	{
		m_DirectionalLightPipeline.SetMat4(fmt::format("lightViewProjMatrix[{}]", i), lightViewProjMatrices.at(i));
	}

	// GetPosition
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(0));

	// Normal
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(1));

	// Base Color + Specular
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GBufferFramebuffer.GetColorAttachment(2));

	// Shadow map
	for (usize i = 0; i < m_LightDepthMapFramebuffers.size(); i++)
	{
		glActiveTexture(static_cast<GLenum>(GL_TEXTURE3 + i));
		const auto& depthStencilAttachment = m_LightDepthMapFramebuffers.at(i).GetDepthStencilAttachment();
		if (!depthStencilAttachment)
		{
			spdlog::error("No depth stencil attachment... {} {}", __FILE__, __LINE__);
			continue;
		}

		glBindTexture(GL_TEXTURE_2D, depthStencilAttachment.value());
	}

	const DirectionalLight& directionalLight = m_DirectionalLight.value();

	m_DirectionalLightPipeline.SetVec3("directionalLight.direction",
		glm::vec3(m_Camera->GetViewMatrix() * glm::vec4(directionalLight.direction, 1.0f)));
	m_DirectionalLightPipeline.SetVec3("directionalLight.color", directionalLight.color);

	m_RenderQuad.GetVertexArray().Bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_RenderQuad.GetIndicesSize()), GL_UNSIGNED_INT, nullptr);
	m_RenderQuad.GetVertexArray().UnBind();
	m_MatricesUniformBuffer.UnBind();
	m_DirectionalLightPipeline.UnBind();
}

void stw::Renderer::RenderDebugLights()
{
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	m_HdrFramebuffer.Bind();
	m_DebugLightsPipeline.Bind();

	for (usize i = 0; i < m_PointLightsCount; i++)
	{
		constexpr f32 debugLightScale = 0.4f;
		m_MatricesUniformBuffer.Bind();
		const auto& pointLight = m_PointLights.at(i);

		m_DebugLightsPipeline.SetVec3("lightColor", pointLight.color);

		glm::mat4 model{ 1.0f };
		model = glm::translate(model, pointLight.position);
		model = glm::scale(model, glm::vec3{ debugLightScale });

		m_DebugSphereLight.Bind({ &model, 1 });
		glDrawElementsInstanced(GL_TRIANGLES,
			static_cast<GLsizei>(m_DebugSphereLight.GetIndicesSize()),
			GL_UNSIGNED_INT,
			nullptr,
			1);
		m_DebugSphereLight.UnBind();

		m_MatricesUniformBuffer.UnBind();
	}
}

void stw::Renderer::RenderBloomToBloomFramebuffer(const GLuint hdrTexture, const f32 filterRadius)
{
	m_BloomFramebuffer.Bind();
	RenderDownsamples(hdrTexture);
	RenderUpsamples(filterRadius);
	m_BloomFramebuffer.UnBind();
	glViewport(0, 0, static_cast<GLsizei>(m_ViewportSize.x), static_cast<GLsizei>(m_ViewportSize.y));
}

void stw::Renderer::RenderDownsamples(const GLuint hdrTexture)
{
	m_DownsamplePipeline.Bind();
	m_DownsamplePipeline.SetVec2("srcResolution", glm::vec2(m_ViewportSize));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	for (const BloomMip& bloomMip : m_BloomFramebuffer.MipChain())
	{
		glViewport(0, 0, bloomMip.intSize.x, bloomMip.intSize.y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomMip.texture, 0);

		// Render current mip
		m_RenderQuad.GetVertexArray().Bind();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_RenderQuad.GetIndicesSize()), GL_UNSIGNED_INT, nullptr);
		m_RenderQuad.GetVertexArray().UnBind();

		m_DownsamplePipeline.SetVec2("srcResolution", bloomMip.size);
		glBindTexture(GL_TEXTURE_2D, bloomMip.texture);
	}

	m_DownsamplePipeline.UnBind();
}

void stw::Renderer::RenderUpsamples(const f32 filterRadius)
{
	m_UpsamplePipeline.Bind();
	m_UpsamplePipeline.SetFloat("filterRadius", filterRadius);

	// Enable additive blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	const auto mipChain = m_BloomFramebuffer.MipChain();

	for (usize i = mipChain.size() - 1; i > 0; i--)
	{
		const BloomMip& bloomMip = mipChain[i];
		const BloomMip& nextBloomMip = mipChain[i - 1];

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bloomMip.texture);

		glViewport(0, 0, nextBloomMip.intSize.x, nextBloomMip.intSize.y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextBloomMip.texture, 0);

		m_RenderQuad.GetVertexArray().Bind();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_RenderQuad.GetIndicesSize()), GL_UNSIGNED_INT, nullptr);
		m_RenderQuad.GetVertexArray().UnBind();
	}

	glDisable(GL_BLEND);

	m_UpsamplePipeline.UnBind();
}

void stw::Renderer::RenderCubemap()
{
	m_HdrFramebuffer.Bind();
	m_CubemapPipeline.Bind();
	m_MatricesUniformBuffer.Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvironmentCubemap);

	m_CubemapMesh.GetVertexArray().Bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_CubemapMesh.GetIndicesSize()), GL_UNSIGNED_INT, nullptr);
	m_CubemapMesh.GetVertexArray().UnBind();

	m_MatricesUniformBuffer.UnBind();
	m_CubemapPipeline.UnBind();
	m_HdrFramebuffer.UnBind();
}

#pragma region Osef

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
	glDepthFunc(depthFunction);
}

void stw::Renderer::SetEnableCullFace(const bool enableCullFace)
{
	SetOpenGlCapability(enableCullFace, GL_CULL_FACE, m_EnableCullFace);
}

[[maybe_unused]] void stw::Renderer::SetCullFace(const GLenum cullFace)
{
	m_CullFace = cullFace;
	glCullFace(cullFace);
}

[[maybe_unused]] void stw::Renderer::SetFrontFace(const GLenum frontFace)
{
	m_FrontFace = frontFace;
	glFrontFace(m_FrontFace);
}

[[maybe_unused]] void stw::Renderer::SetClearColor(const glm::vec4& clearColor)
{
	m_ClearColor = clearColor;
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
}

void stw::Renderer::UpdateProjectionMatrix()
{
	assert(m_IsInitialized);
	m_MatricesUniformBuffer.Bind();
	m_MatricesUniformBuffer.SetSubData(0, sizeof(glm::mat4), value_ptr(m_Camera->GetProjectionMatrix()));
	m_MatricesUniformBuffer.UnBind();
}

void stw::Renderer::UpdateViewMatrix()
{
	assert(m_IsInitialized);
	m_MatricesUniformBuffer.Bind();
	m_MatricesUniformBuffer.SetSubData(sizeof(glm::mat4), sizeof(glm::mat4), value_ptr(m_Camera->GetViewMatrix()));
	m_MatricesUniformBuffer.UnBind();
}

void stw::Renderer::SetViewport(const glm::ivec2 pos, const glm::uvec2 size)
{
	m_ViewportSize = size;
	glViewport(pos.x, pos.y, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));

	m_HdrFramebuffer.Resize(size);
	m_GBufferFramebuffer.Resize(size);
	m_SsaoFramebuffer.Resize(size);
	m_SsaoBlurFramebuffer.Resize(size);
}

void stw::Renderer::Clear(const GLbitfield mask)// NOLINT(readability-convert-member-functions-to-static)
{
	glClear(mask);
}

void stw::Renderer::SetOpenGlCapability(const bool enabled, const GLenum capability, bool& field)
{
	field = enabled;
	if (field)
	{
		glEnable(capability);
	}
	else
	{
		glDisable(capability);
	}
}

#pragma endregion Osef

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
	m_SsaoPipeline.Delete();
	m_SsaoFramebuffer.Delete();
	m_SsaoBlurFramebuffer.Delete();
	m_SsaoBlurPipeline.Delete();
	m_SkyboxCaptureFramebuffer.Delete();
	m_EquirectangularToCubemapPipeline.Delete();
	m_HdrTexture.Delete();
	glDeleteTextures(1, &m_EnvironmentCubemap);
	m_CubemapMesh.Delete();
	m_CubemapPipeline.Delete();
	m_IrradiancePipeline.Delete();
	m_PrefilterShader.Delete();
	glDeleteTextures(1, &m_IrradianceMap);
	glDeleteTextures(1, &m_PrefilterMap);
	m_BrdfPipeline.Delete();
	m_BrdfFramebuffer.Delete();
	m_AmbientIblPipeline.Delete();
	m_GBufferNoAoPipeline.Delete();
	m_GBufferArmPipeline.Delete();
}

[[maybe_unused]] stw::TextureManager& stw::Renderer::GetTextureManager()
{
	return m_TextureManager;
}

std::expected<std::vector<usize>, std::string> stw::Renderer::LoadModel(const std::filesystem::path& path, bool flipUVs)
{
	Assimp::Importer importer;
	u32 assimpImportFlags = aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices |
	                        aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_SortByPType;
	if (flipUVs)
	{
		assimpImportFlags |= aiProcess_FlipUVs;
	}

	const auto pathString = path.string();

	Timer timer;
	timer.Start();

	const aiScene* assimpScene = importer.ReadFile(pathString.c_str(), assimpImportFlags);

	if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
	{
		return std::unexpected(importer.GetErrorString());
	}

	spdlog::info("Assimp imported the model {} in {:0.0f} ms",
		pathString,
		timer.RestartAndGetElapsedTime().GetInMilliseconds());

	auto workingDirectory = path.parent_path();
	const std::size_t materialIndexOffset = m_MaterialManager.Size();
	const std::vector<std::size_t> materialIndicesLoaded = m_MaterialManager.LoadMaterialsFromAssimpScene(assimpScene,
		workingDirectory,
		m_TextureManager);

	spdlog::info("Loaded materials in {:0.0f} ms", timer.RestartAndGetElapsedTime().GetInMilliseconds());

	m_Meshes.reserve(m_Meshes.size() + assimpScene->mNumMeshes);

	std::queue<const aiNode*> assimpNodes;
	assimpNodes.push(assimpScene->mRootNode);
	const std::span assimpSceneMeshes{ assimpScene->mMeshes, assimpScene->mNumMeshes };

	std::vector<usize> addedNodes;

	// Add the node that holds the mesh
	std::optional currentParent = m_SceneGraph.AddElementToRoot(InvalidId, InvalidId, glm::mat4(1.0f));
	addedNodes.push_back(currentParent.value());

	std::optional<usize> currentSibling{};
	while (!assimpNodes.empty())
	{
		const aiNode* currentAssimpNode = assimpNodes.front();
		assimpNodes.pop();
		const std::span nodeMeshIndices{ currentAssimpNode->mMeshes, currentAssimpNode->mNumMeshes };

		// If this node has no mesh, we create an empty one
		if (nodeMeshIndices.empty())
		{
			const usize nodeIndex = m_SceneGraph.AddChild(currentParent.value(),
				InvalidId,
				InvalidId,
				glm::mat4{ 1.0f });
			currentParent = nodeIndex;
			addedNodes.emplace_back(nodeIndex);
		}

		for (const u32 meshIndex : nodeMeshIndices)
		{
			const aiMesh* assimpMesh = assimpSceneMeshes[meshIndex];

			// Check if we loaded this material, if not, go to the next mesh
			if (std::ranges::find(materialIndicesLoaded, assimpMesh->mMaterialIndex) == materialIndicesLoaded.end())
			{
				continue;
			}

			auto processMeshResult = ProcessMesh(assimpMesh, materialIndexOffset, materialIndicesLoaded);

			if (!processMeshResult)
			{
				assert(false);
			}

			auto& [mesh, meshMaterialIndex] = processMeshResult.value();

			m_Meshes.push_back(std::move(mesh));

			const glm::mat4 transformMatrix = ConvertMatAssimpToGlm(currentAssimpNode->mTransformation);

			usize nodeIndex = InvalidId;

			if (currentSibling)
			{
				nodeIndex = m_SceneGraph.AddSibling(currentSibling.value(),
					m_Meshes.size() - 1,
					meshMaterialIndex,
					transformMatrix);
				currentSibling = nodeIndex;
			}

			if (currentParent)
			{
				nodeIndex = m_SceneGraph.AddChild(currentParent.value(),
					m_Meshes.size() - 1,
					meshMaterialIndex,
					transformMatrix);
				currentParent = std::nullopt;
				currentSibling = nodeIndex;
			}

			assert(nodeIndex != InvalidId);
			addedNodes.emplace_back(nodeIndex);
		}

		const std::span nodeChildren{ currentAssimpNode->mChildren, currentAssimpNode->mNumChildren };
		for (usize i = 0; i < nodeChildren.size(); i++)
		{
			// Update current parent with the last added node on the first iteration
			// and if the current node had meshes
			if (i == 0 && !nodeMeshIndices.empty())
			{
				currentParent = addedNodes.size() - 1;
			}
			assimpNodes.push(nodeChildren[i]);
		}
	}

	spdlog::info("Converted model {} in {:0.0f} ms", pathString, timer.RestartAndGetElapsedTime().GetInMilliseconds());

	return { std::move(addedNodes) };
}

std::optional<stw::ProcessMeshResult> stw::Renderer::ProcessMesh(const aiMesh* assimpMesh,
	std::size_t materialIndexOffset,
	const std::vector<std::size_t>& loadedMaterialsIndices)
{
	std::vector<Vertex> vertices{};
	vertices.reserve(assimpMesh->mNumVertices);
	const std::span assimpMeshVertices{ assimpMesh->mVertices, assimpMesh->mNumVertices };
	const std::span assimpMeshNormals{ assimpMesh->mNormals, assimpMesh->mNumVertices };
	const std::span assimpMeshTangents{ assimpMesh->mTangents, assimpMesh->mNumVertices };

	for (std::size_t i = 0; i < assimpMesh->mNumVertices; ++i)
	{
		const aiVector3D meshVertex = assimpMeshVertices[i];
		const aiVector3D meshNormal = assimpMeshNormals[i];
		const aiVector3D meshTangent = assimpMeshTangents[i];

		glm::vec2 textureCoords(0.0f);
		if (assimpMesh->mTextureCoords[0])
		{
			const std::span assimpMeshTextureCoords{ assimpMesh->mTextureCoords[0], assimpMesh->mNumVertices };
			const auto meshTextureCoords = assimpMeshTextureCoords[i];
			textureCoords.x = meshTextureCoords.x;
			textureCoords.y = meshTextureCoords.y;
		}

		const Vertex vertex{ { meshVertex.x, meshVertex.y, meshVertex.z, },
		                     { meshNormal.x, meshNormal.y, meshNormal.z, },
		                     textureCoords,
		                     { meshTangent.x, meshTangent.y, meshTangent.z } };
		vertices.push_back(vertex);
	}

	std::vector<u32> indices{};
	indices.reserve(static_cast<std::size_t>(assimpMesh->mNumFaces) * 3);
	const std::span assimpMeshFaces{ assimpMesh->mFaces, assimpMesh->mNumFaces };
	for (const aiFace& face : assimpMeshFaces)
	{
		const std::span faceIndices{ face.mIndices, face.mNumIndices };
		for (const u32 index : faceIndices)
		{

			indices.push_back(index);
		}
	}

	for (usize i = 0; i < loadedMaterialsIndices.size(); i++)
	{
		if (loadedMaterialsIndices.at(i) == assimpMesh->mMaterialIndex)
		{
			Mesh mesh;

			const std::size_t materialIndex = materialIndexOffset + i;
			mesh.Init(std::move(vertices), std::move(indices));

			return stw::ProcessMeshResult{ std::move(mesh), materialIndex };
		}
	}

	// We should not be here
	spdlog::error("Did not find material index for this mesh {} {}", __LINE__, __FILE__);
	return std::nullopt;
}

void stw::Renderer::SetDirectionalLight(stw::DirectionalLight directionalLight)
{
	if (directionalLight.direction == glm::vec3{ 0.0f, -1.0f, 0.0f })
	{
		directionalLight.direction = glm::normalize(glm::vec3{ 0.0f, -1.0f, 0.000001f });
	}
	m_DirectionalLight.emplace(directionalLight);
}

[[maybe_unused]] void stw::Renderer::RemoveDirectionalLight()
{
	m_DirectionalLight.reset();
}

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

stw::SceneGraph& stw::Renderer::GetSceneGraph()
{
	return m_SceneGraph;
}

stw::Renderer::Renderer(const gsl::not_null<Camera*> camera)
	: m_Camera(camera)
{
}

std::optional<std::array<glm::mat4, stw::ShadowMapNumCascades>> stw::Renderer::GetLightViewProjMatrices()
{
	if (!m_DirectionalLight)
	{
		return std::nullopt;
	}

	const glm::mat4 inverseView = glm::inverse(m_Camera->GetViewMatrix());
	std::array<glm::mat4, stw::ShadowMapNumCascades> lightViewProjMatrices{};
	for (usize i = 0; i < m_Intervals.size(); i++)
	{
		const auto interval = m_Intervals.at(i);
		if (i == 0)
		{
			lightViewProjMatrices.at(i) = ComputeLightViewProjMatrix(NearPlane, interval) * inverseView;
		}
		else if (i < m_Intervals.size())
		{
			lightViewProjMatrices.at(i) = ComputeLightViewProjMatrix(m_Intervals.at(i - 1), interval) * inverseView;
		}
		else
		{
			lightViewProjMatrices.at(i) = ComputeLightViewProjMatrix(m_Intervals.at(i - 1), FarPlane) * inverseView;
		}
	}

	//	m_LightViewProjMatrices = lightViewMatrices;
	return lightViewProjMatrices;
}

glm::mat4 stw::Renderer::ComputeLightViewProjMatrix(f32 nearPlane, f32 farPlane)
{
	constexpr glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	const glm::mat4 proj = glm::perspective(glm::radians(m_Camera->GetFovY()),
		m_Camera->GetAspectRatio(),
		nearPlane,
		farPlane);

	const auto frustumCorners = ComputeFrustumCorners(proj, m_Camera->GetViewMatrix());

	//	const glm::vec3 target = m_Camera->GetPosition() + m_Camera->GetFront() * NearPlane;
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

	f32 midLenX = (maxX - minX) / 2.0f;
	f32 midLenY = (maxY - minY) / 2.0f;
	f32 midLenZ = (maxZ - minZ) / 2.0f;

	const f32 midX = minX + midLenX;
	const f32 midY = minY + midLenY;
	const f32 midZ = minZ + midLenZ;

	constexpr float zMultiplier = 5.0f;
	constexpr f32 xyMargin = 1.1f;
	midLenX *= xyMargin;
	midLenY *= xyMargin;
	midLenZ *= zMultiplier;

	const glm::vec3 minLightProj{ midX - midLenX, midY - midLenY, midZ - midLenZ };
	const glm::vec3 maxLightProj{ midX + midLenX, midY + midLenY, midZ + midLenZ };

	const glm::mat4 lightProjection = glm::ortho(minLightProj.x,
		maxLightProj.x,
		minLightProj.y,
		maxLightProj.y,
		minLightProj.z,
		maxLightProj.z);

	return lightProjection * lightView;
}

stw::PointLight::PointLight(const glm::vec3 position, const glm::vec3 color)
	: position(position), color(color)
{
	const f32 maxColor = std::fmax(std::fmax(color.r, color.g), color.b);
	radius = (std::sqrt(-4.0f * (1.0f - (256.0f / MinLightIntensity) * maxColor))) / (2.0f);
}
