#pragma once
#include <juce_dsp/juce_dsp.h>
namespace field { namespace modules {
struct Node_Gain_Stub {
	void prepare(double,int,int) {}
	template<typename Sample> void process(juce::dsp::AudioBlock<Sample>&) {}
	int latencySamples() const noexcept { return 0; }
};
}} // namespace field::modules
