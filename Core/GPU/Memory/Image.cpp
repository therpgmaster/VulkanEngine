#include "Core/GPU/Memory/Image.h"
#include <cassert>
#include <stdexcept>

// image importer, can only be defined in one (source) file
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"

namespace EngineCore 
{

	Image::Image(EngineDevice& device, const char* path) : device{ device }
	{
		loadFromDisk(path);
	}

	Image::Image(EngineDevice& device, GBuffer& src, 
				uint32_t width, uint32_t height) : device{ device }
	{
		assert((src.getUsageFlags() & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) &&
			"image source buffer did not have VK_BUFFER_USAGE_TRANSFER_SRC_BIT set");
		
		initImage(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, width, height);
		copyBufferToImage(src, static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1);
	}

	Image::~Image() 
	{
		vkDestroyImage(device.device(), image, nullptr);
		vkFreeMemory(device.device(), imageMemory, nullptr);
	}

	// loads the image from a file
	void Image::loadFromDisk(const char* path)
	{
		// import (see Vulkan Tutorial - Texture mapping)
		int width, height, channels;
		stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
		VkDeviceSize imageSize = width * height * (uint32_t)4;
		if (!pixels) { throw std::runtime_error("failed to load image"); }

		// temporary buffer to transfer from CPU (host) to GPU (device)
		GBuffer stagingBuffer
		{
			device, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		// map buffer to host so we can write to it from the host
		stagingBuffer.map(imageSize);
		memcpy(stagingBuffer.getMappedMemory(), pixels, static_cast<size_t>(imageSize)); 
		stagingBuffer.unmap();

		stbi_image_free(pixels); // free importer memory

		/*	allocate and prep the image for write - device local memory is the fastest
			but does not allow direct modification from the host */
		initImage(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, width, height);

		// transfer data from buffer to image
		copyBufferToImage(stagingBuffer, static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1);
	}

	VkImageCreateInfo Image::makeImageCreateInfo(const uint32_t& width, const uint32_t& height)
	{
		VkImageCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ci.imageType = VK_IMAGE_TYPE_2D; // regular flat image texture
		ci.extent.width = static_cast<uint32_t>(width);
		ci.extent.height = static_cast<uint32_t>(height);
		ci.extent.depth = 1;
		ci.mipLevels = 1;
		ci.arrayLayers = 1;
		ci.format = VK_FORMAT_R8G8B8A8_SRGB; // format must be supported by GPU
		ci.tiling = VK_IMAGE_TILING_OPTIMAL;
		ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // shader sample-able
		ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ci.samples = VK_SAMPLE_COUNT_1_BIT;
		return ci;
	}

	// performs the memory allocation for the underlying VkImage
	void Image::initImage(VkMemoryPropertyFlags memProps,
							uint32_t width, uint32_t height)
	{
		// uses defaults set by the function
		VkImageCreateInfo imgInfo = makeImageCreateInfo(width, height);

		if (vkCreateImage(device.device(), &imgInfo, nullptr, &image) != VK_SUCCESS)
		{ throw std::runtime_error("failed to create image"); }

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.device(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, memProps);

		if (vkAllocateMemory(device.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{ throw std::runtime_error("failed to allocate memory for image"); }

		if (vkBindImageMemory(device.device(), image, imageMemory, 0) != VK_SUCCESS)
		{ throw std::runtime_error("failed to bind memory for image"); }
	}

	void Image::transitionImageLayout(VkFormat format, 
								VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else { throw std::invalid_argument("unsupported image layout transition"); }
		
		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
							nullptr, 0, nullptr, 1, &barrier);

		device.endSingleTimeCommands(commandBuffer);
	}

	void Image::copyBufferToImage(const GBuffer& buffer, uint32_t width, uint32_t height,
									uint32_t layerCount)
								
	{
		// vkCmdCopyBufferToImage requires the right image layout, that is handled here
		transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount; // default = 1
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer.getBuffer(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		device.endSingleTimeCommands(commandBuffer);
		// transition (again) to a more useful format
		transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

} // namespace