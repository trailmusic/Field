#pragma once
#include <vector>
#include <juce_dsp/juce_dsp.h>

// Offline probe: measures internal latency (samples) by feeding an impulse
// through the graph using a provided processing callback.
struct LatencyProbe
{
	template <typename Sample, typename ProcessFn>
	static int measure (int maxBlock, ProcessFn&& processOnce)
	{
		const int N = juce::jmax (maxBlock * 8, 4096);
		std::vector<Sample> in (N, (Sample)0), out (N, (Sample)0);
		in[0] = (Sample) 1;

		for (int i = 0; i < N; i += maxBlock)
		{
			const int n = juce::jmin (maxBlock, N - i);
			Sample* inChan  = &in[i];
			Sample* outChan = &out[i];
			Sample* inPtrs[1]  = { inChan };
			Sample* outPtrs[1] = { outChan };
			juce::dsp::AudioBlock<Sample> bIn  (inPtrs,  1, (size_t) n);
			juce::dsp::AudioBlock<Sample> bOut (outPtrs, 1, (size_t) n);
			processOnce (bIn, bOut);
		}

		int peak = 0; Sample maxv = (Sample) 0;
		for (int i = 0; i < N; ++i)
		{
			const Sample a = (Sample) std::abs (out[i]);
			if (a > maxv) { maxv = a; peak = i; }
		}
		return peak;
	}
};
