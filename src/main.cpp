#include <spdlog/spdlog.h>
#include <SDL_main.h>

#include "job_system.hpp"
#include "scenes/ssao_scene.hpp"
#include "window.hpp"


int main(int, char*[])
{
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	stw::job_system::Initialize();
	stw::Window<stw::SsaoScene> window("OpenGL Scene");
	window.Loop();
	return 0;
}
