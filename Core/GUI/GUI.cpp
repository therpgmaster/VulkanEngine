#include "GUI.h"

namespace EngineGUI
{
	GUI::GUI() 
	{
	}

	GUI::~GUI()
	{
		// destroy windows
		for (auto* w : windows) { delete w; }
	}

	void GUI::render()
	{
		
	}

	int32_t GUI::createWindow()
	{
		Window* w = new Window(); // construct window
		w->setId(getNewWindowId()); // set unique id for the new window
		windows.push_back(w); // add
		return w->getId();
	}

	Window* GUI::getWindowById(const int32_t& id) 
	{
		for (auto* w : windows) { if (id == w->getId()) { return w; } }
		return nullptr;
	}

} // namespace
