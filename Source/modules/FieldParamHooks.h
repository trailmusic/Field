#pragma once
#include "FieldChain.h"
#include "core/params/Snapshot.h"

namespace field { namespace modules {

static inline int msToSamples (float ms, double sr) noexcept
{
	if (ms <= 0.f || sr <= 0.0) return 0;
	const double s = (double)ms / 1000.0;
	return (int) std::lround (s * sr);
}

static inline void applyLatencyFromSnapshot (FieldChain& chain,
                                             const field::params::ChainParamSnapshot& snap,
                                             double sampleRate)
{
	const int osGroupDelay = (snap.osFactor > 1) ? 0 : 0;
	// NOTE: Adapt these calls to your node accessors as you implement real engines.
	// For now, we keep placeholders inactive; recomputeLatency will still sum zeros.
	(void) sampleRate; (void) osGroupDelay; (void) chain; (void) snap;
	chain.recomputeLatency();
}

}} // namespace field::modules
