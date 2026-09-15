module;

#include<glm/glm.hpp>

#include <cstdint>
#include <optional>

export module Celestial.Simulation;

import Celestial.GPUDatatypes;
import Celestial.Simulation.Camera;
import Celestial.Simulation.Input;

export class Simulation
{
public:
	Simulation(float fieldOfView, std::uint32_t windowWidth, std::uint32_t windowHeight);
	~Simulation();

	void StepSimulation(float deltaTime, const SimulationInput& userInput);

	SimulationGPUState GetGPUState() const;

private:
	static const glm::vec3 WORLD_UP;
	static constexpr float MOUSE_SENSITIVITY{ 0.01f };
	static constexpr float CAMERA_MOVE_SPEED{ 2.0f };
	static constexpr float EPSILON{ 0.001f };

	std::optional<Camera> _camera;
};