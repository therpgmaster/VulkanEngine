#include "application.h"
#include "mesh_rendersys.h"

#include "UserIntegrals/CameraComponent.h"
#include "materials.h"

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
		loadActors();
	}

	EngineApplication::~EngineApplication()
	{
	}

	void EngineApplication::startExecution()
	{
		MeshRenderSystem renderSys{ device, renderer.getSwapchainRenderPass() };
		// TODO: hardcoded paths
		ShaderFilePaths shaders("G:/VulkanDev/VulkanEngine/Core/Shaders/shader.vert.spv",
								"G:/VulkanDev/VulkanEngine/Core/Shaders/shader.frag.spv");
		MaterialCreateInfo matInfo(device, renderer.getSwapchainRenderPass(), shaders);
		if (loadedMeshes.size() > 0 && loadedMeshes[0]) { for (auto* m : loadedMeshes) { m->setMaterial(matInfo); } }
		else { throw std::runtime_error("could not access loaded mesh"); }
		
		// TODO: this is a temporary single-camera setup, remember that we also delete this object below
		CameraComponent* camera = new CameraComponent(45.f, 0.8f, 10.f);
		camera->transform.rotation = { 0.f, Transform3D::degToRad(-90.f), Transform3D::degToRad(-90.f) };

		// input setup
		window.input.captureMouseCursor(true);
		setupDefaultInputs();

		double deltaTime = 0.0;
		double elapsedTime = 0.0;
				bool materialSwitchTestDone = false; // only do the temporary test (below) once

		// window event loop (every frame)
		while (!window.getCloseWindow()) 
		{
			measureTiming();
			elapsedTime = elapsedTime + deltaTime;
			//std::cout << " FPS " << getFps(deltaTime) << " time(s) " << elapsedTime << "\n";
			window.input.resetInputValues(); // set all input values to zero
			window.input.updateBoundInputs(); // get input states
			window.pollEvents();
			if (auto commandBuffer = renderer.beginFrame()) 
			{
				renderer.beginSwapchainRenderPass(commandBuffer);
				// render meshes
				renderSys.renderMeshes(commandBuffer, loadedMeshes, camera, deltaTime, elapsedTime, &window.input);
				renderer.endSwapchainRenderPass(commandBuffer);
				renderer.endFrame(); // submit command buffer
				camera->aspectRatio = window.getAspectRatio();
			}
			deltaTime = getTiming();

			// temporary test of materials system (TODO)
			if ((false) && elapsedTime > 1.5 && elapsedTime < 1.8 && !materialSwitchTestDone)
			{
				ShaderFilePaths sp_2("G:/VulkanDev/VulkanEngine/Core/Shaders/shader.vert.spv",
					"G:/VulkanDev/VulkanEngine/Core/Shaders/shader2test.frag.spv");
				MaterialCreateInfo matInfo_2(device, renderer.getSwapchainRenderPass(), sp_2);
				if (loadedMeshes.size() > 0 && loadedMeshes[0]) { loadedMeshes[0]->setMaterial(matInfo_2); }
				materialSwitchTestDone = true;
			}

		}
		delete camera;
		vkDeviceWaitIdle(device.device()); // block until GPU finished
	}

	// creates a 1x1x1 cube centered at offset
	/*std::unique_ptr<EngineModel> createCubeModel(EngineDevice& device, glm::vec3 offset, float size) {
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
	}*/

	void EngineApplication::loadActors() 
	{
		StaticMesh::MeshBuilder builder{};
		builder.loadFromFile("G:/VulkanDev/VulkanEngine/Core/Meshes/x_y_.obj"); // TODO: hardcoded paths
		loadedMeshes.push_back(new StaticMesh(device, builder));

		/*std::shared_ptr<EngineModel> cubemodel = createCubeModel(device, { 0.f, 0.f, 0.f }, 1.f);
		auto cube = EngineObject::createObject();
		cube.model = cubemodel;
		cube.transform.translation = { 0.f, 0.f, 0.f };
		cube.transform.scale = { 0.35f, 0.35f, 0.35f };
		engineObjects.push_back(std::move(cube));

		// 2nd cube (test)
		std::shared_ptr<EngineModel> cubemodel2 = createCubeModel(device, { 0.f, 0.f, 0.f }, 1.f);
		auto cube2 = EngineObject::createObject();
		cube2.model = cubemodel2;
		cube2.transform.translation = { -0.15f, 0.15f, -0.5f };
		cube2.transform.scale = { 0.2f, 0.2f, 0.2f };
		cube2.transform.rotation = { 45.f, 0.f, 0.f };
		engineObjects.push_back(std::move(cube2));*/
	}

	void EngineApplication::setupDefaultInputs()
	{
		assert(&window.input && "error setting up default input bindings");

		InputSystem& inputSys = window.input;

		// add binding for forwards (and backwards) movement
		uint32_t fwdAxisIndex = inputSys.addBinding(KeyBinding(GLFW_KEY_W, 1.f), "kbForwardAxis");
		inputSys.addBinding(KeyBinding(GLFW_KEY_S, -1.f), fwdAxisIndex);
		// right/left
		uint32_t rightAxisIndex = inputSys.addBinding(KeyBinding(GLFW_KEY_D, 1.f), "kbRightAxis");
		inputSys.addBinding(KeyBinding(GLFW_KEY_A, -1.f), rightAxisIndex);
		// up/down
		uint32_t upAxisIndex = inputSys.addBinding(KeyBinding(GLFW_KEY_R, 1.f), "kbUpAxis");
		inputSys.addBinding(KeyBinding(GLFW_KEY_F, -1.f), upAxisIndex);
	}

} // namespace