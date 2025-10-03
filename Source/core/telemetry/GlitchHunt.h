#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <algorithm>

// 1) Sanitize non-finite (NaN/Inf) and hard-limit
template <typename Sample>
inline void sanitizeAudioBlock(juce::dsp::AudioBlock<Sample> b, Sample limit = (Sample)1.2)
{
	const Sample lim = limit;
	for (size_t c=0; c<b.getNumChannels(); ++c)
	{
		Sample* p = b.getChannelPointer(c);
		for (size_t i=0; i<b.getNumSamples(); ++i)
		{
			Sample x = p[i];
			if (!std::isfinite((double)x)) x = (Sample)0;
			if (x >  lim) x = lim;
			if (x < -lim) x = -lim;
			p[i] = x;
		}
	}
}

// 2) Tiny soft fade-in to absorb insert/start edges
struct MiniFadeIn {
	int remaining = 0;
	void arm (int n) { remaining = juce::jmax(0, n); }
	template <typename Sample>
	void apply(juce::dsp::AudioBlock<Sample> b)
	{
		if (remaining <= 0) return;
		const int N = (int)b.getNumSamples();
		int nThis = juce::jmin(remaining, N);
		for (size_t c=0; c<b.getNumChannels(); ++c)
		{
			auto* p = b.getChannelPointer(c);
			for (int i=0; i<nThis; ++i)
			{
				const float g = (float)(i+1) / (float)juce::jmax(1, remaining);
				p[i] *= (Sample)g;
			}
		}
		remaining -= nThis;
	}
};

// 3) Quick RMS helper (debug-only checks)
template <typename Sample>
inline double rmsOf(juce::dsp::AudioBlock<Sample> b)
{
	long double s = 0.0;
	size_t n = b.getNumSamples()*b.getNumChannels();
	for (size_t c=0; c<b.getNumChannels(); ++c)
		for (size_t i=0; i<b.getNumSamples(); ++i)
			s += (long double)b.getChannelPointer(c)[i] * (long double)b.getChannelPointer(c)[i];
	return std::sqrt((double)(s / (long double)std::max<size_t>(1, n)));
}

// 4) Count-and-sanitize helper (returns number of corrections applied)
template <typename Sample>
inline int sanitizeAndCount(juce::dsp::AudioBlock<Sample> b, Sample limit=(Sample)1.25)
{
	int hits = 0;
	for (size_t c=0;c<b.getNumChannels();++c)
	{
		Sample* p = b.getChannelPointer(c);
		for (size_t i=0;i<b.getNumSamples();++i)
		{
			Sample x = p[i];
			const bool bad = !std::isfinite((double)x);
			if (bad) { x = (Sample)0; ++hits; }
			if (x >  limit) { x =  limit; ++hits; }
			if (x < -limit) { x = -limit; ++hits; }
			p[i] = x;
		}
	}
	return hits;
}
