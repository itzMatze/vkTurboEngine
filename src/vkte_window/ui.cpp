#include "vkte_window/ui.hpp"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_sdl3.h"

namespace vkte
{
UI::UI(const vkte::VulkanMainContext& vmc) : vmc(vmc)
{}

void UI::construct(vkte::VulkanCommandContext& vcc, const vkte::RenderPass& render_pass, uint32_t frames)
{
	std::vector<vk::DescriptorPoolSize> pool_sizes =
	{
		{ vk::DescriptorType::eSampler, 1000 },
		{ vk::DescriptorType::eCombinedImageSampler, 1000 },
		{ vk::DescriptorType::eSampledImage, 1000 },
		{ vk::DescriptorType::eStorageImage, 1000 },
		{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
		{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
		{ vk::DescriptorType::eUniformBuffer, 1000 },
		{ vk::DescriptorType::eStorageBuffer, 1000 },
		{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
		{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
		{ vk::DescriptorType::eInputAttachment, 1000 }
	};

	vk::DescriptorPoolCreateInfo dpci{};
	dpci.sType = vk::StructureType::eDescriptorPoolCreateInfo;
	dpci.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	dpci.maxSets = 1000;
	dpci.poolSizeCount = pool_sizes.size();
	dpci.pPoolSizes = pool_sizes.data();

	imgui_pool = vmc.logical_device.get().createDescriptorPool(dpci);

	ImGui::CreateContext();
	ImGui_ImplSDL3_InitForVulkan(vmc.window.get());
	ImGui_ImplVulkan_InitInfo init_info{};
	init_info.Instance = vmc.instance.get();
	init_info.PhysicalDevice = vmc.physical_device.get();
	init_info.Device = vmc.logical_device.get();
	init_info.Queue = vmc.get_graphics_queue();
	init_info.DescriptorPool = imgui_pool;
	init_info.RenderPass = render_pass.get();
	init_info.Subpass = 0;
	init_info.MinImageCount = frames;
	init_info.ImageCount = frames;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_3, [](const char* function_name, void* userData) {
		return VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(VkInstance(userData), function_name);
	}, (void*)(vmc.instance.get()));

	ImGui_ImplVulkan_Init(&init_info);
	ImGui::StyleColorsDark();
}

void UI::destruct()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
	vmc.logical_device.get().destroyDescriptorPool(imgui_pool);
}

void UI::new_frame(const std::string& title)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin(title.c_str());
}

void UI::end_frame(vk::CommandBuffer& cb)
{
	ImGui::End();
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb);
}
} // namespace vkte
