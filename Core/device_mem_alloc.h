#pragma once

#include "engine_device.h"
#include <stdint.h> // should not be necessary


namespace EngineCore 
{
	// info about an individual resource and where it is in device memory
	struct Allocation
	{
		// size of the resource
		VkDeviceSize size;
		// offset within memory block
		VkDeviceSize offset;
		// type of memory the resource resides in
		uint32_t memoryType;
		// parent block
		VkDeviceMemory memoryBlockHandle;
		uint32_t id;
	};

	struct OffsetSizePair { uint64_t offset; uint64_t size; };
	struct BlockSpanIndexPair { uint32_t blockIndex; uint32_t spanIndex; };

	// an allocation block which may contain multiple individual allocations
	struct DeviceMemoryBlock
	{
		Allocation mem; // alloc for the block itself
		std::vector<OffsetSizePair> layout;
		bool pageReserved;
	};

	// a collection of memory blocks
	struct DeviceMemoryPool
	{
		std::vector<DeviceMemoryBlock> blocks;

		DeviceMemoryBlock& getBlock(uint32_t i) { return blocks[i]; }
	};

	/* this allocator manages resources (e.g. textures) in device memory (vram) */
	class DeviceMemoryAllocator
	{
	public:
		DeviceMemoryAllocator(EngineDevice& device);
		~DeviceMemoryAllocator();

		// find space for a resource and assign it to a location in device memory
		void alloc(Allocation& allocOut, VkDeviceSize size, uint32_t memType, VkMemoryPropertyFlags usage);
		// clears the memory of a resource, remember to stop using the associated resource
		void free(Allocation& allocation);

		// returns the currently allocated size of the specified memory type
		size_t getAllocMemTypeSize(uint32_t memoryType) { return memTypeAllocSizes[memoryType]; }
		// returns the total number of resources allocated across all memory types
		const uint32_t& getNumAllocs() { return numAllocsTotal; }

	private:
		EngineDevice& device;

		// each pool corresponds to a memory type
		std::vector<DeviceMemoryPool> pools;

		// total number of allocated blocks
		uint32_t numAllocsTotal = 0;
		// allocation sizes for each memory type
		std::vector<size_t> memTypeAllocSizes;

		// pagesize is set based on gpu bufferImageGranularity
		uint32_t pageSize;
		VkDeviceSize blockMinSize;
		
		inline VkMemoryAllocateInfo makeMemoryAllocateInfo(VkDeviceSize size, uint32_t memType)
		{
			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = size;
			allocInfo.memoryTypeIndex = memType;
			return allocInfo;
		}

		// checks all blocks in pool and returns a chunk of free memory that can fit the allocation
		bool findSpaceInPoolForAlloc(BlockSpanIndexPair& locationOut,
					uint32_t memType, VkDeviceSize size, bool needsWholePage);

		// adds a new block to the specified pool
		uint32_t addBlockToPool(VkDeviceSize size,
					uint32_t memType, bool fitToAlloc);

		void markMemoryUsedInBlock(uint32_t memType, BlockSpanIndexPair location,
					VkDeviceSize size);


	};

} // namespace
