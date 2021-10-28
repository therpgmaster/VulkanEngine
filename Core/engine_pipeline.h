#pragma once

#include "engine_device.h"
#include <string>
#include <vector>

namespace EngineCore 
{
	struct PipelineConfig 
	{
		PipelineConfig() = default;
		PipelineConfig(const PipelineConfig&) = delete;
		PipelineConfig& operator=(const PipelineConfig&) = delete;

		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class EnginePipeline
	{
	public:
		// default constructor
		EnginePipeline(EngineDevice& device, const PipelineConfig& config, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		// default destructor
		~EnginePipeline();

		EnginePipeline(const EnginePipeline&) = delete;
		EnginePipeline& operator=(const EnginePipeline&) = delete;

		// binds this pipeline object to a command buffer
		void bind(VkCommandBuffer commandBuffer);

		// sets the cfg param to the default graphics pipeline configuration settings
		static void defaultPipelineConfig(PipelineConfig& cfg);

	private:
		// reads shader code from a file
		static std::vector<char> readFile(const std::string& path);

		void createGraphicsPipeline(const PipelineConfig& config, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

		void createShaderModule(const std::vector<char>& shader, VkShaderModule* shaderModule);

		EngineDevice& engineDevice;
		VkPipeline graphicsPipeline;
		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;
	};

} // namespace