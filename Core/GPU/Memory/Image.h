#pragma once
#include "Core/GPU/engine_device.h"
#include "Core/GPU/Memory/Buffer.h"

namespace EngineCore
{
	/* Image is an abstraction for an image or texture in GPU memory (VkImage), as the name implies */
	class Image
	{
	public:
		Image(EngineDevice& device, const char* path);
		Image(EngineDevice& device, GBuffer& src, uint32_t width, uint32_t height);
		~Image();

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;

		VkImage getImage() { return image; }
		VkDeviceMemory getMemory() { return imageMemory; }
		bool isLoaded() const { return image != VK_NULL_HANDLE && imageMemory != VK_NULL_HANDLE; }

	private:
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory imageMemory = VK_NULL_HANDLE;
		EngineDevice& device;

		void loadFromDisk(const char* path);
		void initImage(VkMemoryPropertyFlags memProps, uint32_t width, uint32_t height);

		static VkImageCreateInfo makeImageCreateInfo(const uint32_t& width, const uint32_t& height);

		void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		void copyBufferToImage(const GBuffer& buffer, uint32_t width, uint32_t height, uint32_t layerCount);
								

	};
}

