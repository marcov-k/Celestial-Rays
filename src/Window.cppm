module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>

export module Celestial.Window;

export class Window
{
public:
	Window(HINSTANCE instance, std::uint32_t width, std::uint32_t height, const wchar_t* title);

	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	void ProcessEvents();

	[[nodiscard]]
	bool ShouldClose() const noexcept;

	bool Resized() const noexcept;

	void ClearResizedFlag() noexcept;

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
};