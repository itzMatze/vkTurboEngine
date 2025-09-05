#pragma once

#include <optional>
#include <vulkan/vulkan.hpp>

#include "vkte/extensions_handler.hpp"
#include "vkte/instance.hpp"

namespace vkte
{
class PhysicalDevice
{
public:
	PhysicalDevice() = default;
	void construct(const Instance& instance, const std::vector<const char*>& required_extensions, const std::optional<vk::SurfaceKHR>& surface);
	vk::PhysicalDevice get() const;
	const std::vector<const char*>& get_extensions() const;
	const std::vector<const char*>& get_missing_extensions();

private:
	vk::PhysicalDevice physical_device;
	ExtensionsHandler extensions_handler;

	bool is_device_suitable(uint32_t idx, const vk::PhysicalDevice p_device, const std::optional<vk::SurfaceKHR>& surface);
};
} // namespace vkte
