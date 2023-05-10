#include <spdlog/common.h>

#include "cube_scene.hpp"
#include "window.hpp"

int main(int, char *[])
{
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif

    constexpr std::int32_t windowWidth = 1280;
    constexpr std::int32_t windowHeight = 720;
    stw::Window window(std::make_unique<stw::CubeScene>(windowWidth, windowHeight), "OpenGL Scene", windowWidth,
                       windowHeight);
    window.Loop();
    return 0;
}
