#include "window.hpp"
#include "triangle_scene.hpp"

int main()
{
	stw::Window window(std::make_unique<stw::TriangleScene>(), "OpenGL Scene");
	window.Loop();
}
