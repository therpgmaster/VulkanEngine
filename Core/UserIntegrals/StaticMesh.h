#pragma once

#include "../engine_device.h"
#include "ActorComponent.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

namespace EngineCore
{

	class StaticMesh : public ActorComponent
	{
	public:
		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		glm::vec3 color;

		StaticMesh(EngineDevice& engineDevice, const std::vector<Vertex>& vertices);
		StaticMesh(EngineDevice& engineDevice);
		~StaticMesh();

		StaticMesh(const StaticMesh&) = delete;
		StaticMesh& operator=(const StaticMesh&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

		// creates a simple cube mesh
		void makeCubeMesh();

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);

		EngineDevice& engineDevice;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;
	};
}