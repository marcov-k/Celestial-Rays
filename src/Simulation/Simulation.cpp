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

	Material rockyPlanetMaterial{
		.albedo = glm::vec3{ 0.55f, 0.32f, 0.18f },
		.roughness = 0.8f,
		.specular = 0.2f,
		.emission = glm::vec3{ 0.0f }
	};

	Material starMaterial{
		.albedo = glm::vec3{ 1.0f, 0.75f, 0.35f },
		.roughness = 0.2f,
		.specular = 0.1f,
		.emission = glm::vec3{ 4.0f, 2.0f, 0.5f }
	};

	_materials.push_back(rockyPlanetMaterial);
	_materials.push_back(starMaterial);

	Sphere rockyPlanet{
		.radius = 1.0f,
		.position = glm::vec3{ 0.0f, 1.0f, -5.0f },
		.materialIndex = 0
	};

	Sphere star{
		.radius = 2.0f,
		.position = glm::vec3{ 2.0f, 0.0f, -15.0f },
		.materialIndex = 1
	};

	_spheres.push_back(rockyPlanet);
	_spheres.push_back(star);
}

Simulation::~Simulation() { }

void Simulation::StepSimulation(float deltaTime, const SimulationInput& userInput)
{
	_camera->SetAspectRatio(static_cast<float>(userInput.windowWidth) / static_cast<float>(userInput.windowHeight));

	glm::vec2& cameraRotation{ _camera->GetRotation() };
	cameraRotation.y += userInput.mouseDeltaX * MOUSE_SENSITIVITY;
	cameraRotation.x -= userInput.mouseDeltaY * MOUSE_SENSITIVITY;

	const static float halfPi{ std::numbers::pi_v<float> / 2.0f };
	cameraRotation.x = std::clamp(cameraRotation.x, -halfPi + EPSILON, halfPi - EPSILON);

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

std::vector<MaterialGPUData> Simulation::GetMaterialGPUData() const
{
	std::size_t materialCount{ _materials.size() };
	std::vector<MaterialGPUData> gpuMaterials(materialCount);
	for (std::size_t m{}; m < materialCount; ++m)
	{
		gpuMaterials[m] = _materials[m].ToGPUData();
	}
	return gpuMaterials;
}