#include <spdlog/spdlog.h>

#include "scenes/deferred_shading_scene.hpp"
#include "window.hpp"

int main(int, char*[])
{
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	stw::Window<stw::DeferredShadingScene> window("OpenGL Scene");
	window.Loop();
	return 0;
}
