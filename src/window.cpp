//
// Created by stowy on 03/05/2023.
//
#include "window.hpp"

#include <chrono>
#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"

stw::Window::Window(std::unique_ptr<Scene> scene, const char* windowName, const i32 windowWidth, const i32 windowHeight)
	: m_Scene(std::move(scene))
{
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
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	m_GlRenderContext = SDL_GL_CreateContext(m_Window);
	SDL_GL_SetSwapInterval(1);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	if (glewInit() != GLEW_OK)
	{
		spdlog::error("Failed to initialize OpenGL context");
		assert(false);
	}

	m_Scene->Begin();
}

stw::Window::~Window()
{
	spdlog::info("Closing window");

	m_Scene->End();
	SDL_GL_DeleteContext(m_GlRenderContext);
	SDL_DestroyWindow(m_Window);
	SDL_Quit();
}

void stw::Window::Loop()
{
	std::chrono::time_point<std::chrono::system_clock> clock = std::chrono::system_clock::now();

	bool isOpen = true;
	while (isOpen)
	{
		const auto start = std::chrono::system_clock::now();
		using Seconds = std::chrono::duration<f32, std::ratio<1, 1>>;
		const auto deltaTime = std::chrono::duration_cast<Seconds>(start - clock);
		clock = start;

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
					glViewport(0, 0, windowWidth, windowHeight);
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
			default:
				break;
			}

			if (m_IsActive)
			{
				m_Scene->OnEvent(event);
			}
		}

		if (m_IsActive)
		{
			m_Scene->Update(deltaTime.count());
		}
		SDL_GL_SwapWindow(m_Window);
	}
}
