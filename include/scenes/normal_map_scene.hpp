//
// Created by stowy on 19/06/2023.
//

#pragma once
#include <random>
#include <GL/glew.h>
#include <glm/ext.hpp>

#include "camera.hpp"
#include "model.hpp"
#include "number_types.hpp"
#include "scene.hpp"
#include "ogl/pipeline.hpp"
#include "ogl/renderer.hpp"

namespace stw
{
class NormalMapScene final : public Scene
{
public:
	void Init() override
	{
		m_WallModel = Model::LoadFromPath("data/wall/wall.obj").value();

		if (GLEW_VERSION_4_3)
		{
			constexpr auto messageCallback = 
				[](GLenum source, GLenum type, GLuint id, GLenum severity, const GLsizei length, const GLchar* message, const void*)
				{
					spdlog::error("[OpenGL Error source {}, type {}, id {}, severity {}] {}",
						source,
						type,
						id,
						severity,
						std::string_view(message, length));
				};
			glDebugMessageCallback(messageCallback, nullptr);
		}

		m_Camera.SetMovementSpeed(20.0f);

		m_Renderer.Init();
		m_Renderer.SetEnableMultisample(true);
		m_Renderer.SetEnableDepthTest(true);
		m_Renderer.SetDepthFunc(GL_LEQUAL);

		m_Renderer.SetEnableCullFace(true);

		m_Pipeline.InitFromPath("shaders/normal_map/mesh.vert", "shaders/normal_map/mesh_no_specular.frag");

		UpdateProjection();
	}

	static void SetupPipeline(Pipeline& pipeline)
	{
		pipeline.Bind();
		pipeline.SetFloat("material.shininess", 64.0f);

		// Setup lights
		constexpr PointLight pointLight{
			{0.0f, 0.0f, 1.0f},
			0.01f,
			0.1f,
			0.5f,
			glm::vec3{0.5f},
			glm::vec3{0.3f},
			{1.0f, 1.0f, 1.0f}
		};
		pipeline.SetPointLightsCount(1);
		pipeline.SetPointLight("pointLights", 0, pointLight);

		pipeline.UnBind();
	}

	void UpdateProjection() const
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

		m_Pipeline.SetFloat("material.specular", 0.5f);


		auto modelMatrix = glm::mat4(1.0f);
		modelMatrix = translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		modelMatrix = scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));

		m_Renderer.Draw(m_WallModel, m_Pipeline, modelMatrix);

		m_Pipeline.UnBind();

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
		m_Renderer.SetViewport({0, 0}, {windowWidth, windowHeight});
		m_Camera.SetAspectRatio(static_cast<f32>(windowWidth) / static_cast<f32>(windowHeight));
	}

	void Delete() override
	{
		m_Pipeline.Delete();
		m_WallModel.Delete();
		m_Renderer.Delete();
	}

private:
	Pipeline m_Pipeline{};
	Camera m_Camera{glm::vec3{0.0f, 5.0f, 40.0f}};
	Renderer m_Renderer{};
	Model m_WallModel;
};
} // namespace stw
