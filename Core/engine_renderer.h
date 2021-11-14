#pragma once

#include "engine_window.h"
#include "engine_device.h"
#include "engine_swap_chain.h"

// std
#include <memory>
#include <vector>
#include <cassert>

namespace EngineCore
{
	class EngineRenderer
	{
	public:
		EngineRenderer(EngineWindow& windowIn, EngineDevice& deviceIn);
		~EngineRenderer();
		EngineRenderer(const EngineRenderer&) = delete;
		EngineRenderer& operator=(const EngineRenderer&) = delete;

		VkRenderPass getSwapchainRenderPass() const { return swapchain->getRenderPass(); }
		bool getIsFrameInProgress() const { return isFrameStarted; }
		VkCommandBuffer getCurrentCommandBuffer() const 
		{ 
			assert(isFrameStarted && "getCurrentCommandBuffer failed, no frame in progress");
			return commandBuffers[currentFrameIndex];
		}
		int getFrameIndex() const
		{
			assert(isFrameStarted && "getFrameIndex failed, no frame in progress");
			return currentFrameIndex;
		}
		float getAspectRatio() const { return swapchain->getExtentAspectRatio(); }

		// returns a command buffer object for writing commands to
		VkCommandBuffer beginFrame();
		// submit command buffer to finalize the frame
		void endFrame();
		void beginSwapchainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapchainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapchain();

		// reference to the application window (using GLFW) 
		EngineWindow& window;
		// reference to the vulkan render device (gpu) wrapper
		EngineDevice& device;
		// graphics swap chain
		std::unique_ptr<EngineSwapChain> swapchain;
		// command buffers
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted{ false };
	};

} // namespace