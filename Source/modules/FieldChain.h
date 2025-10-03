#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <variant>
#include <cstdint>

#include "Mixing/Node_Gain.h"
#include "Mixing/Node_MSMatrix.h"
#include "Mixing/Node_Meter.h"

#include "FieldNodes/Node_Reverb.h"
#include "FieldNodes/Node_Delay.h"
#include "FieldNodes/Node_DynEq.h"

namespace field { namespace modules {

/**
 * FieldChain
 * - Config-driven set of optional stages.
 * - No heap allocations; latency=0 with all placeholders/stages as shipped here.
 * - Rebuild only via buildFromConfig(); never rebuild mid-block.
 */
struct FieldChain
{
	struct Config
	{
		bool enableMeter = false;
		bool enableMS    = false;
		bool enableGain  = false;

		bool enableDelay = false;
		bool enableDynEq = false;
		bool enableReverb= false;

		uint32_t reserved = 0;
	};

	FieldChain() = default;

	void setConfig (const Config& c) noexcept { cfg_ = c; }
	const Config& getConfig() const noexcept   { return cfg_; }

	void buildFromConfig() noexcept;

	template <typename Sample>
	void prepare (double sampleRate, int maxBlock, int channels) noexcept
	{
		sr_ = sampleRate;
		maxBlock_ = (maxBlock > 0 ? maxBlock : 512);
		chans_ = (channels > 0 ? channels : 2);

		meter_.template  prepare<Sample> (sr_, maxBlock_, chans_);
		ms_.template     prepare<Sample> (sr_, maxBlock_, chans_);
		gain_.template   prepare<Sample> (sr_, maxBlock_, chans_);

		delay_.template  prepare<Sample> (sr_, maxBlock_, chans_);
		dyneq_.template  prepare<Sample> (sr_, maxBlock_, chans_);
		reverb_.template prepare<Sample> (sr_, maxBlock_, chans_);
	}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& io) const noexcept
	{
		if (active_.meter)  meter_.template  process<Sample>(io);
		if (active_.ms)     ms_.template     process<Sample>(io);
		if (active_.gain)   gain_.template   process<Sample>(io);

		if (active_.delay)  delay_.template  process<Sample>(io);
		if (active_.dyneq)  dyneq_.template  process<Sample>(io);
		if (active_.reverb) reverb_.template process<Sample>(io);
	}

	void reset() noexcept
	{
		meter_.reset();  ms_.reset();  gain_.reset();
		delay_.reset();  dyneq_.reset(); reverb_.reset();
	}

	int latencySamples() const noexcept { return 0; }

	void buildUnity() noexcept { cfg_ = {}; buildFromConfig(); }

	auto& meterNode()  noexcept { return meter_; }
	auto& msNode()     noexcept { return ms_; }
	auto& gainNode()   noexcept { return gain_; }
	auto& delayNode()  noexcept { return delay_; }
	auto& dynEqNode()  noexcept { return dyneq_; }
	auto& reverbNode() noexcept { return reverb_; }

private:
	mixing::Node_Meter<>  meter_{};
	mixing::Node_MSMatrix ms_{};
	mixing::Node_Gain     gain_{};

	nodes::Node_Delay     delay_{};
	nodes::Node_DynEq     dyneq_{};
	nodes::Node_Reverb    reverb_{};

	double sr_ = 48000.0;
	int    maxBlock_ = 512;
	int    chans_ = 2;

	Config cfg_{};

	struct Active
	{
		bool meter  = false;
		bool ms     = false;
		bool gain   = false;
		bool delay  = false;
		bool dyneq  = false;
		bool reverb = false;
	} active_{};
};

}} // namespace field::modules
