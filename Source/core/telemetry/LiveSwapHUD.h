#pragma once
#include <atomic>

namespace field { namespace core { namespace telemetry {

struct LiveSwapHUD
{
	enum class State { Idle=0, Armed, DeferredLatency };

	void setArmed (int ms=1200)            noexcept { state_.store(State::Armed);           ttlMs_.store(ms); }
	void setDeferredLatency (int ms=1600)  noexcept { state_.store(State::DeferredLatency); ttlMs_.store(ms); }
	void clear()                           noexcept { state_.store(State::Idle);            ttlMs_.store(0);  }

	void tick (int elapsedMs) noexcept
	{
		int ttl = ttlMs_.load();
		if (ttl <= 0) { clear(); return; }
		ttl -= elapsedMs;
		ttlMs_.store(ttl > 0 ? ttl : 0);
		if (ttl <= 0) clear();
	}

	const char* text() const noexcept
	{
		switch (state_.load())
		{
			case State::Armed:            return "LIVE SWAP: ARMED";
			case State::DeferredLatency:  return "LIVE SWAP: DEFERRED (latency mismatch)";
			default:                      return "";
		}
	}

	bool visible() const noexcept { return state_.load() != State::Idle; }

private:
	std::atomic<State> state_ { State::Idle };
	std::atomic<int>   ttlMs_ { 0 };
};

}}} // namespace field::core::telemetry
