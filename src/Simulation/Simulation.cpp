module;

#include <glm/glm.hpp>

#include <omp.h>

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
	.emission = glm::vec3{ 8.0f, 4.0f, 1.0f }
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

	Sphere star{ 10.0f, 4000.0f, glm::vec3{ 0.0f, 0.0f, 50.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, 0 };

	Sphere waterPlanet{ 1.0f, 1.0f, glm::vec3{ 0.0f, 1.0f, -6.0f }, glm::vec3{ 5.0f, 5.0f, 0.0f }, 1 };

	Sphere waterPlanet2{ 7.0f, 3.0f, glm::vec3{ -15.0f, 0.0f, -3.0f }, glm::vec3{ 5.0f, 5.0f, 0.0f }, 1 };

	Sphere rockyPlanet{ 1.5f, 2.0f, glm::vec3{ 0.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, -7.0f, 0.0f }, 2 };

	Sphere rockyPlanet2{ 3.0f, 5.0f, glm::vec3{ 10.0f, 0.0f, -2.0f }, glm::vec3{ 7.0f, 0.0f, 0.0f }, 2 };

	_spheres.push_back(star);
	_spheres.push_back(waterPlanet);
	_spheres.push_back(waterPlanet2);
	_spheres.push_back(rockyPlanet);
	_spheres.push_back(rockyPlanet2);
}

Simulation::~Simulation() { }

void Simulation::StepSimulation(bool paused, float deltaTime, const SimulationInput& userInput)
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

	if (paused) return;

	size_t sphereCount{ _spheres.size() };
	bool parallel{ sphereCount > PARALLEL_THRESHOLD };

	#pragma omp parallel for if (parallel)
	for (std::int64_t i{}; i < sphereCount; ++i)
	{
		glm::vec3 acceleration{ CalculateAcceleration(i) };
		UpdateVelocity(i, acceleration, deltaTime);
		PrepareNewPosition(i, deltaTime);
	}

	ResolveCollisions();

	#pragma omp parallel for if (parallel)
	for (std::int64_t i{}; i < sphereCount; ++i)
	{
		UpdateCurrentPosition(i);
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

glm::vec3 Simulation::CalculateAcceleration(size_t index) const
{
	glm::vec3 acceleration{ 0.0f };
	for (size_t other{}; other < _spheres.size(); ++other)
	{
		if (other == index) continue;

		glm::vec3 diff{ _spheres[other].position - _spheres[index].position };

		float dist{ glm::length(diff) };
		if (dist < EPSILON) continue;

		float gravMag{ GRAVITY_CONSTANT * _spheres[other].mass / (dist * dist) };
		acceleration += gravMag * glm::normalize(diff);
	}
	return acceleration;
}

void Simulation::UpdateVelocity(size_t index, const glm::vec3& acceleration, float deltaTime)
{
	_spheres[index].velocity += acceleration * deltaTime;
}

void Simulation::PrepareNewPosition(size_t index, float deltaTime)
{
	_spheres[index].nextPosition = _spheres[index].position + _spheres[index].velocity * deltaTime;
}

void Simulation::ResolveCollisions()
{
	size_t sphereCount{ _spheres.size() };
	for (size_t s{}; s + 1 < sphereCount; ++s)
	{
		Sphere& sphere{ _spheres[s] };
		for (size_t o{ s + 1 }; o < sphereCount; ++o)
		{
			Sphere& other{ _spheres[o] };

			glm::vec3 diff{ other.nextPosition - sphere.nextPosition };
			float dist{ glm::length(diff) };
			float minDist{ other.radius + sphere.radius };

			if (dist > minDist) continue;

			float inverseMassSum{ other.inverseMass + sphere.inverseMass };
			if (inverseMassSum == 0.0f) continue;

			float penetration{ minDist - dist };

			glm::vec3 dir{};
			if (dist == 0.0f) dir = glm::vec3{ 1.0f, 0.0f, 0.0f };
			else dir = glm::normalize(diff);

			float sphereCorrection{ -penetration * sphere.inverseMass / inverseMassSum };
			float otherCorrection{ penetration * other.inverseMass / inverseMassSum };

			sphere.nextPosition += sphereCorrection * dir;
			other.nextPosition += otherCorrection * dir;
		}
	}
}

void Simulation::UpdateCurrentPosition(size_t index)
{
	_spheres[index].position = _spheres[index].nextPosition;
}