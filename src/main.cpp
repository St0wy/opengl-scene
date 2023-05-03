#include "window.hpp"
#include "scene.hpp"

class TriangleScene : public stw::Scene
{
public:
	void Begin() override
	{

	}

	void End() override
	{

	}

	void Update(float dt) override
	{

	}
};

int main()
{
	stw::Window window(std::make_unique<TriangleScene>(), "OpenGL Scene");
	window.Loop();
}
