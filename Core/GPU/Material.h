#pragma once

#include "Core/GPU/engine_device.h"
#include "Core/GPU/Memory/Descriptors.h"
#include "Core/EngineSettings.h"

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
		ShaderFilePaths(const std::string& vert, const std::string& frag) : vertPath{ vert }, fragPath{ frag } {};
	};

	// holds common material-specific properties
	struct MaterialShadingProperties
	{
		VkPrimitiveTopology primitiveType = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
		VkCullModeFlags cullModeFlags = VK_CULL_MODE_BACK_BIT; // backface culling
		float lineWidth = 1.f;
	};

	// holds all properties needed to create a material object (used to generate a pipeline config)
	struct MaterialCreateInfo 
	{
		MaterialCreateInfo(const ShaderFilePaths& shadersIn,
					const std::vector<VkDescriptorSetLayout>& setLayoutsIn) 
					: shaderPaths(shadersIn), descriptorSetLayouts(setLayoutsIn) {};
			 
		MaterialShadingProperties shadingProperties{};
		ShaderFilePaths shaderPaths; // SPIR-V shaders
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	};

	// a material object is mainly an abstraction around a VkPipeline
	class Material 
	{
	public:
		Material(const MaterialCreateInfo& matInfo, const EngineRenderSettings& rs,
				VkRenderPass pass, EngineDevice& deviceIn);
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
		const EngineRenderSettings& engineRenderSettings;
		VkRenderPass renderPass = VK_NULL_HANDLE;

		EngineDevice& device;
		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;

		static void getDefaultPipelineConfig(PipelineConfig& cfg);
		static void applyMatPropsToPipelineConfig(const MaterialShadingProperties& mp, PipelineConfig& cfg);

		void createShaderModule(const std::string& path, VkShaderModule* shaderModule);
		void createPipelineLayout();
		void createPipeline();
	};

	// handle to a managed material
	struct MaterialHandle
	{
		MaterialHandle() = default;
		MaterialHandle(Material* m, class MaterialsManager* mg) : materialPtr{ m }, mgr{ mg } {};
		Material* get() const { return materialPtr; }
		// report to materials manager one user started/stopped using this material 
		void matUserAdd(const bool& remove = false) const;
		void matUserRemove() const { matUserAdd(true); } // calls matUserAdd with remove flag
	private:
		Material* materialPtr = nullptr;
		class MaterialsManager* mgr = nullptr;
	};

} // namespace
