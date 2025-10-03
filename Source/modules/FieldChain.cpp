#include "FieldChain.h"

namespace field { namespace modules {

void FieldChain::buildFromConfig() noexcept
{
	// snapshot cfg_ into active_ so process() is branch-light
	active_.meter = cfg_.enableMeter;
	active_.ms    = cfg_.enableMS;
	active_.gain  = cfg_.enableGain;
	// Defaults keep unity:
	// - Node_Meter only taps; no gain change
	// - Node_MSMatrix is Bypass by default
	// - Node_Gain is 1.0 by default
}

}} // namespace field::modules
