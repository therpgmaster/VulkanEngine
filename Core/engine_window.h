#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> // GL Framework (GLFW) used to create an engine window
#include <string>

namespace EngineCore
{
	/* engine window wrapper - use windowPtr to get the actual GLFW window */
	class EngineWindow
	{
	public:
		// default constructor/destructor
		EngineWindow(int w, int h, std::string name);
		~EngineWindow();

		EngineWindow(const EngineWindow&) = delete;
		EngineWindow& operator=(const EngineWindow&) = delete;

		// true if the window should be destroyed
		const bool& getCloseWindow() { return glfwWindowShouldClose(windowPtr); }

		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }

		// creates the surface that acts as an interface between the engine and vulkan
		void createWindowSurface(VkInstance inst, VkSurfaceKHR* surface);

	private:
		// window initialization using GLFW (on construct)
		void initWindow();
		static void framebufferResizedCallback(GLFWwindow* window, int width, int height);

		int width;
		int height;
		bool framebufferResized = false;

		// pointer to the GL Framework window object
		GLFWwindow* windowPtr;
		// window name
		std::string wndName;
	};

} // namespace