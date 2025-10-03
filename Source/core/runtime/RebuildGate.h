#pragma once
#include <atomic>
struct RebuildGate {
	std::atomic<bool> need{false};
	void request() noexcept { need.store(true, std::memory_order_release); }
	bool consume() noexcept { return need.exchange(false, std::memory_order_acq_rel); }
};
