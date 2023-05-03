//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <memory>

#define SDL_MAIN_HANDLED
#include<SDL.h>

#include "scene.hpp"

namespace stw
{
class Window
{
public:
	Window(std::unique_ptr<Scene> scene, const char* windowName, int32_t windowWidth = 1280, int32_t windowHeight = 720);
	~Window();

	void Loop();
private:
	std::unique_ptr<Scene> m_Scene;
	SDL_Window* m_Window;
	SDL_GLContext m_GlRenderContext;
};
}
