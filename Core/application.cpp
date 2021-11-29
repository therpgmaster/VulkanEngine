#include "application.h"
#include "mesh_rendersys.h"

#include "ECS/CameraComponent.h"
#include "Core/GPU/Material.h"
#include "Core/GPU/Memory/Buffer.h"

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
		MeshRenderSystem renderSys{ device, renderer.getSwapchainRenderPass() };
		
		// TODO: this is a temporary single-camera setup, remember that we also delete this object below
		CameraComponent* camera = new CameraComponent(45.f, 0.8f, 10.f);
		
		// input setup
		window.input.captureMouseCursor(true);
		setupDefaultInputs();

		// TODO: hardcoded paths - create test material
		ShaderFilePaths shaders("G:/VulkanDev/VulkanEngine/Core/DevResources/Shaders/shader.vert.spv",
			"G:/VulkanDev/VulkanEngine/Core/DevResources/Shaders/shader.frag.spv");
		std::vector<VkDescriptorSetLayout> setLayouts =
		{ globalDSetMgr.sceneGlobalSetLayout.get()->getDescriptorSetLayout() };
		MaterialCreateInfo matInfo(device, renderer.getSwapchainRenderPass(), shaders, setLayouts);
		if (loadedMeshes.size() > 0 && loadedMeshes[0]) { for (auto* m : loadedMeshes) { m->setMaterial(matInfo); } }
		else { throw std::runtime_error("could not access loaded mesh"); }

		// window event loop
		while (!window.getCloseWindow()) 
		{
			epTimer.deltaStart();
			//std::cout << " FPS " << getFps(deltaTime) << " time(s) " << elapsedTime << "\n";
			window.input.resetInputValues(); // set all input values to zero
			window.input.updateBoundInputs(); // get input states
			window.pollEvents();
			// render frame
			if (auto commandBuffer = renderer.beginFrame()) 
			{
				int frameIndex = renderer.getFrameIndex();

				// update scene global descriptors
				SceneGlobalDataBuffer bufferData{};
				bufferData.projectionViewMatrix = camera->getProjectionMatrixBlender()
					* CameraComponent::getWorldBasisMatrix() * camera->getViewMatrix(true);
				globalDSetMgr.writeToSceneGlobalBuffer(frameIndex, bufferData, true);

				renderer.beginSwapchainRenderPass(commandBuffer);
				// render meshes
				renderSys.renderMeshes(commandBuffer, loadedMeshes, camera, epTimer.delta(), epTimer.elapsed(),
									 &window.input, globalDSetMgr.sceneGlobalSets[frameIndex]);
				renderer.endSwapchainRenderPass(commandBuffer);
				renderer.endFrame(); // submit command buffer
				camera->aspectRatio = renderer.getAspectRatio();
			}
			epTimer.deltaEnd();
		}
		delete camera;
		vkDeviceWaitIdle(device.device()); // block until GPU finished
		std::cout << "FPS: " << epTimer.fps() << " Elapsed: " << (float)epTimer.elapsed();
	}

	void EngineApplication::loadActors() 
	{
		
		StaticMesh::MeshBuilder builder{};
		builder.loadFromFile("G:/VulkanDev/VulkanEngine/Core/DevResources/Meshes/torus.obj"); // TODO: hardcoded paths

		// create objects
		for (int i = 0; i < 6; i++) 
		{
			glm::vec3 positions[]
			{
				{ 1.f, 0.f, 0.f },
				{ -1.f, 0.f, 0.f },
				{ 0.f, 1.f, 0.f },
				{ 0.f, -1.f, 0.f },
				{ 0.f, 0.f, 1.f },
				{ 0.f, 0.f, -1.f },
			};
			loadedMeshes.push_back(new StaticMesh(device, builder));
			loadedMeshes[i]->transform.translation = positions[i];
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