
//
// Created by stowy on 14/07/2023.
//
#pragma once

#include <GL/glew.h>
#include <glm/ext.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <numbers>
#include <random>
#include <variant>

#include "camera.hpp"
#include "material.hpp"
#include "number_types.hpp"
#include "ogl/pipeline.hpp"
#include "ogl/renderer.hpp"
#include "scene.hpp"

namespace stw
{
class SsaoScene final : public Scene
{
public:
	void Init(glm::uvec2 screenSize) override
	{
		if (GLEW_VERSION_4_3)
		{
			constexpr auto messageCallback = [](GLenum source,
												 GLenum type,
												 GLuint id,
												 GLenum severity,
												 const GLsizei length,
												 const GLchar* message,
												 const void*) {
				spdlog::error("[OpenGL Error source {}, type {}, id {}, severity {}] {}",
					source,
					type,
					id,
					severity,
					std::string_view(message, length));
			};
			GLCALL(glDebugMessageCallback(messageCallback, nullptr));
		}

		m_Camera.SetMovementSpeed(4.0f);

		m_Renderer = std::make_unique<Renderer>(&m_Camera);
		m_Renderer->Init(screenSize);
		//		m_Renderer->SetEnableMultisample(true);
		m_Renderer->SetEnableDepthTest(true);
		m_Renderer->SetDepthFunc(GL_LEQUAL);
		m_Renderer->SetClearColor(glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });

		m_Renderer->SetEnableCullFace(true);

		UpdateProjection();

		auto result = m_Renderer->LoadModel("./data/cat/cat.obj", true);
		if (!result.has_value())
		{
			spdlog::error("Error on model loading : {}", result.error());
		}

		auto nodeVec = result.value();

		const auto& node = m_Renderer->GetSceneGraph().GetNodes()[0];
		m_Model = &m_Renderer->GetSceneGraph().GetElements()[node.elementId];

		glm::vec3 direction{ 0.0f, -1.0f, -1.0f };
		direction = glm::normalize(direction);
		const DirectionalLight directionalLight{ direction, glm::vec3{ 5.0f } };
		m_Renderer->SetDirectionalLight(directionalLight);

		const PointLight p{ glm::vec3{ 0.0f, 0.0f, 4.0f }, glm::vec3{ 20.0f } };
		m_Renderer->PushPointLight(p);
	}

	void UpdateProjection() { m_Renderer->UpdateProjectionMatrix(); }

	void UpdateView() { m_Renderer->UpdateViewMatrix(); }

	void Update(const f32 deltaTime) override
	{
		angle += deltaTime;

		const glm::mat4 model{ 1.0f };
		m_Model->localTransformMatrix = glm::rotate(model, angle, glm::vec3{ 0.0f, 1.0f, 0.0f });

		auto& sceneGraphElems = m_Renderer->GetSceneGraph().GetElements();
		for (usize i = 1; i < sceneGraphElems.size(); i++)
		{
			sceneGraphElems[i].parentTransformMatrix = m_Model->localTransformMatrix;
		}

		m_Renderer->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		UpdateView();

		m_Renderer->DrawScene();

#pragma region Camera
		i32 keyboardStateLength = 0;
		const uint8_t* keyboardStatePtr = SDL_GetKeyboardState(&keyboardStateLength);
		const std::span<const u8> keyboardState{ keyboardStatePtr, static_cast<usize>(keyboardStateLength) };

		const CameraMovementState cameraMovementState{ .forward = static_cast<bool>(keyboardState[SDL_SCANCODE_W]),
			.backward = static_cast<bool>(keyboardState[SDL_SCANCODE_S]),
			.left = static_cast<bool>(keyboardState[SDL_SCANCODE_A]),
			.right = static_cast<bool>(keyboardState[SDL_SCANCODE_D]),
			.up = static_cast<bool>(keyboardState[SDL_SCANCODE_SPACE]),
			.down = static_cast<bool>(keyboardState[SDL_SCANCODE_LSHIFT]) };

		if (cameraMovementState.HasMovement())
		{
			m_Camera.ProcessMovement(cameraMovementState, deltaTime);
		}
#pragma endregion Camera
	}

	void OnEvent(const SDL_Event& event) override
	{
		switch (event.type)
		{
		case SDL_MOUSEMOTION: {
			const auto xOffset = static_cast<f32>(event.motion.xrel);
			const auto yOffset = static_cast<f32>(-event.motion.yrel);
			m_Camera.ProcessMouseMovement(xOffset, yOffset);
			break;
		}
		case SDL_MOUSEWHEEL: {
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
		m_Renderer->SetViewport({ 0, 0 }, { windowWidth, windowHeight });
		m_Camera.SetAspectRatio(static_cast<f32>(windowWidth) / static_cast<f32>(windowHeight));
		UpdateProjection();
	}

	void Delete() override { m_Renderer->Delete(); }

private:
	f32 angle = 0.0f;
	Camera m_Camera{ glm::vec3{ 0.0f, 0.0f, 2.0f } };
	std::unique_ptr<Renderer> m_Renderer{};
	stw::SceneGraphElement* m_Model;
};
}// namespace stw
