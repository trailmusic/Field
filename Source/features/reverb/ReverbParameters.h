#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "ReverbParamIDs.h"

struct ReverbParameters
{
    static void addParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    static juce::StringArray dreqApplyChoices();
    static juce::StringArray duckModeChoices();
    static juce::StringArray duckDetectorChoices();
};

inline void addReverbParameters (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& p)
{
    using namespace juce;
    using namespace ReverbParamIDs;
    
    auto F = [] (const String& id, const String& nm, NormalisableRange<float> r, float def)
    {
        return std::make_unique<AudioParameterFloat> (ParameterID{ id, 1 }, nm, r, def);
    };
    auto B = [] (const String& id, const String& nm, bool def)
    {
        return std::make_unique<AudioParameterBool> (ParameterID{ id, 1 }, nm, def);
    };
    auto C = [] (const String& id, const String& nm, StringArray choices, int defIdx)
    {
        return std::make_unique<AudioParameterChoice> (ParameterID{ id, 1 }, nm, choices, defIdx);
    };

    // Core routing
    p.push_back (B (enabled, "Reverb Enable", true));
    p.push_back (B (killDry, "Wet Only", false));

    // Structure / space
    p.push_back (F (preDelayMs, "Pre", NormalisableRange<float>(0.f, 200.f, 0.1f), 0.f));
    p.push_back (F (decaySec, "Decay", NormalisableRange<float>(0.2f, 20.f, 0.001f, 0.4f), 2.4f));
    p.push_back (F (sizePct, "Size", NormalisableRange<float>(10.f, 200.f, 0.1f), 100.f));

    // Early reflections
    p.push_back (F (erLevelDb, "ER Lvl", NormalisableRange<float>(-24.f, 6.f, 0.1f), -6.f));
    p.push_back (F (erDensityPct, "ER Den", NormalisableRange<float>(0.f, 100.f, 0.1f), 60.f));
    p.push_back (F (erWidthPct, "ER Wid", NormalisableRange<float>(0.f, 200.f, 0.1f), 110.f));
    p.push_back (F (erTimeMs, "ER Time", NormalisableRange<float>(50.f, 500.f, 0.1f), 120.f));
    p.push_back (F (erToTailPct, "ER→T", NormalisableRange<float>(0.f, 100.f, 0.1f), 40.f));

    // Diffusion / density
    p.push_back (F (diffusionPct, "Diff", NormalisableRange<float>(0.f, 100.f, 0.1f), 70.f));
    p.push_back (F (densityPct, "Dens", NormalisableRange<float>(0.f, 100.f, 0.1f), 65.f));

    // Modulation
    p.push_back (F (modDepthCents, "Mod Dep", NormalisableRange<float>(0.f, 25.f, 0.01f), 3.5f));
    p.push_back (F (modRateHz, "Mod Rate", NormalisableRange<float>(0.05f, 5.f, 0.0001f, 0.4f), 0.35f));

    // Stereo + rotation
    p.push_back (F (widthPct, "Width", NormalisableRange<float>(0.f, 200.f, 0.1f), 100.f));
    p.push_back (F (rotationDeg, "Rot", NormalisableRange<float>(-45.f, 45.f, 0.1f), 0.f));

    // Motion follow
    p.push_back (B (followWidth, "Follow W", false));
    p.push_back (F (followWidthAmt, "W Amt", NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));
    p.push_back (B (followRot, "Follow R", false));
    p.push_back (F (followRotAmt, "R Amt", NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));

    // Mix & specials
    p.push_back (F (wetMix01, "Wet", NormalisableRange<float>(0.f, 1.f, 0.001f), 0.25f));
    p.push_back (F (bloomPct, "Bloom", NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));
    p.push_back (F (distancePct, "Distance", NormalisableRange<float>(0.f, 100.f, 0.1f), 50.f));
    p.push_back (B (freeze, "Freeze", false));
    p.push_back (F (shimmerAmtPct, "Shim Amt", NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));
    p.push_back (F (shimmerInt, "Shim Int", NormalisableRange<float>(0.f, 100.f, 0.1f), 50.f));
    p.push_back (F (gateAmtPct, "Gate", NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));
    p.push_back (F (outTrimDb, "Trim", NormalisableRange<float>(-24.f, 12.f, 0.01f), 0.f));

    // Reverb EQ routing
    p.push_back (F (dreqXoverLoHz, "DREQ XO Lo", NormalisableRange<float>(80.f, 400.f, 0.1f, 0.35f), 160.f));
    p.push_back (F (dreqXoverHiHz, "DREQ XO Hi", NormalisableRange<float>(1000.f, 6000.f, 0.1f, 0.35f), 3000.f));
    p.push_back (C (dreqApply, "EQ Apply", ReverbParameters::dreqApplyChoices(), 1)); // default Post

    // Ducking (floating)
    p.push_back (B (duckOn, "Duck On", false));
    p.push_back (C (duckMode, "Duck Mode", ReverbParameters::duckModeChoices(), 0));
    p.push_back (F (duckDepthDb, "Duck Depth", NormalisableRange<float>(0.f, 24.f, 0.1f), 6.f));
    p.push_back (F (duckAtkMs, "Attack", NormalisableRange<float>(1.f, 100.f, 0.1f), 10.f));
    p.push_back (F (duckRelMs, "Release", NormalisableRange<float>(50.f, 2000.f, 0.1f, 0.35f), 300.f));
    p.push_back (F (duckThrDb, "Threshold", NormalisableRange<float>(-60.f, -6.f, 0.1f), -24.f));
    p.push_back (F (duckRatio, "Ratio", NormalisableRange<float>(1.f, 8.f, 0.01f), 3.f));
    p.push_back (F (duckKneeDb, "Knee", NormalisableRange<float>(0.f, 24.f, 0.1f), 6.f));
    p.push_back (F (duckBandHz, "Focus Hz", NormalisableRange<float>(50.f, 8000.f, 0.1f, 0.35f), 2000.f));
    p.push_back (F (duckBandQ, "Focus Q", NormalisableRange<float>(0.3f, 4.f, 0.01f, 0.35f), 1.0f));
    p.push_back (C (duckDetectorSrc, "Detector", ReverbParameters::duckDetectorChoices(), 3)); // Wet Sum
}


