#pragma once

#include "Core/engine_window.h"
#include "Core/GPU/engine_device.h"
#include "engine_renderer.h"
#include "Core/GPU/MaterialsManager.h"

// std
#include <memory>
#include <vector>
#include <chrono> // timing
#include <algorithm> // min()

#include "Core/ECS/Primitive.h"
#include "ECS/Actor.h"
#include "Types/CommonTypes.h"
#include "Core/GPU/Memory/descriptors.h"
#include "Core/EngineSettings.h"

class SharedMaterialsPool;

namespace EngineCore
{
	class EngineClock
	{
		using clock = std::chrono::steady_clock;
		using timePoint = clock::time_point;
		const double deltaMax = 0.2;
	public:
		EngineClock() : start{ clock::now() } {};
		void measureFrameDelta(const uint32_t& currentframeIndex) 
		{
			if (currentframeIndex != lastFrameIndex) /* new frame started? */
			{
				if (lastFrameIndex != 959) /* update delta (except on very first frame) */
				{
					std::chrono::duration<double, std::milli> ms = clock::now() - frameDeltaStart;
					frameDelta = std::min(deltaMax, (ms.count() / 1000.0)); 
				}
				// reset timer
				frameDeltaStart = clock::now();
				lastFrameIndex = currentframeIndex;
			}
		}
		const double& getDelta() const { return frameDelta; }
		uint32_t getFps() const { return (uint32_t) 1 / frameDelta; }
		double getElapsed() const
		{
			std::chrono::duration<double, std::milli> ms = clock::now() - start;
			return ms.count() / 1000.0;
		}
	private:
		timePoint start;
		timePoint frameDeltaStart;
		double frameDelta = 0.01;
		uint32_t lastFrameIndex = 959;
	};

	// base class for an object representing the entire engine
	class EngineApplication
	{
	public:
		// hardcoded window size in pixels
		static constexpr int WIDTH = 1100;
		static constexpr int HEIGHT = 720;

		EngineRenderSettings renderSettings{};
		
		EngineApplication();
		~EngineApplication();

		EngineApplication(const EngineApplication&) = delete;
		EngineApplication& operator=(const EngineApplication&) = delete;

		// begins the main window event loop
		void startExecution();

		// TODO: move this function somewhere else
		// also remove temporaries, search for "FakeScaleTest082"
		//void simulateDistanceByScale(const StaticMesh& mesh, const Transform& cameraTransform);
		Transform simDistOffsets{};
		//static double ddist(const Vector3D<double>& a, const Vector3D<double>& b);
		//static Vector3D<double> ddir(const Vector3D<double>& a, const Vector3D<double>& b);

		void applyWorldOriginOffset(Transform& cameraTransform);

	private:
		void loadActors();
		void setupDefaultInputs();

		// engine application window (creates a window using GLFW) 
		EngineWindow window{ WIDTH, HEIGHT, "Vulkan Window" };
		// render device (instantiates vulkan)
		EngineDevice device{ window };
		// the renderer manages the swapchain and the vulkan command buffers
		EngineRenderer renderer{ window, device, renderSettings };
		// the primary materials manager (contains the master material objects)
		MaterialsManager materialsMgr{ renderer, renderSettings, device };

		EngineClock engineClock{};

		//GlobalDescriptorSetManager globalDSetMgr{ device, EngineSwapChain::MAX_FRAMES_IN_FLIGHT };

		DescriptorSet dset{ device, EngineSwapChain::MAX_FRAMES_IN_FLIGHT };

		std::unique_ptr<DescriptorPool> globalDescriptorPool{};
		std::vector<ECS::Primitive*> loadedMeshes;

	};

} // namespace