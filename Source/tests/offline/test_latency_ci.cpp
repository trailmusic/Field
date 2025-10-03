#include "../../core/telemetry/LatencyProbe.h"
#include "../../modules/FieldChain.h"
#include <juce_dsp/juce_dsp.h>
#include <cassert>
#include <cmath>

using field::modules::FieldChain;

template<typename Sample>
static int probeOnce (FieldChain& chain, double sr, int maxBlock, int chans)
{
	chain.prepare(sr, maxBlock, chans);
	const int N = std::max(4096, maxBlock * 8);
	juce::AudioBuffer<Sample> inBuf (chans, N), outBuf (chans, N);
	inBuf.clear(); outBuf.clear();
	inBuf.setSample(0, 0, (Sample)1);
	for (int i = 0; i < N; i += maxBlock)
	{
		const auto n = std::min(maxBlock, N - i);
		juce::dsp::AudioBlock<Sample> inBlock (inBuf.getArrayOfWritePointers(), (size_t)chans, (size_t)n);
		juce::dsp::AudioBlock<Sample> outBlock(outBuf.getArrayOfWritePointers(), (size_t)chans, (size_t)n);
		inBlock = inBlock.getSubBlock((size_t)i, (size_t)n);
		outBlock= outBlock.getSubBlock((size_t)i, (size_t)n);
		// in-place for now
		chain.process(inBlock);
		outBlock.copyFrom(inBlock);
	}
	int peak = 0; Sample maxv = (Sample)0;
	const auto* p = outBuf.getReadPointer(0);
	for (int i = 0; i < N; ++i) { const auto a = std::abs(p[i]); if (a > maxv) { maxv = a; peak = i; } }
	return peak;
}

int main()
{
	FieldChain chain;
	FieldChain::Config cfg{};
	chain.setConfig(cfg);
	chain.buildFromConfig();
	for (double sr : { 44100.0, 48000.0, 96000.0 })
	for (int bs : { 32, 64, 128, 256, 512, 1024 })
	{
		const int measuredF = probeOnce<float> (chain, sr, bs, 2);
		const int measuredD = probeOnce<double>(chain, sr, bs, 2);
		const int reported  = chain.latencySamples();
		assert(std::abs(measuredF - reported) <= 1);
		assert(std::abs(measuredD - reported) <= 1);
	}
	return 0;
}
