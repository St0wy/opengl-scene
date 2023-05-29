#include <spdlog/spdlog.h>

#include "window.hpp"
#include "scenes/backpack_scene_outline.hpp"
#include "scenes/backpack_scene.hpp"

int main(int, char*[])
{
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	stw::Window<stw::BackpackSceneOutline> window("OpenGL Scene");
	window.Loop();
	return 0;
}
