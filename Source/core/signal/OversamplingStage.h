#pragma once
#include <juce_dsp/juce_dsp.h>

// Minimal frame-based oversampling wrapper.
// Prepare once; processFrame expects exact frame size.

template <typename Sample>
class OversamplingStage
{
public:
	using OS = juce::dsp::Oversampling<Sample>;

	void setEnabled (bool shouldEnable) { enabled = shouldEnable; }
	bool isEnabled() const              { return enabled; }

	// osPower: 0=bypass(1x), 1=2x, 2=4x, ...
	void prepare (double sampleRate,
				  int channels,
				  int osPower,
				  int engineFrame,
				  typename OS::FilterType filterType = OS::FilterType::filterHalfBandPolyphaseIIR)
	{
		sr    = sampleRate > 0.0 ? sampleRate : 48000.0;
		chans = juce::jmax (1, channels);
		power = juce::jmax (0, osPower);
		frame = juce::jmax (1, engineFrame);

		if (power > 0)
		{
			oversampling = std::make_unique<OS> ((size_t) chans, (size_t) power, filterType);
			oversampling->initProcessing ((size_t) frame);
		}
		else
		{
			oversampling.reset();
		}
	}

	void reset()
	{
		if (oversampling) oversampling->reset();
	}

	void processFrame (juce::dsp::AudioBlock<Sample> frameBlock)
	{
		jassert ((int) frameBlock.getNumChannels() == chans);
		jassert ((int) frameBlock.getNumSamples () == frame);

		if (!enabled || power == 0 || oversampling == nullptr)
			return; // 1x unity

		// Up-sample
		auto upBlock = oversampling->processSamplesUp (frameBlock);
		// High-rate processing would go here (currently unity)
		oversampling->processSamplesDown (frameBlock);
	}

private:
	bool   enabled = false;
	int    chans   = 0;
	int    power   = 0;
	int    frame   = 0;
	double sr      = 48000.0;

	std::unique_ptr<OS> oversampling;
};
