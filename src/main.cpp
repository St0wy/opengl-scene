#include "window.hpp"
#include "square_scene.hpp"

int main()
{
	stw::Window window(std::make_unique<stw::SquareScene>(), "OpenGL Scene");
	window.Loop();
}
