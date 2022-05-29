#include "ShadowRenderer.h"
#include "GPU/Memory/Image.h"

#include <array>
#include <stdexcept>

#define SHADOWMAP_RESOLUTION 2048

namespace EngineCore
{
	VkFormat ShadowRenderer::findDepthFormat()
	{
		return device.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D16_UNORM },
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	void ShadowRenderer::createRenderPass(VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
	{
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = samples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // make sure depth will be readable
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1; // attachment index, see VkRenderPassCreateInfo::pAttachments
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		// subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &depthAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &shadowPass.renderPass) != VK_SUCCESS)
		{ throw std::runtime_error("failed to create shadow render pass"); }
	}

	void ShadowRenderer::createFramebuffers() 
	{
		shadowPass.width = SHADOWMAP_RESOLUTION;
		shadowPass.height = SHADOWMAP_RESOLUTION;
		// create render target image (framebuffer attachment)
		VkImageCreateInfo info = Image::makeImageCreateInfo(shadowPass.width, shadowPass.height);
		info.format = findDepthFormat();
		info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		shadowPass.image = std::make_unique<Image>(device, info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		shadowPass.imageView = Image::createImageView(device, shadowPass.image.get()->getImage(), info.format, VK_IMAGE_ASPECT_DEPTH_BIT);
		// TODO: create image view (and sampler)
		
		// create framebuffer
		assert(shadowPass.renderPass != VK_NULL_HANDLE && "failed to create framebuffer, renderpass not initialized");
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.renderPass = shadowPass.renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &shadowPass.imageView;
		framebufferInfo.width = shadowPass.width;
		framebufferInfo.height = shadowPass.height;
		framebufferInfo.layers = 1;
		vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &shadowPass.frameBuffer);
	}

	void ShadowRenderer::beginShadowRenderPass(VkCommandBuffer commandBuffer)
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = shadowPass.renderPass;
		renderPassInfo.framebuffer = shadowPass.frameBuffer;

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent.width = shadowPass.width;
		renderPassInfo.renderArea.extent.height = shadowPass.height;

		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // inline = no secondary cmdbuffers

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(shadowPass.width);
		viewport.height = static_cast<float>(shadowPass.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, {shadowPass.width, shadowPass.height} };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

}