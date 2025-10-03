#pragma once
#include <juce_dsp/juce_dsp.h>

// Unity-through node to keep graphs connected during refactor
// Provides typed process() overloads with no allocations.

struct NullNode
{
	void prepare (double, int, int) {}
	void reset() {}

	void process (juce::dsp::AudioBlock<float> block) noexcept
	{
		// unity; nothing to do
		juce::ignoreUnused (block);
	}

	void process (juce::dsp::AudioBlock<double> block) noexcept
	{
		juce::ignoreUnused (block);
	}
};
