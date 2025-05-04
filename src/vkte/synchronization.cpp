#include "vkte/synchronization.hpp"

#include "vkte/vkte_log.hpp"

namespace vkte
{
Synchronization::Synchronization(const vk::Device& logical_device) : device(logical_device)
{}

void Synchronization::construct(uint32_t semaphore_count, uint32_t fence_count)
{
	for (uint32_t i = 0; i < semaphore_count; ++i)
	{
		vk::SemaphoreCreateInfo sci{};
		sci.sType = vk::StructureType::eSemaphoreCreateInfo;
		semaphores.push_back(device.createSemaphore(sci));
	}
	// all fences are created as signaled, if an unsignaled fence is needed use reset_fence
	for (uint32_t i = 0; i < fence_count; ++i)
	{
		vk::FenceCreateInfo fci{};
		fci.sType = vk::StructureType::eFenceCreateInfo;
		fci.flags = vk::FenceCreateFlagBits::eSignaled;
		fences.push_back(device.createFence(fci));
	}
}

void Synchronization::destruct()
{
	device.waitIdle();
	for (auto& s : semaphores) device.destroy(s);
	semaphores.clear();
	for (auto& f : fences) device.destroyFence(f);
	fences.clear();
}

const vk::Semaphore& Synchronization::get_semaphore(uint32_t semaphore_index) const
{
	return semaphores[semaphore_index];
}

const vk::Fence& Synchronization::get_fence(uint32_t fence_index) const
{
	return fences[fence_index];
}

void Synchronization::wait_for_fence(uint32_t fence_index) const
{
	VKTE_CHECK(device.waitForFences(fences[fence_index], 1, uint64_t(-1)), "Failed to wait for fence!");
}

bool Synchronization::is_fence_finished(uint32_t fence_index) const
{
	return (device.getFenceStatus(fences[fence_index]) == vk::Result::eSuccess);
}

void Synchronization::reset_fence(uint32_t fence_index) const
{
	device.resetFences(fences[fence_index]);
}
} // namespace vkte
