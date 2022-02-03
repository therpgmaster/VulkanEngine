#pragma once

#include "Core/GPU/engine_device.h"
#include "Core/GPU/Memory/Buffer.h"
#include "Core/ECS/ActorComponent.h"
#include "Core/GPU/Material.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>
#include <stdexcept>
#include <memory>

namespace ECS 
{
	class Primitive
	{
		Transform transform{}; // TODO: primitive should not have its own transform, 
		// instead, getTransform() should be overloaded on components that inherit from Primitive - also remove setTransform()!
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

		Primitive(EngineCore::EngineDevice& engineDevice, const MeshBuilder& builder);
		Primitive(EngineCore::EngineDevice& engineDevice, const std::vector<Vertex>& vertices);
		Primitive(EngineCore::EngineDevice& engineDevice);
		~Primitive();

		Primitive(const Primitive&) = delete;
		Primitive& operator=(const Primitive&) = delete;

		// binds the primitive's vertices to a command buffer (preparation to render)
		void bind(VkCommandBuffer commandBuffer);
		// records a draw call to the command buffer (final step to render mesh)
		void draw(VkCommandBuffer commandBuffer);

		// apply a new material (reports one user removed from previous material)
		void setMaterial(const EngineCore::MaterialHandle& newMaterial);
		EngineCore::Material* getMaterial() const { return materialHandle.get(); }

		bool useFakeScale = false; //TODO: TMP - FakeScaleTest082

		Transform& getTransform() { return transform; } // this should be changed to virtual, returning const
		void setTransform(const Transform& t) { transform = t; }

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		EngineCore::EngineDevice& engineDevice;

		std::unique_ptr<EngineCore::GBuffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;
		std::unique_ptr<EngineCore::GBuffer> indexBuffer;
		uint32_t indexCount;

		EngineCore::MaterialHandle materialHandle{};
	};
} // namespace