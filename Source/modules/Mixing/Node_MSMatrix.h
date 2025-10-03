#pragma once
#include <juce_dsp/juce_dsp.h>

namespace field { namespace modules { namespace mixing {

enum class MSMode { Bypass, ToMS, ToLR };

struct Node_MSMatrix
{
	void setMode (MSMode m) noexcept { mode_ = m; }
	MSMode getMode() const noexcept { return mode_; }

	template <typename Sample>
	void prepare (double /*sr*/, int /*maxBlock*/, int /*chans*/) {}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& io) const noexcept
	{
		if (mode_ == MSMode::Bypass) return;
		if (io.getNumChannels() < 2) return;
		const size_t n = io.getNumSamples();
		auto* L = io.getChannelPointer(0);
		auto* R = io.getChannelPointer(1);
		if (mode_ == MSMode::ToMS)
		{
			for (size_t i = 0; i < n; ++i)
			{
				const Sample l = L[i], r = R[i];
				L[i] = (l + r) * (Sample)kInvSqrt2;
				R[i] = (l - r) * (Sample)kInvSqrt2;
			}
		}
		else
		{
			for (size_t i = 0; i < n; ++i)
			{
				const Sample m = L[i], s = R[i];
				L[i] = (m + s) * (Sample)kInvSqrt2;
				R[i] = (m - s) * (Sample)kInvSqrt2;
			}
		}
	}

	void reset() noexcept {}
	int latencySamples() const noexcept { return 0; }

private:
	static constexpr float kInvSqrt2 = 0.7071067811865475f;
	MSMode mode_ = MSMode::Bypass;
};

}}} // namespace field::modules::mixing
