#include "window.hpp"
#include "texture_scene.hpp"

int main()
{
	stw::Window window(std::make_unique<stw::TextureScene>(), "OpenGL Scene");
	window.Loop();
}
