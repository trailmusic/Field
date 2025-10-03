#include "../../modules/FieldChain.h"
#include "../../modules/FieldParamHooks.h"
#include <cassert>

using field::modules::FieldChain;

int main()
{
	FieldChain chain;
	FieldChain::Config cfg{};
	cfg.enableDelay = cfg.enableDynEq = cfg.enableReverb = true;
	chain.setConfig(cfg);
	chain.buildFromConfig();
	chain.prepare(48000.0, 512, 2);

	field::params::ChainParamSnapshot snap{};
	snap.osFactor = 1;
	snap.reverbLinearPhase = true;
	snap.reverbFIRHalfLen  = 256;
	snap.dynEqLookAheadMs  = 2.0f;
	snap.delayLookAheadMs  = 0.5f;

	field::modules::applyLatencyFromSnapshot(chain, snap, 48000.0);
	// Placeholders currently sum to 0 latency
	assert(chain.latencySamples() == 0);
	return 0;
}
