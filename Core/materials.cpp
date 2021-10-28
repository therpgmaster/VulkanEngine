#include "materials.h"
#include <stdexcept>
#include <cassert>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace EngineCore 
{
	/* Material */

	Material::Material(const MaterialCreateInfo& mci)
	{
		if (mci.devicePtr == nullptr)
		{ throw std::runtime_error("material creation failed, invalid device pointer"); }
		device = mci.devicePtr;
		info = mci; 
		createPipelineLayout();
		createPipeline(mci.renderPass);
	}

	Material::~Material() 
	{
		if (pipelineLayout != VK_NULL_HANDLE) 
		{ vkDestroyPipelineLayout(device->device(), pipelineLayout, nullptr); }
	};
	

	// TODO: move this to descriptor sets instead
	struct SimplePushConstantData
	{
		glm::mat4 transform{ 1.f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color; // data to gpu must be aligned
	};
	
	void Material::createPipelineLayout()
	{
		VkPushConstantRange pushConstRange{};
		pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstRange.offset = 0;
		pushConstRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstRange;
		if (vkCreatePipelineLayout(device->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void Material::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "tried to create pipeline with uninitialized layout");
		PipelineConfig pipelineConfig{};
		EnginePipeline::defaultPipelineConfig(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		if (!info.shaderPaths.valid()) throw std::runtime_error("invalid path to shader file (too long/short)");
		// read SPIR-V shader files
		pipeline = std::make_unique<EnginePipeline>(*device, pipelineConfig,
								info.shaderPaths.vertexShaderPath, 
								info.shaderPaths.fragmentShaderPath);
	}

} // namespace