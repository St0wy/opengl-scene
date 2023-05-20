//
// Created by stowy on 11/05/2023.
//

#pragma once
#include <array>
#include <GL/glew.h>
#include <glm/ext.hpp>
#include <spdlog/spdlog.h>

#include "camera.hpp"
#include "number_types.hpp"
#include "pipeline.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "utils.hpp"

namespace stw
{
class LightScene final : public Scene
{
public:
	static constexpr std::array Vertices{
		-0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		0.0f,
		-1.0f,
		0.0f,
		0.0f,
		0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		0.0f,
		-1.0f,
		1.0f,
		0.0f,
		0.5f,
		0.5f,
		-0.5f,
		0.0f,
		0.0f,
		-1.0f,
		1.0f,
		1.0f,
		0.5f,
		0.5f,
		-0.5f,
		0.0f,
		0.0f,
		-1.0f,
		1.0f,
		1.0f,
		-0.5f,
		0.5f,
		-0.5f,
		0.0f,
		0.0f,
		-1.0f,
		0.0f,
		1.0f,
		-0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		0.0f,
		-1.0f,
		0.0f,
		0.0f,
		-0.5f,
		-0.5f,
		0.5f,
		0.0f,
		0.0f,
		1.0f,
		0.0f,
		0.0f,
		0.5f,
		-0.5f,
		0.5f,
		0.0f,
		0.0f,
		1.0f,
		1.0f,
		0.0f,
		0.5f,
		0.5f,
		0.5f,
		0.0f,
		0.0f,
		1.0f,
		1.0f,
		1.0f,
		0.5f,
		0.5f,
		0.5f,
		0.0f,
		0.0f,
		1.0f,
		1.0f,
		1.0f,
		-0.5f,
		0.5f,
		0.5f,
		0.0f,
		0.0f,
		1.0f,
		0.0f,
		1.0f,
		-0.5f,
		-0.5f,
		0.5f,
		0.0f,
		0.0f,
		1.0f,
		0.0f,
		0.0f,
		-0.5f,
		0.5f,
		0.5f,
		-1.0f,
		0.0f,
		0.0f,
		1.0f,
		0.0f,
		-0.5f,
		0.5f,
		-0.5f,
		-1.0f,
		0.0f,
		0.0f,
		1.0f,
		1.0f,
		-0.5f,
		-0.5f,
		-0.5f,
		-1.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f,
		-0.5f,
		-0.5f,
		-0.5f,
		-1.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f,
		-0.5f,
		-0.5f,
		0.5f,
		-1.0f,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		-0.5f,
		0.5f,
		0.5f,
		-1.0f,
		0.0f,
		0.0f,
		1.0f,
		0.0f,
		0.5f,
		0.5f,
		0.5f,
		1.0f,
		0.0f,
		0.0f,
		1.0f,
		0.0f,
		0.5f,
		0.5f,
		-0.5f,
		1.0f,
		0.0f,
		0.0f,
		1.0f,
		1.0f,
		0.5f,
		-0.5f,
		-0.5f,
		1.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f,
		0.5f,
		-0.5f,
		-0.5f,
		1.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f,
		0.5f,
		-0.5f,
		0.5f,
		1.0f,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		0.5f,
		0.5f,
		0.5f,
		1.0f,
		0.0f,
		0.0f,
		1.0f,
		0.0f,
		-0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		-1.0f,
		0.0f,
		0.0f,
		1.0f,
		0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		-1.0f,
		0.0f,
		1.0f,
		1.0f,
		0.5f,
		-0.5f,
		0.5f,
		0.0f,
		-1.0f,
		0.0f,
		1.0f,
		0.0f,
		0.5f,
		-0.5f,
		0.5f,
		0.0f,
		-1.0f,
		0.0f,
		1.0f,
		0.0f,
		-0.5f,
		-0.5f,
		0.5f,
		0.0f,
		-1.0f,
		0.0f,
		0.0f,
		0.0f,
		-0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		-1.0f,
		0.0f,
		0.0f,
		1.0f,
		-0.5f,
		0.5f,
		-0.5f,
		0.0f,
		1.0f,
		0.0f,
		0.0f,
		1.0f,
		0.5f,
		0.5f,
		-0.5f,
		0.0f,
		1.0f,
		0.0f,
		1.0f,
		1.0f,
		0.5f,
		0.5f,
		0.5f,
		0.0f,
		1.0f,
		0.0f,
		1.0f,
		0.0f,
		0.5f,
		0.5f,
		0.5f,
		0.0f,
		1.0f,
		0.0f,
		1.0f,
		0.0f,
		-0.5f,
		0.5f,
		0.5f,
		0.0f,
		1.0f,
		0.0f,
		0.0f,
		0.0f,
		-0.5f,
		0.5f,
		-0.5f,
		0.0f,
		1.0f,
		0.0f,
		0.0f,
		1.0f
	};

	static constexpr std::array CubePositions{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(2.0f, 5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f, 3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f, 2.0f, -2.5f),
		glm::vec3(1.5f, 0.2f, -1.5f),
		glm::vec3(-1.3f, 1.0f, -1.5f)
	};

	void Begin() override
	{
		glEnable(GL_DEPTH_TEST);

		m_PipelineLightCube.InitFromPath("shaders/light/light_cube.vert", "shaders/light/light_cube.frag");
		m_PipelineLightSource.InitFromPath("shaders/light/light_source.vert", "shaders/light/light_source.frag");

		glGenVertexArrays(1, &m_VaoCube);
		glGenBuffers(1, &m_Vbo);

		glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
		glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(float), Vertices.data(), GL_STATIC_DRAW);

		glBindVertexArray(m_VaoCube);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

		auto offset = reinterpret_cast<void*>(3 * sizeof(float)); // NOLINT(performance-no-int-to-ptr)
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), offset);
		glEnableVertexAttribArray(1);

		offset = reinterpret_cast<void*>(6 * sizeof(float)); // NOLINT(performance-no-int-to-ptr)
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), offset);
		glEnableVertexAttribArray(2);

		glGenVertexArrays(1, &m_VaoLightSource);
		glBindVertexArray(m_VaoLightSource);

		glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

		m_DiffuseMap.Init("data/container2.png", "material.diffuse", 0, &m_PipelineLightCube, GL_RGBA);
		m_SpecularMap.Init("data/container2_specular.png", "material.specular", 1, &m_PipelineLightCube, GL_RGBA);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void End() override
	{
		glDeleteVertexArrays(1, &m_VaoCube);
		glDeleteBuffers(1, &m_Vbo);
	}

	void Update(const f32 deltaTime) override
	{
		m_Time += deltaTime;

		constexpr f32 lightRadius = 2.0f;
		m_LightPosition.x = std::cos(m_Time) * lightRadius;
		m_LightPosition.y = std::cos(m_Time * 6.0f) * 0.1f + 1.0f;
		m_LightPosition.z = std::sin(m_Time) * lightRadius;

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Cube 
		m_PipelineLightCube.Use();

		m_PipelineLightCube.SetFloat("material.shininess", 32.0f);

		m_PipelineLightCube.SetVec3("light.ambient", {0.2f, 0.2f, 0.2f});
		m_PipelineLightCube.SetVec3("light.diffuse", {0.5f, 0.5f, 0.5f}); // darken diffuse light a bit
		m_PipelineLightCube.SetVec3("light.specular", {1.0f, 1.0f, 1.0f});
		m_PipelineLightCube.SetFloat("light.constant", 1.0f);
		m_PipelineLightCube.SetFloat("light.linear", 0.09f);
		m_PipelineLightCube.SetFloat("light.quadratic", 0.032f);

		m_PipelineLightCube.SetVec3("viewPos", m_Camera.Position());

		const glm::mat4 view = m_Camera.GetViewMatrix();
		const glm::mat4 projection = m_Camera.GetProjectionMatrix();
		m_PipelineLightCube.SetMat4("projection", projection);
		m_PipelineLightCube.SetMat4("view", view);

		m_PipelineLightCube.SetVec3("light.position", m_Camera.Position());
		m_PipelineLightCube.SetVec3("light.direction", m_Camera.Front());
		m_PipelineLightCube.SetFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
		m_PipelineLightCube.SetFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));

		m_DiffuseMap.Bind();
		m_SpecularMap.Bind();

		glBindVertexArray(m_VaoCube);
		for (u32 i = 0; i < 10; i++)
		{
			glm::mat4 model{1.0f};
			model = translate(model, CubePositions[i]);
			const f32 angle = 20.0f * static_cast<float>(i);
			model = rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			m_PipelineLightCube.SetMat4("model", model);

			const glm::mat3 normalMatrix = inverseTranspose(view * model);
			m_PipelineLightCube.SetMat3("normal", normalMatrix);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// Light source
		/*m_PipelineLightSource.Use();

		m_PipelineLightSource.SetMat4("projection", projection);
		m_PipelineLightSource.SetMat4("view", view);

		glm::mat4 lightModel{1.0f};
		lightModel = translate(lightModel, m_LightPosition);
		lightModel = scale(lightModel, glm::vec3{0.2f});
		m_PipelineLightSource.SetMat4("model", lightModel);

		glBindVertexArray(m_VaoLightSource);
		glDrawArrays(GL_TRIANGLES, 0, 36);*/

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
	Pipeline m_PipelineLightCube{};
	Pipeline m_PipelineLightSource{};
	GLuint m_VaoCube{};
	GLuint m_VaoLightSource{};
	GLuint m_Vbo{};
	f32 m_Time{};
	Camera m_Camera{glm::vec3{0.0f, 0.0f, 3.0f}};
	glm::vec3 m_LightPosition{1.2f, 1.0f, 2.0f};
	Texture m_DiffuseMap{};
	Texture m_SpecularMap{};
};
} // namespace stw
