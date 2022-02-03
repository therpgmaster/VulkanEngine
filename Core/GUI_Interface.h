#pragma once

#include "Core/engine_window.h"
#include "Core/GPU/engine_device.h"

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_impl_vulkan.h"
#include "ThirdParty/imgui/imgui_impl_glfw.h"

#include <string>

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
		// then gets the draw data from imgui and uses it to record the necessary draw commands
		// to the provided command buffer 
		void render(VkCommandBuffer commandBuffer) 
		{
			ImGui::Render();
			ImDrawData* drawdata = ImGui::GetDrawData();
			ImGui_ImplVulkan_RenderDrawData(drawdata, commandBuffer);
		}

		void demo() { ImGui::ShowDemoWindow(); }

		// for low-level text rendering, see void ImFont::RenderText at line 3536 imgui_draw.h
		// for font atlas building, see ImFontAtlas::Build() at line 2255 imgui_draw.cpp
		// two build modes are available, StbTruetype and FreeType (FreeType seems to be the best)
		void loadFont(const std::string& path);

	private:
		EngineDevice& device;
		VkDescriptorPool descriptorPool;
		GUIRenderPass renderPass;
		void createImages();
	};

} // namespace

