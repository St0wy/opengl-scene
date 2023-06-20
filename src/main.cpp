#include <spdlog/spdlog.h>

#include "window.hpp"
#include "scenes/normal_map_scene.hpp"
#include "ogl/framebuffer.hpp"

int main(int, char*[])
{
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	stw::Window<stw::NormalMapScene> window("OpenGL Scene");
	window.Loop();
	return 0;
}
