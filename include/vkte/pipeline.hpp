#pragma once

#include "vulkan/vulkan.hpp"
#include "vkte/render_pass.hpp"
#include "vkte/vulkan_main_context.hpp"
#include "vkte/shader.hpp"

namespace vkte
{
class Pipeline
{
public:
	Pipeline(const VulkanMainContext& vmc);

	struct GraphicsSettings
	{
		const RenderPass* render_pass;
		const vk::DescriptorSetLayout* set_layout;
		const std::vector<ShaderInfo>* shader_infos;
		vk::PolygonMode polygon_mode = vk::PolygonMode::eFill;
		const vk::VertexInputBindingDescription* binding_descriptions = nullptr;
		const std::vector<vk::VertexInputAttributeDescription>* attribute_description = nullptr;
		vk::PrimitiveTopology primitive_topology = vk::PrimitiveTopology::eTriangleList;
		const std::vector<vk::PushConstantRange>* pcrs = nullptr;
	};

	void construct(const GraphicsSettings& settings);

	struct ComputeSettings
	{
		const vk::DescriptorSetLayout* set_layout;
		const ShaderInfo* shader_info;
		uint32_t push_constant_byte_size = 0;
	};

	void construct(const ComputeSettings& settings);
	void destruct();
	const vk::Pipeline& get() const;
	const vk::PipelineLayout& get_layout() const;

private:
	const VulkanMainContext& vmc;
	vk::PipelineLayout pipeline_layout;
	vk::Pipeline pipeline;
};
} // namespace vkte
