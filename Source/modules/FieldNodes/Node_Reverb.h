#pragma once
#include <juce_dsp/juce_dsp.h>
#include "NodeLatency.h"

namespace field { namespace modules { namespace nodes {
struct Node_Reverb : NodeLatencyMixin<Node_Reverb>
{
	template <typename Sample>
	void prepare (double /*sr*/, int /*maxBlock*/, int /*chans*/) noexcept {}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& /*io*/) const noexcept {}

	void reset() noexcept {}
};
}}} // namespace field::modules::nodes
