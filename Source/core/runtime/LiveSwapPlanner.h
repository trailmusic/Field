#pragma once
#include "modules/FieldDualChain.h"
#include "modules/FieldParamHooks.h"
#include "core/params/Snapshot.h"

namespace field { namespace core { namespace runtime {

struct LiveSwapPlanner
{
	struct Result { bool armed{false}; bool sameLatency{false}; };

	template <typename DualChainLike>
	static Result armIfSameLatency (DualChainLike& dual,
									 const field::params::ChainParamSnapshot& snap,
									 double sampleRate,
									 int maxBlock,
									 int channels,
									 int warmupBlocks = 2,
									 int rampSamples  = 64)
	{
		// Start from active topology
		auto cfg = dual.activeChain().getConfig();
		cfg.needsRebuild = true;
		dual.stagingChain().setConfig (cfg);
		dual.stagingChain().buildFromConfig();
		dual.stagingChain().prepare (sampleRate, maxBlock, channels);

		field::modules::applyLatencyFromSnapshot (dual.stagingChain(), snap, sampleRate);

		const int Lactive  = dual.activeChain().latencySamples();
		const int Lstaging = dual.stagingChain().latencySamples();
		if (Lactive != Lstaging) return {};

		dual.warmupStaging (warmupBlocks);
		dual.armLiveSwapAtSameLatency (0, 0, rampSamples);
		return { true, true };
	}
};

}}} // namespace field::core::runtime
