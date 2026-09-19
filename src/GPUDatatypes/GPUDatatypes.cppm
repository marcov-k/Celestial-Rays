module;

#include <glm/glm.hpp>

#include <vector>

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

export struct alignas(16) GPUVec4
{
	GPUVec4() { }
	GPUVec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { }
	GPUVec4(const glm::vec4& source) : x(source.x), y(source.y), z(source.z), w(source.w) { }

	float x{};
	float y{};
	float z{};
	float w{};
};

static_assert(sizeof(GPUVec4) == 16);
static_assert(alignof(GPUVec4) == 16);

export struct alignas(16) CameraGPUData
{
	GPUVec3 position{};
	GPUVec3 forward{};
	GPUVec3 up{};
	GPUVec3 right{};
	float planeWidth{};
	float planeHeight{};
};

static_assert(sizeof(CameraGPUData) == 80);
static_assert(alignof(CameraGPUData) == 16);

export struct alignas(16) SphereGPUData
{
	GPUVec4 spatialData{}; // (x, y, z, radius)
	GPUVec4 rotation{};
	std::uint32_t materialIndex{};
};

static_assert(sizeof(SphereGPUData) == 48);
static_assert(alignof(SphereGPUData) == 16);

export struct PushConstants
{
	std::uint32_t sphereCount{};
	std::uint32_t frameIndex{};
};

export struct SimulationGPUState
{
	CameraGPUData cameraData;
	std::vector<SphereGPUData>& sphereData;
};

export struct alignas(16) MaterialGPUData
{
	GPUVec3 albedo{};
	float roughness{};
	float specular{};
	float indexOfRefraction{};
	GPUVec3 emission{};
};

static_assert(sizeof(MaterialGPUData) == 48);
static_assert(alignof(MaterialGPUData) == 16);