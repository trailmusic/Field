#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <variant>
#include <cstdint>

#include "Mixing/Node_Gain.h"
#include "Mixing/Node_MSMatrix.h"
#include "Mixing/Node_Meter.h"

// Optional future nodes (currently unity stubs; keep includes commented until engines move)
// #include "FieldNodes/Node_Reverb.h"
// #include "FieldNodes/Node_Delay.h"
// #include "FieldNodes/Node_DynEq.h"
// #include "FieldNodes/Node_Phase.h"
// #include "FieldNodes/Node_Imager.h"

namespace field { namespace modules {

/**
 * FieldChain
 * - Small, fixed sequence that can enable/disable stages via Config.
 * - No heap allocations; reset() returns to unity.
 * - Latency: 0 in this configuration (update when linear-phase/OS enters).
 */
struct FieldChain
{
	// ---- Public config ------------------------------------------------------
	struct Config
	{
		bool enableMeter = false;
		bool enableMS    = false;
		bool enableGain  = false;

		// reserved for future (keep stable ABI)
		uint32_t reserved = 0;
	};

	// ---- Lifecycle ----------------------------------------------------------
	FieldChain() = default;

	void setConfig (const Config& c) noexcept { cfg_ = c; }
	const Config& getConfig() const noexcept   { return cfg_; }

	// Rebuild internal stage activation based on cfg (no allocations).
	void buildFromConfig() noexcept;

	template <typename Sample>
	void prepare (double sampleRate, int maxBlock, int channels) noexcept
	{
		sr_ = sampleRate;
		maxBlock_ = (maxBlock > 0 ? maxBlock : 512);
		chans_ = (channels > 0 ? channels : 2);

		// Prepare all nodes; cost is tiny and we may toggle activity via flags.
		meter_.template prepare<Sample> (sr_, maxBlock_, chans_);
		ms_.template prepare<Sample>    (sr_, maxBlock_, chans_);
		gain_.template prepare<Sample>  (sr_, maxBlock_, chans_);
	}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& io) const noexcept
	{
		// Order (when enabled): Meter → MS → Gain
		if (active_.meter) meter_.template process<Sample>(io);
		if (active_.ms)    ms_.template process<Sample>(io);
		if (active_.gain)  gain_.template process<Sample>(io);
	}

	void reset() noexcept
	{
		meter_.reset();
		ms_.reset();
		gain_.reset();
	}

	int latencySamples() const noexcept { return 0; } // update if stages add delay

	// Back-compat for early tests: unity build alias
	void buildUnity() noexcept { cfg_ = {}; buildFromConfig(); }

	// ---- Stage accessors (optional; useful for wiring UI later) -------------
	auto& gainNode()  noexcept { return gain_;  }
	auto& msNode()    noexcept { return ms_;    }
	auto& meterNode() noexcept { return meter_; }

private:
	// ---- Concrete stages ----------------------------------------------------
	mixing::Node_Meter<>  meter_{};
	mixing::Node_MSMatrix ms_{};
	mixing::Node_Gain     gain_{};

	// ---- Runtime state ------------------------------------------------------
	double sr_ = 48000.0;
	int    maxBlock_ = 512;
	int    chans_ = 2;

	Config cfg_{};

	struct Active
	{
		bool meter = false;
		bool ms    = false;
		bool gain  = false;
	} active_{};
};

}} // namespace field::modules
