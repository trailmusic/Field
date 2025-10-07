#pragma once
#include <juce_dsp/juce_dsp.h>
#include "NodeLatency.h"

namespace field { namespace modules { namespace nodes {
struct Node_Reverb : NodeLatencyMixin<Node_Reverb>
{
    struct Params { bool enabled{false}; float preDelaySec{0.02f}; float sizeNorm{0.62f}; float dampingHz{6000.f}; };
    void setParameters (const Params& p) noexcept { params_ = p; }
	template <typename Sample>
	void prepare (double /*sr*/, int /*maxBlock*/, int /*chans*/) noexcept {}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& /*io*/) const noexcept {}

	void reset() noexcept {}
private:
    Params params_{};
};
}}} // namespace field::modules::nodes
