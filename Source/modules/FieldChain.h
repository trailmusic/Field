#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cstdint>
#include <type_traits>
#include <cstring>

#include "Mixing/Node_Gain.h"
#include "Mixing/Node_MSMatrix.h"
#include "Mixing/Node_Meter.h"

#include "FieldNodes/Node_Reverb.h"
#include "FieldNodes/Node_Delay.h"
#include "FieldNodes/Node_DynEq.h"
#include "FieldNodes/Node_Phase.h"
#include "FieldNodes/Node_Imager.h"

namespace field { namespace modules {
struct FieldChain
{
	struct Config
	{
		bool enableMeter  = false;
		bool enableMS     = false;
		bool enableGain   = false;

		bool enableDelay  = false;
		bool enableDynEq  = false;
		bool enableReverb = false;
		bool enablePhase  = false;
		bool enableImager = false;

		bool needsRebuild = false;
		uint32_t reserved = 0;
	};

	FieldChain() = default;

	bool setConfig (const Config& c) noexcept
	{
		if (std::memcmp(&cfg_, &c, sizeof(Config)) != 0)
		{
			cfg_ = c;
			dirty_ = true;
			return true;
		}
		return false;
	}

	const Config& getConfig() const noexcept { return cfg_; }

	void buildFromConfig() noexcept
	{
		active_.meter   = cfg_.enableMeter;
		active_.ms      = cfg_.enableMS;
		active_.gain    = cfg_.enableGain;
		active_.delay   = cfg_.enableDelay;
		active_.dyneq   = cfg_.enableDynEq;
		active_.reverb  = cfg_.enableReverb;
		active_.phase   = cfg_.enablePhase;
		active_.imager  = cfg_.enableImager;

		recomputeLatency();
		dirty_ = false;
	}

	void recomputeLatency() noexcept
	{
		int sum = 0;
		sum += delay_.latencySamples();
		sum += dyneq_.latencySamples();
		sum += reverb_.latencySamples();
		sum += phase_.latencySamples();
		sum += imager_.latencySamples();
		latencySum_ = sum;
	}

	template <typename Sample>
	void prepare (double sampleRate, int maxBlock, int channels) noexcept
	{
		if (cfg_.needsRebuild || dirty_)
		{
			buildFromConfig();
			cfg_.needsRebuild = false;
		}

		sr_ = sampleRate;
		maxBlock_ = (maxBlock > 0 ? maxBlock : 512);
		chans_ = (channels > 0 ? channels : 2);

		meter_.template  prepare<Sample> (sr_, maxBlock_, chans_);
		ms_.template     prepare<Sample> (sr_, maxBlock_, chans_);
		gain_.template   prepare<Sample> (sr_, maxBlock_, chans_);
		delay_.template  prepare<Sample> (sr_, maxBlock_, chans_);
		dyneq_.template  prepare<Sample> (sr_, maxBlock_, chans_);
		reverb_.template prepare<Sample> (sr_, maxBlock_, chans_);
		phase_.template  prepare<Sample> (sr_, maxBlock_, chans_);
		imager_.template prepare<Sample> (sr_, maxBlock_, chans_);
	}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& io) const noexcept
	{
		if (active_.meter)   meter_.template  process<Sample>(io);
		if (active_.ms)      ms_.template     process<Sample>(io);
		if (active_.gain)    gain_.template   process<Sample>(io);
		if (active_.delay)   delay_.template  process<Sample>(io);
		if (active_.dyneq)   dyneq_.template  process<Sample>(io);
		if (active_.reverb)  reverb_.template process<Sample>(io);
		if (active_.phase)   phase_.template  process<Sample>(io);
		if (active_.imager)  imager_.template process<Sample>(io);
	}

	void reset() noexcept
	{
		meter_.reset();  ms_.reset();  gain_.reset();
		delay_.reset();  dyneq_.reset(); reverb_.reset();
		phase_.reset();  imager_.reset();
	}

	int latencySamples() const noexcept { return latencySum_; }

private:
    mixing::Node_Meter<>  meter_{};
    mixing::Node_MSMatrix ms_{};
    mixing::Node_Gain     gain_{};
	nodes::Node_Delay     delay_{};
	nodes::Node_DynEq     dyneq_{};
	nodes::Node_Reverb    reverb_{};
	nodes::Node_Phase     phase_{};
	nodes::Node_Imager    imager_{};

	double sr_ = 48000.0;
	int    maxBlock_ = 512;
	int    chans_ = 2;

	Config cfg_{};
	bool   dirty_ = true;

	struct Active
	{
		bool meter=false, ms=false, gain=false;
		bool delay=false, dyneq=false, reverb=false, phase=false, imager=false;
	} active_{};

	int latencySum_ = 0;
};
}} // namespace field::modules
