module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <array>
#include <cstdint>

export module Celestial.Window;

import Celestial.Simulation.Input;

export class Window
{
public:
	Window(HINSTANCE instance, std::uint32_t width, std::uint32_t height, const wchar_t* title);
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	void ProcessEvents();
	SimulationInput GetSimulationInput();

	[[nodiscard]]
	bool ShouldClose() const noexcept;
	bool Resized() const noexcept;
	void ClearResizedFlag() noexcept;
	bool Focused() const noexcept;
	bool Paused() const noexcept;

	std::uint32_t Width() const noexcept;
	std::uint32_t Height() const noexcept;

	[[nodiscard]]
	HWND Handle() const noexcept;
	[[nodiscard]]
	HINSTANCE Instance() const noexcept;

private:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	static constexpr wchar_t _className[] = L"CelestialRaysWindow";

	HINSTANCE _instance{};
	HWND _hwnd{};
	std::uint32_t _width{};
	std::uint32_t _height{};
	bool _shouldClose{};
	bool _resized{};
	bool _focused{ true };
	bool _paused{ true };
	bool _cameraMode{ false };
	bool _setMousePos{ false };
	int _mouseDefaultX{};
	int _mouseDefaultY{};
	int _mouseX{};
	int _mouseY{};
	int _mouseDeltaX{};
	int _mouseDeltaY{};
	std::array<bool, 256> _keys{};

	void LoseFocus();
	void GainFocus();

	void UpdateCameraMode();
	void EnableCameraMode();
	void DisableCameraMode();
	void HandleMouseMove(LPARAM lParam);
	void HandleToggles(WPARAM keyIndex);
};