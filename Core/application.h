#pragma once

#include "engine_window.h"
#include "engine_device.h"
#include "engine_renderer.h"

// std
#include <memory>
#include <vector>
#include <chrono> // timing

#include "UserIntegrals/StaticMesh.h"
#include "UserIntegrals/GenericActor.h"

namespace EngineCore
{
	// base class for an object representing the entire engine
	class EngineApplication
	{
	public:
		// hardcoded window size in pixels
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		EngineApplication();
		~EngineApplication();

		EngineApplication(const EngineApplication&) = delete;
		EngineApplication& operator=(const EngineApplication&) = delete;

		// begins the main window event loop (call on newly created engine application instances)
		void startExecution();

		bool keyWPressed = false; // TODO: temporary input system
		bool keyAPressed = false;
		bool keySPressed = false;
		bool keyDPressed = false;
		bool keyRPressed = false;
		bool keyFPressed = false;
		void resetPressedKeys() { keyWPressed = false; keyAPressed = false; 
		keySPressed = false; keyDPressed = false; keyRPressed = false; keyFPressed = false;}

	private:
		void loadActors();

		// engine application window (creates a window using GLFW) 
		EngineWindow window{ WIDTH, HEIGHT, "Vulkan Window" };
		// render device (instantiates vulkan)
		EngineDevice device{ window };
		// engine renderer object
		EngineRenderer renderer{ window, device };

		std::vector<StaticMesh*> loadedMeshes{ nullptr };

		// timing
		std::chrono::steady_clock::time_point timingStartPoint;
		void measureTiming()
		{
			timingStartPoint = std::chrono::steady_clock::now();
		};
		double getTiming()
		{ /* don't mess with this function, it'll break */
			std::chrono::duration<double, std::milli> ms = std::chrono::steady_clock::now() - timingStartPoint;
			return ms.count() / 1000.0;
		};
		uint32_t getFps(double deltaTimeSeconds)
		{
			return 1.0 / deltaTimeSeconds;
		};

	};

} // namespace