#include "../../modules/FieldDualChain.h"
#include "../../core/runtime/LiveSwapPlanner.h"
#include "../../core/params/Snapshot.h"
#include <juce_dsp/juce_dsp.h>
#include <cassert>

int main()
{
	using field::modules::FieldDualChain;
	using field::modules::FieldChain;
	using field::core::runtime::LiveSwapPlanner;

	FieldDualChain dual;
	FieldChain::Config cfg{};
	cfg.enableReverb = true;
	dual.activeChain().setConfig(cfg);
	dual.activeChain().buildFromConfig();
	dual.activeChain().prepare(48000.0, 512, 2);
	assert(dual.latencySamples() == 0);

	field::params::ChainParamSnapshot snap{};
	snap.enableReverb = true;
	auto res = LiveSwapPlanner::armIfSameLatency(dual, snap, 48000.0, 512, 2, 1, 32);
	assert(res.sameLatency && res.armed);

	juce::AudioBuffer<float> buf(2, 512);
	buf.clear();
	juce::dsp::AudioBlock<float> b{buf};
	dual.process(b);
	return 0;
}
