cmake_minimum_required(VERSION 3.25)

project(Celestial-Rays
	VERSION 1.0.0
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Celestial-Rays
	src/main.cpp
)

target_compile_definitions(Celestial-Rays PRIVATE
	VK_USE_PLATFORM_WIN32_KHR
)

find_package(Vulkan REQUIRED)

target_link_libraries(Celestial-Rays PRIVATE
	Vulkan::Vulkan
)