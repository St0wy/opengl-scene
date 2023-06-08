//
// Created by stowy on 25/05/2023.
//

#pragma once
#include <array>
#include <GL/glew.h>
#include <glm/ext.hpp>
#include <glm/gtx/norm.hpp>

#include "camera.hpp"
#include "model.hpp"
#include "number_types.hpp"
#include "pipeline.hpp"
#include "scene.hpp"
#include "utils.hpp"

namespace stw
{
class CubeMapScene final : public Scene
{
public:
	static constexpr std::array TransparentPositions{
		glm::vec3{-1.5f, 0.0f, -0.48f},
		glm::vec3{1.5f, 0.0f, 0.51f},
		glm::vec3{0.0f, 0.0f, 0.7f},
		glm::vec3{-0.3f, 0.0f, -2.3f},
		glm::vec3{0.5f, 0.0f, -0.6f},
	};

	CubeMapScene()
		: m_GroundModel(Model::LoadFromPath("data/ground/ground.obj").value()),
		m_BackpackModel(Model::LoadFromPath("data/backpack/backpack.obj").value())
	{
		m_Camera.SetMovementSpeed(4.0f);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		m_CubeMap = Texture::LoadCubeMap({
			"data/skybox/right.jpg",
			"data/skybox/left.jpg",
			"data/skybox/top.jpg",
			"data/skybox/bottom.jpg",
			"data/skybox/front.jpg",
			"data/skybox/back.jpg"
		}).value();

		m_Pipeline.InitFromPath("shaders/mesh/mesh.vert", "shaders/mesh/mesh.frag");
		m_PipelineNoSpecular.InitFromPath("shaders/mesh/mesh.vert", "shaders/mesh/mesh_no_specular.frag");
		m_PipelineCubeMap.InitFromPath("shaders/cubemap/cubemap.vert", "shaders/cubemap/cubemap.frag");
		m_PipelineReflection.InitFromPath("shaders/mesh/mesh.vert", "shaders/cubemap/reflection.frag");
		m_PipelineRefraction.InitFromPath("shaders/mesh/mesh.vert", "shaders/cubemap/refraction.frag");

		glGenVertexArrays(1, &m_CubeMapVao);
		glGenBuffers(1, &m_CubeMapVbo);

		glBindVertexArray(m_CubeMapVao);
		glBindBuffer(GL_ARRAY_BUFFER, m_CubeMapVbo);

		constexpr auto size = static_cast<GLsizeiptr>(CubeMapVertices.size() * sizeof(f32));
		glBufferData(GL_ARRAY_BUFFER, size, CubeMapVertices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), nullptr);

		const GLuint cubeMapMatricesUniformBlockIndex = glGetUniformBlockIndex(m_PipelineCubeMap.Id(), "Matrices");
		const GLuint pipelineMatricesUniformBlockIndex = glGetUniformBlockIndex(m_Pipeline.Id(), "Matrices");
		const GLuint noSpecularMatricesUniformBlockIndex =
		glGetUniformBlockIndex(m_PipelineNoSpecular.Id(), "Matrices");
		const GLuint reflectionMatricesUniformBlockIndex =
		glGetUniformBlockIndex(m_PipelineReflection.Id(), "Matrices");
		const GLuint refractionMatricesUniformBlockIndex =
		glGetUniformBlockIndex(m_PipelineRefraction.Id(), "Matrices");

		glUniformBlockBinding(m_PipelineCubeMap.Id(), cubeMapMatricesUniformBlockIndex, 0);
		glUniformBlockBinding(m_PipelineCubeMap.Id(), pipelineMatricesUniformBlockIndex, 0);
		glUniformBlockBinding(m_PipelineCubeMap.Id(), noSpecularMatricesUniformBlockIndex, 0);
		glUniformBlockBinding(m_PipelineCubeMap.Id(), reflectionMatricesUniformBlockIndex, 0);
		glUniformBlockBinding(m_PipelineCubeMap.Id(), refractionMatricesUniformBlockIndex, 0);

		glGenBuffers(1, &m_UboMatrices);
		glBindBuffer(GL_UNIFORM_BUFFER, m_UboMatrices);

		// Allocate buffer memory on the gpu
		constexpr GLsizeiptr matricesSize = 2 * sizeof(glm::mat4);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// Bind buffer to binding point 0
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_UboMatrices, 0, matricesSize);
		UpdateProjection();
	}

	void UpdateProjection() const
	{
		const auto projection = m_Camera.GetProjectionMatrix();
		glBindBuffer(GL_UNIFORM_BUFFER, m_UboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void UpdateView() const
	{
		glm::mat4 view = m_Camera.GetViewMatrix();
		glBindBuffer(GL_UNIFORM_BUFFER, m_UboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void SetupLight(Pipeline& pipeline) const
	{
		pipeline.Use();
		pipeline.SetFloat("material.shininess", 32.0f);

		const glm::mat4 view = m_Camera.GetViewMatrix();

		pipeline.SetDirectionalLightsCount(1);
		constexpr DirectionalLight directionalLight{
			{-0.2f, -1.0f, -0.3f},
			{0.5f, 0.5f, 0.5f},
			{0.5f, 0.5f, 0.5f},
			{1.0f, 1.0f, 1.0f}
		};
		pipeline.SetDirectionalLight("directionalLights", 0, directionalLight);

		pipeline.SetSpotLightsCount(1);

		SpotLight spotLight{
			m_Camera.Position(),
			m_Camera.Position() + m_Camera.Front(),
			glm::cos(glm::radians(12.5f)),
			glm::cos(glm::radians(15.0f)),
			1.0f,
			0.09f,
			0.032f,
			{0.0f, 0.0f, 0.0f},
			{1.0f, 1.0f, 1.0f},
			{1.0f, 1.0f, 1.0f},
		};
		pipeline.SetSpotLight("spotLights", 0, spotLight, view);
	}

	void RenderCubeMap() const
	{
		const glm::mat4 view = m_Camera.GetViewMatrix();
		const auto viewNoTranslation = glm::mat4(glm::mat3(view));

		m_PipelineCubeMap.Use();

		m_PipelineCubeMap.SetMat4("viewNoTranslation", viewNoTranslation);

		glBindVertexArray(m_CubeMapVao);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMap.textureId);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	void RenderGround(const Pipeline& pipeline) const
	{
		const glm::mat4 view = m_Camera.GetViewMatrix();

		pipeline.Use();
		pipeline.SetFloat("material.specular", 0.1f);
		auto groundModel = glm::mat4(1.0f);
		groundModel = translate(groundModel, glm::vec3(0.0f, -2.0f, 0.0f));
		groundModel = scale(groundModel, glm::vec3(1.0f, 1.0f, 1.0f));
		pipeline.SetMat4("model", groundModel);

		const auto groundNormalMatrix = inverseTranspose(view * groundModel);
		pipeline.SetMat3("normal", groundNormalMatrix);

		m_GroundModel.DrawNoSpecular(pipeline);
		CHECK_GL_ERROR();
	}

	void RenderBackpack(const Pipeline& pipeline) const
	{
		const glm::mat4 view = m_Camera.GetViewMatrix();

		pipeline.Use();
		auto model = glm::mat4(1.0f);
		model = translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		pipeline.SetMat4("model", model);

		const glm::mat3 normalMatrix = inverseTranspose(view * model);
		pipeline.SetMat3("normal", normalMatrix);

		m_BackpackModel.Draw(pipeline);
		CHECK_GL_ERROR();
	}

	void Update(const f32 deltaTime) override
	{
		m_Time += deltaTime;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		CHECK_GL_ERROR();

		UpdateView();

		SetupLight(m_Pipeline);
		SetupLight(m_PipelineNoSpecular);
		CHECK_GL_ERROR();

		RenderGround(m_PipelineNoSpecular);

		m_PipelineReflection.Use();
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMap.textureId);
		CHECK_GL_ERROR();
		RenderBackpack(m_PipelineReflection);

		RenderCubeMap();

#pragma region Camera
		const uint8_t* keyboardState = SDL_GetKeyboardState(nullptr);
		const CameraMovementState cameraMovementState{
			.forward = static_cast<bool>(keyboardState[SDL_SCANCODE_W]),
			.backward = static_cast<bool>(keyboardState[SDL_SCANCODE_S]),
			.left = static_cast<bool>(keyboardState[SDL_SCANCODE_A]),
			.right = static_cast<bool>(keyboardState[SDL_SCANCODE_D]),
			.up = static_cast<bool>(keyboardState[SDL_SCANCODE_SPACE]),
			.down = static_cast<bool>(keyboardState[SDL_SCANCODE_LSHIFT])
		};

		if (cameraMovementState.HasMovement())
		{
			m_Camera.ProcessMovement(cameraMovementState, deltaTime);
		}
#pragma endregion Camera

		if (CHECK_GL_ERROR())
		{
			SDL_SetRelativeMouseMode(SDL_FALSE);
		}
	}

	void OnEvent(const SDL_Event& event) override
	{
		switch (event.type)
		{
		case SDL_MOUSEMOTION:
		{
			const auto xOffset = static_cast<f32>(event.motion.xrel);
			const auto yOffset = static_cast<f32>(-event.motion.yrel);
			m_Camera.ProcessMouseMovement(xOffset, yOffset);
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			const f32 yOffset = event.wheel.preciseY;
			m_Camera.ProcessMouseScroll(yOffset);
			UpdateProjection();
			break;
		}
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_o)
			{
				m_Camera.IncrementMovementSpeed(-1.0f);
			}
			else if (event.key.keysym.sym == SDLK_p)
			{
				m_Camera.IncrementMovementSpeed(1.0f);
			}
			break;
		default:
			break;
		}
	}

	void OnResize(const i32 windowWidth, const i32 windowHeight) override
	{
		glViewport(0, 0, windowWidth, windowHeight);
		m_Camera.SetAspectRatio(static_cast<f32>(windowWidth) / static_cast<f32>(windowHeight));
	}

private:
	GLuint m_CubeMapVao{};
	GLuint m_CubeMapVbo{};
	Pipeline m_Pipeline{};
	Pipeline m_PipelineNoSpecular{};
	Pipeline m_PipelineCubeMap{};
	Pipeline m_PipelineReflection{};
	Pipeline m_PipelineRefraction{};
	f32 m_Time{};
	Camera m_Camera{glm::vec3{0.0f, 0.0f, 3.0f}};
	Model m_GroundModel;
	Model m_BackpackModel;
	Texture m_CubeMap{};
	GLuint m_UboMatrices{};

	static constexpr std::array CubeMapVertices{
		-1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		1.0f,
		1.0f,
		-1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		-1.0f,
		1.0f,
		1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		-1.0f,
		1.0f,
		1.0f,
		-1.0f,
		1.0f
	};
};
} // namespace stw
