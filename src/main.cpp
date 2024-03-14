#include <SDL_main.h>
#include <spdlog/spdlog.h>

import number_types;
import window;
import ssao_scene;

int main(int, char*[])
{
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	stw::Window<stw::SsaoScene> window("OpenGL Scene");
	window.Loop();
	return 0;
}
