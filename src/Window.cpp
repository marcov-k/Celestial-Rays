module;

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include <cstdint>
#include <stdexcept>

module Celestial.Window;

Window::Window(HINSTANCE instance, std::uint32_t width, std::uint32_t height, const wchar_t* title)
	: _instance(instance), _width(width), _height(height)
{
	WNDCLASSW windowClass{};
	windowClass.lpfnWndProc = &Window::WindowProc;
	windowClass.hInstance = _instance;
	windowClass.lpszClassName = _className;
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClassW(&windowClass);

	_hwnd = CreateWindowExW(
		0,
		_className,
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		nullptr,
		nullptr,
		_instance,
		this
	);

	if (!_hwnd) throw std::runtime_error("Failed to create window");

	ShowWindow(_hwnd, SW_SHOW);
	UpdateWindow(_hwnd);
}

Window::~Window()
{
	if (_hwnd) DestroyWindow(_hwnd);

	UnregisterClassW(_className, _instance);
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* window{ nullptr };

	if (message == WM_NCCREATE)
	{
		auto* createInfo = reinterpret_cast<CREATESTRUCTW*>(lParam);

		window = static_cast<Window*>(createInfo->lpCreateParams);

		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
	}
	else
	{
		window = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
	}

	if (window)
	{
		switch (message)
		{
		case WM_CLOSE:
			window->_shouldClose = true;
			DestroyWindow(hwnd);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SIZE:
			window->_width = LOWORD(lParam);
			window->_height = HIWORD(lParam);
			window->_resized = true;
			return 0;

		case WM_NCDESTROY:
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
			window->_hwnd = nullptr;
			break;

		case WM_KEYDOWN:
			window->_keys[wParam] = true;
			return 0;

		case WM_KEYUP:
			window->_keys[wParam] = false;
			return 0;

		case WM_MOUSEMOVE:
			window->HandleMouseMove(lParam);
			return 0;
		}
	}

	return DefWindowProcW(hwnd, message, wParam, lParam);
}

void Window::ProcessEvents()
{
	MSG message{};

	while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
	{
		if (message.message == WM_QUIT)
		{
			_shouldClose = true;
			continue;
		}

		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

SimulationInput Window::GetSimulationInput()
{
	float mouseDeltaX{ static_cast<float>(_mouseDeltaX) };
	float mouseDeltaY{ static_cast<float>(_mouseDeltaY) };
	_mouseDeltaX = 0;
	_mouseDeltaY = 0;

	float forward{};
	if (_keys['W']) forward += 1.0f;
	if (_keys['S']) forward -= 1.0f;

	float up{};
	if (_keys['E']) up += 1.0f;
	if (_keys['Q']) up -= 1.0f;

	float right{};
	if (_keys['D']) right += 1.0f;
	if (_keys['A']) right -= 1.0f;

	return { _width, _height, mouseDeltaX, mouseDeltaY, forward, up, right };
}

bool Window::ShouldClose() const noexcept
{
	return _shouldClose;
}

bool Window::Resized() const noexcept
{
	return _resized;
}

void Window::ClearResizedFlag() noexcept
{
	_resized = false;
}

std::uint32_t Window::Width() const noexcept
{
	return _width;
}

std::uint32_t Window::Height() const noexcept
{
	return _height;
}

HWND Window::Handle() const noexcept
{
	return _hwnd;
}

HINSTANCE Window::Instance() const noexcept
{
	return _instance;
}

void Window::HandleMouseMove(LPARAM lParam)
{
	int newX{ GET_X_LPARAM(lParam) };
	int newY{ GET_Y_LPARAM(lParam) };

	if (_haveMousePosition)
	{
		_mouseDeltaX += newX - _mouseX;
		_mouseDeltaY += newY - _mouseY;
	}

	_mouseX = newX;
	_mouseY = newY;
	_haveMousePosition = true;
}