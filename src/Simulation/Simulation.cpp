module;

#include <glm/glm.hpp>

#include <algorithm>
#include <cstdint>
#include <numbers>
#include <optional>
#include <print>

module Celestial.Simulation;

const glm::vec3 Simulation::WORLD_UP{ 0.0f, 1.0f, 0.0f };

Simulation::Simulation(float fieldOfView, std::uint32_t windowWidth, std::uint32_t windowHeight)
{
	float aspectRatio{ static_cast<float>(windowWidth) / static_cast<float>(windowHeight) };
	_camera.emplace(fieldOfView, aspectRatio, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec2{ 0.0f, 0.0f });

	_spheres.push_back({ 1.0f, glm::vec3{ 0.0f, 1.0f, -5.0f } });
}

Simulation::~Simulation() { }

void Simulation::StepSimulation(float deltaTime, const SimulationInput& userInput)
{
	_camera->SetAspectRatio(static_cast<float>(userInput.windowWidth) / static_cast<float>(userInput.windowHeight));

	glm::vec2& cameraRotation{ _camera->GetRotation() };
	cameraRotation.y += userInput.mouseDeltaX * MOUSE_SENSITIVITY;
	cameraRotation.x -= userInput.mouseDeltaY * MOUSE_SENSITIVITY;

	cameraRotation.x = std::clamp(cameraRotation.x, -std::numbers::pi_v<float> / 2.0f + EPSILON, std::numbers::pi_v<float> / 2.0f - EPSILON);

	glm::vec3& cameraPosition{ _camera->GetPosition() };
	CameraBasisVectors basisVectors{ _camera->GetBasisVectors() };

	glm::vec3 movement{ basisVectors.forward * userInput.forward + basisVectors.up * userInput.up + basisVectors.right * userInput.right };
	if (glm::length(movement) > 0.0f)
	{
		movement = glm::normalize(movement);
		cameraPosition += movement * deltaTime * CAMERA_MOVE_SPEED;
	}
}

SimulationGPUState Simulation::GetGPUState() const
{
	std::size_t sphereCount{ _spheres.size() };
	_gpuSpheres.resize(sphereCount);
	for (std::size_t s{}; s < sphereCount; ++s)
	{
		_gpuSpheres[s] = _spheres[s].ToGPUData();
	}

	return { _camera->GetGPUData(), _gpuSpheres };
}