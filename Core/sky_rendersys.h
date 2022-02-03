#pragma once
#include "Core/ECS/Primitive.h"
#include "Core/GPU/MaterialsManager.h"
#include <memory>
#include <string>
#include <glm/glm.hpp>

namespace EngineCore
{
	class SkyRenderSystem 
	{
	public:
		SkyRenderSystem(MaterialsManager& mgr, std::vector<VkDescriptorSetLayout>& setLayouts,
						EngineDevice& device);

		void renderSky(VkCommandBuffer commandBuffer, VkDescriptorSet sceneGlobalDescriptorSet, 
						const glm::vec3& observerPosition);

	private:
		std::unique_ptr<ECS::Primitive> skyMesh;
	};

} // namespace
