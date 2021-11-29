#pragma once

#include "Core/GPU/engine_device.h"
#include "Core/GPU/Memory/Buffer.h"
#include "Core/ECS/ActorComponent.h"
#include "Core/GPU/Material.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// included here because warning-ignore hack only works in header files
#pragma warning(push, 0)
#include "../../ThirdParty/tiny_obj_loader.h"
#pragma warning(pop)

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

		// binds the primitive's vertices to a command buffer (preparation to render)
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

		std::unique_ptr<GBuffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;
		std::unique_ptr<GBuffer> indexBuffer;
		uint32_t indexCount;
		

		// owned material object
		std::unique_ptr<Material> material;

	};
}