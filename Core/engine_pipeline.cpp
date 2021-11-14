#include "engine_pipeline.h"
#include "ECS/StaticMesh.h"

// std
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cassert>


namespace EngineCore
{

	EnginePipeline::EnginePipeline(EngineDevice& device, const PipelineConfig& config, 
						const std::string& vertexShaderPath, const std::string& fragmentShaderPath) : engineDevice{device}
	{
		createGraphicsPipeline(config, vertexShaderPath, fragmentShaderPath);
	}

	EnginePipeline::~EnginePipeline() 
	{
		vkDestroyShaderModule(engineDevice.device(), vertexShaderModule, nullptr);
		vkDestroyShaderModule(engineDevice.device(), fragmentShaderModule, nullptr);
		vkDestroyPipeline(engineDevice.device(), graphicsPipeline, nullptr);
	}

	void EnginePipeline::createGraphicsPipeline(const PipelineConfig& config, const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
	{
		assert(config.pipelineLayout != VK_NULL_HANDLE && "tried to create pipeline with null param: pipelineLayout");
		assert(config.renderPass != VK_NULL_HANDLE && "tried to create pipeline with null param: renderPass");
		auto vertexShader = readFile(vertexShaderPath); // read shader code
		auto fragmentShader = readFile(fragmentShaderPath);
		createShaderModule(vertexShader, &vertexShaderModule); // create shader modules
		createShaderModule(fragmentShader, &fragmentShaderModule);
		// vertex shader stage
		VkPipelineShaderStageCreateInfo shaderStages[2];
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
		pipelineInfo.pInputAssemblyState = &config.inputAssemblyInfo;
		pipelineInfo.pViewportState = &config.viewportInfo;
		pipelineInfo.pRasterizationState = &config.rasterizationInfo;
		pipelineInfo.pMultisampleState = &config.multisampleInfo;
		pipelineInfo.pColorBlendState = &config.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &config.depthStencilInfo;
		pipelineInfo.pDynamicState = &config.dynamicStateInfo; 

		pipelineInfo.layout = config.pipelineLayout;
		pipelineInfo.renderPass = config.renderPass;
		pipelineInfo.subpass = config.subpass;
		
		pipelineInfo.basePipelineIndex = -1; // these can apparently be used for "optimizations"
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		// create vulkan graphics pipeline object
		if (vkCreateGraphicsPipelines(engineDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) 
		{ throw std::runtime_error("failed to create graphics pipeline"); }
	}

	void EnginePipeline::bind(VkCommandBuffer commandBuffer)
	{
		/* a pipeline binding affects subsequent commands until a different pipeline is bound */
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	}

	std::vector<char> EnginePipeline::readFile(const std::string& path)
	{
		// create data stream
		std::ifstream file{ path, std::ios::ate | std::ios::binary };

		if (!file.is_open()) { throw std::runtime_error("could not open file " + path); }

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		// read
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	// VkGraphicsPipelineCreateInfo is a structure specifying parameters of a newly created graphics pipeline
	void EnginePipeline::defaultPipelineConfig(PipelineConfig& cfg)
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

	void EnginePipeline::createShaderModule(const std::vector<char>& shader, VkShaderModule* shaderModule)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shader.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.data());

		if (vkCreateShaderModule(engineDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("could not create shader module");
		}
	}


} // namespace