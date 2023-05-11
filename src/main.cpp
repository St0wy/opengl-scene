#include <spdlog/common.h>

#include "cube_scene.hpp"
#include "window.hpp"

int main(int, char*[])
{
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	stw::Window window(std::make_unique<stw::CubeScene>(), "OpenGL Scene");
	window.Loop();
	return 0;
}
