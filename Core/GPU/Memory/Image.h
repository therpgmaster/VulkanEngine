#pragma once
#include "Core/GPU/engine_device.h"
#include "Core/GPU/Memory/Buffer.h"

namespace EngineCore
{
	/* Image is an abstraction for an image or texture in GPU memory (VkImage), as the name implies */
	class Image
	{
	public:
		Image(EngineDevice& device, const std::string& path);
		Image(EngineDevice& device, VkImageCreateInfo info, VkMemoryPropertyFlags memProps);
		~Image();

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;

		VkImage getImage() { return image; }
		VkDeviceMemory getMemory() { return imageMemory; }

		static VkImageCreateInfo makeImageCreateInfo(uint32_t width, uint32_t height);
		static VkImageView createImageView(EngineDevice& device, VkImage image, VkFormat format,
						VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);
		static void createSampler(VkSampler& samplerHandleOut, EngineDevice& device, const float& anisotropy = 0.f);

		VkImageView imageView = VK_NULL_HANDLE; // the image view handle could be stored here, or anywhere outside the object
		VkSampler sampler = VK_NULL_HANDLE; // note: samplers are not connected to specific images

	private:
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory imageMemory = VK_NULL_HANDLE;
		EngineDevice& device;

		void loadFromDisk(const std::string& path);
		void initImage(VkMemoryPropertyFlags memProps, VkImageCreateInfo info);

		void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(const GBuffer& buffer, uint32_t width, uint32_t height, uint32_t layerCount);
	};
}

