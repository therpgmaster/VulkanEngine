#pragma once

#include <vector>
#include <string>
#include "CommonTypes.h"

struct GLFWwindow;

namespace EngineCore 
{
	class EngineWindow;
	
	// forward declaration, real class declared below
	class InputSystem;

	enum class InputBindingType { Axis, Event }; // TODO: implement event bindings

	class KeyBinding
	{
	public:
		// axis binding constructor
		KeyBinding(const uint32_t& bindKey, const float& axisInfluence = 1.f);
		
		const uint32_t& getKey() { return key; }
		const InputBindingType& getBindingType() { return bindingType; }
		void execute(InputSystem& context);
		void setAxisIndex(const uint32_t& v) { axisIndex = v; };
		bool consumesKeyEvents = true;

	private:
		uint32_t key = -2;
		InputBindingType bindingType;
		float axisValueInfluence = 1.f;
		uint32_t axisIndex = -1;
	};

	struct InputAxis 
	{
		InputAxis() { name = "input_axis"; }
		InputAxis(const std::string& nameIn) { name = nameIn; }
		float value = 0.f;
		std::string name;
	};

	class InputSystem 
	{
	public:
		InputSystem(EngineWindow* window);

	private:
		EngineWindow* parentWindow;
		std::vector<KeyBinding> bindings;
		std::vector<InputAxis> axisValues;
		Vector2D<double> mousePosition = { 0.f };
		Vector2D<double> mouseDelta = { 0.f };

	public:
		// main window keyboard event callback, executed by the engine window class
		void keyPressedCallback(const int& key, const int& scancode, const int& action, const int& mods);
		void mousePosUpdatedCallback(const double& x, const double& y);

		//	returns the axis index if the binding is an axis input
		uint32_t addBinding(KeyBinding binding, const std::string& newAxisName = "NONE");
		// add another binding to an existing axis
		void addBinding(KeyBinding binding, const uint32_t& axisIndex);
		float getAxisValue(const uint32_t& index);
		void setAxisValue(const uint32_t& index, const float& v);
		void resetInputValues();

		// disables the system cursor, allowing for raw mouse input (use capture=false to release)
		void captureMouseCursor(const bool& capture = true);

		Vector2D<double> getMouseDelta() { return mouseDelta; }

	};

} // namespace
