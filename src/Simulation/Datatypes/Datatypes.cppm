module;

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstdint>

export module Celestial.Simulation.Datatypes;

import Celestial.GPUDatatypes;

export struct Sphere
{
	Sphere(float radius, float mass, float restitution, float friction,
		glm::vec3 position, glm::quat rotation, glm::vec3 velocity,
		glm::vec3 angularVelocity, std::uint32_t materialIndex);

	float radius{};
	float radiusSquare{};
	float mass{};
	float inverseMass{};
	float momentOfInertia{};
	float inverseMomentOfInertia{};
	float restitution{};
	float friction{};
	glm::vec3 position{};
	glm::quat rotation{};
	glm::vec3 nextPosition{};
	glm::vec3 velocity{};
	glm::vec3 angularVelocity{};
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