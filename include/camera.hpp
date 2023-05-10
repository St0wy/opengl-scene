#pragma once

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

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
};

class Camera
{
public:
	explicit Camera(glm::vec3 position = glm::vec3(0.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		float yaw = DefaultYaw,
		float pitch = DefaultPitch);

	[[nodiscard]] glm::mat4 GetViewMatrix() const;
	void ProcessMovement(const CameraMovementState& cameraMovementState, float deltaTime);
	void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
	void ProcessMouseScroll(float yOffset);
	[[nodiscard]] float FovY() const;

private:
	static constexpr float DefaultYaw = -90.0f;
	static constexpr float DefaultPitch = 0.0f;
	static constexpr float DefaultSpeed = 2.5f;
	static constexpr float DefaultSensitivity = 0.1f;
	static constexpr float DefaultFovY = 45.0f;
	static constexpr float MinFovY = 1.0f;
	static constexpr float MaxFovY = 120.0f;
	static constexpr float MaxPitchAngle = 89.0f;

	glm::vec3 m_Position;
	glm::vec3 m_Front;
	glm::vec3 m_Up;
	glm::vec3 m_Right;
	glm::vec3 m_WorldUp;
	float m_Yaw;
	float m_Pitch;
	float m_MovementSpeed;
	float m_MouseSensitivity;
	float m_FovY;

	void UpdateCameraVectors();
};
}
