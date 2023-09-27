#include <SDL_main.h>
#include <spdlog/spdlog.h>

#include "window.hpp"
#include "scenes/ssao_scene.hpp"


int main(int, char*[])
{
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	stw::Window<stw::SsaoScene> window("OpenGL Scene");
	window.Loop();
	return 0;
}
