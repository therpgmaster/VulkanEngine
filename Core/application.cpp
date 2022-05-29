#include "application.h"
#include "mesh_rendersys.h"
#include "sky_rendersys.h"

#include "Core/Camera.h"
#include "Core/GPU/Material.h"
#include "Core/GPU/Memory/Buffer.h"
#include "Core/GPU/Memory/Image.h"
#include "Core/GUI_Interface.h"

#include <stdexcept>
#include <array>
#include <iostream>
#include <string>

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

		// texture test TODO: this is not an ideal way to store these objects
		Image& marsTexture = *new Image(device, makePath("Textures/mars6k_v2.jpg"));
		Image& spaceTexture = *new Image(device, makePath("Textures/space.png"));

		// runtime descriptors test
		UBOCreateInfo ubo1{ device };
		ubo1.addMember(UBOCreateInfo::IntrMemberType::mat4); // MVP matrix
		ubo1.addMember(UBOCreateInfo::IntrMemberType::vec3); // camera position

		dset.addUBO(ubo1);
		dset.addImageArray(std::vector<VkImageView>{ marsTexture.imageView, spaceTexture.imageView });
		dset.addSampler(marsTexture.sampler);
		//dset.addCombinedImageSampler(marsTexture.imageView, marsTexture.sampler);
		//dset.addCombinedImageSampler(spaceTexture.imageView, spaceTexture.sampler);
		dset.finalize();
		std::vector<VkDescriptorSetLayout> dsetLayout = { dset.getLayout() };
		
		// prepare for sky rendering
		SkyRenderSystem skyRenderSys{ materialsMgr, dsetLayout, device };
		
		// TODO: this is a temporary single-camera setup
		Camera camera{ 45.f, 0.8f, 10.f };
		camera.transform.translation.x = -8.f;
		
		// input setup
		window.input.captureMouseCursor(true);
		setupDefaultInputs();

		// create test materials
		ShaderFilePaths shader(makePath("Shaders/shader.vert.spv"),
								makePath("Shaders/shader.frag.spv"));

		auto mat1 = materialsMgr.createMaterial(MaterialCreateInfo(shader, dsetLayout));
		//auto mat2 = materialsMgr.createMaterial(MaterialCreateInfo(shader2, setLayout));

		if (loadedMeshes.size() > 0 && loadedMeshes[0]) { for (auto* m : loadedMeshes) 
		{ 
			//if (m->useFakeScale) { m->setMaterial(mat2); continue; } //FakeScaleTest082
			m->setMaterial(mat1); }
		}
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
				const uint32_t frameIndex = renderer.getFrameIndex(); // current framebuffer index
				engineClock.measureFrameDelta(frameIndex);

				glm::mat4 pvm{ 1.f };
				pvm = camera.getProjectionMatrix() * Camera::getWorldBasisMatrix() * camera.getViewMatrix(true);
				dset.writeUBOMember(0, pvm, 0, frameIndex);

				//applyWorldOriginOffset(camera.transform); //(TODO: ) experimental

				//imguiObj.newFrame(); // imgui

				renderer.beginSwapchainRenderPass(commandBuffer);

				//imguiObj.demo(); // imgui demo
				//ImGui::Text("Hello, world %d", 123);
				//ImGui::Button("Save");
				
				// render sky sphere
				skyRenderSys.renderSky(commandBuffer, dset.getDescriptorSet(frameIndex), camera.transform.translation);

				//simulateDistanceByScale(*loadedMeshes[1], camera.transform); //FakeScaleTest082

				// render meshes
				meshRenderSys.renderMeshes(commandBuffer, loadedMeshes, engineClock.getDelta(), engineClock.getElapsed(),
											dset.getDescriptorSet(frameIndex), simDistOffsets); //FakeScaleTest082
				
				//imguiObj.render(commandBuffer); // imgui

				// camera movement
				auto lookInput = window.input.getMouseDelta();
				auto mf = window.input.getAxisValue(0);
				auto mr = window.input.getAxisValue(1);
				auto mu = window.input.getAxisValue(2);
				auto xs = window.input.getAxisValue(3) > 0 ? true : false;
				camera.moveInPlaneXY(lookInput, mf, mr, mu, xs, engineClock.getDelta());

				renderer.endSwapchainRenderPass(commandBuffer);
				renderer.endFrame(); // submit command buffer
				camera.aspectRatio = renderer.getAspectRatio();
			}
		}
		delete& marsTexture; delete& spaceTexture;
		// window pending close, wait for GPU
		vkDeviceWaitIdle(device.device());
	}

	void EngineApplication::loadActors() 
	{
		using namespace ECS;

		Primitive::MeshBuilder builder{};
		builder.loadFromFile(makePath("Meshes/mars.obj")); // TODO: hardcoded path
		loadedMeshes.push_back(new Primitive(device, builder));
		loadedMeshes[0]->getTransform().translation = Vec{160.f, 0.f, 0.f};
		loadedMeshes[0]->getTransform().scale = 120.f;

		/*builder.loadFromFile("G:/VulkanDev/VulkanEngine/Core/DevResources/Meshes/sphere.obj");
		loadedMeshes.push_back(new Primitive(device, builder)); 
		loadedMeshes[1]->useFakeScale = true;//FakeScaleTest082 testing on mesh at index 1
		loadedMeshes[1]->getTransform().translation.x = 0.f;
		loadedMeshes[1]->getTransform().scale = 1.f;*/

		return; // TODO: function terminates here!
		for (uint32_t i = 0; i < 1; i++) 
		{ 
			loadedMeshes.push_back(new Primitive(device, builder));
			loadedMeshes[i]->getTransform().translation.x = 0.5f * i;
			if (i == 1) { loadedMeshes[i]->useFakeScale = true; } 
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
		// move faster
		inputSys.addBinding(KeyBinding(GLFW_KEY_LEFT_SHIFT, 1.f), "kbFasterAxis");
	}
	/*
	void EngineApplication::simulateDistanceByScale(const StaticMesh& mesh, const Transform& cameraTransform)
	{
		// old, regular-precision version
		//const auto actualScale = mesh.transform.scale.x; // uniform scale required! (for now)
		//const auto actualLoc = mesh.transform.translation;
		//const auto camLoc = cameraTransform.translation;
		//
		//const float max = 2.f;
		//const auto dst = Vec::distance(actualLoc, camLoc); std::cout << "\n" << dst;
		//
		//
		//auto dir = Vec::direction(camLoc, actualLoc);
		//const auto offset = dst - max; // offset between actual location and render location, along direction
		//const auto scale = max / dst; // fake scale, decided by observer distance
		//
		//simDistOffsets.translation = actualLoc + (dir * -offset);
		//simDistOffsets.scale = actualScale * scale;

		const float actualScale_f = 696340 * 2000;
		const float actualLoc_f = 2000000 * 1000;

		double actualScale = (double)actualScale_f;
		auto actualLoc = Vector3D<double>{ (double)actualLoc_f, 0.0, 0.0 };
		const auto camLoc = Vector3D<double>{ cameraTransform.translation.x, cameraTransform.translation.y, cameraTransform.translation.z };

		const double max = 8.0;
		const double dst = ddist(actualLoc, camLoc);
		const auto dir = ddir(camLoc, actualLoc);
		const auto offset = dst - max;
		const auto scale = max / dst;

		const Vector3D<double> t = actualLoc + (dir * -offset);
		const double s = actualScale * scale;

		simDistOffsets.translation = { (float)t.x, (float)t.y, (float)t.z };
		simDistOffsets.scale = (float)s;
	}

	double EngineApplication::ddist(const Vector3D<double>& a, const Vector3D<double>& b)
	{
		const double dsq = pow(b.x - a.x, 2) + pow(b.y - a.y, 2) + pow(b.z - a.z, 2);
		return sqrt(dsq);
	}

	Vector3D<double> EngineApplication::ddir(const Vector3D<double>& a, const Vector3D<double>& b) 
	{
		Vector3D<double> k = b - a; // direction
		// normalization to unit
		const auto sum = (k.x * k.x) + (k.y * k.y) + (k.z * k.z);
		const auto f = 1.0 / sqrt(sum);
		return { k.x * f, k.y * f, k.z * f };
	}
		*/
	void EngineApplication::applyWorldOriginOffset(Transform& cameraTransform)
	{
		const auto threshold = 250000.f;
		auto& tft = cameraTransform.translation;

		const auto x = std::max(std::max(abs(tft.x), abs(tft.y)), abs(tft.z));
		if (x < threshold) { return; }

		const auto nw = cameraTransform.translation;

		std::cout << "\n\n\n\n\n rebasing world origin to x:" << nw.x << " y:" << nw.y << " z:" << nw.z;

		cameraTransform.translation = {0.f,0.f,0.f};
		if (loadedMeshes.size() > 0) 
		{
			for (auto* m : loadedMeshes) { m->getTransform().translation = m->getTransform().translation - nw; }
		}
	}

} // namespace