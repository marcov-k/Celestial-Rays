module;

#include <glm/glm.hpp>

export module Celestial.GPUDatatypes;

export struct alignas(16) GPUVec3
{
	GPUVec3() { }
	GPUVec3(const glm::vec3& source) : x(source.x), y(source.y), z(source.z) { }

	float x{};
	float y{};
	float z{};

private:
	float padding{};
};

static_assert(sizeof(GPUVec3) == 16);
static_assert(alignof(GPUVec3) == 16);

export struct CameraGPUData
{
	GPUVec3 position{};
	GPUVec3 forward{};
	GPUVec3 up{};
	GPUVec3 right{};
	float planeWidth{};
	float planeHeight{};
};

export struct SimulationGPUState
{
	CameraGPUData cameraData;
};