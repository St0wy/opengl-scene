module;
#include <SDL_main.h>
#include <spdlog/spdlog.h>

export module main;

import number_types;
import window;
import ssao_scene;

export int main(int, char*[])
{
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	stw::Window<stw::SsaoScene> window("OpenGL Scene");
	window.Loop();
	return 0;
}
