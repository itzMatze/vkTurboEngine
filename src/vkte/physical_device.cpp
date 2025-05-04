#include "vkte/physical_device.hpp"

#include <iostream>
#include <unordered_set>

#include "vkte/vkte_log.hpp"

namespace vkte
{
void PhysicalDevice::construct(const Instance& instance, const std::optional<vk::SurfaceKHR>& surface)
{
	const std::vector<const char*> required_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	const std::vector<const char*> optional_extensions{VK_KHR_RAY_QUERY_EXTENSION_NAME, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME};
	extensions_handler.add_extensions(required_extensions, true);
	extensions_handler.add_extensions(optional_extensions, false);

	std::vector<vk::PhysicalDevice> physical_devices = instance.get_physical_devices();
	std::unordered_set<uint32_t> suitable_p_devices;
	for (uint32_t i = 0; i < physical_devices.size(); ++i)
	{
		if (is_device_suitable(i, physical_devices[i], surface))
		{
			suitable_p_devices.insert(i);
		}
	}
	if (suitable_p_devices.size() > 1)
	{
		uint32_t pd_idx = 0;
		do
		{
			std::cerr << "Select one of the suitable GPUs by typing the number" << std::endl;
			std::cin >> pd_idx;
		} while (!suitable_p_devices.contains(pd_idx));
		physical_device = physical_devices[pd_idx];
	}
	else if (suitable_p_devices.size() == 1)
	{
		VKTE_INFO("vkte: Only one suitable GPU. Using this one.");
		physical_device = physical_devices[*(suitable_p_devices.begin())];
	}
	else
	{
		VKTE_THROW("No suitable GPUs found!");
	}
	vk::PhysicalDeviceProperties pdp = physical_device.getProperties();
	VKTE_INFO("vkte: GPU: {}", std::string(pdp.deviceName.data()));
	extensions_handler.remove_missing_extensions();
}

vk::PhysicalDevice PhysicalDevice::get() const
{
	return physical_device;
}

const std::vector<const char*>& PhysicalDevice::get_extensions() const
{
	return extensions_handler.get_extensions();
}

const std::vector<const char*>& PhysicalDevice::get_missing_extensions()
{
	return extensions_handler.get_missing_extensions();
}

bool is_swapchain_supported(const vk::PhysicalDevice p_device, const vk::SurfaceKHR& surface)
{
	std::vector<vk::SurfaceFormatKHR> f = p_device.getSurfaceFormatsKHR(surface);
	std::vector<vk::PresentModeKHR> pm = p_device.getSurfacePresentModesKHR(surface);
	return !f.empty() && !pm.empty();
}

bool PhysicalDevice::is_device_suitable(uint32_t idx, const vk::PhysicalDevice p_device, const std::optional<vk::SurfaceKHR>& surface)
{
	vk::PhysicalDeviceProperties pdp = p_device.getProperties();
	vk::PhysicalDeviceFeatures p_device_features = p_device.getFeatures();
	std::vector<vk::ExtensionProperties> available_extensions = p_device.enumerateDeviceExtensionProperties();
	std::vector<const char*> avail_ext_names;
	for (const auto& ext : available_extensions) avail_ext_names.push_back(ext.extensionName);
	std::cerr << "  " << idx << " " << pdp.deviceName << " ";
	int32_t missing_extensions = extensions_handler.check_extension_availability(avail_ext_names);
	if (missing_extensions == -1 || !p_device_features.samplerAnisotropy || (surface.has_value() && extensions_handler.find_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME) && !is_swapchain_supported(p_device, surface.value())))
	{
		std::cerr << "(not suitable)\n";
		return false;
	}
	std::cerr << "(suitable, " << missing_extensions << " missing optional extensions)\n";
	return true;
}
} // namespace vkte
