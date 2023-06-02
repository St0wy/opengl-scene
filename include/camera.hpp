#pragma once

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

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

	[[nodiscard]] f32 FovY() const;
	[[nodiscard]] glm::vec3 Position() const;
	[[nodiscard]] glm::vec3 Front() const;
	void SetAspectRatio(f32 aspectRatio);
	void SetMovementSpeed(f32 speed);
	void IncrementMovementSpeed(f32 speedDelta);

	[[nodiscard]] glm::mat4 GetViewMatrix() const;
	[[nodiscard]] glm::mat4 GetProjectionMatrix() const;

	void ProcessMovement(const CameraMovementState& cameraMovementState, f32 deltaTime);
	void ProcessMouseMovement(f32 xOffset, f32 yOffset, bool constrainPitch = true);
	void ProcessMouseScroll(f32 yOffset);

private:
	static constexpr f32 DefaultYaw = -90.0f;
	static constexpr f32 DefaultPitch = 0.0f;
	static constexpr f32 DefaultSpeed = 2.5f;
	static constexpr f32 DefaultSensitivity = 0.1f;
	static constexpr f32 DefaultFovY = 45.0f;
	static constexpr f32 MinFovY = 1.0f;
	static constexpr f32 MaxFovY = 120.0f;
	static constexpr f32 MaxPitchAngle = 89.0f;
	static constexpr f32 DefaultAspectRatio = 16.0f / 9.0f;
	static constexpr f32 NearPlane = 0.001f;
	static constexpr f32 FarPlane = 1000.0f;

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

	void UpdateCameraVectors();
};
}
