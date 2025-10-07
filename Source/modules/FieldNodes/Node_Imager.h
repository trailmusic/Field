#pragma once
#include <juce_dsp/juce_dsp.h>
#include "NodeLatency.h"

namespace field { namespace modules { namespace nodes {
struct Node_Imager : NodeLatencyMixin<Node_Imager>
{
    void setWidth (float w) noexcept { width_ = w; }

	template <typename Sample>
	void prepare (double /*sr*/, int /*maxBlock*/, int /*chans*/) noexcept {}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& io) const noexcept
	{
		if (io.getNumChannels() < 2) return;
		const size_t n = io.getNumSamples();
		auto* L = io.getChannelPointer(0);
		auto* R = io.getChannelPointer(1);
		const Sample kInvSqrt2 = (Sample) 0.7071067811865475;
		const Sample w = (Sample) juce::jlimit (0.0f, 2.0f, width_);
		if (w == (Sample) 1) return;
		for (size_t i = 0; i < n; ++i)
		{
			const Sample l = L[i], r = R[i];
			const Sample m = (l + r) * kInvSqrt2;
			const Sample s = (l - r) * kInvSqrt2;
			const Sample s2 = s * w;
			L[i] = (m + s2) * kInvSqrt2;
			R[i] = (m - s2) * kInvSqrt2;
		}
	}

	void reset() noexcept {}

private:
    float width_ { 1.0f };
};
}}} // namespace field::modules::nodes
