#include <windows.h>

import Celestial.Window;

int main()
{
	HINSTANCE instance = GetModuleHandleW(nullptr);

	Window window(instance, 1280, 720, L"Celestial-Rays");

	while (!window.ShouldClose())
	{
		window.ProcessEvents();
	}

	return 0;
}