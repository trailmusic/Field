#pragma once
#include <atomic>

namespace field { namespace core { namespace runtime {

struct TailManager
{
	void reset() noexcept { secs.store(0.0, std::memory_order_release); }
	void setSeconds (double s) noexcept { secs.store (s >= 0.0 ? s : 0.0, std::memory_order_release); }
	double getSeconds() const noexcept { return secs.load(std::memory_order_acquire); }
private:
	std::atomic<double> secs { 0.0 };
};

}}} // namespace field::core::runtime
