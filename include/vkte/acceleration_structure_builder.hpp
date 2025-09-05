#pragma once

#include "vkte/vulkan_main_context.hpp"
#include "vkte/vulkan_command_context.hpp"
#include "vkte/storage.hpp"

namespace vkte
{
struct AccelerationStructure {
	vk::AccelerationStructureKHR handle;
	uint64_t device_address = 0;
	int32_t buffer = -1;
	int32_t scratch_buffer = -1;
};

class AccelerationStructureBuilder
{
public:
	AccelerationStructureBuilder(const VulkanMainContext& vmc, Storage& storage);
	void destruct();
	void clean_up_scratch_buffers();
	// take id of the vertex and index buffer as returned by the storage class
	uint32_t add_blas(vk::CommandBuffer& cb, QueueFamilyFlags build_queue, const std::string& buffer_name, uint32_t vertex_buffer_id, uint32_t index_buffer_id, vk::DeviceSize vertex_stride, const std::vector<uint32_t>& index_offsets = {0}, const std::vector<uint32_t>& index_counts = {});
	uint32_t add_instance(uint32_t blas_idx, const vk::TransformMatrixKHR& M, uint32_t custom_index);
	void update_instance(uint32_t instance_idx, const vk::TransformMatrixKHR& M);
	void construct(vk::CommandBuffer& cb, const std::string& buffer_name);

private:
	const VulkanMainContext& vmc;
	Storage& storage;
	vk::WriteDescriptorSetAccelerationStructureKHR wdsas;
	std::vector<AccelerationStructure> bottom_level_as;
	std::vector<vk::AccelerationStructureInstanceKHR> instances;
	int32_t instances_buffer = -1;
	AccelerationStructure top_level_as;
};
} // namespace vkte
