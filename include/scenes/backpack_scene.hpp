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
class BackpackScene final : public Scene
{
public:
	static constexpr std::array PointLightPositions = {
		glm::vec3(1.2f, 1.0f, 2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f, 2.0f, -12.0f),
		glm::vec3(0.0f, 0.0f, -3.0f)
	};

	BackpackScene()
		: m_BackpackModel(Model::LoadFromPath("data/backpack/backpack.obj").value())
	{
		glEnable(GL_DEPTH_TEST);

		m_PipelineMesh.InitFromPath("shaders/mesh/mesh.vert", "shaders/light/mesh.frag");

		//const auto loadResult = ;
		//if (!loadResult.has_value())
		//{
		//	spdlog::error("Could not load backpack");
		//	assert(false);
		//}

		//m_BackpackModel = loadResult.value();
	}

	void Update(const f32 deltaTime) override
	{
		m_Time += deltaTime;

		//constexpr f32 lightRadius = 2.0f;
		//m_LightPosition.x = std::cos(m_Time) * lightRadius;
		//m_LightPosition.y = std::cos(m_Time * 6.0f) * 0.1f + 1.0f;
		//m_LightPosition.z = std::sin(m_Time) * lightRadius;

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_PipelineMesh.Use();

		const glm::mat4 view = m_Camera.GetViewMatrix();
		const glm::mat4 projection = m_Camera.GetProjectionMatrix();

		m_PipelineMesh.SetMat4("projection", projection);
		m_PipelineMesh.SetMat4("view", view);

		// Render model
		auto model = glm::mat4(1.0f);
		model = translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		m_PipelineMesh.SetMat4("model", model);
		m_BackpackModel.Draw(m_PipelineMesh);

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
	Pipeline m_PipelineMesh{};
	f32 m_Time{};
	Camera m_Camera{glm::vec3{0.0f, 0.0f, 3.0f}};
	Model m_BackpackModel;
};
} // namespace stw
