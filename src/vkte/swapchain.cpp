#include "vkte/swapchain.hpp"

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_vulkan.h"
#include "vkte/vkte_log.hpp"

namespace vkte
{
Swapchain::Swapchain(const VulkanMainContext& vmc, VulkanCommandContext& vcc, Storage& storage) : vmc(vmc), vcc(vcc), storage(storage), render_pass(vmc)
{}

const vk::SwapchainKHR& Swapchain::get() const
{
	return swapchain;
}

const RenderPass& Swapchain::get_render_pass() const
{
	return render_pass;
}

vk::Extent2D Swapchain::get_extent() const
{
	return extent;
}

vk::Framebuffer Swapchain::get_framebuffer(uint32_t idx) const
{
	return framebuffers[idx];
}

void Swapchain::construct(bool vsync)
{
	construct(vsync, true);
}

void Swapchain::destruct()
{
	destruct(true);
}

void Swapchain::recreate(bool vsync)
{
	destruct(false);
	construct(vsync, false);
}

void Swapchain::construct(bool vsync, bool full)
{
	extent = choose_extent();
	surface_format = choose_surface_format();
	depth_format = choose_depth_format();
	if (full) render_pass.construct(surface_format.format, depth_format);
	swapchain = create_swapchain(vsync);
	depth_buffer = storage.add_image("depth_buffer", extent.width, extent.height, vk::ImageUsageFlagBits::eDepthStencilAttachment, depth_format, vk::SampleCountFlagBits::e1, false, 0, QueueFamilyFlags::Graphics);
	create_framebuffers();
}

void Swapchain::destruct(bool full)
{
	for (auto& framebuffer : framebuffers) vmc.logical_device.get().destroyFramebuffer(framebuffer);
	framebuffers.clear();
	for (auto& image_view : image_views) vmc.logical_device.get().destroyImageView(image_view);
	image_views.clear();
	storage.destroy_image(depth_buffer);
	vmc.logical_device.get().destroySwapchainKHR(swapchain);
	if (full)
	{
		render_pass.destruct();
	}
}

vk::SwapchainKHR Swapchain::create_swapchain(bool vsync)
{
	vk::SurfaceCapabilitiesKHR capabilities = vmc.get_surface_capabilities();
	uint32_t image_count = capabilities.maxImageCount > 0 ? std::min(capabilities.minImageCount + 1, capabilities.maxImageCount) : capabilities.minImageCount + 1;

	vk::SwapchainCreateInfoKHR sci{};
	sci.sType = vk::StructureType::eSwapchainCreateInfoKHR;
	sci.surface = vmc.surface;
	sci.minImageCount = image_count;
	sci.imageFormat = surface_format.format;
	sci.imageColorSpace = surface_format.colorSpace;
	sci.imageExtent = extent;
	sci.imageArrayLayers = 1;
	sci.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	sci.preTransform = capabilities.currentTransform;
	sci.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	sci.presentMode = choose_present_mode(vsync);
	sci.clipped = VK_TRUE;
	sci.oldSwapchain = VK_NULL_HANDLE;
	std::vector<uint32_t> queue_family_indices = vmc.queue_families.get(QueueFamilyFlags::Graphics | QueueFamilyFlags::Present);
	if (queue_family_indices.size() > 1)
	{
		sci.imageSharingMode = vk::SharingMode::eConcurrent;
		sci.queueFamilyIndexCount = queue_family_indices.size();
		sci.pQueueFamilyIndices = queue_family_indices.data();
	}
	else
	{
		sci.imageSharingMode = vk::SharingMode::eExclusive;
	}
	return vmc.logical_device.get().createSwapchainKHR(sci);
}

void Swapchain::create_framebuffers()
{
	images = vmc.logical_device.get().getSwapchainImagesKHR(swapchain);

	for (const auto& image : images)
	{
		vk::ImageViewCreateInfo ivci{};
		ivci.sType = vk::StructureType::eImageViewCreateInfo;
		ivci.image = image;
		ivci.viewType = vk::ImageViewType::e2D;
		ivci.format = surface_format.format;
		ivci.components.r = vk::ComponentSwizzle::eIdentity;
		ivci.components.g = vk::ComponentSwizzle::eIdentity;
		ivci.components.b = vk::ComponentSwizzle::eIdentity;
		ivci.components.a = vk::ComponentSwizzle::eIdentity;
		ivci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		ivci.subresourceRange.baseMipLevel = 0;
		ivci.subresourceRange.levelCount = 1;
		ivci.subresourceRange.baseArrayLayer = 0;
		ivci.subresourceRange.layerCount = 1;
		image_views.push_back(vmc.logical_device.get().createImageView(ivci));
	}

	for (const auto& image_view : image_views)
	{
		std::vector<vk::ImageView> attachments = {image_view, storage.get_image(depth_buffer).get_view()};
		vk::FramebufferCreateInfo fbci{};
		fbci.sType = vk::StructureType::eFramebufferCreateInfo;
		fbci.renderPass = render_pass.get();
		fbci.attachmentCount = attachments.size();
		fbci.pAttachments = attachments.data();
		fbci.width = extent.width;
		fbci.height = extent.height;
		fbci.layers = 1;

		framebuffers.push_back(vmc.logical_device.get().createFramebuffer(fbci));
	}
}

vk::PresentModeKHR Swapchain::choose_present_mode(bool vsync)
{
	std::vector<vk::PresentModeKHR> present_modes = vmc.get_surface_present_modes();
	for (const auto& pm : present_modes)
	{
		if (vsync && pm == vk::PresentModeKHR::eFifo) return pm;
		if (!vsync && pm == vk::PresentModeKHR::eImmediate) return pm;
	}
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::choose_extent()
{
	vk::SurfaceCapabilitiesKHR capabilities = vmc.get_surface_capabilities();
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		extent = capabilities.currentExtent;
	}
	else
	{
		int32_t width, height;
		SDL_Event e;
		do
		{
			SDL_GetWindowSizeInPixels(vmc.window.get(), &width, &height);
			SDL_WaitEvent(&e);
		} while (width == 0 || height == 0);
		vk::Extent2D extent(width, height);
		extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	}
	return extent;
}

vk::SurfaceFormatKHR Swapchain::choose_surface_format()
{
	std::vector<vk::SurfaceFormatKHR> formats = vmc.get_surface_formats();
	for (const auto& format : formats)
	{
		if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) return format;
	}
	return formats[0];
}

vk::Format Swapchain::choose_depth_format()
{
	std::vector<vk::Format> candidates{vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint};
	for (vk::Format format : candidates)
	{
		vk::FormatProperties props = vmc.physical_device.get().getFormatProperties(format);
		if ((props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) == vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		{
			return format;
		}
	}
	VKTE_THROW("Failed to find supported format!");
}
} // namespace vkte
