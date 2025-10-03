#include "FieldChain.h"

namespace field { namespace modules {

void FieldChain::buildFromConfig() noexcept
{
	active_.meter  = cfg_.enableMeter;
	active_.ms     = cfg_.enableMS;
	active_.gain   = cfg_.enableGain;

	active_.delay  = cfg_.enableDelay;
	active_.dyneq  = cfg_.enableDynEq;
	active_.reverb = cfg_.enableReverb;

	// All nodes are unity by default; when engines arrive, keep defaults unity.
}

}} // namespace field::modules
