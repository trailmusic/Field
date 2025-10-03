#pragma once
#include <cstdint>

namespace field { namespace modules { namespace nodes {

struct LatencyParts
{
	int osGroupDelay = 0;
	int firGroupDelay = 0;
    int lookahead = 0;
	int extra = 0;

	int sum() const noexcept
	{
		auto clamp0 = [](int v){ return v < 0 ? 0 : v; };
		return clamp0(osGroupDelay) + clamp0(firGroupDelay)
			 + clamp0(lookahead)    + clamp0(extra);
	}
};

template <typename Derived>
struct NodeLatencyMixin
{
	void setOversamplingGroupDelay (int samples) noexcept { parts_.osGroupDelay = samples; }
	void setLinearPhaseFIRGroupDelay (int samples) noexcept { parts_.firGroupDelay = samples; }
    void setLookahead (int samples) noexcept { parts_.lookahead = samples; }
    void setLookAheadSamples (int samples) noexcept { parts_.lookahead = samples; }
	void setExtraLatency (int samples) noexcept { parts_.extra = samples; }
	int latencySamples() const noexcept { return parts_.sum(); }
protected:
	LatencyParts parts_{};
};

}}} // namespace field::modules::nodes
