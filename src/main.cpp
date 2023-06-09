#include <spdlog/spdlog.h>

#include "window.hpp"
#include "scenes/instancing_scene.hpp"

int main(int, char*[])
{
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	stw::Window<stw::InstancingScene> window("OpenGL Scene");
	window.Loop();
	return 0;
}
