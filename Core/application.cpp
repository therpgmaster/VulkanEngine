#include "application.h"
#include "engine_render_system.h"

#include <stdexcept>
#include <array>
#include <iostream>
// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace EngineCore
{
	EngineApplication::EngineApplication() 
	{
		loadEngineObjects();
	}

	EngineApplication::~EngineApplication()
	{
	}

	void EngineApplication::startExecution()
	{
		EngineRenderSystem renderSys{ device, renderer.getSwapchainRenderPass() };
		// window event loop
		double deltaTime = 0.0;
		double elapsedTime = 0.0;
		while (!window.getCloseWindow()) 
		{
			measureTiming();
			elapsedTime = elapsedTime + deltaTime;
			//std::cout << " FPS " << getFps(deltaTime) << " time(s) " << elapsedTime << "\n";

			glfwPollEvents();
			if (auto commandBuffer = renderer.beginFrame()) 
			{
				renderer.beginSwapchainRenderPass(commandBuffer);
				renderSys.renderEngineObjects(commandBuffer, engineObjects, deltaTime, elapsedTime);
				renderer.endSwapchainRenderPass(commandBuffer);
				renderer.endFrame();
			}
			deltaTime = getTiming();
		}

		vkDeviceWaitIdle(device.device()); // block until all resources freed
	}

	// temporary helper function, creates a 1x1x1 cube centered at offset
	std::unique_ptr<EngineModel> createCubeModel(EngineDevice& device, glm::vec3 offset, float size) {
		float s = size / 2; // default half size is point five
		std::vector<EngineModel::Vertex> vertices
		{
			// left face (white)
			{{-s, -s, -s}, {.9f, .9f, .9f}},
			{{-s, s, s}, {.9f, .9f, .9f}},
			{{-s, -s, s}, {.9f, .9f, .9f}},
			{{-s, -s, -s}, {.9f, .9f, .9f}},
			{{-s, s, -s}, {.9f, .9f, .9f}},
			{{-s, s, s}, {.9f, .9f, .9f}},
		
			// right face (yellow)
			{{s, -s, -s}, {.8f, .8f, .1f}},
			{{s, s, s}, {.8f, .8f, .1f}},
			{{s, -s, s}, {.8f, .8f, .1f}},
			{{s, -s, -s}, {.8f, .8f, .1f}},
			{{s, s, -s}, {.8f, .8f, .1f}},
			{{s, s, s}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-s, -s, -s}, {.9f, .6f, .1f}},
			{{s, -s, s}, {.9f, .6f, .1f}},
			{{-s, -s, s}, {.9f, .6f, .1f}},
			{{-s, -s, -s}, {.9f, .6f, .1f}},
			{{s, -s, -s}, {.9f, .6f, .1f}},
			{{s, -s, s}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-s, s, -s}, {.8f, .1f, .1f}},
			{{s, s, s}, {.8f, .1f, .1f}},
			{{-s, s, s}, {.8f, .1f, .1f}},
			{{-s, s, -s}, {.8f, .1f, .1f}},
			{{s, s, -s}, {.8f, .1f, .1f}},
			{{s, s, s}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-s, -s, s}, {.1f, .1f, .8f}},
			{{s, s, s}, {.1f, .1f, .8f}},
			{{-s, s, s}, {.1f, .1f, .8f}},
			{{-s, -s, s}, {.1f, .1f, .8f}},
			{{s, -s, s}, {.1f, .1f, .8f}},
			{{s, s, s}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-s, -s, -s}, {.1f, .8f, .1f}},
			{{s, s, -s}, {.1f, .8f, .1f}},
			{{-s, s, -s}, {.1f, .8f, .1f}},
			{{-s, -s, -s}, {.1f, .8f, .1f}},
			{{s, -s, -s}, {.1f, .8f, .1f}},
			{{s, s, -s}, {.1f, .8f, .1f}},
		};
		for (auto& v : vertices) 
		{
			v.position += offset;
		}
		return std::make_unique<EngineModel>(device, vertices);
	}

	void EngineApplication::loadEngineObjects() 
	{
		std::shared_ptr<EngineModel> cubemodel = createCubeModel(device, { 0.f, 0.f, 0.f }, 0.35f);
		auto cube = EngineObject::createObject();
		cube.model = cubemodel;
		cube.transform.translation = { 0.f, 0.f, 0.f };
		cube.transform.scale = { 1.f, 1.f, 1.f };
		engineObjects.push_back(std::move(cube));
		// 2nd cube (test)
		std::shared_ptr<EngineModel> cubemodel2 = createCubeModel(device, { -0.15f, 0.15f, -0.5f }, 0.2f);
		auto cube2 = EngineObject::createObject();
		cube2.model = cubemodel2;
		cube2.transform.translation = { 0.f, 0.f, 0.f };
		cube2.transform.scale = { 1.f, 1.f, 1.f };
		engineObjects.push_back(std::move(cube2));
	}

} // namespace