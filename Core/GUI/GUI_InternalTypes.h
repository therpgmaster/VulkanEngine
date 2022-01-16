#pragma once
#include <vulkan/vulkan.h>
#include "Core/GPU/Memory/Buffer.h"
#include <vector>
#include <memory>

namespace EngineGUI
{
	struct Vec2ui { uint32_t x = 0; uint32_t y = 0; };
	struct Vertex
	{
		Vertex() = default;
		Vertex(const int32_t& _x, const int32_t& _y)
		{
			position.x = _x;
			position.y = _y;	
		};
		Vec2ui position;

		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	};

	class WindowElement 
	{
	public:
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		Vec2ui extent;
		bool visibility = true;
		uint32_t zorder = 0;
	};

	class Rectangle : public WindowElement
	{
	public:
		Rectangle(const int32_t& xSize, const int32_t& ySize);
	};

	class Window 
	{
		int32_t id = -1; // window id, use getId() to access

		std::vector<WindowElement*> elements;
		void createBuffers();
		std::unique_ptr<EngineCore::GBuffer> vertexBuffer;
		std::unique_ptr<EngineCore::GBuffer> indexBuffer;

	public:
		void setId(const int32_t& _id); // setId() is called ONCE by the parent
		const int32_t& getId() const { return id; }

		Rectangle windowBackground{ 100, 80 }; // window main primitive
		bool visibility = true;

		// stores a pointer, do not move elements after adding them
		void addElement(WindowElement* element);
		
	};


} // namespace

