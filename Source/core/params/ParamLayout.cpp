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
