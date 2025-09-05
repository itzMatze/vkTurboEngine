#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "vkte/extensions_handler.hpp"

namespace vkte
{
class Instance
{
public:
	Instance() = default;
	void construct(std::vector<const char*> required_extensions, std::vector<const char*> validation_layers);
	void destruct();
	const vk::Instance& get() const;
	const std::vector<const char*>& get_missing_extensions() const;
	std::vector<vk::PhysicalDevice> get_physical_devices() const;

private:
	vk::Instance instance;
	ExtensionsHandler extensions_handler;
	ExtensionsHandler validation_handler;
};
} // namespace vkte
