#include <windows.h>

#include <vulkan/vulkan.h>

#include <chrono>
#include <print>
#include <stdexcept>

import Celestial.Simulation.TestData;
import Celestial.Rendering;
import Celestial.Vulkan;
import Celestial.Window;

int main()
{
	try
	{
		HINSTANCE instance = GetModuleHandleW(nullptr);
		Window window(instance, 1920, 1080, L"Celestial-Rays");
		VulkanContext vulkan{ window };
		Renderer renderer{ vulkan };

		TestData testData{ 1.0f, 0.25f, 0.5f };

		auto startTime = std::chrono::steady_clock::now();

		while (!window.ShouldClose())
		{
			window.ProcessEvents();

			if (window.Width() == 0 || window.Height() == 0) continue;

			float time = std::chrono::duration<float>(std::chrono::steady_clock::now() - startTime).count();

			renderer.Render(time, testData);
		}

		return 0;
	}
	catch (const std::exception& exception)
	{
		std::println(stderr, "Fatal error: {}", exception.what());
		return 1;
	}
}