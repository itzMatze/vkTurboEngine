#pragma once

#include <vector>
#include "vulkan/vulkan.hpp"

namespace vkte
{
	class CommandPool
	{
	public:
		CommandPool() = default;
		CommandPool(const vk::Device& logical_device, uint32_t queue_family_idx);
		std::vector<vk::CommandBuffer> create_command_buffers(uint32_t count);
		void destruct();

	private:
		vk::Device device;
		vk::CommandPool command_pool;
	};
} // namespace vkte
