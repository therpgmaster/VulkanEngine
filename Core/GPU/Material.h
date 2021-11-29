#pragma once

#include "engine_pipeline.h"
#include "Core/GPU/engine_device.h"
#include "Core/GPU/Memory/Descriptors.h"
#include <glm/glm.hpp>
#include <string>
#include <memory>

namespace EngineCore 
{
	struct ShaderFilePaths
	{
		std::string vertexShaderPath;
		std::string fragmentShaderPath;
		ShaderFilePaths() {};
		ShaderFilePaths(const char* vert, const char* frag) 
			: vertexShaderPath{ vert }, fragmentShaderPath{ frag } {};
		bool valid()
		{ 
			return (vertexShaderPath.size() < 250 && vertexShaderPath.size() > 3) &&
				(fragmentShaderPath.size() < 250 && fragmentShaderPath.size() > 3); 
		}
	};

	struct MaterialCreateInfo 
	{
		MaterialCreateInfo() {};
		MaterialCreateInfo(EngineDevice& deviceIn, VkRenderPass renderPassIn, 
						ShaderFilePaths& shadersIn, std::vector<VkDescriptorSetLayout>& descriptorSetLayoutsIn)
			: device(&deviceIn), renderPass(renderPassIn), 
			shaderPaths(shadersIn), descriptorSetLayouts(descriptorSetLayoutsIn) {};
		bool operator==(MaterialCreateInfo& b) const
		{
			return (shaderPaths.fragmentShaderPath == b.shaderPaths.fragmentShaderPath)
				&& (shaderPaths.vertexShaderPath == b.shaderPaths.vertexShaderPath);
		}
		EngineDevice* device = nullptr; // needed for creating pipeline
		VkRenderPass renderPass = VK_NULL_HANDLE;
		ShaderFilePaths shaderPaths; // directory paths to SPIR-V shaders
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	};

	class Material 
	{
	public:
		
		Material(const MaterialCreateInfo& materialCreateInfo);

		~Material();

		// move constructor
		Material(Material&& other) noexcept
		{
			pipelineLayout = other.pipelineLayout;
			pipeline = std::move(other.pipeline); // transfer ownership of pipeline
			info = other.info;
		}

		// the parameters this object was constructed from
		MaterialCreateInfo info;
		
		EnginePipeline* getPipeline() { return pipeline.get(); }
		VkPipelineLayout& getPipelineLayout() { return pipelineLayout; }
		// binds this material's pipeline to the specified command buffer
		void bindToCommandBuffer(VkCommandBuffer commandBuffer) { pipeline.get()->bind(commandBuffer); }

		struct MeshPushConstants 
		{ 
			glm::mat4 transform{1.f};
			glm::mat4 normalMatrix{1.f};
		};
		// updates push constant values for a mesh-specific pipeline (only mesh materials)
		void writePushConstantsForMesh(VkCommandBuffer commandBuffer, MeshPushConstants& data);
		
	private:
		VkPipelineLayout pipelineLayout;
		// pipeline directly owned by the material
		std::unique_ptr<EnginePipeline> pipeline{};

		void createPipelineLayout();
		void createPipeline();

	};

} // namespace
