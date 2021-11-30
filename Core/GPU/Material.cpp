#include "Core/GPU/Material.h"
#include "Core/ECS/StaticMesh.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cassert>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace EngineCore 
{
	Material::Material(const MaterialCreateInfo& matInfo, EngineDevice& deviceIn) 
						: device{ deviceIn }, materialCreateInfo{ matInfo }
	{
		if (materialCreateInfo.descriptorSetLayouts.empty()) { throw std::runtime_error("material error, no descriptor set layouts specified"); }
		createPipelineLayout();
		createPipeline();
	}

	Material::~Material() 
	{
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);

		vkDestroyShaderModule(device.device(), vertexShaderModule, nullptr);
		vkDestroyShaderModule(device.device(), fragmentShaderModule, nullptr);
		vkDestroyPipeline(device.device(), pipeline, nullptr);
	};

	void Material::bindToCommandBuffer(VkCommandBuffer commandBuffer) 
	{
		/* a pipeline binding affects subsequent commands until a different pipeline is bound */
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}
	
	void Material::createShaderModule(const std::string& path, VkShaderModule* shaderModule)
	{
		// read SPIR-V shader from file
		std::ifstream file{ path, std::ios::ate | std::ios::binary };
		if (!file.is_open()) { throw std::runtime_error("pipeline error, could not read file " + path); }
		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> shader(fileSize); // file output
		file.seekg(0);
		file.read(shader.data(), fileSize);
		file.close();

		// create module
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shader.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.data());
		if (vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
		{ throw std::runtime_error("pipeline error, could not create shader module"); }
	}

	void Material::defaultPipelineConfig(PipelineConfig& cfg)
	{
		cfg.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		cfg.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		cfg.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		cfg.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		cfg.viewportInfo.viewportCount = 1;
		cfg.viewportInfo.pViewports = nullptr;
		cfg.viewportInfo.scissorCount = 1;
		cfg.viewportInfo.pScissors = nullptr;

		cfg.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		cfg.rasterizationInfo.depthClampEnable = VK_FALSE;
		cfg.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		cfg.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;// polygon render mode
		cfg.rasterizationInfo.lineWidth = 1.0f;
		cfg.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT; // backface culling
		cfg.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		cfg.rasterizationInfo.depthBiasEnable = VK_FALSE;
		cfg.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		cfg.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
		cfg.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

		// multisampling (MSAA)
		cfg.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		cfg.multisampleInfo.sampleShadingEnable = VK_FALSE;
		cfg.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		cfg.multisampleInfo.minSampleShading = 1.0f;           // Optional
		cfg.multisampleInfo.pSampleMask = nullptr;             // Optional
		cfg.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
		cfg.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

		cfg.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		cfg.colorBlendAttachment.blendEnable = VK_FALSE;
		cfg.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		cfg.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		cfg.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		cfg.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		cfg.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		cfg.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		cfg.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cfg.colorBlendInfo.logicOpEnable = VK_FALSE;
		cfg.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		cfg.colorBlendInfo.attachmentCount = 1;
		cfg.colorBlendInfo.pAttachments = &cfg.colorBlendAttachment;
		cfg.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		cfg.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		cfg.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		cfg.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		cfg.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		cfg.depthStencilInfo.depthTestEnable = VK_TRUE;
		cfg.depthStencilInfo.depthWriteEnable = VK_TRUE;
		cfg.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		cfg.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		cfg.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		cfg.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		cfg.depthStencilInfo.stencilTestEnable = VK_FALSE;
		cfg.depthStencilInfo.front = {};  // Optional
		cfg.depthStencilInfo.back = {};   // Optional

		cfg.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		cfg.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		cfg.dynamicStateInfo.pDynamicStates = cfg.dynamicStateEnables.data();
		cfg.dynamicStateInfo.dynamicStateCount =
			static_cast<uint32_t>(cfg.dynamicStateEnables.size());
		cfg.dynamicStateInfo.flags = 0;
	}

	void Material::createPipelineLayout()
	{
		VkPushConstantRange pushConstRange{};
		pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstRange.offset = 0;
		pushConstRange.size = sizeof(MeshPushConstants);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(materialCreateInfo.descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = materialCreateInfo.descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstRange;
		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{ throw std::runtime_error("material error, failed to create pipeline layout"); }
	}

	void Material::createPipeline()
	{
		PipelineConfig cfg{};
		defaultPipelineConfig(cfg); // get default pipeline config
		cfg.renderPass = materialCreateInfo.renderPass;
		cfg.pipelineLayout = pipelineLayout;

		assert(cfg.pipelineLayout != VK_NULL_HANDLE && "pipeline error, null pipelineLayout");
		assert(cfg.renderPass != VK_NULL_HANDLE && "pipeline error, null renderPass");
		// load shaders
		createShaderModule(materialCreateInfo.shaderPaths.vertPath, &vertexShaderModule); 
		createShaderModule(materialCreateInfo.shaderPaths.fragPath, &fragmentShaderModule);
		// vertex shader stage
		VkPipelineShaderStageCreateInfo shaderStages[2]{};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertexShaderModule;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;
		// fragment shader stage
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragmentShaderModule;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;

		auto bindingDescriptions = StaticMesh::Vertex::getBindingDescriptions();
		auto attributeDescriptions = StaticMesh::Vertex::getAttributeDescriptions();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &cfg.inputAssemblyInfo;
		pipelineInfo.pViewportState = &cfg.viewportInfo;
		pipelineInfo.pRasterizationState = &cfg.rasterizationInfo;
		pipelineInfo.pMultisampleState = &cfg.multisampleInfo;
		pipelineInfo.pColorBlendState = &cfg.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &cfg.depthStencilInfo;
		pipelineInfo.pDynamicState = &cfg.dynamicStateInfo;

		pipelineInfo.layout = cfg.pipelineLayout;
		pipelineInfo.renderPass = cfg.renderPass;
		pipelineInfo.subpass = cfg.subpass;

		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		// create vulkan pipeline object
		if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		{ throw std::runtime_error("failed to create pipeline"); }
	}

	void Material::writePushConstantsForMesh(VkCommandBuffer commandBuffer, MeshPushConstants& data)
	{
		// the command buffer must be in the recording state for the command to succeed
		vkCmdPushConstants(commandBuffer, pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(MeshPushConstants), (void*) &data);
	}

} // namespace