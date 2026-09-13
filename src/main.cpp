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

		Window window(instance, 1920, 1080, L"Celestial-Rays");

		VulkanContext vulkan{ window };

		while (!window.ShouldClose())
		{
			window.ProcessEvents();

			if (window.Width() == 0 || window.Height() == 0) continue;

			vulkan.DrawFrame();
		}

		return 0;
	}
	catch (const std::exception& exception)
	{
		std::println(stderr, "Fatal error: {}", exception.what());
		return 1;
	}
}