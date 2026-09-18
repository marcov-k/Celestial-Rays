module;

#include <glm/glm.hpp>

#include <cstdint>

export module Celestial.Simulation.Datatypes;

import Celestial.GPUDatatypes;

export struct Sphere
{
	float radius{};
	glm::vec3 position{};
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