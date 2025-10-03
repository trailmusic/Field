#pragma once
#include <juce_dsp/juce_dsp.h>

// Scrub NaN/Inf and subnormals; keep UI safe during broken-signal refactor

template <typename Sample>
inline void sanitizeSampleBuffer (Sample* data, int count) noexcept
{
	if (data == nullptr || count <= 0) return;
	for (int i = 0; i < count; ++i)
	{
		Sample v = data[i];
		if (! std::isfinite (v)) v = (Sample) 0;
#if defined(__FAST_MATH__) || defined(JUCE_DISABLE_DENORMALS)
		// DAZ/FTZ likely active, still zero tiny magnitudes defensively
#endif
		if (std::abs (v) < (Sample) 1.0e-30) v = (Sample) 0;
		data[i] = v;
	}
}

template <typename Sample>
inline void sanitizeAudioBlock (juce::dsp::AudioBlock<Sample>& block) noexcept
{
	const int C = (int) block.getNumChannels();
	const int N = (int) block.getNumSamples();
	for (int c = 0; c < C; ++c)
		sanitizeSampleBuffer (block.getChannelPointer ((size_t) c), N);
}
