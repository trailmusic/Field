#include <cassert>
#include <iostream>
#include "../../core/telemetry/LatencyProbe.h"
#include "../../modules/FieldChain.h"

using namespace field::modules;

int main()
{
	FieldChain chain; chain.buildUnity();
	chain.prepare (48000.0, 512, 2);

	const int maxBlock = 512;

	auto runOnceFloat = [&] (juce::dsp::AudioBlock<float>& in, juce::dsp::AudioBlock<float>& out)
	{
		juce::AudioBuffer<float> tmp (2, (int) in.getNumSamples());
		juce::dsp::AudioBlock<float> tmpBlock (tmp);
		tmpBlock.getSingleChannelBlock(0).copyFrom (in);
		tmpBlock.getSingleChannelBlock(1).clear();
		chain.process<float> (tmpBlock);
		out.copyFrom (tmpBlock.getSingleChannelBlock(0));
	};

	const int measured = LatencyProbe::measure<float> (maxBlock, runOnceFloat);
	std::cout << "measured=" << measured << " desired=" << chain.latencySamples() << "\n";
	assert (measured == chain.latencySamples());
	return 0;
}
