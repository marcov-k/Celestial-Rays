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
 "src/VulkanContext.cpp" "src/VulkanWrappers/VulkanInstance.cpp" "src/VulkanWrappers/VulkanDebugMessenger.cpp" "src/VulkanWrappers/VulkanResource.cpp" "src/VulkanWrappers/VulkanSurface.cpp" "src/VulkanWrappers/VulkanDevice.cpp" "src/VulkanWrappers/VulkanSwapchain.cpp" "src/VulkanWrappers/VulkanImage.cpp" "src/VulkanWrappers/VulkanImageView.cpp" "src/VulkanWrappers/VulkanDeviceMemory.cpp" "src/VulkanWrappers/VulkanCommandPool.cpp" "src/VulkanWrappers/VulkanShaderModule.cpp" "src/VulkanWrappers/VulkanDescriptorSetLayout.cpp" "src/VulkanWrappers/VulkanPipelineLayout.cpp"  "src/VulkanWrappers/VulkanComputePipeline.cpp" "src/VulkanWrappers/VulkanDescriptorPool.cpp")

target_sources(Celestial-Rays
	PRIVATE
		FILE_SET modules
		TYPE CXX_MODULES
		FILES
			src/Window.cppm
 "src/VulkanContext.cppm" "src/VulkanWrappers/VulkanWrappers.cppm")

target_compile_definitions(Celestial-Rays PRIVATE
	VK_USE_PLATFORM_WIN32_KHR
	CELESTIAL_SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders"
)

find_package(Vulkan REQUIRED)

find_program(GLSLC_EXECUTABLE
	NAMES glslc
	REQUIRED
)

target_link_libraries(Celestial-Rays PRIVATE
	Vulkan::Vulkan
)

set(SHADER_SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/gradient.comp
)

set(SHADER_BINARY
	${CMAKE_CURRENT_BINARY_DIR}/shaders/gradient.comp.spv
)

add_custom_command(
	OUTPUT ${SHADER_BINARY}
	COMMAND ${CMAKE_COMMAND} -E make_directory
			${CMAKE_CURRENT_BINARY_DIR}/shaders

	COMMAND ${GLSLC_EXECUTABLE}
			--target-env=vulkan1.4
			-o ${SHADER_BINARY}
			${SHADER_SOURCE}

	DEPENDS ${SHADER_SOURCE}

	COMMENT "Compiling GLSL shader: gradient.comp"
)

add_custom_target(Celestial-Rays-Shaders
	DEPENDS ${SHADER_BINARY}
)

add_dependencies(Celestial-Rays
	Celestial-Rays-Shaders
)