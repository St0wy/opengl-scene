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
		0.5f,
		-0.5f,
		-0.5f,
		1.0f,
		0.0f,
		0.5f,
		0.5f,
		-0.5f,
		1.0f,
		1.0f,
		0.5f,
		0.5f,
		-0.5f,
		1.0f,
		1.0f,
		-0.5f,
		0.5f,
		-0.5f,
		0.0f,
		1.0f,
		-0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		0.0f,
		-0.5f,
		-0.5f,
		0.5f,
		0.0f,
		0.0f,
		0.5f,
		-0.5f,
		0.5f,
		1.0f,
		0.0f,
		0.5f,
		0.5f,
		0.5f,
		1.0f,
		1.0f,
		0.5f,
		0.5f,
		0.5f,
		1.0f,
		1.0f,
		-0.5f,
		0.5f,
		0.5f,
		0.0f,
		1.0f,
		-0.5f,
		-0.5f,
		0.5f,
		0.0f,
		0.0f,
		-0.5f,
		0.5f,
		0.5f,
		1.0f,
		0.0f,
		-0.5f,
		0.5f,
		-0.5f,
		1.0f,
		1.0f,
		-0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		1.0f,
		-0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		1.0f,
		-0.5f,
		-0.5f,
		0.5f,
		0.0f,
		0.0f,
		-0.5f,
		0.5f,
		0.5f,
		1.0f,
		0.0f,
		0.5f,
		0.5f,
		0.5f,
		1.0f,
		0.0f,
		0.5f,
		0.5f,
		-0.5f,
		1.0f,
		1.0f,
		0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		1.0f,
		0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		1.0f,
		0.5f,
		-0.5f,
		0.5f,
		0.0f,
		0.0f,
		0.5f,
		0.5f,
		0.5f,
		1.0f,
		0.0f,
		-0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		1.0f,
		0.5f,
		-0.5f,
		-0.5f,
		1.0f,
		1.0f,
		0.5f,
		-0.5f,
		0.5f,
		1.0f,
		0.0f,
		0.5f,
		-0.5f,
		0.5f,
		1.0f,
		0.0f,
		-0.5f,
		-0.5f,
		0.5f,
		0.0f,
		0.0f,
		-0.5f,
		-0.5f,
		-0.5f,
		0.0f,
		1.0f,
		-0.5f,
		0.5f,
		-0.5f,
		0.0f,
		1.0f,
		0.5f,
		0.5f,
		-0.5f,
		1.0f,
		1.0f,
		0.5f,
		0.5f,
		0.5f,
		1.0f,
		0.0f,
		0.5f,
		0.5f,
		0.5f,
		1.0f,
		0.0f,
		-0.5f,
		0.5f,
		0.5f,
		0.0f,
		0.0f,
		-0.5f,
		0.5f,
		-0.5f,
		0.0f,
		1.0f
	};

	static constexpr std::array CubePositions = {
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

		m_Pipeline.InitFromPath("shaders/cube.vert", "shaders/cube.frag");

		glGenVertexArrays(1, &m_Vao);
		glGenBuffers(1, &m_Vbo);
		glGenBuffers(1, &m_Ebo);
		glBindVertexArray(m_Vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
		glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(float), Vertices.data(), GL_STATIC_DRAW);

		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);

		// Texture Coordinates
		glVertexAttribPointer(1,
			2,
			GL_FLOAT,
			GL_FALSE,
			5 * sizeof(float),
			reinterpret_cast<const void*>(3 * sizeof(float))); // NOLINT(performance-no-int-to-ptr)
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_BoxTexture.Init("./data/container.jpg", "texture2", 0, &m_Pipeline);
		m_FaceTexture.Init("./data/face.png", "texture1", 1, &m_Pipeline, GL_RGBA);
	}

	void End() override
	{
		glDeleteVertexArrays(1, &m_Vao);
		glDeleteBuffers(1, &m_Vbo);
	}

	void Update(const f32 deltaTime) override
	{
		m_Time += deltaTime;
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_Pipeline.Use();

		m_BoxTexture.Bind(GL_TEXTURE0);
		m_FaceTexture.Bind(GL_TEXTURE1);

		const glm::mat4 view = m_Camera.GetViewMatrix();
		const glm::mat4 projection = m_Camera.GetProjectionMatrix();
		m_Pipeline.SetMat4("projection", projection);
		m_Pipeline.SetMat4("view", view);

		glBindVertexArray(m_Vao);

		for (u32 i = 0; i < 10; i++)
		{
			glm::mat4 model{1.0f};
			model = translate(model, CubePositions[i]);
			const f32 angle = 20.0f * static_cast<float>(i + 1) * m_Time;
			model = rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			m_Pipeline.SetMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

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

		assert(!CHECK_GL_ERROR());
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
	GLuint m_Vao{};
	GLuint m_Vbo{};
	GLuint m_Ebo{};
	Texture m_BoxTexture;
	Texture m_FaceTexture;
	Pipeline m_Pipeline{};
	f32 m_Time{};
	Camera m_Camera{glm::vec3{0.0f, 0.0f, 3.0f}};
};
} // namespace stw
