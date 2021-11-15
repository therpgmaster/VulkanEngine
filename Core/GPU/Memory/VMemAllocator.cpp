#include "Core/GPU/Memory/VMemAllocator.h"

// std
#include <stdexcept>

namespace EngineCore
{
	DeviceMemoryAllocator::DeviceMemoryAllocator(EngineDevice& device) : device{ device }
	{
		// get properties and memory info about the physical device
		const VkPhysicalDevice& gpu = device.getPhysicalDevice();
		if (gpu == VK_NULL_HANDLE)
		{
			throw std::runtime_error("allocator error, invalid VkPhysicalDevice");
		}

		VkPhysicalDeviceMemoryProperties gpuMemProps;
		vkGetPhysicalDeviceMemoryProperties(gpu, &gpuMemProps);
		VkPhysicalDeviceProperties gpuProps;
		vkGetPhysicalDeviceProperties(gpu, &gpuProps);
		//memTypeAllocSizes = (size_t*)calloc(1, sizeof(size_t) * gpuMemProps.memoryTypeCount);
		memTypeAllocSizes.resize(gpuMemProps.memoryTypeCount);
		pools.resize(gpuMemProps.memoryTypeCount);
		pageSize = gpuProps.limits.bufferImageGranularity;
		blockMinSize = pageSize * (uint64_t)10;
	}

	DeviceMemoryAllocator::~DeviceMemoryAllocator() {}

	void DeviceMemoryAllocator::alloc(Allocation& allocOut, VkDeviceSize size, uint32_t memType, VkMemoryPropertyFlags usage)
	{
		DeviceMemoryPool& pool = pools[memType];

		// always alloc a multiple of pageSize
		VkDeviceSize reqSize = ((size / pageSize) + 1) * pageSize;
		memTypeAllocSizes[memType] += reqSize;

		BlockSpanIndexPair location;

		bool wholePage = (usage != VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (findSpaceInPoolForAlloc(location, memType,
			reqSize, wholePage))
		{
			location = { addBlockToPool(reqSize, memType, wholePage), 0 };

			allocOut.memoryBlockHandle = pool.getBlock(location.blockIndex).mem.memoryBlockHandle;
			allocOut.size = size;
			allocOut.offset = pool.getBlock(location.blockIndex).layout[location.spanIndex].offset;
			allocOut.memoryType = memType;
			allocOut.id = location.blockIndex;

			markMemoryUsedInBlock(memType, location, reqSize);
		}
	}

	void DeviceMemoryAllocator::free(Allocation& alloc)
	{
		VkDeviceSize reqAllocSize = ((alloc.size / pageSize) + 1) * pageSize;

		OffsetSizePair span = { alloc.offset, reqAllocSize };

		DeviceMemoryPool& pool = pools[alloc.memoryType];
		pool.getBlock(alloc.id).pageReserved = false;

		bool found = false;

		uint32_t numLayoutMems = pool.getBlock(alloc.id).layout.size();
		for (uint32_t j = 0; j < numLayoutMems; ++j)
		{
			if (pool.getBlock(alloc.id).layout[j].offset == reqAllocSize + alloc.offset)
			{
				pool.getBlock(alloc.id).layout[j].offset = alloc.offset;
				pool.getBlock(alloc.id).layout[j].size += reqAllocSize;
				found = true;
				break;
			}
		}

		if (!found)
		{
			pools[alloc.memoryType].getBlock(alloc.id).layout.push_back(span);
			memTypeAllocSizes[alloc.memoryType] -= reqAllocSize;
		}
	}

	bool DeviceMemoryAllocator::findSpaceInPoolForAlloc(BlockSpanIndexPair& locationOut,
		uint32_t memType, VkDeviceSize size, bool needsWholePage)
	{
		DeviceMemoryPool& pool = pools[memType];

		for (uint32_t i = 0; i < pool.blocks.size(); ++i)
		{
			for (uint32_t j = 0; j < pool.getBlock(i).layout.size(); ++j)
			{
				bool validOffset = needsWholePage ? pool.getBlock(i).layout[j].offset == 0 : true;
				if (pool.getBlock(i).layout[j].size >= size && validOffset)
				{
					locationOut.blockIndex = i;
					locationOut.spanIndex = j;
					return true;
				}
			}
		}
		return false;
	}

	uint32_t DeviceMemoryAllocator::addBlockToPool(VkDeviceSize size, uint32_t memType, bool fitToAlloc)
	{
		VkDeviceSize newSize = size * 2;
		newSize = (newSize < blockMinSize) ? blockMinSize : newSize;

		VkMemoryAllocateInfo info = makeMemoryAllocateInfo(newSize, memType);

		DeviceMemoryBlock newBlock = {};
		VkResult res = vkAllocateMemory(device.device(), &info, nullptr, &newBlock.mem.memoryBlockHandle);

		if (res == VK_ERROR_TOO_MANY_OBJECTS)
		{
			throw std::runtime_error("allocation failed, limit exceeded");
		}
		if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY)
		{
			throw std::runtime_error("allocation failed, out of device memory");
		}
		if (res != VK_SUCCESS)
		{
			throw std::runtime_error("allocation failed, unknown error");
		}


		newBlock.mem.memoryType = memType;
		newBlock.mem.size = newSize;

		DeviceMemoryPool& pool = pools[memType];
		pool.blocks.push_back(newBlock);

		pool.getBlock(pool.blocks.size() - 1).layout.push_back({ 0, newSize });

		numAllocsTotal++;

		return pool.blocks.size() - 1;
	}

	void DeviceMemoryAllocator::markMemoryUsedInBlock(uint32_t memType, BlockSpanIndexPair location, VkDeviceSize size)
	{
		DeviceMemoryPool& pool = pools[memType];
		pool.getBlock(location.blockIndex).layout[location.spanIndex].offset += size;
		pool.getBlock(location.blockIndex).layout[location.spanIndex].size -= size;
	}

} // namespace