#include "vkte/descriptor_set_handler.hpp"

namespace vkte
{
DescriptorSetHandler::DescriptorSetHandler(const VulkanMainContext& vmc, uint32_t set_count) : vmc(vmc), set_count(set_count)
{}

void DescriptorSetHandler::add_binding(uint32_t binding, vk::DescriptorType type, vk::ShaderStageFlags stages, uint32_t descriptor_count)
{
	// Vulkan does not allow a descriptor count of 0, just pretend one descriptor will be added and never add it
	descriptor_count = std::max(descriptor_count, 1u);
	for (auto i = descriptors.begin(); i != descriptors.end(); ++i)
	{
		if (i->dslb.binding > binding)
		{
			descriptors.insert(i, Descriptor(binding, type, stages, descriptor_count, set_count));
			return;
		}
	}
	descriptors.push_back(Descriptor(binding, type, stages, descriptor_count, set_count));
}

void DescriptorSetHandler::add_descriptor(uint32_t set, uint32_t binding, const std::vector<Buffer>& buffers)
{
	for (auto d = descriptors.begin(); d != descriptors.end(); ++d)
	{
		if (d->dslb.binding == binding)
		{
			for (const Buffer& b : buffers)
			{
				d->dbi[set].push_back(vk::DescriptorBufferInfo(b.get(), 0, b.get_byte_size()));
				d->pNext[set] = b.pNext;
			}
		}
	}
}

void DescriptorSetHandler::add_descriptor(uint32_t set, uint32_t binding, const Buffer& buffer)
{
	add_descriptor(set, binding, std::vector<Buffer>{buffer});
}

void DescriptorSetHandler::add_descriptor(uint32_t set, uint32_t binding, const std::vector<Image>& images)
{
	for (auto d = descriptors.begin(); d != descriptors.end(); ++d)
	{
		if (d->dslb.binding == binding)
		{
			for (const Image& i : images)
			{
				d->dii[set].push_back(vk::DescriptorImageInfo(i.get_sampler(), i.get_view(), i.get_layout()));
			}
		}
	}
}

void DescriptorSetHandler::add_descriptor(uint32_t set, uint32_t binding, const Image& image)
{
	add_descriptor(set, binding, std::vector<Image>{image});
}

void DescriptorSetHandler::construct()
{
	std::vector<vk::DescriptorSetLayoutBinding> layout_bindings;
	for (const auto& d : descriptors) layout_bindings.push_back(d.dslb);
	for (uint32_t i = 0; i < set_count; ++i)
	{
		std::vector<vk::DescriptorBindingFlags> binding_flags(layout_bindings.size(), vk::DescriptorBindingFlagBits::ePartiallyBound);
		vk::DescriptorSetLayoutBindingFlagsCreateInfo dslbfci{};
		dslbfci.sType = vk::StructureType::eDescriptorSetLayoutBindingFlagsCreateInfo;
		dslbfci.bindingCount = layout_bindings.size();
		dslbfci.pBindingFlags = binding_flags.data();

		vk::DescriptorSetLayoutCreateInfo dslci{};
		dslci.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
		dslci.bindingCount = layout_bindings.size();
		dslci.pBindings = layout_bindings.data();
		dslci.pNext = &dslbfci;
		layouts.push_back(vmc.logical_device.get().createDescriptorSetLayout(dslci));
	}

	std::vector<vk::DescriptorPoolSize> pool_sizes;
	for (const auto& d : descriptors)
	{
		vk::DescriptorPoolSize dps{};
		dps.type = d.dslb.descriptorType;
		dps.descriptorCount = d.dslb.descriptorCount * set_count;
		if (dps.descriptorCount > 0) pool_sizes.push_back(dps);
	}

	vk::DescriptorPoolCreateInfo dpci{};
	dpci.sType = vk::StructureType::eDescriptorPoolCreateInfo;
	dpci.poolSizeCount = pool_sizes.size();
	dpci.pPoolSizes = pool_sizes.data();
	dpci.maxSets = set_count;

	pool = vmc.logical_device.get().createDescriptorPool(dpci);

	vk::DescriptorSetAllocateInfo dsai{};
	dsai.sType = vk::StructureType::eDescriptorSetAllocateInfo;
	dsai.descriptorPool = pool;
	dsai.descriptorSetCount = layouts.size();
	dsai.pSetLayouts = layouts.data();

	sets = vmc.logical_device.get().allocateDescriptorSets(dsai);

	std::vector<vk::WriteDescriptorSet> wds_s;
	for (uint32_t i = 0; i < set_count; ++i)
	{
		for (uint32_t j = 0; j < descriptors.size(); ++j)
		{
			vk::WriteDescriptorSet wds{};
			wds.pNext = descriptors[j].pNext[i];
			wds.sType = vk::StructureType::eWriteDescriptorSet;
			wds.dstSet = sets[i];
			wds.dstBinding = descriptors[j].dslb.binding;
			wds.dstArrayElement = 0;

			// descriptorType decides if descriptor is buffer or image, the unused one is empty
			wds.descriptorType = descriptors[j].dslb.descriptorType;
			wds.pImageInfo = descriptors[j].dii[i].data();
			wds.pBufferInfo = descriptors[j].dbi[i].data();
			wds.descriptorCount = std::max(descriptors[j].dii[i].size(), descriptors[j].dbi[i].size());
			wds.pTexelBufferView = nullptr;

			if (wds.descriptorCount > 0) wds_s.push_back(wds);
		}
	}
	vmc.logical_device.get().updateDescriptorSets(wds_s, {});
}

void DescriptorSetHandler::destruct()
{
	for (auto& dsl : layouts) vmc.logical_device.get().destroyDescriptorSetLayout(dsl);
	layouts.clear();
	vmc.logical_device.get().destroyDescriptorPool(pool);
	descriptors.clear();
	sets.clear();
}

const vk::DescriptorSetLayout& DescriptorSetHandler::get_layout() const
{
	return layouts[0];
}

const std::vector<vk::DescriptorSet>& DescriptorSetHandler::get_sets() const
{
	return sets;
}
} // namespace vkte
