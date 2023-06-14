//
// Created by stowy on 03/05/2023.
//

#pragma once

#include <SDL.h>

#include "number_types.hpp"

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

	virtual void Init() = 0;
	virtual void Update(f32 deltaTime) = 0;
	virtual void Delete() = 0;

	virtual void OnEvent(const SDL_Event&)
	{
	}

	virtual void OnResize(const i32, const i32)
	{
	}
};
}
