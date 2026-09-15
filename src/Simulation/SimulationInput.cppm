module;

#include <cstdint>

export module Celestial.Simulation.Input;

export struct SimulationInput
{
	std::uint32_t windowWidth{};
	std::uint32_t windowHeight{};
	float mouseDeltaX{};
	float mouseDeltaY{};
	float forward{};
	float up{};
	float right{};
};