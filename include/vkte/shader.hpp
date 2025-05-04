#pragma once

#include <string>
#include <vulkan/vulkan.hpp>

namespace vkte
{
struct ShaderInfo
{
	std::string shader_name;
	vk::ShaderStageFlagBits stage_flag;
	vk::SpecializationInfo spec_info = {};
};

class Shader
{
public:
	Shader(const vk::Device& device, const std::string filename, vk::ShaderStageFlagBits shader_stage_flag);
	void destruct();
	const vk::ShaderModule get() const;
	const vk::PipelineShaderStageCreateInfo& get_stage_create_info() const;

private:
	const std::string name;
	const vk::Device& device;
	vk::ShaderModule shader_module;
	vk::PipelineShaderStageCreateInfo pssci;

	std::string read_shader_file(const std::string& filename);
};
} // namespace vkte
