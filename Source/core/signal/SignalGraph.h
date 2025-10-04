#pragma once
#include <juce_dsp/juce_dsp.h>
#include "FrameAccumulator.h"
#include "OversamplingStage.h"
#include "../telemetry/DebugTelemetry.h"
#include "../telemetry/GlitchHunt.h"

struct SignalGraph
{
	// Debug controllable switches
	std::atomic<bool> useOversampling { false };
	std::atomic<bool> forceOSOff      { true  };

	int engineFrame = 256; // fixed frame for picky stages

	// Glitch-hunt helpers
	MiniFadeIn  insertFadeF, insertFadeD;
	bool        firstProcessF = true, firstProcessD = true;

	void prepare (double sr, int maxBlock, int chans)
	{
		sampleRate  = (sr > 0.0 ? sr : 48000.0);
		preparedMax = juce::jlimit (1, 1<<18, maxBlock);
		numChans    = juce::jlimit (1, 64, chans);
		temp.setSize (numChans, preparedMax, false, false, true);

		// Prepare accumulators and stages (float/double variants)
		accF.prepare (numChans, engineFrame);
		accD.prepare (numChans, engineFrame);
		const int osPower = 1; // 2x for testing later
		osF.prepare (sampleRate, numChans, osPower, engineFrame);
		osD.prepare (sampleRate, numChans, osPower, engineFrame);
		osF.setEnabled (useOversampling.load() && !forceOSOff.load());
		osD.setEnabled (useOversampling.load() && !forceOSOff.load());
		insertFadeF.arm(128);
		insertFadeD.arm(128);
		firstProcessF = firstProcessD = true;
		reset();
	}

	void reset()
	{
		accF.reset();
		accD.reset();
		osF.reset();
		osD.reset();
		insertFadeF.arm(128);
		insertFadeD.arm(128);
		firstProcessF = firstProcessD = true;
	}

	void process (juce::dsp::AudioBlock<float> block) noexcept
	{
                ::sanitizeAudioBlock (block);
		insertFadeF.apply (block);
		const bool osOn = (useOversampling.load() && !forceOSOff.load());
		if (osOn)
		{
			accF.pushAndConsume (block, [this](juce::dsp::AudioBlock<float> fixed){
				osF.processFrame (fixed);
			});
		}
		FIELD_TELEM_THROTTLED ("[SG] sr=" << (int) sampleRate
								<< " hostN=" << (int) block.getNumSamples()
								<< " frame=" << engineFrame
								<< " chans=" << (int) block.getNumChannels()
								<< " OS=" << (osOn ? "ON" : "OFF"));
                ::sanitizeAudioBlock (block);
	}

	void process (juce::dsp::AudioBlock<double> block) noexcept
	{
                ::sanitizeAudioBlock (block);
		insertFadeD.apply (block);
		const bool osOn = (useOversampling.load() && !forceOSOff.load());
		if (osOn)
		{
			accD.pushAndConsume (block, [this](juce::dsp::AudioBlock<double> fixed){
				osD.processFrame (fixed);
			});
		}
		FIELD_TELEM_THROTTLED ("[SGD] sr=" << (int) sampleRate
								<< " hostN=" << (int) block.getNumSamples()
								<< " frame=" << engineFrame
								<< " chans=" << (int) block.getNumChannels()
								<< " OS=" << (osOn ? "ON" : "OFF"));
                ::sanitizeAudioBlock (block);
	}

	int getPreparedBlockSize() const noexcept { return preparedMax; }

private:
	double sampleRate { 48000.0 };
	int preparedMax { 0 };
	int numChans    { 2 };
	juce::AudioBuffer<float> temp; // reserved for future engines

	// Accumulators & stages
	FrameAccumulator<float>  accF;
	FrameAccumulator<double> accD;
	OversamplingStage<float>  osF;
	OversamplingStage<double> osD;
};
