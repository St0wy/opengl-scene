//
// Created by stowy on 03/05/2023.
//

#pragma once
#define SDL_MAIN_HANDLED
#include <cassert>
#include <memory>
#include <SDL.h>
#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include "number_types.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include "scenes/scene.hpp"

namespace stw
{
template <Derived<Scene> T>
class Window
{
public:
	explicit Window(const char* windowName, i32 windowWidth = 1280, i32 windowHeight = 720);
	Window(Window&& other) = default;
	Window(const Window& other) = delete;
	~Window();
	Window& operator=(Window&& other) = default;
	Window& operator=(const Window& other) = delete;

	void Loop();

private:
	std::unique_ptr<T> m_Scene;
	SDL_Window* m_Window;
	SDL_GLContext m_GlRenderContext;
	bool m_IsActive = true;
};

template <Derived<Scene> T>
Window<T>::Window(const char* windowName, i32 windowWidth, i32 windowHeight)
{
	spdlog::debug("Creating window...");
	SDL_SetHintWithPriority(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2", SDL_HINT_OVERRIDE);

	SDL_SetMainReady();

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	m_Window = SDL_CreateWindow(windowName,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		windowWidth,
		windowHeight,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED);

	m_GlRenderContext = SDL_GL_CreateContext(m_Window);
	SDL_GL_SetSwapInterval(1);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	if (glewInit() != GLEW_OK)
	{
		spdlog::error("Failed to initialize OpenGL context");
		assert(false);
	}

	m_Scene = std::make_unique<T>();
	m_Scene->OnResize(windowWidth, windowHeight);
}

template <Derived<Scene> T>
Window<T>::~Window()
{
	spdlog::info("Closing window");

	SDL_GL_DeleteContext(m_GlRenderContext);
	SDL_DestroyWindow(m_Window);
	SDL_Quit();
}

template <Derived<Scene> T>
void Window<T>::Loop()
{
	Timer timer;

	bool isOpen = true;
	while (isOpen)
	{
		const auto duration = timer.RestartAndGetElapsedTime();
		f32 deltaTime = m_IsActive ? static_cast<float>(duration.GetInSeconds()) : 0.0f;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				isOpen = false;
				break;
			case SDL_WINDOWEVENT:
			{
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					isOpen = false;
					break;
				case SDL_WINDOWEVENT_RESIZED:
				{
					const GLsizei windowWidth = event.window.data1;
					const GLsizei windowHeight = event.window.data2;
					m_Scene->OnResize(windowWidth, windowHeight);

					break;
				}
				default:
					break;
				}
				break;
			}
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE && m_IsActive)
				{
					SDL_SetRelativeMouseMode(SDL_FALSE);
					m_IsActive = false;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				SDL_SetRelativeMouseMode(SDL_TRUE);
				m_IsActive = true;
				break;
			default:
				break;
			}

			if (m_IsActive)
			{
				m_Scene->OnEvent(event);
			}
		}

		m_Scene->Update(deltaTime);

		if (CHECK_GL_ERROR())
		{
			assert(false);
		}

		SDL_GL_SwapWindow(m_Window);
	}
}
} // namespace stw
