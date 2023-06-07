#include <spdlog/spdlog.h>

#include "window.hpp"
#include "scenes/cubemap_scene.hpp"

int main(int, char*[])
{
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	stw::Window<stw::CubeMapScene> window("OpenGL Scene");
	window.Loop();
	return 0;
}
