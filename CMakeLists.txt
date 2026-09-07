cmake_minimum_required(VERSION 4.0)

project(Celestial-Rays
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Celestial-Rays
	src/main.cpp
	src/Window.cpp
)

target_sources(Celestial-Rays
	PRIVATE
		FILE_SET modules
		TYPE CXX_MODULES
		FILES
			src/Window.cppm
)

target_compile_definitions(Celestial-Rays PRIVATE
	VK_USE_PLATFORM_WIN32_KHR
)

find_package(Vulkan REQUIRED)

target_link_libraries(Celestial-Rays PRIVATE
	Vulkan::Vulkan
)