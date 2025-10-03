#pragma once
#include <juce_dsp/juce_dsp.h>

namespace field { namespace modules { namespace nodes {
struct Node_Delay
{
	template <typename Sample>
	void prepare (double /*sr*/, int /*maxBlock*/, int /*chans*/) noexcept {}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& /*io*/) const noexcept {}

	void reset() noexcept {}
	int latencySamples() const noexcept { return 0; }
};
}}} // namespace field::modules::nodes
