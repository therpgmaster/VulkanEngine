#include "Input.h"
#include "engine_window.h"

#include <cassert>

#include <iostream>

namespace EngineCore
{

	KeyBinding::KeyBinding(const uint32_t& bindKey, const float& axisInfluence)
	{
		bindingType = InputBindingType::Axis;
		key = bindKey;
		axisValueInfluence = axisInfluence;
	}

	void KeyBinding::execute(InputSystem& context)
	{
		context.setAxisValue(axisIndex, axisValueInfluence);	
	}

	InputSystem::InputSystem(EngineWindow* window) : parentWindow{ window }
	{
		assert((parentWindow && parentWindow->getGLFWwindow()) && "input system initialized with no window reference");
	}

	/*void InputSystem::keyPressedCallback(const int& key, const int& scancode, const int& action, const int& mods)
	{
		
		if (bindings.empty()) { return; }
		for (auto& kb : bindings)
		{
			// find matching key in bindings
			if (key == kb.getKey())
			{
				kb.execute(*this);
				// this makes a keypress only affect a single binding
				if (kb.consumesKeyEvents) { return; }
			}
		}
	}*/

	void InputSystem::mousePosUpdatedCallback(const double& x, const double& y) 
	{
		Vector2D oldPos = mousePosition;
		mousePosition = { x, y };
		mouseDelta = mousePosition - oldPos;
	}

	uint32_t InputSystem::addBinding(KeyBinding binding, const std::string& newAxisName)
	{
		std::string axisName = newAxisName;
		uint32_t axisIndex = axisValues.size();
		if (newAxisName == "NONE") { axisName = "Axis_" + axisIndex; }
		axisValues.push_back(InputAxis(axisName)); // add input axis
		binding.axisIndex = axisIndex;
		bindings.push_back(binding); // add binding
		return axisIndex;
	}

	void InputSystem::addBinding(KeyBinding binding, const uint32_t& axisIndex) 
	{
		if (axisValues.empty() || axisIndex >= axisValues.size()) 
		{ 
			addBinding(binding); // specified axis does not exist, create one
		}
		else 
		{
			binding.axisIndex = axisIndex;
			bindings.push_back(binding);
		}
	}

	float InputSystem::getAxisValue(const uint32_t& index)
	{
		if (axisValues.empty()) { return 0.f; }
		return axisValues.at(index).value;
	}

	void InputSystem::setAxisValue(const uint32_t& index, const float& v)
	{
		if (axisValues.empty() || index > axisValues.size() - 1) { return; }
		axisValues[index].value = v;
	}

	void InputSystem::resetInputValues() 
	{
		mouseDelta = { 0.0 };
		//for (auto& axis : axisValues) { axis.value = 0.f; }
	}

	void InputSystem::captureMouseCursor(const bool& capture)
	{
		GLFWwindow* gw = parentWindow->getGLFWwindow();
		assert(gw != NULL && "input: could not access glfw window");
		if (capture) 
		{
			// capture (disable) cursor
			glfwSetInputMode(gw, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			if (glfwRawMouseMotionSupported() == GLFW_TRUE)
			{ glfwSetInputMode(gw, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); }
		}
		else 
		{
			// release cursor (return control to system)
			glfwSetInputMode(gw, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
			glfwSetInputMode(gw, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	void InputSystem::updateBoundInputs()
	{
		if (bindings.empty() || axisValues.empty()) { return; }

		std::vector<uint32_t> unpressed;

		for (auto& binding : bindings) 
		{
			int32_t key = binding.getKey();
			if (key <= -1 || binding.axisIndex <= -1) { continue; }

			float v = glfwGetKey(parentWindow->getGLFWwindow(), key) == GLFW_PRESS 
						? binding.axisValueInfluence : 0.f;
			axisValues[binding.axisIndex].influences.push_back(v);
		}

		for (auto& a : axisValues) { a.applyInfluences(); }
	}

} // namespace
