#include <cassert>
#include <cmath>
#include "../../modules/FieldChain.h"

using namespace field::modules;

int main()
{
	FieldChain chain; chain.buildUnity();
	chain.prepare (48000.0, 512, 2);

	juce::AudioBuffer<float> buf (2, 2048);
	buf.clear();
	buf.addSample (0, 0, 1.0f);
	juce::dsp::AudioBlock<float> block (buf);
	chain.process<float> (block);

	assert (std::abs (buf.getSample(0,0) - 1.0f) < 1e-6f);
	for (int i = 1; i < buf.getNumSamples(); ++i)
		assert (buf.getSample(0,i) == 0.0f && buf.getSample(1,i) == 0.0f);
	assert (chain.latencySamples() == 0);
	return 0;
}
