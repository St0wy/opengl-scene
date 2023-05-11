//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <cstdint>
#include <memory>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "scene.hpp"

namespace stw
{
class Window
{
public:
	Window(std::unique_ptr<Scene> scene, const char* windowName, i32 windowWidth = 1280, i32 windowHeight = 720);
	Window(Window&& other) = default;
	Window(const Window& other) = delete;
	~Window();
	Window& operator=(Window&& other) = default;
	Window& operator=(const Window& other) = delete;

	void Loop();

private:
	std::unique_ptr<Scene> m_Scene;
	SDL_Window* m_Window;
	SDL_GLContext m_GlRenderContext;
	bool m_IsActive = true;
};
} // namespace stw
