#pragma once

#include "Core/engine_window.h"
#include "Core/GPU/engine_device.h"
#include "engine_renderer.h"

// std
#include <memory>
#include <vector>
#include <chrono> // timing

#include "ECS/StaticMesh.h"
#include "ECS/Actor.h"
#include "Types/CommonTypes.h"
#include "Core/GPU/Memory/descriptors.h"

class SharedMaterialsPool;

namespace EngineCore
{
	class EngineProgramTimer
	{
		static const uint32_t deltasHistoryNum = 100;
		using clock = std::chrono::steady_clock;
		using timePoint = clock::time_point;
	public:
		EngineProgramTimer() : startTime{ clock::now() }{};
		void deltaStart() { deltaStartTime = clock::now(); }
		void deltaEnd()
		{
			std::chrono::duration<double, std::milli> ms = clock::now() - deltaStartTime;
			recordDelta(ms.count() / 1000.0);
		}
		double delta() const { return deltas[lastDeltaIndex]; }
		uint32_t fps() const { return (uint32_t) 1.0 / avgDelta(); }
		double elapsed() const
		{
			std::chrono::duration<double, std::milli> ms = clock::now() - startTime;
			return ms.count() / 1000.0;
		}
	private:
		timePoint startTime;
		timePoint deltaStartTime;
		uint32_t lastDeltaIndex = 0;
		double deltas[deltasHistoryNum] = { 0.0 };
		void recordDelta(const double& ds) 
		{
			uint32_t i = lastDeltaIndex + 1;
			if (i == deltasHistoryNum) { i = 0; }
			deltas[i] = ds;
			lastDeltaIndex = i;
		}
		double avgDelta() const
		{
			double v = 0.0;
			for (double d : deltas) { v += d; }
			return v / deltasHistoryNum;
		}
	};

	// base class for an object representing the entire engine
	class EngineApplication
	{
	public:
		// hardcoded window size in pixels
		static constexpr int WIDTH = 1100;
		static constexpr int HEIGHT = 720;

		EngineApplication();
		~EngineApplication();

		EngineApplication(const EngineApplication&) = delete;
		EngineApplication& operator=(const EngineApplication&) = delete;

		// begins the main window event loop
		void startExecution();

	private:
		void loadActors();

		void setupDefaultInputs();

		// engine application window (creates a window using GLFW) 
		EngineWindow window{ WIDTH, HEIGHT, "Vulkan Window" };
		// render device (instantiates vulkan)
		EngineDevice device{ window };
		// the renderer manages the swapchain and the vulkan command buffers
		EngineRenderer renderer{ window, device };

		EngineProgramTimer epTimer{};

		GlobalDescriptorSetManager globalDSetMgr{ device, EngineSwapChain::MAX_FRAMES_IN_FLIGHT };

		std::unique_ptr<DescriptorPool> globalDescriptorPool{};
		std::vector<StaticMesh*> loadedMeshes;

	};

} // namespace