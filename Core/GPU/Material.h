#pragma once

#include "Core/GPU/engine_device.h"
#include "Core/GPU/Memory/Descriptors.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <memory>

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

	struct ShaderFilePaths
	{
		std::string vertPath;
		std::string fragPath;
		ShaderFilePaths() = default;
		ShaderFilePaths(const char* vert, const char* frag) : vertPath{ vert }, fragPath{ frag } {};
	};

	struct MaterialCreateInfo 
	{
		MaterialCreateInfo(VkRenderPass renderPassIn, const ShaderFilePaths& shadersIn,
						 std::vector<VkDescriptorSetLayout>& setLayoutsIn) : renderPass(renderPassIn), 
			shaderPaths(shadersIn), descriptorSetLayouts(setLayoutsIn) {};
		MaterialCreateInfo() = default;
		ShaderFilePaths shaderPaths; // SPIR-V shaders
		VkRenderPass renderPass = VK_NULL_HANDLE;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	};

	// a material object is mainly an abstraction around VkPipeline
	class Material 
	{
	public:
		Material(const MaterialCreateInfo& matInfo, EngineDevice& deviceIn);
		~Material();

		Material(const Material&) = delete;
		Material& operator=(const Material&) = delete;

		VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

		// binds this material's pipeline to the specified command buffer
		void bindToCommandBuffer(VkCommandBuffer commandBuffer);

		struct MeshPushConstants
		{ 
			glm::mat4 transform{1.f};
			glm::mat4 normalMatrix{1.f};
		};
		// updates push constant values for a mesh-specific pipeline (only mesh materials)
		void writePushConstantsForMesh(VkCommandBuffer commandBuffer, MeshPushConstants& data);
		
	private:
		MaterialCreateInfo materialCreateInfo;
		EngineDevice& device;
		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;
		
		static void defaultPipelineConfig(PipelineConfig& cfg);

		void createShaderModule(const std::string& path, VkShaderModule* shaderModule);
		void createPipelineLayout();
		void createPipeline();
	};

} // namespace
