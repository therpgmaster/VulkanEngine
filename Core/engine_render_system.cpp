#include "engine_render_system.h"

#include <stdexcept>
#include <array>
#include <iostream> // temporary

// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace EngineCore
{
	struct SimplePushConstantData
	{
		glm::mat4 transform{ 1.f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color; // data to gpu must be aligned to multiples of 16
	};

	EngineRenderSystem::EngineRenderSystem(EngineDevice& deviceIn, VkRenderPass renderPass) : device{deviceIn}
	{
		createPipelineLayout();
		createPipeline(renderPass);
	}

	EngineRenderSystem::~EngineRenderSystem()
	{
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void EngineRenderSystem::createPipelineLayout()
	{
		VkPushConstantRange pushConstRange{};
		pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstRange.offset = 0;
		pushConstRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1; // push constant
		pipelineLayoutInfo.pPushConstantRanges = &pushConstRange;
		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void EngineRenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "tried to create pipeline with uninitialized layout");
		PipelineConfig pipelineConfig{};
		EnginePipeline::defaultPipelineConfig(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<EnginePipeline>(device, pipelineConfig,
			"G:/VulkanDev/VulkanEngine/Core/Shaders/shader.vert.spv",
			"G:/VulkanDev/VulkanEngine/Core/Shaders/shader.frag.spv"); // SPIR-V shader files (hardcoded paths!)
	}

	void EngineRenderSystem::renderEngineObjects(VkCommandBuffer commandBuffer, std::vector<EngineObject>& engineObjects, 
										const float& deltaTimeSeconds, float time)
	{
		pipeline->bind(commandBuffer);

		for (auto& obj : engineObjects)
		{	// spin 3D primitive
			float spinRate = 0.2f;
			obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + (spinRate * deltaTimeSeconds), glm::two_pi<float>());
			obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + ((spinRate/2.f) * deltaTimeSeconds), glm::two_pi<float>());

			obj.transform.translation.z = -0.6f;
			obj.transform.translation.y += deltaTimeSeconds * 0.05f;
			obj.transform.translation.x += deltaTimeSeconds * 0.05f;
			
			SimplePushConstantData push{};
			push.color = obj.color;

			double near = 0.01f;
			double far = 100.f;
			float fov = 45.f;
			float aspect = 800.f / 600.f;

			double l1 = (time / 1); // linear interpolation
			if (l1 > 1.f) { l1 = 1.f; }
			float matlerp = lerp(0.0, 1.0, l1); 
			glm::mat4 lerpM = 
				lerpMat4(matlerp, orthographicProjectionMatrix(near, far), 
					perspectiveProjectionMatrix(aspect, fov, near, far));
			
			push.transform = lerpM * obj.transform.mat4();

			vkCmdPushConstants(
				commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}
	}

	glm::mat4 EngineRenderSystem::lerpMat4(float t, glm::mat4 matA, glm::mat4 matB) 
	{
		glm::mat4 matOut{};

		for (int c = 0; c != 4; c++)
		{
			for (int r = 0; r != 4; r++)
			{
				matOut[c][r] = lerp(matA[c][r], matB[c][r], t);
			}
		}

		return matOut;
	}

} // namespace