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

		assert(arrSize == bindingDescription.descriptorCount &&
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

	void UBOCreateInfo::addMember(IntrMemberType t, uint32_t arrLen)
	{
		MemberTypeCreateInfo mtci;
		mtci.types.push_back(t);
		mtci.arrayLength = arrLen;
		members.push_back(mtci);
	}
	void UBOCreateInfo::addMember(const std::vector<IntrMemberType>& t, uint32_t arrLen)
	{
		assert(!t.empty() && "tried to create ubo member type with no intrinsic types specified");
		MemberTypeCreateInfo mtci;
		mtci.types = t;
		mtci.arrayLength = arrLen;
		members.push_back(mtci);
	}

	// *************** Uniform Buffer wrapper *********************

	UBO::UBO(const UBOCreateInfo& createInfo, uint32_t numBuffers)
	{
		addMembers(createInfo.members);
		createBuffers(createInfo.device, numBuffers);
	}

	void UBO::writeMember(const MemberAccessor& a, void* data, const size_t& dataSize,
						uint32_t bufferIndex, bool flush)
	{
		assert(a.i < members.size() && "cannot write to uniform buffer, invalid member index");
		const auto& m = members[a.i];
		assert(a.arr_i < m.arrayLength || a.arr_i == 0 && "cannot write to uniform buffer, index exceeds array length");

		const auto& dstSize = m.sizes[a.field_i];
		const auto& dstOffset = m.offsets[a.field_i] + (m.arrayStride * a.arr_i);
		if (dataSize != dstSize) { throw std::runtime_error("cannot write to uniform buffer, incompatible data size"); }

		getBuffer(bufferIndex)->writeToBuffer(data, dataSize, dstOffset);
		if (flush) { getBuffer(bufferIndex)->flush(); }
	}

	void UBO::addMembers(const std::vector<MTCI>& memberCreateInfos)
	{
		for (uint32_t m_i = 0; m_i < memberCreateInfos.size(); m_i++)
		{
			const auto& memberInfo = memberCreateInfos[m_i];
			if (memberInfo.types.empty()) { throw std::runtime_error("cannot add member to uniform buffer, unspecified type info"); }
			// calculate alignments
			std::vector<uint32_t> alignments;
			std::vector<uint32_t> sizes;
			for (auto& t : memberInfo.types) 
			{
				uint32_t t_size, t_alignment;
				// get alignment requirement for simple data type
				getIntrTypeAlignment(t, t_size, t_alignment);
				alignments.push_back(t_alignment);
				sizes.push_back(t_size);
			}
			
			// structs must be aligned to the largest alignment of their elements rounded up to 16
			if (alignments.size() > 1) 
			{ 
				uint32_t maxAlignment = 0;
				for (const auto& a : alignments) { if (a > maxAlignment) { maxAlignment = a; } }
				alignments[0] = Math::roundUpToClosestMultiple(maxAlignment, (uint32_t)16);
			}
			// arrays must be aligned to the same alignment as the element type rounded up to 16
			if (memberInfo.arrayLength > 1) 
			{
				alignments[0] = Math::roundUpToClosestMultiple(alignments[0], (uint32_t)16);
			}

			Member m;
			size_t seekPos = size; // memory location relative to buffer start
			// decide location for each individual data element
			for (uint32_t i = 0; i < alignments.size(); i++)
			{
				// find suitable offset (location) for element, at its required alignment
				size_t offset = Math::roundUpToClosestMultiple(seekPos, (size_t)alignments[i]);
				/*	when using a separate buffer for each UBO (although that should be revised in the future)
				start the very first data element at zero, without any specific alignment */
				//if (i == 0 && m_i == 0) { offset = 0; }

				m.offsets.push_back(offset);
				m.sizes.push_back((size_t)sizes[i]);
				const size_t elementSize = offset + sizes[i]; // size including padding
				m.arrayStride += elementSize; // total size of member (per instance, if array)
				seekPos += elementSize;
			}

			
			uint32_t arrSizeMultiplier = 1;
			if (memberInfo.arrayLength > 1) { arrSizeMultiplier = memberInfo.arrayLength; }
			size += m.arrayStride * arrSizeMultiplier; // increase size of buffer
			m.arrayLength = memberInfo.arrayLength;
			members.push_back(m);
		}
	}

	void UBO::getIntrTypeAlignment(const IMT& t, uint32_t& sizeOut, uint32_t& alignmentOut) const
	{
		// vulkan imposes alignment requirements for data used in uniform buffer descriptors
		uint32_t a = 0;
		uint32_t s = 0;
		if (t == IMT::scalar)		{ a = 4; s = 4; } // 1 * scalar
		else if (t == IMT::vec2)	{ a = 8; s = 8; } // 2 * scalar
		else if (t == IMT::vec3)	{ a = 16; s = 12; } // 4 * scalar
		else if (t == IMT::vec4)	{ a = 16; s = 16; } // 4 * scalar
		else if (t == IMT::mat4)	{ a = 16; s = 64; } // 4 * scalar
		assert(s > 0 && "failed to get alignment and size for uniform buffer member, unknown type");
		alignmentOut = a;
		sizeOut = s;
	}

	void UBO::createBuffers(EngineDevice& device, const uint32_t& numBuffers)
	{
		auto minOffsetAlignment = device.properties.limits.minUniformBufferOffsetAlignment;
		for (uint32_t i = 0; i < numBuffers; i++) 
		{
			buffers.push_back(std::make_unique<GBuffer>(device, size, 1,
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
						minOffsetAlignment));
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
		std::vector<VkDescriptorImageInfo> infos{};
		for (const auto& imageView : views)
		{
			VkDescriptorImageInfo imgInfo{};
			imgInfo.sampler = nullptr;
			imgInfo.imageView = imageView;
			imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // correct layout assumed
			infos.push_back(imgInfo);
		}
		imageArraysInfos.push_back(infos); // add array image infos
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
		if (numUBOs > 0) { poolBuilder.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, framesInFlight * numUBOs); }
		if (numSamplerImages > 0) { poolBuilder.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numSamplerImages); }
		if (numImageArrays > 0) { for (auto& s : imageArraysSizes) { poolBuilder.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, s); } }
		if (numSamplers > 0) { poolBuilder.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, numSamplers); }
		
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
			// add uniform buffers
			for (uint32_t u = 0; u < numUBOs; u++)
			{
				// this is required because vulkan keeps a handle to the buffer info,
				// reallocation of info objects causes bindings to fail, either silently or violently
				const auto dBufferInfo = getUBO(u).getBuffer(f)->descriptorInfo();
				bufferInfos.push_back(std::make_unique<VkDescriptorBufferInfo>(dBufferInfo));
				writer.writeBuffer(u, bufferInfos.back().get()); // sending pointer
			}

			// add combined image samplers
			for (uint32_t i = 0; i < numSamplerImages; i++)
			{
				writer.writeImage(i + numUBOs, samplerImageInfos[i].get());
			}

			// add image arrays, one binding per array - currently re-using same images for all frames
			for (uint32_t a = 0; a < numImageArrays; a++)
			{
				writer.writeImage(a + numUBOs + numSamplerImages, imageArraysInfos[a].data(), imageArraysSizes[a]);
			}

			// add samplers
			for (uint32_t i = 0; i < numSamplers; i++)
			{
				writer.writeImage(i + numUBOs + numSamplerImages + numImageArrays, samplerInfos[i].get());
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

	UBO& DescriptorSet::getUBO(uint32_t uboIndex)
	{
		assert(uboIndex < ubos.size() && "ubo index out of range");
		return *ubos[uboIndex].get();
	}

	

}  // namespace lve