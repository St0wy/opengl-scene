//
// Created by stowy on 16/06/2023.
//

#pragma once
#include <array>
#include <random>
#include <GL/glew.h>
#include <glm/ext.hpp>

#include "camera.hpp"
#include "model.hpp"
#include "number_types.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include "ogl/pipeline.hpp"

namespace stw
{
class GammaScene final : public Scene
{
public:
	static constexpr std::size_t RockCount = 10'000;
	std::vector<glm::mat4> rockModelMatrices{RockCount};
	GLuint modelMatricesBuffer = 0;

	void Init() override
	{
		m_PlanetModel = Model::LoadFromPath("data/planet/planet.obj").value();
		m_RockModel = Model::LoadFromPath("data/rock/rock.obj").value();

		if (GLEW_VERSION_4_3)
		{
			glDebugMessageCallback([](GLenum source,
					GLenum type,
					GLuint id,
					GLenum severity,
					const GLsizei length,
					const GLchar* message,
					const void*)
				{
					spdlog::error("[OpenGL Error source {}, type {}, id {}, severity {}] {}",
						source,
						type,
						id,
						severity,
						std::string_view(message, length));
				},
				nullptr);
		}

		m_Camera.SetMovementSpeed(20.0f);

		GLCALL(glEnable(GL_MULTISAMPLE));
		GLCALL(glEnable(GL_DEPTH_TEST));
		GLCALL(glDepthFunc(GL_LEQUAL));

		GLCALL(glEnable(GL_CULL_FACE));
		GLCALL(glCullFace(GL_BACK));
		GLCALL(glFrontFace(GL_CCW));

		m_InstancePipeline.InitFromPathSingleFile("shaders/instancing/instancing.glsl");

		const GLuint pipelineMatricesUniformBlockIndex = glGetUniformBlockIndex(m_InstancePipeline.Id(), "Matrices");
		GLCALL(glUniformBlockBinding(m_InstancePipeline.Id(), pipelineMatricesUniformBlockIndex, 0));
		GLCALL(glGenBuffers(1, &m_UboMatrices));
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, m_UboMatrices));

		// Allocate buffer memory on the gpu
		constexpr GLsizeiptr matricesSize = 2 * sizeof(glm::mat4);
		GLCALL(glBufferData(GL_UNIFORM_BUFFER, matricesSize, nullptr, GL_STATIC_DRAW));
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));

		// Bind buffer to binding point 0
		GLCALL(glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_UboMatrices, 0, matricesSize));
		UpdateProjection();

		SetupRockMatrices();
	}

	void SetupRockMatrices()
	{
		static std::random_device device{};
		static std::mt19937 rng{device()};
		constexpr f32 offset = 25.0f;
		std::uniform_real_distribution displacementDistribution(0.0f, 2.0f * offset * 100.0f);
		std::uniform_real_distribution scaleDistribution(0.0f, 20.0f);
		std::uniform_real_distribution rotationDistribution(0.0f, 360.0f);
		for (std::size_t i = 0; i < rockModelMatrices.size(); i++)
		{
			constexpr f32 radius = 75.0f;
			auto model = glm::mat4(1.0f);
			const f32 angle = static_cast<f32>(i) / static_cast<f32>(rockModelMatrices.size()) * 360.0f;
			f32 displacement = displacementDistribution(rng) / 100.0f - offset;
			const f32 x = std::sin(angle) * radius + displacement;
			displacement = displacementDistribution(rng) / 100.0f - offset;
			const f32 y = displacement * 0.4f;
			displacement = displacementDistribution(rng) / 100.0f - offset;
			const f32 z = std::cos(angle) * radius + displacement;
			model = translate(model, glm::vec3(x, y, z));

			const f32 scale = scaleDistribution(rng) / 100.0f + 0.05f;
			model = glm::scale(model, glm::vec3(scale));

			const f32 rotationAngle = rotationDistribution(rng);
			model = rotate(model, rotationAngle, glm::vec3(0.4f, 0.6f, 0.8f));

			rockModelMatrices[i] = model;
		}
	}

	void UpdateProjection() const
	{
		const auto projection = m_Camera.GetProjectionMatrix();
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, m_UboMatrices));
		GLCALL(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection)));
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	}

	void UpdateView() const
	{
		glm::mat4 view = m_Camera.GetViewMatrix();
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, m_UboMatrices));
		GLCALL(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view)));
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	}

	void Update(const f32 deltaTime) override
	{
		m_Time += deltaTime;

		GLCALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		UpdateView();

		m_InstancePipeline.Bind();

		auto planetModelMat = glm::mat4(1.0f);
		planetModelMat = translate(planetModelMat, glm::vec3(0.0f, 0.0f, 0.0f));
		planetModelMat = scale(planetModelMat, glm::vec3(5.0f, 5.0f, 5.0f));

		m_PlanetModel.DrawNoSpecular(m_InstancePipeline, planetModelMat);

		m_RockModel.DrawNoSpecularInstanced(m_InstancePipeline, rockModelMatrices);

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
		glViewport(0, 0, windowWidth, windowHeight);
		m_Camera.SetAspectRatio(static_cast<f32>(windowWidth) / static_cast<f32>(windowHeight));
	}

	void Delete() override
	{
		m_InstancePipeline.Delete();
		m_PlanetModel.Delete();
		m_RockModel.Delete();
	}

private:
	Pipeline m_InstancePipeline{};
	f32 m_Time{};
	Camera m_Camera{glm::vec3{0.0f, 20.0f, 40.0f}};
	Model m_PlanetModel;
	Model m_RockModel;
	GLuint m_UboMatrices{};
};
} // namespace stw
