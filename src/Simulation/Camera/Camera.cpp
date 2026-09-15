module;

#include <glm/glm.hpp>

module Celestial.Simulation.Camera;

const glm::vec3 Camera::WORLD_UP{ 0.0f, 1.0f, 0.0f };

Camera::Camera(float fieldOfView, float aspectRatio, const glm::vec3& position, const glm::vec2& rotation)
	: _fieldOfView(fieldOfView), _aspectRatio(aspectRatio), _position(position), _rotation(rotation)
{

}

Camera::~Camera() { }

void Camera::SetAspectRatio(float aspectRatio)
{
	_aspectRatio = aspectRatio;
}

glm::vec3& Camera::GetPosition()
{
	return _position;
}

glm::vec2& Camera::GetRotation()
{
	return _rotation;
}

CameraBasisVectors Camera::GetBasisVectors() const
{
	float horizontalMagnitude{ glm::cos(_rotation.x) };
	glm::vec3 forward{ glm::sin(_rotation.y) * horizontalMagnitude, glm::sin(_rotation.x), -glm::cos(_rotation.y) * horizontalMagnitude };
	glm::vec3 right{ glm::cross(forward, WORLD_UP) };
	glm::vec3 up{ glm::cross(right, forward) };
	return { forward, up, right };
}

CameraGPUData Camera::GetGPUData() const
{
	CameraBasisVectors basisVectors{ GetBasisVectors() };

	float planeHeight{ 2.0f * glm::tan(_fieldOfView / 2.0f) };
	float planeWidth{ planeHeight * _aspectRatio };

	return { _position, basisVectors.forward, basisVectors.up, basisVectors.right, planeWidth, planeHeight };
}