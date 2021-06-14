#include "engine_window.h"
#include <stdexcept>

namespace EngineCore
{

	EngineWindow::EngineWindow(int w, int h, std::string name) : width{ w }, height{ h }, wndName{ name }
	{
		initWindow();
	}

	EngineWindow::~EngineWindow()
	{
		glfwDestroyWindow(windowPtr);
		glfwTerminate();
	}

	void EngineWindow::initWindow() 
	{
		glfwInit();
		/* the call below prevents an OpenGL context from being created (since we're using Vulkan) 
		* otherwise can cause error with glfwCreateWindowSurface returning VK_ERROR_NATIVE_WINDOW_IN_USE_KHR */
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		windowPtr = glfwCreateWindow(width, height, wndName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(windowPtr, this);
		glfwSetFramebufferSizeCallback(windowPtr, framebufferResizedCallback); // bind framebufferResizedCallback to resize event
	}

	void EngineWindow::createWindowSurface(VkInstance inst, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(inst, windowPtr, nullptr, surface) != VK_SUCCESS) 
		{
			throw std::runtime_error("could not create engine window surface");
		}
	}

	void EngineWindow::framebufferResizedCallback(GLFWwindow* window, int width, int height) 
	{
		auto engineWindow = reinterpret_cast<EngineWindow*>(glfwGetWindowUserPointer(window));
		engineWindow->framebufferResized = true;
		engineWindow->width = width;
		engineWindow->height = height;
	}

} // namespace