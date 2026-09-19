module;

#include <glm/glm.hpp>

#include <cstdint>
#include <optional>
#include <vector>

export module Celestial.Simulation;

import Celestial.GPUDatatypes;
import Celestial.Simulation.Camera;
import Celestial.Simulation.Datatypes;
import Celestial.Simulation.Input;

export class Simulation
{
public:
	Simulation(float fieldOfView, std::uint32_t windowWidth, std::uint32_t windowHeight);
	~Simulation();

	void StepSimulation(bool paused, float deltaTime, const SimulationInput& userInput);

	SimulationGPUState GetGPUState() const;
	std::vector<MaterialGPUData> GetMaterialGPUData() const;

private:
	static const glm::vec3 WORLD_UP;
	static constexpr float GRAVITY_CONSTANT{ 1.0f };
	static constexpr float MOUSE_SENSITIVITY{ 0.005f };
	static constexpr float CAMERA_MOVE_SPEED{ 15.0f };
	static constexpr size_t PARALLEL_THRESHOLD{ 300 };
	static constexpr float EPSILON{ 0.001f };

	std::optional<Camera> _camera;

	std::vector<Sphere> _spheres{};
	std::vector<Material> _materials{};

	mutable std::vector<SphereGPUData> _gpuSpheres{};

	glm::vec3 CalculateAcceleration(size_t index) const;
	void UpdateVelocity(size_t index, const glm::vec3& acceleration, float deltaTime);
	void PrepareNewPosition(size_t index, float deltaTime);
	void ResolveCollisions();
	void UpdateCurrentPosition(size_t index);
	void UpdateRotation(size_t index, float deltaTime);
};