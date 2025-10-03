#pragma once
#include "FieldChain.h"
#include "core/signal/CrossfadeRamp.h"
#include <atomic>

namespace field { namespace modules {

struct DualChain
{
	using Config = FieldChain::Config;

	void setCrossfadeLength (int samples) { ramp_.setLength(samples); }

	void setConfig (const Config& cfg)      { pendingCfg_ = cfg; }
	const Config& getConfig() const noexcept{ return pendingCfg_; }

	int buildStaging (double sr, int maxBlock, int chans)
	{
		staging_.setConfig (pendingCfg_);
		staging_.buildFromConfig();
		staging_.prepare<float>  (sr, maxBlock, chans);
		staging_.prepare<double> (sr, maxBlock, chans);
		return staging_.latencySamples();
	}

	bool armLiveSwapIfSameLatency()
	{
		const int newLat = staging_.latencySamples();
		const int oldLat = active_.latencySamples();
		if (newLat != oldLat) return false;
		wantSwap_.store(true, std::memory_order_release);
		return true;
	}

	void promoteStagingHard()
	{
		active_ = std::move(staging_);
		wantSwap_.store(false, std::memory_order_release);
	}

	int latencySamples() const noexcept { return active_.latencySamples(); }

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& io)
	{
		if (!ramp_.active())
		{
			if (wantSwap_.load(std::memory_order_acquire))
			{
				ensureTmp (io.getNumChannels(), io.getNumSamples());
				tmp_.setSize ((int)io.getNumChannels(), (int)io.getNumSamples(), false, false, true);
				for (size_t ch = 0; ch < io.getNumChannels(); ++ch)
					tmp_.copyFrom ((int)ch, 0, io.getChannelPointer(ch), (int)io.getNumSamples());

				juce::dsp::AudioBlock<Sample> blockActive (io);
				active_.process<Sample>(blockActive);

				juce::dsp::AudioBlock<Sample> blockStaging (tmp_.getArrayOfWritePointers(), io.getNumChannels(), io.getNumSamples());
				staging_.process<Sample>(blockStaging);

				ramp_.start();
			}
			else
			{
				juce::dsp::AudioBlock<Sample> block (io);
				active_.process<Sample>(block);
				return;
			}
		}

		if (ramp_.active())
		{
			for (size_t ch = 0; ch < io.getNumChannels(); ++ch)
			{
				auto* dst = io.getChannelPointer(ch);
				auto* src = tmp_.getReadPointer((int)ch);
				const size_t N = io.getNumSamples();
				for (size_t i = 0; i < N; ++i)
				{
					float w = ramp_.next();
					dst[i] = (Sample)((1.0f - w) * dst[i] + w * src[i]);
				}
			}
			if (!ramp_.active())
			{
				active_ = std::move(staging_);
				wantSwap_.store(false, std::memory_order_release);
				tmp_.setSize(0, 0);
			}
		}
	}

	void reset()
	{
		active_.reset();
		staging_.reset();
		wantSwap_.store(false, std::memory_order_release);
		tmp_.setSize(0,0);
	}

private:
	void ensureTmp (size_t chans, size_t samples)
	{
		if (tmp_.getNumChannels()  != (int)chans ||
			tmp_.getNumSamples()   != (int)samples)
			tmp_.setSize ((int)chans, (int)samples, false, false, true);
	}

	FieldChain active_{};
	FieldChain staging_{};
	Config     pendingCfg_{};

	std::atomic<bool> wantSwap_{false};
	field::core::signal::CrossfadeRamp ramp_{};
	juce::AudioBuffer<float> tmp_;
};

}} // namespace field::modules
