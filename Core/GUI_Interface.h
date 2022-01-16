#pragma once

#include "Core/engine_window.h"
#include "Core/GPU/engine_device.h"

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_impl_vulkan.h"
#include "ThirdParty/imgui/imgui_impl_glfw.h"

namespace EngineCore 
{
	struct GUIRenderPass 
	{
		struct GUIFrameBufferAttachment 
		{
			VkImage image;
			VkDeviceMemory mem;
			VkImageView view;
		};

		GUIFrameBufferAttachment color;
		GUIFrameBufferAttachment depth;
		uint32_t width, height;
		VkFramebuffer frameBuffer;
		VkRenderPass renderPass;
		VkSampler sampler;
		VkDescriptorImageInfo descriptor;
	};

	class Imgui
	{
	public:
		Imgui(EngineWindow& window, EngineDevice& device, VkRenderPass swapchainRenderPass, 
				uint32_t imageCount, uint32_t width, uint32_t height, VkSampleCountFlagBits samples);
		~Imgui();

		void beginGUIRenderPass(VkCommandBuffer commandBuffer);
		void endGUIRenderPass(VkCommandBuffer commandBuffer);

		void newFrame() 
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

		// this tells imgui that we're done setting up the current frame,
		// then gets the draw data from imgui and uses it to record to the provided
		// command buffer the necessary draw commands
		void render(VkCommandBuffer commandBuffer) 
		{
			ImGui::Render();
			ImDrawData* drawdata = ImGui::GetDrawData();
			ImGui_ImplVulkan_RenderDrawData(drawdata, commandBuffer);
		}

		void demo() { ImGui::ShowDemoWindow(); }

	private:
		EngineDevice& device;
		VkDescriptorPool descriptorPool;
		GUIRenderPass renderPass;
		void createImages();
	};

} // namespace

