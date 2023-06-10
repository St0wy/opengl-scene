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
class InstancingScene final : public Scene
{
public:
	InstancingScene()
		: m_PlanetModel(Model::LoadFromPath("data/planet/planet.obj").value()),
		m_RockModel(Model::LoadFromPath("data/rock/rock.obj").value())
	{
		m_Camera.SetMovementSpeed(4.0f);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		m_InstancePipeline.InitFromPathSingleFile("shaders/instancing/instancing.glsl");
		m_NoInstancePipeline.InitFromPathSingleFile("shaders/instancing/no_instancing.glsl");

		const GLuint pipelineMatricesUniformBlockIndex = glGetUniformBlockIndex(m_InstancePipeline.Id(), "Matrices");
		const GLuint noInstancePipelineMatricesUniformBlockIndex = glGetUniformBlockIndex(m_NoInstancePipeline.Id(),
			"Matrices");
		glUniformBlockBinding(m_InstancePipeline.Id(), pipelineMatricesUniformBlockIndex, 0);
		glUniformBlockBinding(m_NoInstancePipeline.Id(), noInstancePipelineMatricesUniformBlockIndex, 0);

		glGenBuffers(1, &m_UboMatrices);
		glBindBuffer(GL_UNIFORM_BUFFER, m_UboMatrices);

		// Allocate buffer memory on the gpu
		constexpr GLsizeiptr matricesSize = 2 * sizeof(glm::mat4);
		glBufferData(GL_UNIFORM_BUFFER, matricesSize, nullptr, GL_STATIC_DRAW);
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

	void RenderPlanet(const Pipeline& pipeline) const
	{
		pipeline.Use();
		auto planetModelMat = glm::mat4(1.0f);
		planetModelMat = translate(planetModelMat, glm::vec3(0.0f, 0.0f, 0.0f));
		planetModelMat = scale(planetModelMat, glm::vec3(1.0f, 1.0f, 1.0f));
		pipeline.SetMat4("model", planetModelMat);

		m_PlanetModel.DrawNoSpecular(pipeline);
		CHECK_GL_ERROR();
	}

	void Update(const f32 deltaTime) override
	{
		m_Time += deltaTime;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		CHECK_GL_ERROR();

		UpdateView();

		RenderPlanet(m_NoInstancePipeline);

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
	Pipeline m_InstancePipeline{};
	Pipeline m_NoInstancePipeline{};
	f32 m_Time{};
	Camera m_Camera{glm::vec3{0.0f, 0.0f, 20.0f}};
	Model m_PlanetModel;
	Model m_RockModel;
	GLuint m_UboMatrices{};
};
} // namespace stw