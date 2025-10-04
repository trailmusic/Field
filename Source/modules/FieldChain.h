#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cstdint>
#include <type_traits>
#include <cstring>
#include <vector>

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
	enum class Stage : uint8_t { Meter, MS, Gain, Delay, DynEq, Reverb, Phase, Imager };

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

		// Build enabled stage order (enabled-only execution)
		stages_.clear();
		if (active_.meter)  stages_.push_back(Stage::Meter);
		if (active_.ms)     stages_.push_back(Stage::MS);
		if (active_.gain)   stages_.push_back(Stage::Gain);
		if (active_.delay)  stages_.push_back(Stage::Delay);
		if (active_.dyneq)  stages_.push_back(Stage::DynEq);
		if (active_.reverb) stages_.push_back(Stage::Reverb);
		if (active_.phase)  stages_.push_back(Stage::Phase);
		if (active_.imager) stages_.push_back(Stage::Imager);

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
		prepared_ = true;
	}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& io) const noexcept
	{
		for (auto st : stages_)
		{
			switch (st)
			{
				case Stage::Meter:  meter_.template  process<Sample>(io); break;
				case Stage::MS:     ms_.template     process<Sample>(io); break;
				case Stage::Gain:   gain_.template   process<Sample>(io); break;
				case Stage::Delay:  delay_.template  process<Sample>(io); break;
				case Stage::DynEq:  dyneq_.template  process<Sample>(io); break;
				case Stage::Reverb: reverb_.template process<Sample>(io); break;
				case Stage::Phase:  phase_.template  process<Sample>(io); break;
				case Stage::Imager: imager_.template process<Sample>(io); break;
			}
		}
	}

	void reset() noexcept
	{
		meter_.reset();  ms_.reset();  gain_.reset();
		delay_.reset();  dyneq_.reset(); reverb_.reset();
		phase_.reset();  imager_.reset();
		prepared_ = false;
	}

	bool isPrepared() const noexcept { return prepared_; }

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
	bool prepared_ = false;

	// Enabled-only stage execution order
	std::vector<Stage> stages_{};
};
}} // namespace field::modules
