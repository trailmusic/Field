#pragma once
#include <juce_dsp/juce_dsp.h>
#include "NodeLatency.h"

namespace field { namespace modules { namespace nodes {
struct Node_DynEq : NodeLatencyMixin<Node_DynEq>
{
    struct DynEqParams {
        bool enabled{false}; int lookaheadSamples{0}; uint8_t globalMode{0}; uint8_t link{0};
        struct Band { uint8_t enabled{0}, type{0}, direction{0}, sidechain{0};
            float freqHz{}, q{}, staticGainLin{}, rangeDb{}, ratio{}, threshDbfs{}, kneeDb{};
            float atkSec{}, relSec{}, holdSec{}, makeupLin{}, scHP_Hz{}, scLP_Hz{}, wet01{}; } band[24];
    } params_{};

    void setParameters (const DynEqParams& p) noexcept { params_ = p; }

	template <typename Sample>
	void prepare (double /*sr*/, int /*maxBlock*/, int /*chans*/) noexcept {}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& /*io*/) const noexcept {}

	void reset() noexcept {}
};
}}} // namespace field::modules::nodes
