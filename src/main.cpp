#include <spdlog/spdlog.h>

#include "window.hpp"
#include "scenes/light_scene.hpp"

int main(int, char*[])
{
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	stw::Window window(std::make_unique<stw::LightScene>(), "OpenGL Scene");
	window.Loop();
	return 0;
}
