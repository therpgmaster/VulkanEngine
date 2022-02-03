#include "Core/GUI_Interface.h"

#include <array>
#include <stdexcept>
#include <fstream>

namespace EngineCore
{
	Imgui::Imgui(EngineWindow& window, EngineDevice& device,
		VkRenderPass swapchainRenderPass, uint32_t imageCount,
		uint32_t width, uint32_t height, VkSampleCountFlagBits samples) : device{ device }
	{
		renderPass.width = width;
		renderPass.height = height;
		createImages();

		// create descriptor pool for imgui
		// the pool is very oversized, but it's copied from the imgui demo
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		if (vkCreateDescriptorPool(device.device(), &pool_info, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool for imgui");
		}

		// initialize imgui library

		// this initializes the core structures of imgui
		ImGui::CreateContext();

		ImGui::StyleColorsDark();

		// this initializes imgui for GLFW
		ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), false); // install_callbacks = false

		// this initializes imgui for Vulkan
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = device.getVulkanInstance();
		init_info.PhysicalDevice = device.getPhysicalDevice();
		init_info.Device = device.device();
		init_info.Queue = device.graphicsQueue();
		init_info.DescriptorPool = descriptorPool;
		init_info.MinImageCount = 2;
		init_info.ImageCount = imageCount;
		init_info.MSAASamples = samples; // MSAA (VkSampleCountFlagBits)

		ImGui_ImplVulkan_Init(&init_info, swapchainRenderPass);

		// execute a gpu command to upload imgui font textures
		auto cb = device.beginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(cb);
		device.endSingleTimeCommands(cb);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	Imgui::~Imgui()
	{
		vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		// destroy color attachment
		vkDestroyImageView(device.device(), renderPass.color.view, nullptr);
		vkDestroyImage(device.device(), renderPass.color.image, nullptr);
		vkFreeMemory(device.device(), renderPass.color.mem, nullptr);
		// destroy depth attachment
		vkDestroyImageView(device.device(), renderPass.depth.view, nullptr);
		vkDestroyImage(device.device(), renderPass.depth.image, nullptr);
		vkFreeMemory(device.device(), renderPass.depth.mem, nullptr);
		// destroy render renderPass
		vkDestroyRenderPass(device.device(), renderPass.renderPass, nullptr);
		vkDestroySampler(device.device(), renderPass.sampler, nullptr);
		vkDestroyFramebuffer(device.device(), renderPass.frameBuffer, nullptr);
	}

	// stellar "OffScreen::Start"
	void Imgui::beginGUIRenderPass(VkCommandBuffer commandBuffer) 
	{
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		VkDeviceSize offsets[1] = { 0 };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass.renderPass;
		renderPassBeginInfo.framebuffer = renderPass.frameBuffer;
		renderPassBeginInfo.renderArea.extent.width = renderPass.width;
		renderPassBeginInfo.renderArea.extent.height = renderPass.height;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = {};
		viewport.width = (float)renderPass.width;
		viewport.height = (float)renderPass.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.extent.width = renderPass.width;
		scissor.extent.height = renderPass.height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}
	// stellar "OffScreen::End"
	void Imgui::endGUIRenderPass(VkCommandBuffer commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}

	void Imgui::createImages() 
	{
		// Find a suitable depth format
		VkFormat fbDepthFormat = device.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
			
		// color attachment
		VkImageCreateInfo image = {};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = VK_FORMAT_B8G8R8A8_SRGB;
		image.extent.width = renderPass.width;
		image.extent.height = renderPass.height;
		image.extent.depth = 1;
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		// we'll sample directly from the color attachment
		image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		VkMemoryAllocateInfo memAlloc = {};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements memReqs;

		if (vkCreateImage(device.device(), &image, nullptr, &renderPass.color.image) != VK_SUCCESS) 
		{ throw std::runtime_error("failed to create gui renderpass image"); }
			
		vkGetImageMemoryRequirements(device.device(), renderPass.color.image, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (vkAllocateMemory(device.device(), &memAlloc, nullptr, &renderPass.color.mem) != VK_SUCCESS) 
		{ throw std::runtime_error("failed to allocate memory for gui renderpass"); }
		if (vkBindImageMemory(device.device(), renderPass.color.image, renderPass.color.mem, 0) != VK_SUCCESS) 
		{ throw std::runtime_error("failed to bind memory for gui renderpass"); }

		VkImageViewCreateInfo colorImageView = {};
		colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorImageView.format = VK_FORMAT_B8G8R8A8_SRGB;
		colorImageView.subresourceRange = {};
		colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorImageView.subresourceRange.baseMipLevel = 0;
		colorImageView.subresourceRange.levelCount = 1;
		colorImageView.subresourceRange.baseArrayLayer = 0;
		colorImageView.subresourceRange.layerCount = 1;
		colorImageView.image = renderPass.color.image;
		if (vkCreateImageView(device.device(), &colorImageView, nullptr, &renderPass.color.view) != VK_SUCCESS) 
		{ throw std::runtime_error("failed create image for gui renderpass"); }

		// Create sampler to sample from the attachment in the fragment shader
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = samplerInfo.addressModeU;
		samplerInfo.addressModeW = samplerInfo.addressModeU;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &renderPass.sampler) != VK_SUCCESS)
		{ throw std::runtime_error("failed create sampler for gui renderpass"); }

		// Depth stencil attachment
		image.format = fbDepthFormat;
		image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		if (vkCreateImage(device.device(), &image, nullptr, &renderPass.depth.image) != VK_SUCCESS)
		{ throw std::runtime_error("failed create depth image for gui renderpass"); }
		vkGetImageMemoryRequirements(device.device(), renderPass.depth.image, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (vkAllocateMemory(device.device(), &memAlloc, nullptr, &renderPass.depth.mem) != VK_SUCCESS)
		{ throw std::runtime_error("failed to allocate memory for gui renderpass"); }
		if (vkBindImageMemory(device.device(), renderPass.depth.image, renderPass.depth.mem, 0) != VK_SUCCESS)
		{ throw std::runtime_error("failed to bind memory for gui renderpass"); }

		VkImageViewCreateInfo depthStencilView = {};
		depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthStencilView.format = fbDepthFormat;
		depthStencilView.flags = 0;
		depthStencilView.subresourceRange = {};
		// if is something wrong with depth uncomment this
		//depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthStencilView.subresourceRange.baseMipLevel = 0;
		depthStencilView.subresourceRange.levelCount = 1;
		depthStencilView.subresourceRange.baseArrayLayer = 0;
		depthStencilView.subresourceRange.layerCount = 1;
		depthStencilView.image = renderPass.depth.image;
		if (vkCreateImageView(device.device(), &depthStencilView, nullptr, &renderPass.depth.view) != VK_SUCCESS)
		{ throw std::runtime_error("failed to create depth stencil view for gui renderpass"); }

		// Create a separate render renderPass for the offscreen rendering as it may differ from the one used for scene rendering

		std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
		// Color attachment
		attchmentDescriptions[0].format = VK_FORMAT_B8G8R8A8_SRGB;
		attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		// Depth attachment
		attchmentDescriptions[1].format = fbDepthFormat;
		attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pDepthStencilAttachment = &depthReference;

		// Use subrenderPass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderrenderPass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
		renderPassInfo.pAttachments = attchmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass.renderPass) != VK_SUCCESS)
		{ throw std::runtime_error("failed to create gui renderpass"); }

		VkImageView attachments[2];
		attachments[0] = renderPass.color.view;
		attachments[1] = renderPass.depth.view;

		VkFramebufferCreateInfo fbufCreateInfo = {};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.renderPass = renderPass.renderPass;
		fbufCreateInfo.attachmentCount = 2;
		fbufCreateInfo.pAttachments = attachments;
		fbufCreateInfo.width = renderPass.width;
		fbufCreateInfo.height = renderPass.height;
		fbufCreateInfo.layers = 1;

		if (vkCreateFramebuffer(device.device(), &fbufCreateInfo, nullptr, &renderPass.frameBuffer) != VK_SUCCESS)
		{ throw std::runtime_error("failed to create gui renderpass framebuffer"); }

		// Fill a descriptor for later use in a descriptor set
		renderPass.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		renderPass.descriptor.imageView = renderPass.color.view;
		renderPass.descriptor.sampler = renderPass.sampler;
	};

	void Imgui::loadFont(const std::string& path) 
	{
		// read file
		std::ifstream file{ path, std::ios::ate | std::ios::binary };
		if (!file.is_open()) { throw std::runtime_error("could not read font file " + path); }
		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> fileData(fileSize);
		file.seekg(0);
		file.read(fileData.data(), fileSize);
		file.close();
		void* data = reinterpret_cast<void*>(fileData.data()); // correct?



	}
	
} // namespace