cmake_minimum_required(VERSION 3.19)
project(vkTurboEngine)
set(CMAKE_CXX_STANDARD 20)

option(ENABLE_VKTE_LOGGING "Enable vkte logging" OFF)

set(SDL3_DIR "${PROJECT_SOURCE_DIR}/dependencies/SDL3")

set(SOURCE_FILES
	src/vkte/command_pool.cpp
	src/vkte/descriptor_set_handler.cpp
	src/vkte/device_timer.cpp
	src/vkte/extensions_handler.cpp
	src/vkte/image.cpp
	src/vkte/instance.cpp
	src/vkte/logical_device.cpp
	src/vkte/physical_device.cpp
	src/vkte/pipeline.cpp
	src/vkte/queue_families.cpp
	src/vkte/render_pass.cpp
	src/vkte/shader.cpp
	src/vkte/storage.cpp
	src/vkte/synchronization.cpp
	src/vkte/vulkan_command_context.cpp
	src/vkte/vulkan_main_context.cpp
)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
find_package(Vulkan REQUIRED)

add_library(vkte_headless ${SOURCE_FILES})
target_include_directories(vkte_headless PUBLIC
	"${PROJECT_SOURCE_DIR}/include/"
	"${PROJECT_SOURCE_DIR}/private/"
	Vulkan::Headers
	"${PROJECT_SOURCE_DIR}/dependencies/VulkanMemoryAllocator/include/"
)
target_compile_definitions(vkte_headless PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 IMGUI_IMPL_VULKAN_NO_PROTOTYPES=1)
if (ENABLE_VKTE_LOGGING)
	target_compile_definitions(vkte_headless PUBLIC VKTE_LOGGING=1)
endif()

add_library(vkte_window ${SOURCE_FILES}
	src/vkte/swapchain.cpp
	src/vkte_window/event_handler.cpp
	src/vkte_window/ui.cpp
	src/vkte_window/window.cpp
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui.cpp"
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_sdl3.cpp"
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_vulkan.cpp"
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_draw.cpp"
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_widgets.cpp"
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_tables.cpp"
)
target_include_directories(vkte_window PUBLIC
	"${PROJECT_SOURCE_DIR}/include/"
	"${PROJECT_SOURCE_DIR}/private/"
	Vulkan::Headers
	"${SDL3_DIR}/include/"
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/"
	"${PROJECT_SOURCE_DIR}/dependencies/VulkanMemoryAllocator/include/"
)
target_link_libraries(vkte_window PUBLIC SDL3::SDL3)
target_compile_definitions(vkte_window PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 IMGUI_IMPL_VULKAN_NO_PROTOTYPES=1 ENABLE_VKTE_WINDOW=1)
if (ENABLE_VKTE_LOGGING)
	target_compile_definitions(vkte_window PUBLIC VKTE_LOGGING=1)
endif()

add_library(vkte::headless ALIAS vkte_headless)
add_library(vkte::window ALIAS vkte_window)
