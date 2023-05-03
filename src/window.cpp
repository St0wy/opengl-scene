//
// Created by stowy on 03/05/2023.
//
#include "window.hpp"
#include "spdlog/spdlog.h"
#include "GL/glew.h"
#include "utils.hpp"

stw::Window::Window(
	std::unique_ptr<Scene> scene,
	const char* windowName,
	int32_t windowWidth,
	int32_t windowHeight
)
	: m_Scene(std::move(scene))
{
	SDL_SetMainReady();

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	m_Window = SDL_CreateWindow(
		windowName,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		windowWidth,
		windowHeight,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
	);
	m_GlRenderContext = SDL_GL_CreateContext(m_Window);
	SDL_GL_SetSwapInterval(1);

	if (GLEW_OK != glewInit())
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
	bool isOpen = true;
	while (isOpen)
	{
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
				default:
					break;
				}
				break;
			}
			default:
				break;
			}
		}

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		m_Scene->Update(0.0f);
		SDL_GL_SwapWindow(m_Window);
	}
}
