module;

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

module Celestial.Simulation.Datatypes;

Sphere::Sphere(float radius, float mass, float restitution, float friction,
	glm::vec3 position, glm::quat rotation, glm::vec3 velocity,
	glm::vec3 angularVelocity, std::uint32_t materialIndex)
	: radius(radius), radiusSquare(radius * radius), mass(mass), inverseMass(1.0f / mass),
		momentOfInertia(mass* radius* radius * 2.0f / 5.0f),
		inverseMomentOfInertia(1.0f / momentOfInertia),
		restitution(restitution), friction(friction), position(position),
		rotation(rotation), velocity(velocity), angularVelocity(angularVelocity),
		materialIndex(materialIndex)
{ }

SphereGPUData Sphere::ToGPUData(float renderScale) const
{
	return { GPUVec4{ position.x, position.y, position.z, radius * renderScale },
		GPUVec4{ rotation.w, rotation.x, rotation.y, rotation.z }, materialIndex };
}