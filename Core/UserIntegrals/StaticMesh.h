#pragma once

#include "../engine_device.h"
#include "ActorComponent.h"
#include "../materials.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>
#include <stdexcept>
#include <memory>

namespace EngineCore
{

	class StaticMesh : public ActorComponent
	{
	public:
		struct Vertex
		{
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};

			// binding/attribute descriptions are read by the pipeline
			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		glm::vec3 color; // TODO: remove from here and stop using in shaders

		struct MeshBuilder 
		{
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
			void makeCubeMesh();
			void loadFromFile(const std::string& path);
		};
		

		StaticMesh(EngineDevice& engineDevice, const MeshBuilder& builder);
		StaticMesh(EngineDevice& engineDevice, const std::vector<Vertex>& vertices);
		StaticMesh(EngineDevice& engineDevice);
		~StaticMesh();

		StaticMesh(const StaticMesh&) = delete;
		StaticMesh& operator=(const StaticMesh&) = delete;

		// binds the primitive's vertices to prepare for drawing
		void bind(VkCommandBuffer commandBuffer);
		// records a draw call to the command buffer (final step to render mesh)
		void draw(VkCommandBuffer commandBuffer);

		// assigns this mesh a new material, destroying the old one
		void setMaterial(const MaterialCreateInfo& mci)
		{
			material.reset();
			material = std::make_unique<Material>(mci);
		}

		Material* getMaterial() { return material.get(); }
		
	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		EngineDevice& engineDevice;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;
		bool hasIndexBuffer = false;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		uint32_t indexCount;

		// owned material object
		std::unique_ptr<Material> material;

	};
}