#include "engine_window.h"
#include "application.h" // TODO: only used for temporary keyboard input system
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
		glfwSetKeyCallback(windowPtr, keyPressedCallback);
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
		auto thisWindow = reinterpret_cast<EngineWindow*>(glfwGetWindowUserPointer(window));
		thisWindow->framebufferResized = true;
		thisWindow->width = width;
		thisWindow->height = height;
	}

	void EngineWindow::keyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto thisWindow = reinterpret_cast<EngineWindow*>(glfwGetWindowUserPointer(window));
		// TODO: make a more elegant system for registering keyboard input
		if (thisWindow->tmpApplicationObjPtr == nullptr) { return; }
		switch (key) {
		case 87:
			thisWindow->tmpApplicationObjPtr->keyWPressed = true;
			break;
		case 65:
			thisWindow->tmpApplicationObjPtr->keyAPressed = true;
			break;
		case 83:
			thisWindow->tmpApplicationObjPtr->keySPressed = true;
			break;
		case 68:
			thisWindow->tmpApplicationObjPtr->keyDPressed = true;
					}
	}

} // namespace