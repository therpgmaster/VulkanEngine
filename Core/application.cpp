#include "application.h"
#include "mesh_rendersys.h"
#include "sky_rendersys.h"

#include "Core/Camera.h"
#include "Core/GPU/Material.h"
#include "Core/GPU/Memory/Buffer.h"
#include "Core/GUI_Interface.h"

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

	EngineApplication::~EngineApplication() {};

	void EngineApplication::startExecution()
	{
		MeshRenderSystem meshRenderSys{ device, renderer.getSwapchainRenderPass() };

		// global descriptor set layout
		std::vector<VkDescriptorSetLayout> setLayout =
		{ globalDSetMgr.sceneGlobalSetLayout.get()->getDescriptorSetLayout() };

		// prepare for sky rendering TODO: move this somewhere else
		ShaderFilePaths skyShaders("G:/VulkanDev/VulkanEngine/Core/DevResources/Shaders/sky.vert.spv",
								"G:/VulkanDev/VulkanEngine/Core/DevResources/Shaders/sky.frag.spv");
		MaterialCreateInfo skyMatInfo(renderer.getSwapchainRenderPass(), skyShaders, setLayout, renderSettings);

		SkyRenderSystem skyRenderSys{ "G:/VulkanDev/VulkanEngine/Core/DevResources/Meshes/skysphere.obj", skyMatInfo, device };
		
		// TODO: this is a temporary single-camera setup
		Camera camera{ 45.f, 0.8f, 10.f };
		
		// input setup
		window.input.captureMouseCursor(true);
		setupDefaultInputs();

		// TODO: hardcoded paths - create test material
		ShaderFilePaths shaders("G:/VulkanDev/VulkanEngine/Core/DevResources/Shaders/shader.vert.spv",
			"G:/VulkanDev/VulkanEngine/Core/DevResources/Shaders/shader.frag.spv");
		MaterialCreateInfo matInfo(renderer.getSwapchainRenderPass(), shaders, setLayout, renderSettings);

		if (loadedMeshes.size() > 0 && loadedMeshes[0]) { for (auto* m : loadedMeshes) { m->setMaterial(matInfo); } }
		else { throw std::runtime_error("could not access loaded mesh"); }

		// create gui container (EXPERIMENTAL)
		Imgui imguiObj{ window, device, renderer.getSwapchainRenderPass(),
					EngineSwapChain::MAX_FRAMES_IN_FLIGHT, WIDTH, HEIGHT, renderSettings.sampleCountMSAA };

		// window event loop
		while (!window.getCloseWindow()) 
		{
			window.input.resetInputValues(); // set all input values to zero
			window.input.updateBoundInputs(); // get new input states
			window.pollEvents();
			// render frame
			if (auto commandBuffer = renderer.beginFrame()) 
			{
				int frameIndex = renderer.getFrameIndex();
				engineClock.measureFrameDelta(frameIndex);

				// update scene global descriptors
				SceneGlobalDataBuffer bufferData{};
				bufferData.projectionViewMatrix = camera.getProjectionMatrix()
					* Camera::getWorldBasisMatrix() * camera.getViewMatrix(true);
				globalDSetMgr.writeToSceneGlobalBuffer(frameIndex, bufferData, true);

				//imguiObj.newFrame(); // imgui

				renderer.beginSwapchainRenderPass(commandBuffer);

				//imguiObj.demo(); // imgui demo
				//ImGui::Text("Hello, world %d", 123);
				//ImGui::Button("Save");
				
				// render sky sphere
				skyRenderSys.renderSky(commandBuffer, globalDSetMgr.sceneGlobalSets[frameIndex], camera.transform.translation);
				// render meshes
				meshRenderSys.renderMeshes(commandBuffer, loadedMeshes, engineClock.getDelta(), engineClock.getElapsed(),
											globalDSetMgr.sceneGlobalSets[frameIndex]);
				
				//imguiObj.render(commandBuffer); // imgui

				// camera movement
				auto lookInput = window.input.getMouseDelta();
				auto mf = window.input.getAxisValue(0);
				auto mr = window.input.getAxisValue(1);
				auto mu = window.input.getAxisValue(2);
				camera.moveInPlaneXY(lookInput, mf, mr, mu, engineClock.getDelta());

				renderer.endSwapchainRenderPass(commandBuffer);
				renderer.endFrame(); // submit command buffer
				camera.aspectRatio = renderer.getAspectRatio();
			}
		}
		// window pending close, wait for GPU
		vkDeviceWaitIdle(device.device());
	}

	void EngineApplication::loadActors() 
	{
		
		StaticMesh::MeshBuilder builder{};
		builder.loadFromFile("G:/VulkanDev/VulkanEngine/Core/DevResources/Meshes/6star.obj"); // TODO: hardcoded paths

		for (uint32_t i = 0; i < 600; i++) 
		{ 
			loadedMeshes.push_back(new StaticMesh(device, builder));
			loadedMeshes[i]->transform.translation.x = 0.5f * i;
		}

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