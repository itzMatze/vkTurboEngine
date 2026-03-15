#pragma once

#include <string>
#include "vulkan/vulkan.hpp"

namespace vkte
{
class Swapchain;
class VulkanCommandContext;
class VulkanMainContext;

class UI
{
public:
	explicit UI(const vkte::VulkanMainContext& vmc);
	void construct(const vkte::Swapchain& swapchain);
	void destruct();
	void new_frame(const std::string& title);
	void end_frame(vk::CommandBuffer& cb);

private:
	const vkte::VulkanMainContext& vmc;
	vk::DescriptorPool imgui_pool;
};
} // namespace vkte
