#pragma once

#include "engine_pipeline.h"
#include "engine_device.h"
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
		MaterialCreateInfo(EngineDevice& deviceIn, VkRenderPass renderPassIn, ShaderFilePaths& shadersIn)
			: devicePtr(&deviceIn), renderPass(renderPassIn), shaderPaths(shadersIn) {};
		bool operator==(MaterialCreateInfo& b) const
		{
			return (shaderPaths.fragmentShaderPath == b.shaderPaths.fragmentShaderPath)
				&& (shaderPaths.vertexShaderPath == b.shaderPaths.vertexShaderPath);
		}
		// needed for creating pipeline
		EngineDevice* devicePtr = nullptr;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		// directory paths to SPIR-V shaders
		ShaderFilePaths shaderPaths;
	};

	class Material 
	{
	public:
		Material(const MaterialCreateInfo& mci);

		~Material();

		// move constructor
		Material(Material&& other) noexcept
		{
			pipelineLayout = other.pipelineLayout;
			pipeline = std::move(other.pipeline); // transfer ownership of pipeline
			device = other.device;
			info = other.info;
		}

		// the parameters this object was constructed from
		MaterialCreateInfo info;
		
		EnginePipeline* getPipeline() { return pipeline.get(); }
		VkPipelineLayout& getPipelineLayout() { return pipelineLayout; }
		// binds this material's pipeline to the specified command buffer
		void bindToCommandBuffer(VkCommandBuffer commandBuffer) { pipeline.get()->bind(commandBuffer); }
		
	private:
		EngineDevice* device;
		VkPipelineLayout pipelineLayout;
		// pipeline directly owned by the material
		std::unique_ptr<EnginePipeline> pipeline{};

		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

	};

} // namespace
