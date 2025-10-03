#pragma once
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

namespace field { namespace core { namespace telemetry {

struct FirstBadSample
{
	int channel = -1;
	int index   = -1;
	bool ok() const noexcept { return channel < 0; }
};

template <typename Sample>
inline FirstBadSample scanBlock (juce::dsp::AudioBlock<Sample> b) noexcept
{
	FirstBadSample r{};
	for (size_t ch = 0; ch < b.getNumChannels(); ++ch)
	{
		const auto* p = b.getChannelPointer(ch);
		for (size_t i = 0, n = b.getNumSamples(); i < n; ++i)
		{
			const auto x = p[i];
			if (!juce::isFinite(x))
				return { (int)ch, (int)i };
		}
	}
	return r;
}

struct StateSanity
{
	void flagMidBlockRebuild() noexcept { midBlockRebuild.store(true, std::memory_order_release); }
	bool consumeMidBlockRebuild() noexcept
	{
		const bool f = midBlockRebuild.exchange(false, std::memory_order_acq_rel);
		if (f) DBG("[StateSanity] mid-block rebuild detected (gate to prepareToPlay)");
		return f;
	}
private:
	std::atomic<bool> midBlockRebuild{false};
};

}}} // namespace field::core::telemetry
