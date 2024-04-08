/**
 * @file camera.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the Camera class.
 * @version 1.0
 * @date 13/07/2023
 *
 * @copyright SAE (c) 2023
 *
 */

module;

#include <algorithm>
#include <array>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

export module camera;

import number_types;
import consts;
#include <glm/trigonometric.hpp>

export
{
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
		static constexpr usize FrustumVerticesCount = 8;

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
		[[nodiscard]] std::array<glm::vec3, FrustumVerticesCount> GetFrustumCorners() const;

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

	std::array<glm::vec3, Camera::FrustumVerticesCount> ComputeFrustumCorners(
		const glm::mat4& proj, const glm::mat4& view);

	bool CameraMovementState::HasMovement() const { return forward || backward || left || right || up || down; }

	Camera::Camera(glm::vec3 position, glm::vec3 up, f32 yaw, f32 pitch, f32 aspectRatio)
		: m_Position(position), m_Front(glm::vec3(0.0f, 0.0f, -1.0f)), m_Up(up), m_Right(glm::vec3(1.0f, 0.0f, 0.0f)),
		  m_WorldUp(up), m_Yaw(yaw), m_Pitch(pitch), m_MovementSpeed(DefaultSpeed),
		  m_MouseSensitivity(DefaultSensitivity), m_FovY(DefaultFovY), m_AspectRatio(aspectRatio), m_ViewMatrix(),
		  m_ProjectionMatrix()

	{
		UpdateCameraVectors();
		UpdateViewMatrix();
		UpdateProjectionMatrix();
	}

	glm::mat4 Camera::GetViewMatrix() const { return m_ViewMatrix; }

	glm::mat4 Camera::GetProjectionMatrix() const { return m_ProjectionMatrix; }

	void Camera::ProcessMovement(const CameraMovementState& cameraMovementState, const f32 deltaTime)
	{
		const f32 velocity = m_MovementSpeed * deltaTime;

		glm::vec3 flatFront = m_Front;
		flatFront.y = 0;
		flatFront = normalize(flatFront);

		glm::vec3 flatRight = m_Right;
		flatRight.y = 0;
		flatRight = normalize(flatRight);

		if (cameraMovementState.forward)
		{
			m_Position += flatFront * velocity;
		}
		if (cameraMovementState.backward)
		{
			m_Position -= flatFront * velocity;
		}
		if (cameraMovementState.left)
		{
			m_Position -= flatRight * velocity;
		}
		if (cameraMovementState.right)
		{
			m_Position += flatRight * velocity;
		}
		if (cameraMovementState.up)
		{
			m_Position += m_WorldUp * velocity;
		}
		if (cameraMovementState.down)
		{
			m_Position -= m_WorldUp * velocity;
		}

		UpdateViewMatrix();
	}

	void Camera::ProcessMouseMovement(f32 xOffset, f32 yOffset, const bool constrainPitch)
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
		UpdateViewMatrix();
	}

	void Camera::ProcessMouseScroll(const f32 yOffset)
	{
		m_FovY -= yOffset;
		m_FovY = std::clamp(m_FovY, MinFovY, MaxFovY);
		UpdateProjectionMatrix();
	}

	f32 Camera::GetFovY() const { return m_FovY; }

	glm::vec3 Camera::GetPosition() const { return m_Position; }

	glm::vec3 Camera::GetFront() const { return m_Front; }

	void Camera::SetAspectRatio(const f32 aspectRatio)
	{
		m_AspectRatio = aspectRatio;
		UpdateProjectionMatrix();
	}

	void Camera::SetMovementSpeed(const f32 speed) { m_MovementSpeed = speed; }

	void Camera::IncrementMovementSpeed(const f32 speedDelta)
	{
		m_MovementSpeed += speedDelta;
		m_MovementSpeed = std::clamp(m_MovementSpeed, 0.1f, 1000.0f);
	}

	void Camera::UpdateCameraVectors()
	{
		glm::vec3 front;
		front.x = std::cos(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch));
		front.y = std::sin(glm::radians(m_Pitch));
		front.z = std::sin(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch));

		m_Front = normalize(front);
		m_Right = normalize(cross(m_Front, m_WorldUp));
		m_Up = normalize(cross(m_Right, m_Front));
	}

	void Camera::UpdateViewMatrix() { m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up); }

	void Camera::UpdateProjectionMatrix()
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(m_FovY), m_AspectRatio, NearPlane, FarPlane);
	}

	void Camera::SetYaw(const f32 yaw)
	{
		m_Yaw = yaw;
		UpdateCameraVectors();
		UpdateViewMatrix();
	}

	void Camera::SetPitch(const f32 pitch)
	{
		m_Pitch = pitch;
		UpdateCameraVectors();
		UpdateViewMatrix();
	}

	std::array<glm::vec3, Camera::FrustumVerticesCount> Camera::GetFrustumCorners() const
	{
		return ComputeFrustumCorners(m_ProjectionMatrix, m_ViewMatrix);
	}

	f32 Camera::GetAspectRatio() const { return m_AspectRatio; }

	std::array<glm::vec3, Camera::FrustumVerticesCount> ComputeFrustumCorners(
		const glm::mat4& proj, const glm::mat4& view)
	{
		const glm::mat4 inverse = glm::inverse(proj * view);

		usize count = 0;
		std::array<glm::vec3, Camera::FrustumVerticesCount> frustumCorners{};
		for (u32 x = 0; x < 2; ++x)
		{
			for (u32 y = 0; y < 2; ++y)
			{
				for (u32 z = 0; z < 2; ++z)
				{
					const glm::vec4 pt = inverse
										 * glm::vec4(2.0f * static_cast<f32>(x) - 1.0f,
											 2.0f * static_cast<f32>(y) - 1.0f,
											 2.0f * static_cast<f32>(z) - 1.0f,
											 1.0f);
					frustumCorners.at(count++) = pt / pt.w;
				}
			}
		}

		return frustumCorners;
	}
	}// namespace stw
}