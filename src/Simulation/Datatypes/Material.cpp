module;

#include <glm/glm.hpp>

module Celestial.Simulation.Datatypes;

MaterialGPUData Material::ToGPUData() const
{
	return { albedo, roughness, specular, emission };
}