#include <juce_audio_processors/juce_audio_processors.h>
#include "core/params/ParamIDs.h"
#include "core/params/ParamLayout.h"

namespace field { namespace params {

using Layout = juce::AudioProcessorValueTreeState::ParameterLayout;

static inline juce::String msLabel (float ms) { return juce::String (ms, 2) + " ms"; }

Layout makeParameterLayout()
{
	using juce::AudioParameterBool;
	using juce::AudioParameterChoice;
	using juce::AudioParameterFloat;
	using juce::NormalisableRange;

	Layout layout;

	layout.add (std::make_unique<AudioParameterBool>(
		kChainDelayEnable,  "Delay Enable",  false));
	layout.add (std::make_unique<AudioParameterBool>(
		kChainDynEqEnable,  "DynEQ Enable",  false));
	layout.add (std::make_unique<AudioParameterBool>(
		kChainReverbEnable, "Reverb Enable", false));

    {
		const juce::StringArray choices { "1x", "2x", "4x", "8x" };
		layout.add (std::make_unique<AudioParameterChoice>(
			kQualityOSFactor, "Oversampling", choices, 0));
	}

	layout.add (std::make_unique<AudioParameterBool>(
		kReverbLinearPhase, "Reverb Linear-Phase", false));

	{
		auto range = NormalisableRange<float> (0.0f, 4096.0f, 1.0f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kReverbFIRHalfLen, "Reverb FIR HalfLen", range, 0.0f,
			juce::String(), juce::AudioProcessorParameter::genericParameter,
			[] (float v, int) { return juce::String ((int) v) + " samp"; },
			[] (const juce::String& s) { return s.getFloatValue(); }));
	}

	{
		auto range = NormalisableRange<float> (0.0f, 10.0f, 0.01f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kDynEqLookAheadMs, "DynEQ Look-Ahead", range, 0.0f,
			juce::String(), juce::AudioProcessorParameter::genericParameter,
			[] (float v, int) { return msLabel (v); },
			[] (const juce::String& s) { return s.getFloatValue(); }));
	}

	{
		auto range = NormalisableRange<float> (0.0f, 10.0f, 0.01f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kDelayLookAheadMs, "Delay Look-Ahead", range, 0.0f,
			juce::String(), juce::AudioProcessorParameter::genericParameter,
			[] (float v, int) { return msLabel (v); },
			[] (const juce::String& s) { return s.getFloatValue(); }));
	}

	// ---- Right-side master controls ----
	{
		auto range = NormalisableRange<float> (-24.0f, 24.0f, 0.01f, 1.0f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kInGainDb, "Input Gain",
			range, 0.0f,
			juce::String(), juce::AudioProcessorParameter::genericParameter,
			[] (float v, int) { return juce::String (v, 1) + " dB"; },
			[] (const juce::String& s) { return s.getFloatValue(); }));
	}

	{
		auto range = NormalisableRange<float> (-1.0f, 1.0f, 0.001f, 1.0f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kPanBalance, "Balance",
			range, 0.0f));
	}

	layout.add (std::make_unique<AudioParameterBool>(
		kGlobalBypass, "Bypass", false));

	// --- Mix & Output ---
	{
		auto range = NormalisableRange<float> (0.0f, 1.0f, 0.0f, 0.5f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kMixWet01, "Mix",
			range, 0.33f,
			juce::String(), juce::AudioProcessorParameter::genericParameter,
			[] (float v, int) { return juce::String (juce::roundToInt (v * 100.0f)) + " %"; },
			[] (const juce::String& s) { return juce::jlimit (0.0f, 1.0f, s.getFloatValue() / 100.0f); }));
	}
	{
		auto range = NormalisableRange<float> (-24.0f, 24.0f, 0.01f, 1.0f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kOutGainDb, "Output Gain",
			range, 0.0f,
			juce::String(), juce::AudioProcessorParameter::genericParameter,
			[] (float v, int) { return juce::String (v, 1) + " dB"; },
			[] (const juce::String& s) { return s.getFloatValue(); }));
	}
	{
		auto range = NormalisableRange<float> (-1.0f, 1.0f, 0.001f, 1.0f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kPanBalance, "Balance",
			range, 0.0f));
	}

	// --- Tone ---
	{
		auto range = NormalisableRange<float> (-6.0f, 6.0f, 0.01f, 1.0f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kToneTiltDbPerOct, "Tilt",
			range, 1.5f,
			juce::String(), juce::AudioProcessorParameter::genericParameter,
			[] (float v, int) { return juce::String (v, 1) + " dB/oct"; },
			[] (const juce::String& s) { return s.getFloatValue(); }));
	}
	{
		auto range = NormalisableRange<float> (-12.0f, 12.0f, 0.01f, 1.0f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kToneBassDb, "Bass",
			range, 2.0f,
			juce::String(), juce::AudioProcessorParameter::genericParameter,
			[] (float v, int) { return juce::String (v, 1) + " dB"; },
			[] (const juce::String& s) { return s.getFloatValue(); }));
	}

	// --- Reverb voicing ---
	{
		auto range = NormalisableRange<float> (0.0f, 120.0f, 0.01f, 0.6f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kRvPreDelayMs, "Pre-Delay",
			range, 20.0f,
			juce::String(), juce::AudioProcessorParameter::genericParameter,
			[] (float v, int) { return msLabel (v); },
			[] (const juce::String& s) { return s.getFloatValue(); }));
	}
	{
		auto range = NormalisableRange<float> (0.0f, 1.0f, 0.01f, 0.7f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kRvSizeNorm, "Size",
			range, 0.62f));
	}
	{
		// Log-like taper for damping frequency
		auto range = NormalisableRange<float> (1000.0f, 16000.0f);
		range.setSkewForCentre (std::sqrt (1000.0f * 16000.0f));
		layout.add (std::make_unique<AudioParameterFloat>(
			kRvDampingHz, "Damping",
			range, 6000.0f,
			juce::String(), juce::AudioProcessorParameter::genericParameter,
			[] (float v, int) { return juce::String ((int) v) + " Hz"; },
			[] (const juce::String& s) { return s.getFloatValue(); }));
	}

	// --- Imager ---
	{
		auto range = NormalisableRange<float> (0.0f, 2.0f, 0.0f, 0.8f);
		layout.add (std::make_unique<AudioParameterFloat>(
			kImagerWidth, "Width",
			range, 1.15f,
			juce::String(), juce::AudioProcessorParameter::genericParameter,
			[] (float v, int) { return juce::String (juce::roundToInt (v * 100.0f)) + " %"; },
			[] (const juce::String& s) { return s.getFloatValue(); }));
	}

    // Dev HUD (debug/internal only)
    {
    #include "core/runtime/DevHudFlag.h"
    #if FIELD_DEV_HUD_ON
        layout.add (std::make_unique<AudioParameterBool>(
            kDevHudEnable, "Dev HUD", true));
    #endif
    }

    return layout;
}

}} // namespace field::params
