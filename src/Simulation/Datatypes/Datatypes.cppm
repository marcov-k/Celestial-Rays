module;

#include <glm/glm.hpp>

#include <cstdint>

export module Celestial.Simulation.Datatypes;

import Celestial.GPUDatatypes;

export struct Sphere
{
	Sphere(float radius, float mass, glm::vec3 position,
		glm::vec3 velocity, std::uint32_t materialIndex)
		: radius(radius), mass(mass), inverseMass(1.0f / mass),
		  position(position), velocity(velocity), materialIndex(materialIndex)
	{ }

	float radius{};
	float mass{};
	float inverseMass{};
	glm::vec3 position{};
	glm::vec3 nextPosition{};
	glm::vec3 velocity{};
	std::uint32_t materialIndex{};

	SphereGPUData ToGPUData() const;
};

export struct Material
{
	glm::vec3 albedo{};
	float roughness{};
	float specular{};
	float indexOfRefraction{};
	glm::vec3 emission{};

	MaterialGPUData ToGPUData() const;
};