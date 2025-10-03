#include "../../core/telemetry/StateSanity.h"
#include <juce_dsp/juce_dsp.h>
#include <cassert>
#include <limits>

int main()
{
	juce::AudioBuffer<float> buf(2, 128);
	buf.clear();
	buf.getWritePointer(0)[37] = std::numeric_limits<float>::quiet_NaN();
	juce::dsp::AudioBlock<float> b(buf);
	auto bad = field::core::telemetry::scanBlock(b);
	assert(!bad.ok() && bad.channel == 0 && bad.index == 37);
	return 0;
}
