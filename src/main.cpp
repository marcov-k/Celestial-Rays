#include <windows.h>

#include <print>
#include <stdexcept>

import Celestial.Vulkan;
import Celestial.Window;

int main()
{
	try
	{
		HINSTANCE instance = GetModuleHandleW(nullptr);

		Window window(instance, 1280, 720, L"Celestial-Rays");

		VulkanContext vulkan{ window };

		while (!window.ShouldClose())
		{
			window.ProcessEvents();
		}

		return 0;
	}
	catch (const std::exception& exception)
	{
		std::println(stderr, "Fatal error: {}", exception.what());
		return 1;
	}
}