#include "StaticMesh.h"

// std
#include <cassert>
#include <cstring>

namespace EngineCore
{
	StaticMesh::StaticMesh(EngineDevice& device, const std::vector<Vertex>& vertices) : engineDevice{ device }
	{
		createVertexBuffers(vertices);
	}

	StaticMesh::StaticMesh(EngineDevice& device) : engineDevice{ device } 
	{
		makeCubeMesh();
	}

	StaticMesh::~StaticMesh()
	{
		vkDestroyBuffer(engineDevice.device(), vertexBuffer, nullptr);
		vkFreeMemory(engineDevice.device(), vertexBufferMemory, nullptr);
	}

	void StaticMesh::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "vertexCount cannot be below 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		engineDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT /* autosync ram to vram */,
			vertexBuffer, vertexBufferMemory);
		void* data;
		vkMapMemory(engineDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize)); // copy data to ram
		vkUnmapMemory(engineDevice.device(), vertexBufferMemory);
	}

	void StaticMesh::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void StaticMesh::draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	void StaticMesh::makeCubeMesh() 
	{
		// creates a simple cube mesh
		float s = 0.5f; // default 0.5 (size 1)
		std::vector<Vertex> vertices
		{
			// left face (white)
			{{-s, -s, -s}, {.9f, .9f, .9f}},
			{{-s, s, s}, {.9f, .9f, .9f}},
			{{-s, -s, s}, {.9f, .9f, .9f}},
			{{-s, -s, -s}, {.9f, .9f, .9f}},
			{{-s, s, -s}, {.9f, .9f, .9f}},
			{{-s, s, s}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{s, -s, -s}, {.8f, .8f, .1f}},
			{{s, s, s}, {.8f, .8f, .1f}},
			{{s, -s, s}, {.8f, .8f, .1f}},
			{{s, -s, -s}, {.8f, .8f, .1f}},
			{{s, s, -s}, {.8f, .8f, .1f}},
			{{s, s, s}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-s, -s, -s}, {.9f, .6f, .1f}},
			{{s, -s, s}, {.9f, .6f, .1f}},
			{{-s, -s, s}, {.9f, .6f, .1f}},
			{{-s, -s, -s}, {.9f, .6f, .1f}},
			{{s, -s, -s}, {.9f, .6f, .1f}},
			{{s, -s, s}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-s, s, -s}, {.8f, .1f, .1f}},
			{{s, s, s}, {.8f, .1f, .1f}},
			{{-s, s, s}, {.8f, .1f, .1f}},
			{{-s, s, -s}, {.8f, .1f, .1f}},
			{{s, s, -s}, {.8f, .1f, .1f}},
			{{s, s, s}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-s, -s, s}, {.1f, .1f, .8f}},
			{{s, s, s}, {.1f, .1f, .8f}},
			{{-s, s, s}, {.1f, .1f, .8f}},
			{{-s, -s, s}, {.1f, .1f, .8f}},
			{{s, -s, s}, {.1f, .1f, .8f}},
			{{s, s, s}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-s, -s, -s}, {.1f, .8f, .1f}},
			{{s, s, -s}, {.1f, .8f, .1f}},
			{{-s, s, -s}, {.1f, .8f, .1f}},
			{{-s, -s, -s}, {.1f, .8f, .1f}},
			{{s, -s, -s}, {.1f, .8f, .1f}},
			{{s, s, -s}, {.1f, .8f, .1f}},
		};
		createVertexBuffers(vertices);
	}

	std::vector<VkVertexInputBindingDescription> StaticMesh::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> StaticMesh::Vertex::getAttributeDescriptions()
	{
		return
		{
			// location, binding, format, offset
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) },
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) }
		};
	}
}