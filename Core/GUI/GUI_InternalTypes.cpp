#include "GUI_InternalTypes.h"
#include <cassert>

namespace EngineGUI
{
	std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(EngineGUI::Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions()
	{
		return
		{
			// location, binding, format, offset
			{ 0, 0, VK_FORMAT_R32G32_UINT, offsetof(EngineGUI::Vertex, position) }
		};
	}

	Rectangle::Rectangle(const int32_t& xSize, const int32_t& ySize)
	{
		assert((xSize > 0 && ySize > 0) && "gui window element must be created with positive size");
		vertices = { {0, 0}, {xSize, 0}, {xSize, ySize}, {0, ySize} };
		indices = { 0, 1, 2, 2, 3, 0 };
	}

	void Window::setId(const int32_t& _id) 
	{
		assert(id == -1 && "cannot set id more than once per gui window");
		id = _id;
	}

	void Window::addElement(WindowElement* element) 
	{
		assert(element && "tried to add element by null pointer");
		elements.push_back(element);
	}

	void Window::createBuffers() 
	{
		for (auto* e : elements) 
		{
			if (e) 
			{

			}
		}
	}
}