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
#include "utils.hpp"
#include "ogl/pipeline.hpp"

namespace stw
{
class NormalMapScene final : public Scene
{
public:
	static constexpr std::size_t RockCount = 10'000;
	std::vector<glm::mat4> rockModelMatrices{RockCount};
	GLuint modelMatricesBuffer = 0;

	void Init() override
	{
		m_WallModel = Model::LoadFromPath("data/wall/wall.obj").value();

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

		m_Pipeline.InitFromPath("shaders/normal_map/mesh.vert", "shaders/normal_map/mesh_no_specular.frag");

		const GLuint pipelineMatricesUniformBlockIndex = glGetUniformBlockIndex(m_Pipeline.Id(), "Matrices");
		GLCALL(glUniformBlockBinding(m_Pipeline.Id(), pipelineMatricesUniformBlockIndex, 0));
		GLCALL(glGenBuffers(1, &m_UboMatrices));
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, m_UboMatrices));

		// Allocate buffer memory on the gpu
		constexpr GLsizeiptr matricesSize = 2 * sizeof(glm::mat4);
		GLCALL(glBufferData(GL_UNIFORM_BUFFER, matricesSize, nullptr, GL_STATIC_DRAW));
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));

		// Bind buffer to binding point 0
		GLCALL(glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_UboMatrices, 0, matricesSize));
		UpdateProjection();
	}

	void SetupPipeline(Pipeline& pipeline) const
	{
		pipeline.Bind();
		pipeline.SetFloat("material.shininess", 64.0f);

		const glm::mat4 view = m_Camera.GetViewMatrix();

		// Setup lights
		constexpr PointLight pointLight{
			{0.0f, 2.0f, 2.0f},
			0.01f,
			0.1f,
			0.4f,
			{0.5f, 0.5f, 0.5f},
			{0.5f, 0.5f, 0.5f},
			{1.0f, 1.0f, 1.0f}
		};
		pipeline.SetPointLightsCount(1);
		pipeline.SetPointLight("pointLights", 0, pointLight, view);

		pipeline.UnBind();
	}

	void UpdateProjection() const
	{
		const auto projection = m_Camera.GetProjectionMatrix();
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, m_UboMatrices));
		GLCALL(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection)));
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	}

	void UpdateView()
	{
		glm::mat4 view = m_Camera.GetViewMatrix();
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, m_UboMatrices));
		GLCALL(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view)));
		GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		m_Pipeline.Bind();
		m_Pipeline.SetVec3("viewPos", m_Camera.Position());
		m_Pipeline.UnBind();
	}

	void Update(const f32 deltaTime) override
	{
		GLCALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		UpdateView();

		SetupPipeline(m_Pipeline);

		m_Pipeline.Bind();

		m_Pipeline.SetFloat("material.specular", 0.1f);


		auto modelMatrix = glm::mat4(1.0f);
		modelMatrix = translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		modelMatrix = scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));

		m_WallModel.DrawNoSpecular(m_Pipeline, modelMatrix);

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
		glViewport(0, 0, windowWidth, windowHeight);
		m_Camera.SetAspectRatio(static_cast<f32>(windowWidth) / static_cast<f32>(windowHeight));
	}

	void Delete() override
	{
		m_Pipeline.Delete();
		m_WallModel.Delete();
	}

private:
	Pipeline m_Pipeline{};
	//f32 m_Time{};
	Camera m_Camera{glm::vec3{0.0f, 5.0f, 40.0f}};
	Model m_WallModel;
	GLuint m_UboMatrices{};
};
} // namespace stw
