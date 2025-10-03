#pragma once
#include <juce_dsp/juce_dsp.h>

namespace field { namespace core { namespace signal {

struct Warmup
{
	template <typename Sample, typename ProcessFn>
	static void run (int numBlocks, int numChans, int blockSize, ProcessFn&& fn)
	{
		if (numBlocks <= 0 || numChans <= 0 || blockSize <= 0) return;
		juce::AudioBuffer<Sample> buf (numChans, blockSize);
		buf.clear();
		juce::dsp::AudioBlock<Sample> block (buf);
		for (int i = 0; i < numBlocks; ++i)
			fn (block);
	}
};

}}} // namespace field::core::signal
