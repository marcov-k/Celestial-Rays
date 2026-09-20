module;

#include <glm/glm.hpp>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

export module Celestial.Simulation;

import Celestial.GPUDatatypes;
import Celestial.Simulation.Camera;
import Celestial.Simulation.Datatypes;
import Celestial.Simulation.Input;

export class Simulation
{
public:
	Simulation(float fieldOfView, std::uint32_t windowWidth, std::uint32_t windowHeight);
	~Simulation();

	void StartSimulation();

	void SetPaused(bool paused);

	void UpdateCamera(float deltaTime, bool fastMove, bool slowMove, const SimulationInput& userInput);

	SimulationGPUState GetGPUState() const;
	CameraGPUData GetCameraGPUData() const;
	std::vector<MaterialGPUData> GetMaterialGPUData() const;

private:
	static const glm::vec3 WORLD_UP;
	static constexpr float PHYSICS_UPDATES_PER_SECOND{ 120.0f };
	static constexpr float PHYSICS_TIME_STEP{ 1.0f / PHYSICS_UPDATES_PER_SECOND };
	static constexpr float GRAVITY_CONSTANT{ 990.723f };
	static constexpr float MOUSE_SENSITIVITY{ 0.005f };
	static constexpr float CAMERA_MOVE_SPEED{ 30.0f };
	static constexpr float FAST_MOVE_FACTOR{ 10.0f };
	static constexpr float SLOW_MOVE_FACTOR{ 0.2f };
	static constexpr float PLANET_RENDER_SCALE{ 100.0f };
	static constexpr float STAR_RENDER_SCALE{ 10.0f };
	static constexpr size_t PARALLEL_THRESHOLD{ 300 };
	static constexpr float EPSILON{ 0.001f };

	std::jthread _simulationThread;
	std::atomic_bool _paused{ true };

	std::optional<Camera> _camera;

	std::vector<Sphere> _spheres{};
	std::vector<Material> _materials{};

	mutable Snapshot _snapshots[3];
	mutable std::mutex _snapshotStateMutex;
	size_t _publishedSnapshot{};

	void InitializeSnapshots();

	void InitializeSolarSystem();

	void SimulationLoop(std::stop_token stopToken);
	void UpdatePhysics(float timeStep);
	void PublishSnapshot();
	size_t FindAvailableSnapshot();

	glm::vec3 CalculateAcceleration(size_t index) const;
	void UpdateVelocity(size_t index, const glm::vec3& acceleration, float timeStep);
	void PrepareNewPosition(size_t index, float timeStep);
	void ResolveCollisions();
	void UpdateCurrentPosition(size_t index);
	void UpdateRotation(size_t index, float timeStep);
};