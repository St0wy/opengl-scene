/**
 * @file scene.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Scene class.
 * @version 1.0
 * @date 09/11/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <glm/vec2.hpp>
#include <SDL.h>

export module scene;

import number_types;

export namespace stw
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

	virtual void Init(glm::uvec2 screenSize) = 0;
	virtual void Update(f32 deltaTime) = 0;
	virtual void Delete() = 0;

	virtual void OnEvent(const SDL_Event& event) {}

	virtual void OnResize(const i32 sizeX, const i32 sizeY) {}
};
}// namespace stw
