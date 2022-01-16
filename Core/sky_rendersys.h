#pragma once
#include "Core/ECS/StaticMesh.h"
#include <memory>
#include <string>
#include <glm/glm.hpp>

namespace EngineCore
{
	class SkyRenderSystem 
	{
	public:
		SkyRenderSystem(std::string skyMeshPath, MaterialCreateInfo skyMatInfo, EngineDevice& deviceIn);

		void renderSky(VkCommandBuffer commandBuffer, VkDescriptorSet sceneGlobalDescriptorSet, 
						const glm::vec3& observerPosition);

	private:
		EngineDevice& device;
		VkRenderPass renderPass;
		std::unique_ptr<StaticMesh> skyMesh;
	};

} // namespace
