#include "vkte/acceleration_structure_builder.hpp"

namespace vkte
{
AccelerationStructureBuilder::AccelerationStructureBuilder(const VulkanMainContext& vmc, Storage& storage) : vmc(vmc), storage(storage) {}

void AccelerationStructureBuilder::destruct()
{
	vmc.logical_device.get().destroyAccelerationStructureKHR(top_level_as.handle);
	if (top_level_as.buffer > -1) storage.destroy_buffer(top_level_as.buffer);
	if (top_level_as.scratch_buffer > -1) storage.destroy_buffer(top_level_as.scratch_buffer);
	if (instances_buffer > -1) storage.destroy_buffer(instances_buffer);

	for (AccelerationStructure& blas : bottom_level_as)
	{
		vmc.logical_device.get().destroyAccelerationStructureKHR(blas.handle);
		if (top_level_as.buffer > -1) storage.destroy_buffer(blas.buffer);
		if (top_level_as.scratch_buffer > -1) storage.destroy_buffer(blas.scratch_buffer);
	}
	bottom_level_as.clear();
	instances.clear();
}

void AccelerationStructureBuilder::clean_up_scratch_buffers()
{
	for (AccelerationStructure& blas : bottom_level_as)
	{
		if (blas.scratch_buffer > -1) storage.destroy_buffer(blas.scratch_buffer);
	}
	if (top_level_as.scratch_buffer > -1) storage.destroy_buffer(top_level_as.scratch_buffer);
	if (instances_buffer > -1) storage.destroy_buffer(instances_buffer);
}

uint32_t AccelerationStructureBuilder::add_blas(vk::CommandBuffer& cb, QueueFamilyFlags build_queue, const std::string& buffer_name, uint32_t vertex_buffer_id, uint32_t index_buffer_id, vk::DeviceSize vertex_stride, const std::vector<uint32_t>& index_offsets, const std::vector<uint32_t>& index_counts)
{
	Buffer& vertex_buffer = storage.get_buffer(vertex_buffer_id);
	Buffer& index_buffer = storage.get_buffer(index_buffer_id);

	vk::DeviceOrHostAddressConstKHR vertex_buffer_device_adress(vertex_buffer.get_device_address());
	vk::DeviceOrHostAddressConstKHR index_buffer_device_adress(index_buffer.get_device_address());

	std::vector<uint32_t> num_triangles;
	std::vector<vk::AccelerationStructureBuildRangeInfoKHR> asbris;
	std::vector<vk::AccelerationStructureGeometryKHR> asgs;
	for (uint32_t i = 0; i < index_offsets.size(); ++i)
	{
		vk::AccelerationStructureBuildRangeInfoKHR asbri{};
		asbri.primitiveCount = index_counts.empty() ? index_buffer.get_element_count() / 3 : index_counts[i] / 3;
		asbri.primitiveOffset = sizeof(uint32_t) * index_offsets[i];
		asbri.firstVertex = 0;
		asbri.transformOffset = 0;
		asbris.push_back(asbri);
		num_triangles.push_back(asbri.primitiveCount);

		vk::AccelerationStructureGeometryKHR asg{};
		asg.flags = vk::GeometryFlagBitsKHR::eOpaque;
		asg.geometryType = vk::GeometryTypeKHR::eTriangles;
		asg.geometry.triangles.sType = vk::StructureType::eAccelerationStructureGeometryTrianglesDataKHR;
		asg.geometry.triangles.vertexFormat = vk::Format::eR32G32B32Sfloat;
		asg.geometry.triangles.vertexData = vertex_buffer_device_adress;
		asg.geometry.triangles.maxVertex = vertex_buffer.get_element_count();
		asg.geometry.triangles.vertexStride = vertex_stride;
		asg.geometry.triangles.indexType = vk::IndexType::eUint32;
		asg.geometry.triangles.indexData = index_buffer_device_adress;
		asg.geometry.triangles.transformData.deviceAddress = 0;
		asg.geometry.triangles.transformData.hostAddress = nullptr;
		asgs.push_back(asg);
	}
	std::vector<vk::AccelerationStructureBuildRangeInfoKHR*> pasbris;
	pasbris.push_back(asbris.data());

	vk::AccelerationStructureBuildGeometryInfoKHR asbgi{};
	asbgi.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	asbgi.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	asbgi.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	asbgi.geometryCount = asgs.size();
	asbgi.pGeometries = asgs.data();

	AccelerationStructure blas;
	vk::AccelerationStructureBuildSizesInfoKHR asbsi = vmc.logical_device.get().getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, asbgi, num_triangles);
	blas.buffer = storage.add_buffer(buffer_name, asbsi.accelerationStructureSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR, true, QueueFamilyFlags::Compute | QueueFamilyFlags::Graphics | QueueFamilyFlags::Transfer);

	vk::AccelerationStructureCreateInfoKHR asci{};
	asci.sType = vk::StructureType::eAccelerationStructureCreateInfoKHR;
	asci.buffer = storage.get_buffer(blas.buffer).get();
	asci.size = asbsi.accelerationStructureSize;
	asci.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	blas.handle = vmc.logical_device.get().createAccelerationStructureKHR(asci);

	vk::AccelerationStructureDeviceAddressInfoKHR asdai{};
	asdai.sType = vk::StructureType::eAccelerationStructureDeviceAddressInfoKHR;
	asdai.accelerationStructure = blas.handle;

	blas.device_address = vmc.logical_device.get().getAccelerationStructureAddressKHR(&asdai);

	blas.scratch_buffer = storage.add_buffer(buffer_name + " scratch (vkte internal)", asbsi.buildScratchSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress, true, QueueFamilyFlags::Compute | QueueFamilyFlags::Graphics | QueueFamilyFlags::Transfer);

	asbgi.dstAccelerationStructure = blas.handle;
	asbgi.scratchData.deviceAddress = storage.get_buffer(blas.scratch_buffer).get_device_address();
	std::vector<vk::AccelerationStructureBuildGeometryInfoKHR> asbgis{};
	asbgis.push_back(asbgi);

	cb.buildAccelerationStructuresKHR(asbgis, pasbris);
	vk::BufferMemoryBarrier buffer_memory_barrier(vk::AccessFlagBits::eAccelerationStructureWriteKHR, vk::AccessFlagBits::eAccelerationStructureReadKHR, vmc.queue_families.get(build_queue), vmc.queue_families.get(build_queue), storage.get_buffer(blas.buffer).get(), 0, storage.get_buffer(blas.buffer).get_byte_size());
	cb.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, vk::DependencyFlagBits::eDeviceGroup, {}, {buffer_memory_barrier}, {});

	bottom_level_as.push_back(blas);
	return bottom_level_as.size() - 1;
}

uint32_t AccelerationStructureBuilder::add_instance(uint32_t blas_idx, const vk::TransformMatrixKHR& M, uint32_t custom_index)
{
	vk::AccelerationStructureInstanceKHR instance;
	instance.transform = M;
	instance.accelerationStructureReference = bottom_level_as[blas_idx].device_address;
	instance.instanceCustomIndex = custom_index;
	instance.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);
	instance.mask = 0xFF;
	instances.push_back(instance);
	return instances.size() - 1;
}

void AccelerationStructureBuilder::update_instance(uint32_t instance_idx, const vk::TransformMatrixKHR& M)
{
	instances[instance_idx].transform = M;
}

void AccelerationStructureBuilder::construct(vk::CommandBuffer& cb, const std::string& buffer_name)
{
	instances_buffer = storage.add_buffer(buffer_name + " instances (vkte internal)", instances, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR, true, QueueFamilyFlags::Compute | QueueFamilyFlags::Graphics | QueueFamilyFlags::Transfer);

	vk::DeviceOrHostAddressConstKHR instance_data_device_address;
	instance_data_device_address.deviceAddress = storage.get_buffer(instances_buffer).get_device_address();

	vk::AccelerationStructureGeometryKHR asg;
	asg.geometryType = vk::GeometryTypeKHR::eInstances;
	asg.flags = vk::GeometryFlagBitsKHR::eOpaque;
	asg.geometry.instances.sType = vk::StructureType::eAccelerationStructureGeometryInstancesDataKHR;
	asg.geometry.instances.arrayOfPointers = VK_FALSE;
	asg.geometry.instances.data = instance_data_device_address;

	vk::AccelerationStructureBuildGeometryInfoKHR asbgi;
	asbgi.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	asbgi.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	asbgi.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	asbgi.geometryCount = 1;
	asbgi.pGeometries = &asg;

	uint32_t primitive_count = instances.size();

	vk::AccelerationStructureBuildSizesInfoKHR asbsi = vmc.logical_device.get().getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, asbgi, primitive_count);

	top_level_as.buffer = storage.add_buffer(buffer_name, asbsi.accelerationStructureSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR, true, QueueFamilyFlags::Compute | QueueFamilyFlags::Graphics | QueueFamilyFlags::Transfer);

	vk::AccelerationStructureCreateInfoKHR asci{};
	asci.sType = vk::StructureType::eAccelerationStructureCreateInfoKHR;
	asci.buffer = storage.get_buffer(top_level_as.buffer).get();
	asci.size = asbsi.accelerationStructureSize;
	asci.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	top_level_as.handle = vmc.logical_device.get().createAccelerationStructureKHR(asci);

	vk::AccelerationStructureDeviceAddressInfoKHR asdai{};
	asdai.sType = vk::StructureType::eAccelerationStructureDeviceAddressInfoKHR;
	asdai.accelerationStructure = top_level_as.handle;
	top_level_as.device_address = vmc.logical_device.get().getAccelerationStructureAddressKHR(&asdai);

	wdsas.accelerationStructureCount = 1;
	wdsas.pAccelerationStructures = &(top_level_as.handle);
	storage.get_buffer(top_level_as.buffer).pNext = &(wdsas);

	top_level_as.scratch_buffer = storage.add_buffer(buffer_name + " scratch (vkte internal)", asbsi.buildScratchSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress, true, QueueFamilyFlags::Compute | QueueFamilyFlags::Graphics | QueueFamilyFlags::Transfer);

	asbgi.dstAccelerationStructure = top_level_as.handle;
	asbgi.scratchData.deviceAddress = storage.get_buffer(top_level_as.scratch_buffer).get_device_address();

	vk::AccelerationStructureBuildRangeInfoKHR asbri{};
	asbri.primitiveCount = instances.size();
	asbri.primitiveOffset = 0;
	asbri.firstVertex = 0;
	asbri.transformOffset = 0;
	std::vector<vk::AccelerationStructureBuildRangeInfoKHR*> asbris = {&asbri};

	cb.buildAccelerationStructuresKHR(asbgi, asbris);
}
} // namespace vkte
