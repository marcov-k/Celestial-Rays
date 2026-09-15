module;

#include <glm/glm.hpp>

export module Celestial.Simulation.Camera;

import Celestial.GPUDatatypes;

export struct CameraBasisVectors
{
	glm::vec3 forward{};
	glm::vec3 up{};
	glm::vec3 right{};
};

export class Camera
{
public:
	explicit Camera(float fieldOfView, float aspectRatio, const glm::vec3& position, const glm::vec2& rotation);
	~Camera();

	void SetAspectRatio(float aspectRatio);
	glm::vec3& GetPosition();
	glm::vec2& GetRotation();

	CameraBasisVectors GetBasisVectors() const;

	CameraGPUData GetGPUData() const;
	
private:
	static const glm::vec3 WORLD_UP;

	glm::vec3 _position{};
	glm::vec2 _rotation{}; // x = pitch, y = yaw (in radians)
	float _fieldOfView{}; // vertical FOV (in radians)
	float _aspectRatio{};
};