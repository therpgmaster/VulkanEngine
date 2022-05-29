#pragma once
#include "Core/GPU/engine_device.h"

#include <memory>

namespace EngineCore 
{
	class ShadowRenderer // reference: https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp
	{
	public:
		ShadowRenderer(EngineDevice& device) : device{ device } {};
		~ShadowRenderer() = default;

		VkFormat findDepthFormat();
		void createRenderPass(VkSampleCountFlagBits samples);
		void createFramebuffers();

		void beginShadowRenderPass(VkCommandBuffer commandBuffer);
		void endShadowRenderPass(VkCommandBuffer commandBuffer);

	private:
		EngineDevice& device;

		struct ShadowPass 
		{
			uint32_t width, height;
			VkFramebuffer frameBuffer;
			VkRenderPass renderPass = VK_NULL_HANDLE;
			VkSampler depthSampler;
			VkDescriptorImageInfo descriptor;
			// depth attachment
			std::unique_ptr<class Image> image;
			VkImageView imageView;
		} shadowPass;
		
	};
}