//
// Created by stowy on 03/05/2023.
//

#pragma once

#include <SDL.h>

namespace stw
{

class Scene
{
public:
	Scene() = default;
	virtual ~Scene() = default;
	Scene(const Scene& other) = delete;
	Scene(Scene&& other) = default;
	Scene& operator=(const Scene& other) = delete;
	Scene& operator=(Scene&& other) = default;

	virtual void Begin() = 0;
	virtual void End() = 0;
	virtual void Update(float dt) = 0;

	virtual void DrawImGui()
	{
	}

	virtual void OnEvent(const SDL_Event& event)
	{
	}
};
}
