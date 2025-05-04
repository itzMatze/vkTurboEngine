#pragma once

#include "vkte/vkte_log.hpp"
#include "vkte/vulkan_main_context.hpp"
#include <ratio>

namespace vkte
{
class DeviceTimer
{
public:
	DeviceTimer(const VulkanMainContext& vmc);
	void construct(uint32_t timer_count);
	void destruct();
	void reset(vk::CommandBuffer& cb, uint32_t timer_index);
	void reset_all(vk::CommandBuffer& cb);
	void start(vk::CommandBuffer& cb, uint32_t timer_index, vk::PipelineStageFlagBits stage);
	void stop(vk::CommandBuffer& cb, uint32_t timer_index, vk::PipelineStageFlagBits stage);

	template<class Precision = std::milli>
	double inline get_result_by_idx(uint32_t i)
	{
		VKTE_ASSERT(i < result_fetched.size(), "Trying to access invalid timer index.");
		return fetch_result<Precision>(i);
	}

	template<class Precision = std::milli>
	double inline get_result(uint32_t timer_index)
	{
		return fetch_result<Precision>(timer_index);
	}

private:
	template<class Precision>
	double inline fetch_result(uint32_t i)
	{
		// prevent repeated reading of the same timestamp
		if (result_fetched[i]) return -1.0;
		result_fetched[i] = true;
		std::array<uint64_t, 2> results;
		vk::Result result = vmc.logical_device.get().getQueryPoolResults(qp, i * 2, 2, results.size() * sizeof(uint64_t), results.data(), sizeof(uint64_t), vk::QueryResultFlagBits::e64);
		return (result == vk::Result::eSuccess) ? (double(timestamp_period * (results[1] - results[0])) / double(std::ratio_divide<std::nano, Precision>::den)) : -1.0;
	}

	const VulkanMainContext& vmc;
	vk::QueryPool qp;
	float timestamp_period;
	std::vector<bool> result_fetched;
};
} // namespace vkte
