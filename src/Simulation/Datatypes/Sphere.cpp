module;

#include <glm/glm.hpp>

module Celestial.Simulation.Datatypes;

SphereGPUData Sphere::ToGPUData() const
{
	return { GPUVec4{ position.x, position.y, position.z, radius }, 0 };
}