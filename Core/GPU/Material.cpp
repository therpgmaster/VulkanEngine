#include "Core/GPU/Material.h"
#include <stdexcept>
#include <cassert>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace EngineCore 
{
	/* Material */

	Material::Material(const MaterialCreateInfo& materialCreateInfo)
	{
		info = materialCreateInfo;
		if (!info.device) { throw std::runtime_error("material error, invalid device pointer"); }
		if (info.descriptorSetLayouts.empty()) 
		{ throw std::runtime_error("material error, no descriptor set layouts specified"); }
		createPipelineLayout();
		createPipeline();
	}

	Material::~Material() 
	{
		if (pipelineLayout != VK_NULL_HANDLE) 
		{ vkDestroyPipelineLayout(info.device->device(), pipelineLayout, nullptr); }
	};
	
	void Material::createPipelineLayout()
	{
		VkPushConstantRange pushConstRange{};
		pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstRange.offset = 0;
		pushConstRange.size = sizeof(MeshPushConstants);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(info.descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = info.descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstRange;
		if (vkCreatePipelineLayout(info.device->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{ throw std::runtime_error("material error, failed to create pipeline layout"); }
	}

	void Material::createPipeline()
	{
		assert(pipelineLayout != nullptr && "material error, create pipeline with uninitialized layout");
		PipelineConfig pipelineConfig{};
		EnginePipeline::defaultPipelineConfig(pipelineConfig);
		pipelineConfig.renderPass = info.renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		if (!info.shaderPaths.valid()) throw std::runtime_error("invalid path to shader file, check length");
		// read SPIR-V shader files
		pipeline = std::make_unique<EnginePipeline>(*info.device, pipelineConfig,
								info.shaderPaths.vertexShaderPath, 
								info.shaderPaths.fragmentShaderPath);
	}

	void Material::writePushConstantsForMesh(VkCommandBuffer commandBuffer, MeshPushConstants& data)
	{
		// the command buffer must be in the recording state for the command to succeed
		vkCmdPushConstants(commandBuffer, getPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(MeshPushConstants), (void*) &data);
	}

} // namespace