#include "Core/sky_rendersys.h"
#include "Types/CommonTypes.h"

namespace EngineCore
{
	SkyRenderSystem::SkyRenderSystem(std::string skyMeshPath, MaterialCreateInfo skyMatInfo, EngineDevice& deviceIn) 
									: device{ deviceIn }
	{
		// prepare sky mesh
		StaticMesh::MeshBuilder builder{};
		builder.loadFromFile(skyMeshPath);
		skyMesh = std::make_unique<StaticMesh>(device, builder);

		// set the sky material to render backfaces, since it will be viewed from inside
		skyMatInfo.shadingProperties.cullModeFlags = VK_CULL_MODE_NONE;
		skyMesh.get()->setMaterial(skyMatInfo); // load sky shaders
	}

	void SkyRenderSystem::renderSky(VkCommandBuffer commandBuffer, VkDescriptorSet sceneGlobalDescriptorSet, 
									const glm::vec3& observerPosition)
	{
		// aliases for convenience
		auto& sky = *skyMesh.get(); 
		auto& skyMat = *sky.getMaterial();

		skyMat.bindToCommandBuffer(commandBuffer); // bind sky shader pipeline

		// bind scene global descriptor set
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyMat.getPipelineLayout(),
									0, 1, &sceneGlobalDescriptorSet, 0, nullptr);

		// sky mesh position should be centered at the observer (camera) at all times
		Transform otf{}; // zero init transform, only translation is relevant
		otf.translation = observerPosition;
		Material::MeshPushConstants push{};
		push.transform = otf.mat4();
		skyMat.writePushConstantsForMesh(commandBuffer, push);

		// record draw command for sky mesh
		sky.bind(commandBuffer);
		sky.draw(commandBuffer);
	}

} // namespace