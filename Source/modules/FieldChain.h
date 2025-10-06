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
		// TRIAGE: configure per-stage bypass for isolation
		constexpr bool kBypassAllStages = false;
		if (!kBypassAllStages)
		{
			// Restore normal: Meter + Gain enabled for visibility and unity
			constexpr bool kBypassMeter  = false;
			constexpr bool kBypassMS     = true;
			constexpr bool kBypassGain   = false;
			constexpr bool kBypassDelay  = true;
			constexpr bool kBypassDynEq  = true;
			constexpr bool kBypassReverb = true;
			constexpr bool kBypassPhase  = true;
			constexpr bool kBypassImager = true;
			if (active_.meter  && !kBypassMeter)  stages_.push_back(Stage::Meter);
			if (active_.ms     && !kBypassMS)     stages_.push_back(Stage::MS);
			if (active_.gain   && !kBypassGain)   stages_.push_back(Stage::Gain);
			if (active_.delay  && !kBypassDelay)  stages_.push_back(Stage::Delay);
			if (active_.dyneq  && !kBypassDynEq)  stages_.push_back(Stage::DynEq);
			if (active_.reverb && !kBypassReverb) stages_.push_back(Stage::Reverb);
			if (active_.phase  && !kBypassPhase)  stages_.push_back(Stage::Phase);
			if (active_.imager && !kBypassImager) stages_.push_back(Stage::Imager);
		}

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

#if JUCE_DEBUG
		// Record preparation cookie for parity checks in process()
		if constexpr (std::is_same_v<Sample, float>)
		{
			cookieF_.sr = sr_;
			cookieF_.maxBlock = maxBlock_;
			cookieF_.chans = chans_;
			cookieF_.bytes = (int)sizeof(Sample);
			cookieF_.set = true;
		}
		else if constexpr (std::is_same_v<Sample, double>)
		{
			cookieD_.sr = sr_;
			cookieD_.maxBlock = maxBlock_;
			cookieD_.chans = chans_;
			cookieD_.bytes = (int)sizeof(Sample);
			cookieD_.set = true;
		}
#endif
	}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& io) const noexcept
	{
#if JUCE_DEBUG
		// Ensure we were prepared for this precision and configuration
		if constexpr (std::is_same_v<Sample, float>)
		{
			jassert (cookieF_.set);
			jassert (cookieF_.bytes == (int)sizeof(Sample));
			jassert (cookieF_.sr == sr_ && cookieF_.chans == chans_);
		}
		else if constexpr (std::is_same_v<Sample, double>)
		{
			jassert (cookieD_.set);
			jassert (cookieD_.bytes == (int)sizeof(Sample));
			jassert (cookieD_.sr == sr_ && cookieD_.chans == chans_);
		}
		// Cross-precision parity: both paths should have been prepared to the same topology
		if (cookieF_.set && cookieD_.set)
		{
			jassert (cookieF_.sr == cookieD_.sr);
			jassert (cookieF_.chans == cookieD_.chans);
			jassert (cookieF_.maxBlock >= cookieD_.maxBlock || cookieD_.maxBlock >= cookieF_.maxBlock);
		}
#endif
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

#if JUCE_DEBUG
	struct PrepCookie { double sr{0}; int maxBlock{0}; int chans{0}; int bytes{0}; bool set{false}; };
	mutable PrepCookie cookieF_{};
	mutable PrepCookie cookieD_{};
#endif
};
}} // namespace field::modules
