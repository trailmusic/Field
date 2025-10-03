#pragma once
#include "modules/FieldChain.h"

namespace field { namespace processor {

struct LatencyTailCompute
{
	static int computeLatencySamples (const field::modules::FieldChain& chain) noexcept
	{
		return chain.latencySamples();
	}
	static double computeTailSeconds (const field::modules::FieldChain::Config& cfg) noexcept
	{
		(void) cfg;
		return 0.0;
	}
};

}} // namespace field::processor
