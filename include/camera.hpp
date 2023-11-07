/**
 * @file camera.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Camera class.
 * @version 1.0
 * @date 13/07/2023
 *
 * @copyright SAE (c) 2023
 *
 */

#pragma once

#include <array>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "consts.hpp"
#include "number_types.hpp"

namespace stw
{
struct CameraMovementState
{
	bool forward{};
	bool backward{};
	bool left{};
	bool right{};
	bool up{};
	bool down{};

	[[nodiscard]] bool HasMovement() const;
};

class Camera
{
public:
	explicit Camera(glm::vec3 position = glm::vec3(0.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		f32 yaw = DefaultYaw,
		f32 pitch = DefaultPitch,
		f32 aspectRatio = DefaultAspectRatio);

	[[nodiscard]] f32 GetFovY() const;
	[[nodiscard]] f32 GetAspectRatio() const;
	[[nodiscard]] glm::vec3 GetFront() const;
	[[nodiscard]] glm::vec3 GetPosition() const;
	void SetAspectRatio(f32 aspectRatio);
	void SetMovementSpeed(f32 speed);
	void SetYaw(f32 yaw);
	void SetPitch(f32 pitch);
	void IncrementMovementSpeed(f32 speedDelta);

	[[nodiscard]] glm::mat4 GetViewMatrix() const;
	[[nodiscard]] glm::mat4 GetProjectionMatrix() const;

	void ProcessMovement(const CameraMovementState& cameraMovementState, f32 deltaTime);
	void ProcessMouseMovement(f32 xOffset, f32 yOffset, bool constrainPitch = true);
	void ProcessMouseScroll(f32 yOffset);
	[[nodiscard]] std::array<glm::vec3, 8> GetFrustumCorners();

private:
	glm::vec3 m_Position;
	glm::vec3 m_Front;
	glm::vec3 m_Up;
	glm::vec3 m_Right;
	glm::vec3 m_WorldUp;
	f32 m_Yaw;
	f32 m_Pitch;
	f32 m_MovementSpeed;
	f32 m_MouseSensitivity;
	f32 m_FovY;
	f32 m_AspectRatio;
	glm::mat4 m_ViewMatrix;
	glm::mat4 m_ProjectionMatrix;

	void UpdateCameraVectors();
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();
};

std::array<glm::vec3, 8> ComputeFrustumCorners(const glm::mat4& proj, const glm::mat4& view);
}// namespace stw
