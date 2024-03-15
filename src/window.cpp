/**
 * @file window.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Window class.
 * @version 1.0
 * @date 03/05/2023
 *
 * @copyright SAE (c) 2023
 *
 */
module;

#include <cassert>
#include <memory>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include "macros.hpp"

export module window;

import utils;
import number_types;
import timer;
import scene;

export namespace stw
{
/**
 * This is a window that will display a scene passed as a type parameter.
 * @tparam T This type represents and OpenGL that will be shown on the window.
 * It is a remnant from when the app had the objective of having multiple scenes.
 * It's not the case anymore, but right now I'm not gonna refactor it.
 */
template<Derived<Scene> T>
class Window
{
public:
	explicit Window(std::string_view windowName, i32 windowWidth = 1280, i32 windowHeight = 720);
	Window(Window&& other) noexcept = default;
	Window(const Window& other) = delete;
	~Window();

	Window& operator=(Window&& other) noexcept = default;
	Window& operator=(const Window& other) = delete;

	/**
	 * Starts the rendering loop of the window.
	 */
	void Loop();

	/**
	 * Handles the SDL window events.
	 * @return true if the windows should close.
	 */
	bool HandleEvents();

private:
	std::unique_ptr<T> m_Scene;
	SDL_Window* m_Window;
	SDL_GLContext m_GlRenderContext;
	bool m_IsActive = true;
	bool m_IsFullscreen = false;
	std::string m_WindowName{};
};

template<Derived<Scene> T>
Window<T>::Window(const std::string_view windowName, i32 windowWidth, i32 windowHeight)
	: m_Scene(std::make_unique<T>()), m_WindowName(windowName)
{
	spdlog::info("Creating window...");
	SDL_SetHintWithPriority(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2", SDL_HINT_OVERRIDE);
	SDL_SetHintWithPriority(SDL_HINT_RENDER_VSYNC, "0", SDL_HINT_OVERRIDE);

	SDL_SetMainReady();

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	//	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	//	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	m_Window = SDL_CreateWindow(m_WindowName.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		windowWidth,
		windowHeight,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED);

	m_GlRenderContext = SDL_GL_CreateContext(m_Window);// NOLINT(cppcoreguidelines-prefer-member-initializer)
	SDL_GL_SetSwapInterval(0);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	if (glewInit() != GLEW_OK)
	{
		spdlog::error("Failed to initialize OpenGL context");
		assert(false);
	}

	m_Scene->Init({ windowWidth, windowHeight });
}

template<Derived<Scene> T>
Window<T>::~Window()
{
	spdlog::info("Closing window");

	m_Scene->Delete();

	SDL_GL_DeleteContext(m_GlRenderContext);
	SDL_DestroyWindow(m_Window);
	SDL_Quit();
}

template<Derived<Scene> T>
void Window<T>::Loop()
{
	Timer timer;

	constexpr i8 frameCounterMax = 32;
	i8 frameCounter = 0;
	f64 frameDurationAccumulator = 0.0;

	bool isOpen = true;
	while (isOpen)
	{
		const Duration duration = timer.RestartAndGetElapsedTime();
		f32 deltaTime = m_IsActive ? static_cast<float>(duration.GetInSeconds()) : 0.0f;

		isOpen = HandleEvents();

		m_Scene->Update(deltaTime);

		frameCounter++;
		frameDurationAccumulator += duration.GetInSeconds();
		if (frameCounter == frameCounterMax)
		{
			const f64 averageTime = (frameDurationAccumulator / frameCounterMax) * 1000.0;
			const f64 fps = frameCounter / frameDurationAccumulator;
			const auto title = fmt::format("{} | {:.2f} ms | {} fps", m_WindowName, averageTime, static_cast<i64>(fps));
			SDL_SetWindowTitle(m_Window, title.c_str());

			frameDurationAccumulator = 0.0;
			frameCounter = 0;
		}

		if (CHECK_GL_ERROR())
		{
			isOpen = false;
		}

		SDL_GL_SwapWindow(m_Window);
	}
}

template<Derived<Scene> T>
bool Window<T>::HandleEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			return false;
		case SDL_WINDOWEVENT: {
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				return false;
			case SDL_WINDOWEVENT_RESIZED: {
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
			else if (event.key.keysym.sym == SDLK_F11)
			{
				const u32 sdlFlags = m_IsFullscreen ? 0 : SDL_WINDOW_FULLSCREEN;
				m_IsFullscreen = !m_IsFullscreen;
				SDL_SetWindowFullscreen(m_Window, sdlFlags);
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

	return true;
}
}// namespace stw
