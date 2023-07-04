#include <spdlog/spdlog.h>

#include "scenes/shadow_mapping_scene.hpp"
#include "window.hpp"

int main(int, char*[])
{
#ifndef NDEBUG
//	spdlog::set_level(spdlog::level::debug);
#endif

	stw::Window<stw::ShadowMappingScene> window("OpenGL Scene");
	window.Loop();
	return 0;
}
