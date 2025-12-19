#pragma once

#include <vector>
#include "vulkan/vulkan.hpp"
#include "SDL3/SDL_video.h"

namespace vkte
{
class Window
{
public:
	Window() = default;
	Window(const std::string& title, const uint32_t width, const uint32_t height);
	void destruct();
	SDL_Window* get() const;
	std::vector<const char*> get_required_extensions() const;
	vk::SurfaceKHR create_surface(const vk::Instance& instance);

private:
	SDL_Window* window;
};
} // namespace vkte
