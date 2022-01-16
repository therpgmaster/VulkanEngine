#pragma once
#include <vulkan/vulkan.h>
#include "GUI_InternalTypes.h"

#include <vector>

namespace EngineGUI 
{
	/*
		// initialize imgui library
		ImGui::CreateContext();

		// initialize imgui for GLFW
		ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), false); // install_callbacks = false
		// initialize imgui for Vulkan
		ImGui_ImplVulkan_Init(&init_info, swapchainRenderPass);

		// upload imgui font textures
		auto cb = device.beginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(cb);
		device.endSingleTimeCommands(cb);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	*/
	// 1. construct UI
	// 2. call ImGui::Render
	// 3. call RenderDrawData, passing the draw command lists from GImGui.Viewports[0].DrawDataP (GImGui is the current ImGuiContext)

	// all ImGui functions MUST be called between ImGui::NewFrame() and ImGui::Render()
	// ImGui::Begin() adds the default main window (Called from NewFrame())

	class GUI 
	{
		std::vector<Window*> windows; // dynamically allocated window objects
		int32_t idCounter = 0; // used to generate identifiers for the windows
		const int32_t& getNewWindowId() { return idCounter++;  }
	public:
		GUI();
		~GUI();

		// record draw commands for vulkan
		void render();

		// returns window id
		int32_t createWindow();

		// returns null if window could not be found
		Window* getWindowById(const int32_t& id);
	private:


		//void RenderDrawData_Vulkan(ImDrawData* draw_data, VkCommandBuffer command_buffer, VkPipeline pipeline);
	};

} // namespace
