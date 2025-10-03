#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>

namespace field { namespace modules { namespace mixing {

template <size_t MaxChannels = 16>
struct Node_Meter
{
	template <typename Sample>
	void prepare (double /*sr*/, int /*maxBlock*/, int chans)
	{
		chans_ = (chans > 0 ? (chans <= (int)MaxChannels ? chans : (int)MaxChannels) : 2);
		reset();
	}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& io) noexcept
	{
		for (int ch = 0; ch < chans_; ++ch)
		{
			auto* p = io.getChannelPointer((size_t)ch);
			float peak = 0.f;
			for (size_t i = 0, n = io.getNumSamples(); i < n; ++i)
				peak = std::max(peak, (float)std::abs((double)p[i]));
			peaks_[ch].store(peak, std::memory_order_release);
		}
	}

	void reset() noexcept
	{
		for (auto& a : peaks_) a.store(0.f, std::memory_order_relaxed);
	}

	float getPeak (int ch) const noexcept { return (ch >= 0 && ch < (int)MaxChannels) ? peaks_[ch].load() : 0.f; }
	int latencySamples() const noexcept { return 0; }

private:
	int chans_ = 2;
	std::array<std::atomic<float>, MaxChannels> peaks_{};
};

}}} // namespace field::modules::mixing
