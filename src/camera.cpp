#include "camera.hpp"

#include <algorithm>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

stw::Camera::Camera(glm::vec3 position, glm::vec3 up, f32 yaw, f32 pitch, f32 aspectRatio)
	: m_Position(position),
	m_Front(glm::vec3(0.0f, 0.0f, -1.0f)),
	m_Up(up),
	m_Right(glm::vec3(1.0f, 0.0f, 0.0f)),
	m_WorldUp(up),
	m_Yaw(yaw),
	m_Pitch(pitch),
	m_MovementSpeed(DefaultSpeed),
	m_MouseSensitivity(DefaultSensitivity),
	m_FovY(DefaultFovY),
	m_AspectRatio(aspectRatio)

{
	UpdateCameraVectors();
}

glm::mat4 stw::Camera::GetViewMatrix() const
{
	return lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 stw::Camera::GetProjectionMatrix() const
{
	return glm::perspective(glm::radians(m_FovY), m_AspectRatio, NearPlane, FarPlane);
}

void stw::Camera::ProcessMovement(const CameraMovementState& cameraMovementState, const f32 deltaTime)
{
	const float velocity = m_MovementSpeed * deltaTime;

	if (cameraMovementState.forward)
		m_Position += m_Front * velocity;
	if (cameraMovementState.backward)
		m_Position -= m_Front * velocity;
	if (cameraMovementState.left)
		m_Position -= m_Right * velocity;
	if (cameraMovementState.right)
		m_Position += m_Right * velocity;
	if (cameraMovementState.up)
		m_Position += m_WorldUp * velocity;
	if (cameraMovementState.down)
		m_Position -= m_WorldUp * velocity;
}

void stw::Camera::ProcessMouseMovement(f32 xOffset, f32 yOffset, const bool constrainPitch)
{
	xOffset *= m_MouseSensitivity;
	yOffset *= m_MouseSensitivity;

	m_Yaw += xOffset;
	m_Pitch += yOffset;

	if (constrainPitch)
	{
		m_Pitch = std::clamp(m_Pitch, -MaxPitchAngle, MaxPitchAngle);
	}

	UpdateCameraVectors();
}

void stw::Camera::ProcessMouseScroll(const f32 yOffset)
{
	m_FovY -= yOffset;
	m_FovY = std::clamp(m_FovY, MinFovY, MaxFovY);
}

float stw::Camera::FovY() const
{
	return m_FovY;
}

glm::vec3 stw::Camera::Position() const
{
	return m_Position;
}

glm::vec3 stw::Camera::Front() const
{
	return m_Front;
}

void stw::Camera::SetAspectRatio(const f32 aspectRatio)
{
	m_AspectRatio = aspectRatio;
}

void stw::Camera::IncrementMovementSpeed(const f32 speedDelta)
{
	m_MovementSpeed += speedDelta;
}

void stw::Camera::UpdateCameraVectors()
{
	glm::vec3 front;
	front.x = std::cos(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch));
	front.y = std::sin(glm::radians(m_Pitch));
	front.z = std::sin(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch));

	m_Front = normalize(front);
	m_Right = normalize(cross(m_Front, m_WorldUp));
	m_Up = normalize(cross(m_Right, m_Front));
}
