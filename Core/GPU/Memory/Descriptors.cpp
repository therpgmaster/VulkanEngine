#include "Core/GPU/Memory/Descriptors.h"
#include "Core/Types/Math.h"
#include "Core/GPU/Memory/Image.h"

// std
#include <cassert>
#include <stdexcept>
#include <iostream>

namespace EngineCore
{

	// *************** Descriptor Set Layout Builder *********************

	DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(
		uint32_t binding, VkDescriptorType descriptorType,
		VkShaderStageFlags stageFlags, uint32_t count)
	{
		assert(bindings.count(binding) == 0 && "Binding already in use");
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;
		bindings[binding] = layoutBinding;
		return *this;
	}

	std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const
	{
		return std::make_unique<DescriptorSetLayout>(device, bindings);
	}

	// *************** Descriptor Set Layout *********************

	DescriptorSetLayout::DescriptorSetLayout(
		EngineDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
		: device{ device }, bindings{ bindings }
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
		for (auto kv : bindings)
		{
			setLayoutBindings.push_back(kv.second);
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(
			device.device(),
			&descriptorSetLayoutInfo,
			nullptr,
			&descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	DescriptorSetLayout::~DescriptorSetLayout() {
		vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);
	}

	// *************** Descriptor Pool Builder *********************

	DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(
		VkDescriptorType descriptorType, uint32_t count)
	{
		poolSizes.push_back({ descriptorType, count });
		return *this;
	}

	DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(
		VkDescriptorPoolCreateFlags flags)
	{
		poolFlags = flags;
		return *this;
	}
	DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t count)
	{
		maxSets = count;
		return *this;
	}

	std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const
	{
		return std::make_unique<DescriptorPool>(device, maxSets, poolFlags, poolSizes);
	}

	// *************** Descriptor Pool *********************

	DescriptorPool::DescriptorPool(EngineDevice& device, uint32_t maxSets,
		VkDescriptorPoolCreateFlags poolFlags,
		const std::vector<VkDescriptorPoolSize>& poolSizes)
		: device{ device }
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = poolFlags;

		if (vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
	}

	bool DescriptorPool::allocateDescriptor(
		const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		allocInfo.descriptorSetCount = 1;

		// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
		// a new pool whenever an old pool fills up. But this is beyond our current scope
		if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptor) != VK_SUCCESS)
		{
			return false;
		}
		return true;
	}

	void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
	{
		vkFreeDescriptorSets(device.device(), descriptorPool,
			static_cast<uint32_t>(descriptors.size()), descriptors.data());

	}

	void DescriptorPool::resetPool()
	{
		vkResetDescriptorPool(device.device(), descriptorPool, 0);
	}

	// *************** Descriptor Writer *********************

	DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool)
		: setLayout{ setLayout }, pool{ pool } {}

	DescriptorWriter& DescriptorWriter::writeBuffer(
		uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
		assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

		auto& bindingDescription = setLayout.bindings[binding];

		assert(
			bindingDescription.descriptorCount == 1 &&
			"Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = bufferInfo;
		write.descriptorCount = 1;

		writes.push_back(write);
		return *this;
	}

	DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, uint32_t arrSize)
	{
		assert(setLayout.bindings.count(binding) == 1 && "failed to write descriptor binding, not present in layout");

		auto& bindingDescription = setLayout.bindings[binding];

		assert(arraySize == bindingDescription.descriptorCount &&
			"failed to write array descriptor binding, count must match set layout");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = imageInfo;
		write.descriptorCount = arrSize; // if binding multiple, imageInfo must be ptr to array

		writes.push_back(write);
		return *this;
	}

	bool DescriptorWriter::build(VkDescriptorSet& set)
	{
		bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
		if (!success) { return false; }
		overwrite(set);
		return true;
	}

	void DescriptorWriter::overwrite(VkDescriptorSet& set)
	{
		for (auto& write : writes) { write.dstSet = set; }
		vkUpdateDescriptorSets(pool.device.device(), writes.size(), writes.data(), 0, nullptr);
	}

	/*
	GlobalDescriptorSetManager::GlobalDescriptorSetManager(EngineDevice& device, const uint32_t& maxFramesInFlight)
	{
		sets.resize(maxFramesInFlight);

		globalDescriptorPool = DescriptorPool::Builder(device)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxFramesInFlight)
			.build();
		// one buffer for each frame in flight
		for (uint32_t i = 0; i < maxFramesInFlight; i++)
		{
			buffers.push_back(std::make_unique<GBuffer>(device, sizeof(SceneGlobalDataBuffer), 1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
			buffers[i]->map();
		}
		// add uniform buffer to layout
		layout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		for (uint32_t i = 0; i < maxFramesInFlight; i++)
		{
			auto bufferInfo = buffers[i]->descriptorInfo();
			DescriptorWriter(*layout.get(), *globalDescriptorPool)
				.writeBuffer(0, &bufferInfo)
				.build(sets[i]);
		}
	}

	void GlobalDescriptorSetManager::writeToSceneGlobalBuffer(const uint32_t& frameIndex, SceneGlobalDataBuffer& data, const bool& flush)
	{
		buffers[frameIndex]->writeToBuffer((void*)&data);
		if (flush) { buffers[frameIndex]->flush(); }
	}*/

	// *************** Uniform Buffer wrapper *********************

	UBO::UBO(const UBOCreateInfo& createInfo, const uint32_t& numBuffers)
	{
		addMembers(createInfo.memberTypes);
		createBuffers(createInfo.device, numBuffers);
	}

	void UBO::writeMember(const uint32_t& i, void* data, const size_t& dataSize,
								const uint32_t& bufferIndex, const bool& flush)
	{
		assert(i < members.size() && "invalid member index, could not write to uniform buffer");
		auto& m = members[i];
		if (m.size != size) { throw std::runtime_error("incompatible type size, could not write to uniform buffer"); }

		getBuffer(bufferIndex)->writeToBuffer(data, dataSize, m.offset);
		if (flush) { getBuffer(bufferIndex)->flush(); }
	}

	void UBO::addMembers(const std::vector<MT>& memberTypes)
	{
		for (auto& t : memberTypes)
		{
			if (t == MT::none) { throw std::runtime_error("cannot add member to ubo, unspecified type"); }
			// vulkan alignment requirements for data used in uniform buffer descriptors
			auto m = getMemberTypeInfo(t);
			assert(m.size > 0 && "cannot add member to ubo, unknown size");
			// find suitable offset (location) for the data member, at its required alignment
			m.offset = Math::roundUpToClosestMultiple(size, m.alignment);
			members.push_back(m);
			size = m.offset + m.size; // increase total size of buffer
		}
	}

	UBO::MemberInfo UBO::getMemberTypeInfo(const MT& t)
	{
		// vulkan imposes alignment requirements for data used in uniform buffer descriptors
		MemberInfo m{ 0, 0 };
		if (t == MT::scalar) { m = MemberInfo(4, 4); } // 1 * scalar
		else if (t == MT::vec2) { m = MemberInfo(8, 8); } // 2 * scalar
		else if (t == MT::vec3) { m = MemberInfo(16, 12); } // 4 * scalar
		else if (t == MT::vec4) { m = MemberInfo(16, 16); } // 4 * scalar
		else if (t == MT::mat4) { m = MemberInfo(16, 64); } // 4 * scalar
		return m;
	}

	void UBO::createBuffers(EngineDevice& device, const uint32_t& numBuffers)
	{
		auto minOffsetAlignment = device.properties.limits.minUniformBufferOffsetAlignment;
		for (uint32_t i = 0; i < numBuffers; i++) 
		{
			buffers.push_back(std::make_unique<GBuffer>(device, size, 1,
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
						minOffsetAlignment)); // minOffsetAlignment may not be necessary here
			buffers.back()->map();
		}
	}

	// *************** Descriptor set wrapper *********************

	void DescriptorSet::addUBO(const UBOCreateInfo& createInfo)
	{
		ubos.push_back(std::make_unique<UBO>(createInfo, framesInFlight));
	}

	void DescriptorSet::addCombinedImageSampler(const VkImageView& view, const VkSampler& sampler)
	{
		VkDescriptorImageInfo info{};
		info.imageView = view;
		info.sampler = sampler;
		info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // correct layout assumed
		samplerImageInfos.push_back(std::make_unique<VkDescriptorImageInfo>(info));
	}

	void DescriptorSet::addImageArray(const std::vector<VkImageView>& views)
	{
		assert(!views.empty() && "tried to add empty image array descriptor");
		// add array (single binding, but each image in array must have its own info)
		for (const auto& imageView : views)
		{
			VkDescriptorImageInfo info{};
			info.sampler = nullptr;
			info.imageView = imageView;
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // correct layout assumed
			imageArraysInfos.push_back(std::make_unique<VkDescriptorImageInfo>(info));
		}
		imageArraysSizes.push_back(views.size()); // record array length
	}

	void DescriptorSet::addSampler(const VkSampler& sampler)
	{
		VkDescriptorImageInfo info{};
		info.sampler = sampler;
		info.imageView = VK_NULL_HANDLE; // just the sampler, no image assigned
		info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		samplerInfos.push_back(std::make_unique<VkDescriptorImageInfo>(info));
	}

	void DescriptorSet::finalize()
	{
		sets.resize(framesInFlight);

		uint32_t numUBOs = ubos.size();
		uint32_t numSamplerImages = samplerImageInfos.size();
		uint32_t numImageArrays = imageArraysSizes.size();
		uint32_t numSamplers = samplerInfos.size();
		
		DescriptorPool::Builder poolBuilder(device);
		poolBuilder.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, framesInFlight * numUBOs);
		poolBuilder.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numSamplerImages);
		for (auto& s : imageArraysSizes) 
		{ poolBuilder.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, s); }
		poolBuilder.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, numSamplers);
		
		pool = poolBuilder.build();
		
		DescriptorSetLayout::Builder layoutBuilder(device);
		// add uniform buffer bindings to layout
		for (uint32_t i = 0; i < numUBOs; i++) /* UBOs start at binding index 0 */
		{ layoutBuilder.addBinding(i, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS); }
		// add combined image sampler bindings to layout
		for (uint32_t i = 0; i < numSamplerImages; i++) /* place combined sampler bindings after UBOs */
		{ layoutBuilder.addBinding(i + numUBOs, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS); }
		// add image arrays to layout, one binding per array
		for (uint32_t i = 0; i < numImageArrays; i++) /* image arrays after combined image samplers */
		{
			layoutBuilder.addBinding(i + numUBOs + numSamplerImages,
				VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS, imageArraysSizes[i]);
		}
		// add sampler-only bindings
		for (uint32_t i = 0; i < numSamplers; i++) /* last binding category */
		{ 
			layoutBuilder.addBinding(i + numUBOs + numSamplerImages + numImageArrays, 
			VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS);
		}

		layout = layoutBuilder.build();

		// create descriptors for each frame (UBOs have multiple internal buffers)
		for (uint32_t f = 0; f < framesInFlight; f++)
		{
			DescriptorWriter writer(*layout.get(), *pool);
			// add samplers
			for (uint32_t i = 0; i < numSamplers; i++)
			{
				writer.writeImage(i + numUBOs + numSamplerImages + numImageArrays, samplerInfos[i].get());
			}
			// add image arrays, one binding per array - currently re-using same images for all frames
			auto arrStart = 0;
			for (uint32_t a = 0; a < numImageArrays; a++)
			{
				const auto arrSize = imageArraysSizes[a];
				writer.writeImage(a + numUBOs + numSamplerImages,
								imageArraysInfos[arrStart].get(), arrSize);
				arrStart += arrSize; // move offset to next array start index
			}
			// add combined image samplers
			for (uint32_t i = 0; i < numSamplerImages; i++)
			{
				writer.writeImage(i + numUBOs, samplerImageInfos[i].get());
			}
			// add uniform buffers
			for (uint32_t u = 0; u < numUBOs; u++)
			{
				// this is required because vulkan keeps a handle to the buffer info,
				// reallocation of info objects causes bindings to fail silently
				const auto bi = getUBO(u).getBuffer(f)->descriptorInfo();
				bufferInfos.push_back(std::make_unique<VkDescriptorBufferInfo>(bi));
				writer.writeBuffer(u, bufferInfos.back().get()); // sending pointer
			}
			writer.build(sets[f]); // make descriptor set for frame
		}
	}

	VkDescriptorSetLayout DescriptorSet::getLayout()
	{
		assert(layout.get() && "tried to get layout from uninitialized descriptor set");
		if (!layout.get()) { return VK_NULL_HANDLE; }
		return layout.get()->getDescriptorSetLayout();
	}

	UBO& DescriptorSet::getUBO(const uint32_t& uboIndex)
	{
		assert(uboIndex < ubos.size() && "ubo index out of range");
		return *ubos[uboIndex].get();
	}

	

}  // namespace lve