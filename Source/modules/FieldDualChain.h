#pragma once
#include "FieldChain.h"
#include "core/signal/CrossfadeRamp.h"
#include "core/signal/Warmup.h"
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
		auto& staging = stagingChain();
		staging.setConfig (pendingCfg_);
		staging.buildFromConfig();
		staging.prepare<float>  (sr, maxBlock, chans);
		staging.prepare<double> (sr, maxBlock, chans);
		return staging.latencySamples();
	}

    bool armLiveSwapIfSameLatency()
    {
        return armLiveSwapAtSameLatency(0, 0);
    }

    bool armLiveSwapAtSameLatency (int offsetSamples, int warmupBlocks)
    {
		const int newLat = stagingChain().latencySamples();
		const int oldLat = activeChain().latencySamples();
        if (newLat != oldLat) return false;
        pendingOffset_.store (std::max (0, offsetSamples), std::memory_order_release);
        warmupBlocks_.store (std::max (0, warmupBlocks),   std::memory_order_release);
        wantSwap_.store(true, std::memory_order_release);
        return true;
    }

	void promoteStagingHard()
	{
		activeIndex_ ^= 1;
		wantSwap_.store(false, std::memory_order_release);
	}

	int latencySamples() const noexcept { return activeChain().latencySamples(); }

    // Accessors for planner integration
	FieldChain& activeChain() noexcept { return chains_[activeIndex_]; }
	FieldChain& stagingChain() noexcept { return chains_[activeIndex_ ^ 1]; }
	const FieldChain& activeChain() const noexcept { return chains_[activeIndex_]; }
	const FieldChain& stagingChain() const noexcept { return chains_[activeIndex_ ^ 1]; }

    template <typename Sample>
    void process (juce::dsp::AudioBlock<Sample>& io)
    {
        const size_t chans  = io.getNumChannels();
        const size_t frames = io.getNumSamples();

        if (!activeChain().isPrepared()) { io.clear(); return; }

		if (!ramp_.active() && !wantSwap_.load(std::memory_order_acquire))
		{
			activeChain().process<Sample>(io);
			return;
		}

        ensureTmp (chans, frames);

        int warmN = warmupBlocks_.exchange (0, std::memory_order_acq_rel);
        if (warmN > 0)
        {
			field::core::signal::Warmup::run<Sample> (warmN, (int)chans, (int)frames,
				[this] (juce::dsp::AudioBlock<Sample>& blk) { stagingChain().process<Sample>(blk); });
        }

		// active -> io
		activeChain().process<Sample>(io);

        // staging -> tmp_
        {
            juce::dsp::AudioBlock<Sample> tmpBlock (tmp_.getArrayOfWritePointers(), chans, frames);
            for (size_t ch = 0; ch < chans; ++ch)
                tmp_.copyFrom ((int)ch, 0, io.getChannelPointer(ch), (int)frames);
			stagingChain().process<Sample>(tmpBlock);
        }

        int offset = 0;
        if (!ramp_.active())
        {
            offset = std::min<int> ((int)frames, pendingOffset_.exchange (0, std::memory_order_acq_rel));
            if (wantSwap_.exchange (false, std::memory_order_acq_rel))
                ramp_.start();
        }

        for (size_t ch = 0; ch < chans; ++ch)
        {
            auto* dst = io.getChannelPointer(ch);
            auto* src = tmp_.getReadPointer((int)ch);
            for (size_t i = 0; i < (size_t)offset && i < frames; ++i) dst[i] = dst[i];
            for (size_t i = (size_t)offset; i < frames; ++i)
            {
                float w = ramp_.active() ? ramp_.next() : 0.0f;
                dst[i] = (Sample)((1.0f - w) * dst[i] + w * src[i]);
            }
        }

		if (!ramp_.active())
		{
			activeIndex_ ^= 1;
			tmp_.setSize(0, 0);
		}
    }

	void reset()
	{
		chains_[0].reset();
		chains_[1].reset();
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

	FieldChain chains_[2]{};
	int		activeIndex_ { 0 };
	Config     pendingCfg_{};

    std::atomic<bool> wantSwap_{false};
    std::atomic<int>  pendingOffset_{0};
    std::atomic<int>  warmupBlocks_{0};
	field::core::signal::CrossfadeRamp ramp_{};
	juce::AudioBuffer<float> tmp_;
};

}} // namespace field::modules
