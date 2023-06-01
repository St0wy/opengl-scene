//
// Created by stowy on 25/05/2023.
//

#pragma once
#include <array>
#include <GL/glew.h>
#include <glm/ext.hpp>

#include "camera.hpp"
#include "model.hpp"
#include "number_types.hpp"
#include "pipeline.hpp"
#include "scene.hpp"
#include "utils.hpp"

namespace stw
{
class TransparentScene final : public Scene
{
public:
	TransparentScene()
		: m_GroundModel(Model::LoadFromPath("data/ground/ground.obj").value())
	{
		m_Camera.SetMovementSpeed(4.0f);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		m_PipelineSpecular.InitFromPath("shaders/mesh/mesh.vert", "shaders/mesh/mesh.frag");
		m_PipelineNoSpecular.InitFromPath("shaders/mesh/mesh.vert", "shaders/mesh/mesh_no_specular.frag");
	}

	void SetupPipeline(Pipeline& pipeline) const
	{
		pipeline.Use();
		pipeline.SetFloat("material.shininess", 32.0f);

		const glm::mat4 view = m_Camera.GetViewMatrix();
		const glm::mat4 projection = m_Camera.GetProjectionMatrix();

		pipeline.SetMat4("projection", projection);
		pipeline.SetMat4("view", view);

		// Setup lights
		pipeline.SetDirectionalLightsCount(1);
		constexpr DirectionalLight directionalLight{
			{-0.2f, -1.0f, -0.3f},
			{0.2f, 0.2f, 0.2f},
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

	void Update(const f32 deltaTime) override
	{
		m_Time += deltaTime;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		SetupPipeline(m_PipelineSpecular);
		SetupPipeline(m_PipelineNoSpecular);

		const glm::mat4 view = m_Camera.GetViewMatrix();
		const glm::mat4 projection = m_Camera.GetProjectionMatrix();

		// Render ground model
		m_PipelineNoSpecular.Use();
		m_PipelineNoSpecular.SetFloat("material.specular", 0.1f);
		auto groundModel = glm::mat4(1.0f);
		groundModel = translate(groundModel, glm::vec3(0.0f, -2.0f, 0.0f));
		groundModel = scale(groundModel, glm::vec3(1.0f, 1.0f, 1.0f));
		m_PipelineNoSpecular.SetMat4("model", groundModel);

		const auto groundNormalMatrix = inverseTranspose(view * groundModel);
		m_PipelineNoSpecular.SetMat3("normal", groundNormalMatrix);

		m_GroundModel.DrawNoSpecular(m_PipelineNoSpecular);
		CHECK_GL_ERROR();

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);


		// Camera
		const uint8_t* keyboardState = SDL_GetKeyboardState(nullptr);
		const CameraMovementState cameraMovementState{
			.forward = static_cast<bool>(keyboardState[SDL_SCANCODE_W]),
			.backward = static_cast<bool>(keyboardState[SDL_SCANCODE_S]),
			.left = static_cast<bool>(keyboardState[SDL_SCANCODE_A]),
			.right = static_cast<bool>(keyboardState[SDL_SCANCODE_D]),
			.up = static_cast<bool>(keyboardState[SDL_SCANCODE_SPACE]),
			.down = static_cast<bool>(keyboardState[SDL_SCANCODE_LSHIFT])
		};
		m_Camera.ProcessMovement(cameraMovementState, deltaTime);

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
	Pipeline m_PipelineSpecular{};
	Pipeline m_PipelineNoSpecular{};
	f32 m_Time{};
	Camera m_Camera{glm::vec3{0.0f, 0.0f, 3.0f}};
	Model m_GroundModel;
};
} // namespace stw
