#pragma once

#include "vkte/render_pass.hpp"
#include "vkte/vulkan_main_context.hpp"
#include "vkte/vulkan_command_context.hpp"

namespace vkte
{
class UI
{
public:
	explicit UI(const vkte::VulkanMainContext& vmc);
	void construct(vkte::VulkanCommandContext& vcc, const vkte::RenderPass& render_pass, uint32_t frames);
	void destruct();
	void new_frame(const std::string& title);
	void end_frame(vk::CommandBuffer& cb);

private:
	const vkte::VulkanMainContext& vmc;
	vk::DescriptorPool imgui_pool;
};
} // namespace vkte
