#pragma once
#include <cstddef>
#include <atomic>
#include <algorithm>

namespace field { namespace core { namespace signal {

struct CrossfadeRamp
{
	void setLength (int samples) noexcept { len_ = std::max (1, samples); }
	void start() noexcept { pos_.store(0, std::memory_order_release); active_.store(true, std::memory_order_release); }
	bool active() const noexcept { return active_.load(std::memory_order_acquire); }
	inline float next() noexcept
	{
		const int p = pos_.load(std::memory_order_relaxed);
		const int L = len_;
		if (p >= L) { active_.store(false, std::memory_order_release); return 1.0f; }
		pos_.store (p + 1, std::memory_order_relaxed);
		return (float)(p + 1) / (float)L;
	}
private:
	std::atomic<int> pos_{0};
	std::atomic<bool> active_{false};
	int len_ = 64;
};

}}} // namespace field::core::signal
