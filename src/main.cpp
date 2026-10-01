#include <windows.h>

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <chrono>
#include <numbers>
#include <print>
#include <stdexcept>

import Celestial.Simulation;
import Celestial.Simulation.Input;
import Celestial.Rendering;
import Celestial.Vulkan;
import Celestial.Window;

int main()
{
	constexpr std::uint32_t WindowWidth{ 1920 };
	constexpr std::uint32_t WindowHeight{ 1080 };
	constexpr float FieldOfView{ std::numbers::pi_v<float> / 3.0f };
	constexpr float Exposure{ 0.25f };
	constexpr bool CorrectGamma{ false };

	try
	{
		HINSTANCE instance{ GetModuleHandleW(nullptr) };
		Window window{ instance, WindowWidth, WindowHeight, L"Celestial-Rays" };
		VulkanContext vulkan{ window };
		Simulation simulation{ FieldOfView, WindowWidth, WindowHeight };
		Renderer renderer{ vulkan, simulation.GetMaterialGPUData(), Exposure, CorrectGamma };

		auto previousTime{ std::chrono::steady_clock::now() };
		std::uint32_t frameIndex{};

		simulation.StartSimulation();
		while (!window.ShouldClose())
		{
			window.ProcessEvents();
			auto simulationInput{ window.GetSimulationInput() };

			auto currentTime{ std::chrono::steady_clock::now() };

			float deltaTime{ std::chrono::duration<float>(currentTime - previousTime).count() };

			previousTime = currentTime;

			if (!window.Focused() || window.Width() == 0 || window.Height() == 0) continue;

			MoveSpeed moveSpeed{ window.GetMoveSpeed() };
			MouseScroll scroll{ window.GetScroll() };

			simulation.SetPaused(window.Paused());
			simulation.UpdateSimulationSpeed(scroll, moveSpeed);
			simulation.UpdateCamera(deltaTime, moveSpeed, simulationInput);

			renderer.Render(simulation.GetGPUState(), simulation.GetCameraGPUData(), frameIndex);

			frameIndex++;
		}

		return 0;
	}
	catch (const std::exception& exception)
	{
		std::println(stderr, "Fatal error: {}", exception.what());
		MessageBoxA(nullptr, exception.what(), "Celestial-Rays - Fatal error", MB_OK | MB_ICONERROR);
		return 1;
	}
	catch (...)
	{
		MessageBoxA(nullptr, "Unknown fatal error", "Celestial-Rays - Fatal error", MB_OK | MB_ICONERROR);
		return 1;
	}
}