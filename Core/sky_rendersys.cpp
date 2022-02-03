#include "Core/sky_rendersys.h"
#include "Types/CommonTypes.h"

namespace EngineCore
{
	SkyRenderSystem::SkyRenderSystem(MaterialsManager& mgr, std::vector<VkDescriptorSetLayout>& setLayouts,
									EngineDevice& device)
	{
		// TODO: hardcoded paths
		const std::string meshPath = "G:/VulkanDev/VulkanEngine/Core/DevResources/Meshes/skysphere.obj";
		ShaderFilePaths skyShaders("G:/VulkanDev/VulkanEngine/Core/DevResources/Shaders/sky.vert.spv",
									"G:/VulkanDev/VulkanEngine/Core/DevResources/Shaders/sky.frag.spv");
		// prepare sky mesh
		ECS::Primitive::MeshBuilder builder{};
		builder.loadFromFile(meshPath);
		skyMesh = std::make_unique<ECS::Primitive>(device, builder);

		// create unique material for sky
		MaterialCreateInfo matInfo(skyShaders, setLayouts);
		// set the sky material to render backfaces, since it will be viewed from inside
		matInfo.shadingProperties.cullModeFlags = VK_CULL_MODE_NONE;
		auto m = mgr.createMaterial(matInfo); // create
		skyMesh.get()->setMaterial(m); // use
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