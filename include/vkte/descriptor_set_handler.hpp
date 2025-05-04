#pragma once

#include <vulkan/vulkan.hpp>
#include "vkte/buffer.hpp"
#include "vkte/image.hpp"
#include "vkte/vulkan_main_context.hpp"

namespace vkte
{
class DescriptorSetHandler
{
public:
	DescriptorSetHandler(const VulkanMainContext& vmc, uint32_t set_count);
	// first, describe the whole layout of the descriptor set
	void add_binding(uint32_t binding, vk::DescriptorType type, vk::ShaderStageFlags stages, uint32_t descriptor_count = 1);
	// second, add the descriptors to each set
	void add_descriptor(uint32_t set, uint32_t binding, const std::vector<Image>& images);
	void add_descriptor(uint32_t set, uint32_t binding, const Image& image);
	void add_descriptor(uint32_t set, uint32_t binding, const std::vector<Buffer>& buffers);
	void add_descriptor(uint32_t set, uint32_t binding, const Buffer& buffer);
	// third, construct the descriptor set
	void construct();
	void destruct();
	const vk::DescriptorSetLayout& get_layout() const;
	const std::vector<vk::DescriptorSet>& get_sets() const;

private:
	struct Descriptor {
		Descriptor(uint32_t binding, vk::DescriptorType type, vk::ShaderStageFlags stages, uint32_t descriptor_count, uint32_t set_count) : dslb(binding, type, descriptor_count, stages), dbi(set_count), dii(set_count), pNext(set_count)
		{}
		std::vector<std::vector<vk::DescriptorBufferInfo>> dbi;
		std::vector<std::vector<vk::DescriptorImageInfo>> dii;
		std::vector<void*> pNext;
		vk::DescriptorSetLayoutBinding dslb;
	};

	const VulkanMainContext& vmc;
	std::vector<Descriptor> descriptors;
	uint32_t set_count;
	// those are all the same layout to allocate multiple descriptor sets at once
	std::vector<vk::DescriptorSetLayout> layouts;
	vk::DescriptorPool pool;
	std::vector<vk::DescriptorSet> sets;
};
} // namespace vkte
