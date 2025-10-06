#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cstring>

// Fixed-size frame feeder for stages that require exact frame sizes
// (e.g., oversampling, FDN, FFT). No heap/locks/logging in audio thread.
// prepare() allocates once; pushAndConsume() copies host samples and
// calls processFixed() whenever a full frame is available.

template <typename Sample>
struct FrameAccumulator
{
	juce::HeapBlock<Sample> buf;
	juce::HeapBlock<Sample*> chPtr; // per-channel base pointers (no RT alloc)
	int frame = 0;   // desired fixed frame size
	int fill  = 0;   // current fill level [0..frame]
	int chans = 0;   // channel count

	void prepare (int channels, int frameSize)
	{
		jassert (channels > 0 && frameSize > 0);
		chans = channels;
		frame = frameSize;
		fill  = 0;
		buf.allocate ((size_t) chans * (size_t) frame, true);
		chPtr.allocate ((size_t) chans, true);
		for (int c = 0; c < chans; ++c)
			chPtr[(size_t) c] = buf.get() + (size_t) c * (size_t) frame;
	}

	void reset() noexcept { fill = 0; }

	template <typename Fn>
	int pushAndConsume (juce::dsp::AudioBlock<Sample> in, Fn processFixed)
	{
		const int N = (int) in.getNumSamples();
		if (N <= 0 || chans <= 0 || frame <= 0) return 0;

		int consumed = 0;
		while (consumed < N)
		{
			const int need  = frame - fill;
			const int nThis = juce::jmin (need, N - consumed);

			for (int c = 0; c < chans; ++c)
			{
				auto* dst = buf.get() + (size_t) c * (size_t) frame + (size_t) fill;
				auto* src = in.getChannelPointer ((size_t) c) + (size_t) consumed;
				std::memcpy (dst, src, sizeof (Sample) * (size_t) nThis);
			}

			fill     += nThis;
			consumed += nThis;

			if (fill == frame)
			{
				juce::dsp::AudioBlock<Sample> fixed (
					(Sample* const*) chPtr.get(), (size_t) chans, (size_t) frame);
				processFixed (fixed);
				fill = 0;
			}
		}
		#if JUCE_DEBUG
		jassert (fill >= 0 && fill < frame);
		#endif
		return consumed;
	}
};
