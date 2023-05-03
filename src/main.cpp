#include <iostream>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#define SDL_MAIN_HANDLED
#include<SDL.h>

int main(int argc, char* argv[])
{
	SDL_SetMainReady();

	fmt::print("Hello {} !\n", "world");

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	auto window = SDL_CreateWindow(
		"GPR5300",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		1280,
		720,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
	);
	auto glRenderContext = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);

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

			SDL_GL_SwapWindow(window);
		}
	}
	spdlog::info("Closing window");

	SDL_GL_DeleteContext(glRenderContext);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
