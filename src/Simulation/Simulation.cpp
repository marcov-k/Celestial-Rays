module;

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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

	InitializeSolarSystem();
}

Simulation::~Simulation() { }

void Simulation::StepSimulation(bool paused, bool fastMove, bool slowMove, float deltaTime, const SimulationInput& userInput)
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

		float speed{ CAMERA_MOVE_SPEED };
		if (fastMove) speed *= FAST_MOVE_FACTOR;
		if (slowMove) speed *= SLOW_MOVE_FACTOR;

		cameraPosition += movement * deltaTime * speed;
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
		UpdateRotation(i, deltaTime);
	}
}

SimulationGPUState Simulation::GetGPUState() const
{
	std::size_t sphereCount{ _spheres.size() };
	_gpuSpheres.resize(sphereCount);
	_gpuEmitters.clear();
	for (std::size_t s{}; s < sphereCount; ++s)
	{
		const Sphere& sphere{ _spheres[s] };
		const Material& material{ _materials[sphere.materialIndex] };
		_gpuSpheres[s] = sphere.ToGPUData(glm::length(material.emission) > 0.0f ? STAR_RENDER_SCALE : PLANET_RENDER_SCALE);

		if (glm::length(material.emission) > 0.0f)
		{
			float luminosity{ 0.2126f * material.emission.r +
				0.7152f * material.emission.g + 0.0722f * material.emission.b };
			float samplingWeight{ luminosity * sphere.radiusSquare };
			_gpuEmitters.push_back({ samplingWeight, static_cast<std::uint32_t>(s) });
		}
	}

	return { _camera->GetGPUData(), _gpuSpheres, _gpuEmitters };
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

void Simulation::InitializeSolarSystem()
{
	Material sunMaterial{
	.albedo = glm::vec3{ 1.0f, 0.65f, 0.25f },
	.roughness = 0.25f,
	.specular = 0.0f,
	.indexOfRefraction = 1.0f,
	.emission = glm::vec3{ 5000.0f, 2750.0f, 1100.0f }
	};
	_materials.push_back(sunMaterial);

	Material mercuryMaterial{
		.albedo = glm::vec3{ 0.30f, 0.28f, 0.25f },
		.roughness = 0.65f,
		.specular = 0.04f,
		.indexOfRefraction = 1.50f,
		.emission = glm::vec3{ 0.0f }
	};
	_materials.push_back(mercuryMaterial);

	Material venusMaterial{
		.albedo = glm::vec3{ 0.75f, 0.65f, 0.45f },
		.roughness = 0.80f,
		.specular = 0.04f,
		.indexOfRefraction = 1.45f,
		.emission = glm::vec3{ 0.0f }
	};
	_materials.push_back(venusMaterial);

	Material earthMaterial{
		.albedo = glm::vec3{ 0.18f, 0.30f, 0.22f },
		.roughness = 0.60f,
		.specular = 0.04f,
		.indexOfRefraction = 1.33f,
		.emission = glm::vec3{ 0.0f }
	};
	_materials.push_back(earthMaterial);

	Material marsMaterial{
		.albedo = glm::vec3{ 0.55f, 0.20f, 0.10f },
		.roughness = 0.85f,
		.specular = 0.04f,
		.indexOfRefraction = 1.50f,
		.emission = glm::vec3{ 0.0f }
	};
	_materials.push_back(marsMaterial);

	Material jupiterMaterial{
		.albedo = glm::vec3{ 0.65f, 0.52f, 0.32f },
		.roughness = 0.75f,
		.specular = 0.04f,
		.indexOfRefraction = 1.40f,
		.emission = glm::vec3{ 0.0f }
	};
	_materials.push_back(jupiterMaterial);

	Material saturnMaterial{
		.albedo = glm::vec3{ 0.72f, 0.60f, 0.38f },
		.roughness = 0.75f,
		.specular = 0.04f,
		.indexOfRefraction = 1.40f,
		.emission = glm::vec3{ 0.0f }
	};
	_materials.push_back(saturnMaterial);

	Material uranusMaterial{
		.albedo = glm::vec3{ 0.35f, 0.70f, 0.78f },
		.roughness = 0.70f,
		.specular = 0.04f,
		.indexOfRefraction = 1.38f,
		.emission = glm::vec3{ 0.0f }
	};
	_materials.push_back(uranusMaterial);

	Material neptuneMaterial{
		.albedo = glm::vec3{ 0.12f, 0.35f, 0.70f },
		.roughness = 0.70f,
		.specular = 0.04f,
		.indexOfRefraction = 1.38f,
		.emission = glm::vec3{ 0.0f }
	};
	_materials.push_back(neptuneMaterial);

	Sphere sun{
		0.6957f, // radius
		1.0f, // mass
		0.0f, // restitution
		0.0f, // friction
		{ 0.0f, 0.0f, 0.0f }, // position
		{ 1.0f, 0.0f, 0.0f, 0.0f }, // rotation
		{ -0.0001972194f, 0.0012223776f, 0.0f }, // velocity
		{ 0.0f, 0.0f, 0.0f }, // angular velocity
		0 // material index
	};
	_spheres.push_back(sun);

	Sphere mercury{
		0.0024395f,
		1.65962583e-7f,
		0.1f,
		0.8f,
		{ 57.9f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 4.13653631f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		1
	};
	_spheres.push_back(mercury);

	Sphere venus{
		0.006052f,
		2.44920539e-6f,
		0.1f,
		0.8f,
		{ 76.5089537f, 76.5089537f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ -2.13967343f, 2.13967343f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		2
	};
	_spheres.push_back(venus);

	Sphere earth{
		0.006378f,
		3.00241400e-6f,
		0.1f,
		0.8f,
		{ 0.0f, 149.6f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ -2.57341795f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		3
	};
	_spheres.push_back(earth);

	Sphere mars{
		0.003396f,
		3.22872661e-7f,
		0.1f,
		0.8f,
		{ -161.220346f, 161.220346f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ -1.47398716f, -1.47398716f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		4
	};
	_spheres.push_back(mars);

	Sphere jupiter{
		0.071492f,
		9.54536311e-4f,
		0.05f,
		0.3f,
		{ -778.5f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, -1.12809796f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		5
	};
	_spheres.push_back(jupiter);

	Sphere saturn{
		0.060268f,
		2.85656809e-4f,
		0.05f,
		0.3f,
		{ -1012.576911f, -1012.576911f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.58815205f, -0.58815205f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		6
	};
	_spheres.push_back(saturn);

	Sphere uranus{
		0.025559f,
		4.36531885e-5f,
		0.05f,
		0.3f,
		{ 0.0f, -2867.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.58784425f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		7
	};
	_spheres.push_back(uranus);

	Sphere neptune{
		0.024764f,
		5.12975256e-5f,
		0.05f,
		0.3f,
		{ 3192.587117f, -3192.587117f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.33123197f, 0.33123197f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		8
	};
	_spheres.push_back(neptune);
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
			
			// Resolve position

			glm::vec3 diff{ other.nextPosition - sphere.nextPosition };
			float dist{ glm::length(diff) };
			float minDist{ other.radius + sphere.radius };

			if (dist > minDist) continue;

			float inverseMassSum{ other.inverseMass + sphere.inverseMass };
			if (inverseMassSum == 0.0f) continue;

			float penetration{ minDist - dist };

			glm::vec3 normal{};
			if (dist == 0.0f) normal = glm::vec3{ 1.0f, 0.0f, 0.0f };
			else normal = glm::normalize(diff);

			float sphereCorrection{ -penetration * sphere.inverseMass / inverseMassSum };
			float otherCorrection{ penetration * other.inverseMass / inverseMassSum };

			sphere.nextPosition += sphereCorrection * normal;
			other.nextPosition += otherCorrection * normal;

			if (dist == 0.0f) continue;

			// Resolve impulse

			glm::vec3 sphereContactPoint{ sphere.radius * normal };
			glm::vec3 sphereContactVelocity{ sphere.velocity + glm::cross(sphere.angularVelocity, sphereContactPoint) };

			glm::vec3 otherContactPoint{ -other.radius * normal };
			glm::vec3 otherContactVelocity{ other.velocity + glm::cross(other.angularVelocity, otherContactPoint) };

			glm::vec3 relativeVelocity{ otherContactVelocity - sphereContactVelocity };

			float relativeNormalVelocity{ glm::dot(relativeVelocity, normal) };
			if (relativeNormalVelocity >= 0.0f) continue;
			glm::vec3 normalVelocity{ relativeNormalVelocity * normal };
			glm::vec3 tangentVelocity{ relativeVelocity - normalVelocity };

			float e{ (other.restitution + sphere.restitution) / 2.0f };
			float cof{ (other.friction + sphere.friction) / 2.0f };

			float normalImpulse{ -(1.0f + e) * relativeNormalVelocity / inverseMassSum };
			glm::vec3 normalImpulseVec{ normalImpulse * normal };

			sphere.velocity -= normalImpulseVec * sphere.inverseMass;
			other.velocity += normalImpulseVec * other.inverseMass;

			if (glm::length(tangentVelocity) <= 0.0f) continue;

			glm::vec3 tangent{ glm::normalize(tangentVelocity) };

			float tangentEffectiveMass{ inverseMassSum +
				sphere.radiusSquare * sphere.inverseMomentOfInertia +
				other.radiusSquare * other.inverseMomentOfInertia };

			float tangentImpulse{ -glm::dot(relativeVelocity, tangent) / tangentEffectiveMass };
			float maxTangentImpulse{ cof * normalImpulse };
			tangentImpulse = std::clamp(tangentImpulse, -maxTangentImpulse, maxTangentImpulse);
			glm::vec3 tangentImpulseVec{ tangentImpulse * tangent };

			sphere.velocity -= tangentImpulseVec * sphere.inverseMass;
			other.velocity += tangentImpulseVec * other.inverseMass;

			sphere.angularVelocity += sphere.inverseMomentOfInertia * glm::cross(sphereContactPoint, tangentImpulseVec);
			other.angularVelocity += other.inverseMomentOfInertia * glm::cross(otherContactPoint, tangentImpulseVec);
		}
	}
}

void Simulation::UpdateCurrentPosition(size_t index)
{
	_spheres[index].position = _spheres[index].nextPosition;
}

void Simulation::UpdateRotation(size_t index, float deltaTime)
{
	Sphere& sphere{ _spheres[index] };

	glm::quat angularVelocityQuat{ 0.0f, sphere.angularVelocity };

	sphere.rotation += 0.5f * angularVelocityQuat * sphere.rotation * deltaTime;

	sphere.rotation = glm::normalize(sphere.rotation);
}