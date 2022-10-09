#pragma once

#include "Core/GPU/engine_device.h"
#include "Core/GPU/Memory/Buffer.h"
#include <glm/glm.hpp>

// std
#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>// debug only

namespace EngineCore
{
	class DescriptorSetLayout
	{
	public:
		class Builder
		{
		public:
			Builder(EngineDevice& device) : device{ device } {}

			Builder& addBinding(uint32_t binding, VkDescriptorType descriptorType,
				VkShaderStageFlags stageFlags, uint32_t count = 1);
			std::unique_ptr<DescriptorSetLayout> build() const;
		private:
			EngineDevice& device;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
		};

		DescriptorSetLayout(EngineDevice& device,
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
		~DescriptorSetLayout();
		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

		VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

	private:
		EngineDevice& device;
		VkDescriptorSetLayout descriptorSetLayout;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

		friend class DescriptorWriter;
	};

	class DescriptorPool
	{
	public:
		class Builder
		{
		public:
			Builder(EngineDevice& device) : device{ device } {}

			Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
			Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
			Builder& setMaxSets(uint32_t count);
			std::unique_ptr<DescriptorPool> build() const;

		private:
			EngineDevice& device;
			std::vector<VkDescriptorPoolSize> poolSizes{};
			uint32_t maxSets = 1000;
			VkDescriptorPoolCreateFlags poolFlags = 0;
		};

		DescriptorPool(EngineDevice& device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
			const std::vector<VkDescriptorPoolSize>& poolSizes);
		~DescriptorPool();
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;

		bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;
		void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

		void resetPool();

	private:
		EngineDevice& device;
		VkDescriptorPool descriptorPool;

		friend class DescriptorWriter;
	};

	class DescriptorWriter
	{
	public:
		DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

		DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, uint32_t arrSize = 1);

		bool build(VkDescriptorSet& set);
		void overwrite(VkDescriptorSet& set);

	private:
		DescriptorSetLayout& setLayout;
		DescriptorPool& pool;
		std::vector<VkWriteDescriptorSet> writes;
	};

	/*
	struct SceneGlobalDataBuffer
	{
		glm::mat4 projectionViewMatrix{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .16f };  // w = intensity
		glm::vec3 lightPosition{ -1.f };
		alignas(16) glm::vec4 lightColor{ 0.1f, 0.4f, 0.8f, 12.f };  // w = intensity
	};
	
	class GlobalDescriptorSetManager 
	{
		std::unique_ptr<DescriptorPool> globalDescriptorPool{};
	public:
		GlobalDescriptorSetManager(EngineDevice& device, const uint32_t& maxFramesInFlight);

		std::vector<std::unique_ptr<GBuffer>> buffers;
		std::unique_ptr<DescriptorSetLayout> layout;
		std::vector<VkDescriptorSet> sets;
		// write to buffer in global set (frameIndex refers to the currect framebuffer, probably 0, 1, or 2)
		void writeToSceneGlobalBuffer(const uint32_t& frameIndex, SceneGlobalDataBuffer& data, const bool& flush);
	};*/

	class UBOCreateInfo
	{
	public:
		UBOCreateInfo(EngineDevice& device) : device{ device } {};

		enum IntrMemberType { scalar, vec2, vec3, vec4, mat4 };
		// single or array of single type
		void addMember(IntrMemberType t, uint32_t arrLen = 0);
		// struct or array of struct
		void addMember(const std::vector<IntrMemberType>& t, uint32_t arrLen = 0);
	private:
		friend class UBO;
		struct MemberTypeCreateInfo
		{
			// single element if member is not a struct
			std::vector<IntrMemberType> types;
			// zero if member is not an array
			uint32_t arrayLength = 0;
		};
		std::vector<MemberTypeCreateInfo> members;
		EngineDevice& device;
	};

	/* uniform buffer abstraction - this represents a specialized GPU buffer for in-shader (descriptor set) use */
	class UBO
	{
	public:
		UBO(const UBOCreateInfo& createInfo, uint32_t numBuffers);
		GBuffer* getBuffer(uint32_t index) { return buffers[index].get(); }
		// index of: ubo member, instance (if array), struct field (if struct)
		struct MemberAccessor
		{
			uint32_t i, arr_i, field_i;
			MemberAccessor(uint32_t memberIndex, uint32_t arrayIndex = 0, uint32_t subMemberIndex = 0)
				: i{ memberIndex }, arr_i{ arrayIndex }, field_i{ subMemberIndex }{};
		};
	private:
		friend class DescriptorSet;
		using IMT = UBOCreateInfo::IntrMemberType;
		using MTCI = UBOCreateInfo::MemberTypeCreateInfo;
		struct Member
		{
			std::vector<size_t> offsets;
			std::vector<size_t> sizes; // without padding
			uint32_t arrayStride = 0; // member instance size
			uint32_t arrayLength = 0; // instances
		};
		
		std::vector<Member> members;
		std::vector<std::unique_ptr<GBuffer>> buffers;
		size_t size = 0; // size of one buffer, padding included

		void addMembers(const std::vector<MTCI>& memberCreateInfos);
		void getIntrTypeAlignment(const IMT& t, uint32_t& sizeOut, uint32_t& alignmentOut) const;
		void createBuffers(EngineDevice& device, const uint32_t& numBuffers);
		void writeMember(const MemberAccessor& a, void* data, const size_t& dataSize,
						uint32_t bufferIndex, bool flush);
	};

	/*	descriptor set abstraction, this enables descriptor sets to be managed as self-contained objects, 
		and allows descriptors to be easily defined and bound at runtime */
	class DescriptorSet
	{
	public:
		DescriptorSet(EngineDevice& device, const uint32_t& maxFramesInFlight)
			: device{ device }, framesInFlight{ maxFramesInFlight } 
			{ assert(maxFramesInFlight > 0); };
		DescriptorSet(const DescriptorSet&) = delete;
		DescriptorSet& operator=(const DescriptorSet&) = delete;

		// add a descriptor to the set, actual binding indices depend on the order in the finalize function
		void addUBO(const UBOCreateInfo& createInfo);
		void addCombinedImageSampler(const VkImageView& view, const VkSampler& sampler);
		void addImageArray(const std::vector<VkImageView>& views);
		void addSampler(const VkSampler& sampler);

		void finalize(); // allocates descriptors, builds the set layout and VkDescriptorSets  

		template<typename T> // user-friendly uniform buffer data push function
		void writeUBOMember(uint32_t uboIndex, T& data, const UBO::MemberAccessor& position,
							uint32_t frameIndex, bool flush = true)
		{ getUBO(uboIndex).writeMember(position, (void*)&data, sizeof(T), frameIndex, flush); }

		UBO& getUBO(uint32_t uboIndex);
		VkDescriptorSetLayout getLayout();
		VkDescriptorSet getDescriptorSet(uint32_t frameIndex) { return sets[frameIndex]; }

	private:
		std::unique_ptr<DescriptorPool> pool{};
		std::unique_ptr<DescriptorSetLayout> layout; // layout of this set
		std::vector<VkDescriptorSet> sets; // per frame (identical layout)
		std::vector<std::unique_ptr<UBO>> ubos; // managed ubo (each has internal per-frame buffers)
		// descriptor info containers necessary to preserve pointers for vulkan
		std::vector<std::unique_ptr<VkDescriptorBufferInfo>> bufferInfos;
		std::vector<std::unique_ptr<VkDescriptorImageInfo>> samplerImageInfos;
		std::vector<std::vector<VkDescriptorImageInfo>> imageArraysInfos; // must be contiguous
		std::vector<uint32_t> imageArraysSizes;
		std::vector<std::unique_ptr<VkDescriptorImageInfo>> samplerInfos;
		
		EngineDevice& device;
		uint32_t framesInFlight; // num copies to create of each buffer
	};

}
