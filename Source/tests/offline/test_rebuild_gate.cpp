#include <juce_dsp/juce_dsp.h>
#include "../../modules/FieldChain.h"
#include <cassert>

int main()
{
	using namespace field::modules;

	FieldChain chain;
	FieldChain::Config cfg{};
	chain.setConfig(cfg);
	chain.buildFromConfig();
	chain.prepare<float>(48000.0, 256, 2);

	juce::AudioBuffer<float> buf(2, 512);
	buf.clear();
	buf.setSample(0, 0, 1.0f);
	juce::dsp::AudioBlock<float> block(buf);
	chain.process<float>(block);

	cfg.enableReverb = true;
	chain.setConfig(cfg); // marks dirty only
	assert(chain.latencySamples() == 0);

	chain.process<float>(block); // still unity without rebuild
	assert(buf.getSample(0, 0) == 1.0f);

	chain.buildFromConfig();
	chain.prepare<float>(48000.0, 256, 2);
	assert(chain.latencySamples() == 0);
	return 0;
}
