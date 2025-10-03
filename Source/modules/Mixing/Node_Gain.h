#pragma once
#include <juce_dsp/juce_dsp.h>

namespace field { namespace modules { namespace mixing {

struct Node_Gain
{
	void setLinear (float g) noexcept { gain_ = g; }
	void setDecibels (float dB) noexcept { gain_ = juce::Decibels::decibelsToGain(dB); }

	template <typename Sample>
	void prepare (double /*sr*/, int /*maxBlock*/, int /*chans*/) {}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& io) const noexcept
	{
		if (gain_ == Sample(1)) return;
		for (size_t ch = 0; ch < io.getNumChannels(); ++ch)
		{
			auto* p = io.getChannelPointer(ch);
			for (size_t i = 0, n = io.getNumSamples(); i < n; ++i)
				p[i] = (Sample)gain_ * p[i];
		}
	}

	void reset() noexcept { gain_ = 1.0f; }
	int latencySamples() const noexcept { return 0; }

private:
	float gain_ = 1.0f;
};

}}} // namespace field::modules::mixing
