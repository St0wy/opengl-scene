#include "texture_scene.hpp"
#include "window.hpp"

int main(int, char* [])
{
	stw::Window window(std::make_unique<stw::TextureScene>(), "OpenGL Scene");
	window.Loop();
	return 0;
}
