
//
// Created by stowy on 11/07/2023.
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
class DeferredShadingScene final : public Scene
{
public:
	void GenerateSceneDuplicateElement(const SceneGraphElement element)
	{
		constexpr usize elementCount = 500;
		constexpr usize lightCount = Renderer::MaxPointLights;
		constexpr float maxPos = 30.0f;
		constexpr float minPos = -maxPos;
		constexpr float twoPi = 2.0f * std::numbers::pi;
		constexpr float minScale = 0.6f;
		constexpr float maxScale = 1.2f;

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> distPos(minPos, maxPos);
		std::uniform_real_distribution<float> distHeightElem(-1.0f, 1.0f);
		std::uniform_real_distribution<float> distRotate(0.0f, twoPi);
		std::uniform_real_distribution<float> distScale(minScale, maxScale);
		std::uniform_real_distribution<float> distHeightLight(1.0f, 2.0f);
		std::uniform_real_distribution<float> distColor(0.0f, 2.0f);

		SceneGraph& sceneGraph = m_Renderer.GetSceneGraph();
		for (usize i = 0; i < elementCount; i++)
		{
			glm::mat4 transform{ 1.0f };
			transform = glm::translate(transform, glm::vec3{ distPos(gen), distHeightElem(gen), distPos(gen) });
			transform = glm::rotate(transform, distRotate(gen), glm::vec3{ 1.0f, 0.0f, 0.0f });
			transform = glm::rotate(transform, distRotate(gen), glm::vec3{ 0.0f, 1.0f, 0.0f });
			transform = glm::rotate(transform, distRotate(gen), glm::vec3{ 0.0f, 0.0f, 1.0f });
			transform = glm::scale(transform, glm::vec3{ distScale(gen) });
			sceneGraph.AddElementToRoot(element.meshId, element.materialId, transform);
		}

		for (usize i = 0; i < lightCount; i++)
		{

			const PointLight pointLight{
				glm::vec3{ distPos(gen), distHeightLight(gen), distPos(gen) },
				0.1f,
				0.2f,
				glm::vec3(distColor(gen), distColor(gen), distColor(gen)),
			};
			m_Renderer.PushPointLight(pointLight);
		}
	}

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

		m_Renderer.Init(screenSize);
		//		m_Renderer.SetEnableMultisample(true);
		m_Renderer.SetEnableDepthTest(true);
		m_Renderer.SetDepthFunc(GL_LEQUAL);
		m_Renderer.SetClearColor(glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });

		m_Renderer.SetEnableCullFace(true);

		m_Pipeline.InitFromPath("shaders/shadow_map/shadow_map.vert", "shaders/shadow_map/shadow_map.frag");
		m_Pipeline.Bind();
		m_Pipeline.SetInt("shadowMap", 3);

		UpdateProjection();

		auto result = m_Renderer.LoadModel("./data/sphere/myHonestSphere.obj", m_Pipeline);
		if (!result.has_value())
		{
			spdlog::error("Error on model loading : {}", result.error());
		}

		SceneGraph& sceneGraph = m_Renderer.GetSceneGraph();
		const SceneGraphNode& node = result.value()[0];
		const SceneGraphElement& element = sceneGraph.GetElements()[node.elementId];
		GenerateSceneDuplicateElement(element);

		glm::vec3 direction{ 0.0f, -1.0f, -0.5f };
		direction = glm::normalize(direction);
		const DirectionalLight directionalLight{ direction, glm::vec3{ 0.3f } };
		m_Renderer.SetDirectionalLight(directionalLight);
	}

	void SetupPipeline(Pipeline& pipeline)
	{
		pipeline.Bind();
		pipeline.SetVec3("viewPos", m_Camera.Position());
		pipeline.UnBind();
	}

	void UpdateProjection()
	{
		const auto projection = m_Camera.GetProjectionMatrix();
		m_Renderer.SetProjectionMatrix(projection);
	}

	void UpdateView()
	{
		const glm::mat4 view = m_Camera.GetViewMatrix();
		m_Renderer.SetViewMatrix(view);
		m_Renderer.viewPosition = m_Camera.Position();
	}

	void Update(const f32 deltaTime) override
	{
		m_Renderer.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		UpdateView();

		SetupPipeline(m_Pipeline);

		m_Pipeline.Bind();

		m_Renderer.DrawScene();

		m_Pipeline.UnBind();

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
		m_Renderer.SetViewport({ 0, 0 }, { windowWidth, windowHeight });
		m_Camera.SetAspectRatio(static_cast<f32>(windowWidth) / static_cast<f32>(windowHeight));
		UpdateProjection();
	}

	void Delete() override
	{
		m_Pipeline.Delete();
		m_Renderer.Delete();
	}

private:
	Pipeline m_Pipeline{};
	Camera m_Camera{ glm::vec3{ 5.0f, 2.0f, -6.0f } };
	Renderer m_Renderer{};
};
}// namespace stw
