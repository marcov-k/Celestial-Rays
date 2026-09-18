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

	Material starMaterial{
	.albedo = glm::vec3{ 1.0f, 0.75f, 0.35f },
	.roughness = 0.2f,
	.specular = 0.1f,
	.indexOfRefraction = 1.0f,
	.emission = glm::vec3{ 4.0f, 2.0f, 0.5f }
	};

	Material waterPlanetMaterial{
		.albedo = glm::vec3{ 0.03f, 0.17f, 0.82f },
		.roughness = 0.4f,
		.specular = 0.3f,
		.indexOfRefraction = 1.23f,
		.emission = glm::vec3{ 0.0f }
	};

	Material rockyPlanetMaterial{
		.albedo = glm::vec3{ 0.55f, 0.32f, 0.18f },
		.roughness = 0.8f,
		.specular = 0.2f,
		.indexOfRefraction = 1.5f,
		.emission = glm::vec3{ 0.0f }
	};

	_materials.push_back(starMaterial);
	_materials.push_back(waterPlanetMaterial);
	_materials.push_back(rockyPlanetMaterial);

	Sphere star{
	.radius = 10.0f,
	.position = glm::vec3{ 0.0f, 0.0f, 25.0f },
	.materialIndex = 0
	};

	Sphere waterPlanet{
		.radius = 1.0f,
		.position = glm::vec3{ 0.0f, 0.0f, -6.0f },
		.materialIndex = 1
	};

	Sphere waterPlanet2{
		.radius = 7.0f,
		.position = glm::vec3{ -15.0f, 0.0f, -3.0f },
		.materialIndex = 1
	};

	Sphere rockyPlanet{
		.radius = 1.5f,
		.position = glm::vec3{ 0.0f, 0.0f, -1.0f },
		.materialIndex = 2
	};

	Sphere rockyPlanet2{
		.radius = 3.0f,
		.position = glm::vec3{ 10.0f, 0.0f, -2.0f },
		.materialIndex = 2
	};

	_spheres.push_back(star);
	_spheres.push_back(waterPlanet);
	_spheres.push_back(waterPlanet2);
	_spheres.push_back(rockyPlanet);
	_spheres.push_back(rockyPlanet2);
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