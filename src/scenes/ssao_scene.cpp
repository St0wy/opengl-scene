/**
 * @file ssao_scene.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the SsaoScene class.
 * @version 1.0
 * @date 09/11/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <numbers>
#include <random>
#include <span>

#include <GL/glew.h>
#include <glm/ext.hpp>
#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>

export module ssao_scene;

import number_types;
import camera;
import scene;
import pipeline;
import renderer;

export namespace stw
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
			glDebugMessageCallback(messageCallback, nullptr);
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

		auto result = m_Renderer->LoadModel("./data/terrain/terrain.gltf", true);
		if (!result.has_value())
		{
			spdlog::error("Error on model loading : {}", result.error());
		}

		result = m_Renderer->LoadModel("./data/cat_gltf/cat.gltf", true);
		if (!result.has_value())
		{
			spdlog::error("Error on model loading : {}", result.error());
		}

		auto nodeVec = result.value();
		m_CatNodeIndex = nodeVec[0];
		m_Renderer->GetSceneGraph().TranslateElement(m_CatNodeIndex, glm::vec3{ 0.3f, 0.4f, 0.0f });
		m_Renderer->GetSceneGraph().ScaleElement(m_CatNodeIndex, glm::vec3{ 4.0f, 4.0f, 4.0f });

		result = m_Renderer->LoadModel("./data/backpack_gltf/backpack.gltf");
		if (!result.has_value())
		{
			spdlog::error("Error on model loading : {}", result.error());
		}

		nodeVec = result.value();
		auto nodeIdx = nodeVec[0];
		m_Renderer->GetSceneGraph().TranslateElement(nodeIdx, glm::vec3{ -2.0f, 2.0f, 0.0f });

		result = m_Renderer->LoadModel("./data/ball/ball.gltf", true);
		if (!result.has_value())
		{
			spdlog::error("Error on model loading : {}", result.error());
		}

		nodeVec = result.value();
		nodeIdx = nodeVec[0];
		m_Renderer->GetSceneGraph().TranslateElement(nodeIdx, glm::vec3{ 2.0f, 1.0f, 0.0f });
		m_Renderer->GetSceneGraph().RotateElement(nodeIdx, glm::radians(-90.0f), glm::vec3{ 0.0f, 1.0f, 0.0f });
		m_Renderer->GetSceneGraph().ScaleElement(nodeIdx, glm::vec3{ 7.0f, 7.0f, 7.0f });

		glm::vec3 direction{ 0.0f, -1.0f, -1.0f };
		direction = normalize(direction);
		const DirectionalLight directionalLight{ direction, glm::vec3{ 5.0f } };
		m_Renderer->SetDirectionalLight(directionalLight);

		const PointLight p{ glm::vec3{ 5.0f, 0.0f, 4.0f }, glm::vec3{ 20.0f } };
		m_Renderer->PushPointLight(p);
	}

	void UpdateProjection() { m_Renderer->UpdateProjectionMatrix(); }

	void UpdateView() { m_Renderer->UpdateViewMatrix(); }

	void Update(const f32 deltaTime) override
	{
		angle += deltaTime;
		if (angle >= std::numbers::pi_v<float> * 2.0f)
		{
			angle = 0.0f;
		}

		m_Renderer->GetSceneGraph().RotateElement(m_CatNodeIndex, deltaTime, glm::vec3{ 0.0f, 1.0f, 0.0f });

		m_Renderer->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		UpdateView();

		m_Renderer->DrawScene();

#pragma region Camera
		i32 keyboardStateLength = 0;
		const uint8_t* keyboardStatePtr = SDL_GetKeyboardState(&keyboardStateLength);
		const std::span keyboardState{ keyboardStatePtr, static_cast<usize>(keyboardStateLength) };

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
	Camera m_Camera{ glm::vec3{ 0.0f, 1.5f, 5.0f } };
	std::unique_ptr<Renderer> m_Renderer{};
	usize m_CatNodeIndex{};
	bool isFullscreen = false;
};
}// namespace stw
