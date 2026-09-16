module;

#include <glm/glm.hpp>

export module Celestial.Simulation.Datatypes;

import Celestial.GPUDatatypes;

export struct Sphere
{
	float radius{};
	glm::vec3 position{};

	SphereGPUData ToGPUData() const;
};