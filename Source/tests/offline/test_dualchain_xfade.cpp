#include <juce_dsp/juce_dsp.h>
#include "../../modules/FieldDualChain.h"
#include <cassert>
#include <cmath>

int main()
{
	using namespace field::modules;
	DualChain dc;
	dc.setCrossfadeLength(64);

	FieldChain::Config cfg{};
	dc.setConfig(cfg);
	(void)dc.buildStaging(48000.0, 256, 2);
	dc.promoteStagingHard();

	cfg.enableGain = true;
	dc.setConfig(cfg);
	(void)dc.buildStaging(48000.0, 256, 2);
	bool armed = dc.armLiveSwapIfSameLatency();
	assert(armed);

	juce::AudioBuffer<float> buf(2, 256);
	buf.clear();
	buf.setSample(0, 0, 1.0f);
	juce::dsp::AudioBlock<float> block(buf);
	dc.process<float>(block);

	for (int i = 0; i < buf.getNumSamples(); ++i)
		assert(std::isfinite(buf.getReadPointer(0)[i]));
	return 0;
}
