#pragma once

#include "vulkan/vulkan.hpp"

namespace vkte
{
class Synchronization
{
public:
	Synchronization(const vk::Device& logical_device);
	void construct(uint32_t semaphore_count, uint32_t fence_count);
	void destruct();
	const vk::Semaphore& get_semaphore(uint32_t semaphore_index) const;
	const vk::Fence& get_fence(uint32_t fence_index) const;
	bool is_fence_finished(uint32_t fence_index) const;
	void wait_for_fence(uint32_t fence_index) const;
	void reset_fence(uint32_t fence_index) const;

private:
	const vk::Device& device;
	std::vector<vk::Semaphore> semaphores;
	std::vector<vk::Fence> fences;
};
} // namespace vkte
