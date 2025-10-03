#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// ReverbParameters — Factory for APVTS layout + shared choice lists
// ----------------------------------------------------------------------------
// DEV NOTES
// - Header-only for simplicity. If compile times grow, split to .cpp.
// - Uses labels for most floats; adds pretty string formatters for:
//     * wetMix01  → "25 %"
//     * decaySec  → "2.4 s" (smart precision)
//     * duckRatio → "3:1"
// - Audio/render behavior unchanged; this is display-only.
// ─────────────────────────────────────────────────────────────────────────────

#include <juce_audio_processors/juce_audio_processors.h>
#include "ReverbParamIDs.h"

struct ReverbParameters
{
    // Primary: fill an APVTS ParameterLayout
    static void addParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout);

    // Choice lists (shared across UI + factory)
    static juce::StringArray dreqApplyChoices ();
    static juce::StringArray duckModeChoices ();
    static juce::StringArray duckDetectorChoices ();
    static juce::StringArray decaySmoothingChoices ();
    static juce::StringArray decayModeChoices ();
};

// Legacy helper — vector-based param list (independent implementation for backward compatibility)
inline void addReverbParameters (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& out)
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
    out.push_back (B (enabled, "Reverb Enable", true));
    out.push_back (B (killDry, "Wet Only", false));

    // Structure / space
    out.push_back (F (preDelayMs, "Pre", NormalisableRange<float>(0.f, 200.f, 0.1f), 0.f));
    out.push_back (F (decaySec, "Decay", NormalisableRange<float>(0.2f, 20.f, 0.001f, 0.4f), 2.4f));
    out.push_back (F (sizePct, "Size", NormalisableRange<float>(10.f, 200.f, 0.1f), 100.f));

    // Early reflections
    out.push_back (F (erLevelDb, "ER Lvl", NormalisableRange<float>(-24.f, 6.f, 0.1f), -6.f));
    out.push_back (F (erDensityPct, "ER Den", NormalisableRange<float>(0.f, 100.f, 0.1f), 60.f));
    out.push_back (F (erWidthPct, "ER Wid", NormalisableRange<float>(0.f, 200.f, 0.1f), 110.f));
    out.push_back (F (erTimeMs, "ER Time", NormalisableRange<float>(50.f, 500.f, 0.1f), 120.f));
    out.push_back (F (erToTailPct, "ER→T", NormalisableRange<float>(0.f, 100.f, 0.1f), 40.f));

    // Diffusion / density
    out.push_back (F (diffusionPct, "Diff", NormalisableRange<float>(0.f, 100.f, 0.1f), 70.f));
    out.push_back (F (densityPct, "Dens", NormalisableRange<float>(0.f, 100.f, 0.1f), 65.f));

    // Modulation
    out.push_back (F (modDepthCents, "Mod Dep", NormalisableRange<float>(0.f, 25.f, 0.01f), 3.5f));
    out.push_back (F (modRateHz, "Mod Rate", NormalisableRange<float>(0.05f, 5.f, 0.0001f, 0.4f), 0.35f));

    // Stereo + rotation
    out.push_back (F (widthPct, "Width", NormalisableRange<float>(0.f, 200.f, 0.1f), 100.f));
    out.push_back (F (rotationDeg, "Rot", NormalisableRange<float>(-45.f, 45.f, 0.1f), 0.f));

    // Motion follow
    out.push_back (B (followWidth, "Follow W", false));
    out.push_back (F (followWidthAmt, "W Amt", NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));
    out.push_back (B (followRot, "Follow R", false));
    out.push_back (F (followRotAmt, "R Amt", NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));

    // Mix & specials
    out.push_back (F (wetMix01, "Wet", NormalisableRange<float>(0.f, 1.f, 0.001f), 0.25f));
    out.push_back (F (bloomPct, "Bloom", NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));
    out.push_back (F (distancePct, "Distance", NormalisableRange<float>(0.f, 100.f, 0.1f), 50.f));
    out.push_back (B (freeze, "Freeze", false));
    out.push_back (F (shimmerAmtPct, "Shim Amt", NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));
    out.push_back (F (shimmerInt, "Shim Int", NormalisableRange<float>(0.f, 100.f, 0.1f), 50.f));
    out.push_back (F (gateAmtPct, "Gate", NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));
    out.push_back (F (outTrimDb, "Trim", NormalisableRange<float>(-24.f, 12.f, 0.01f), 0.f));

    // Reverb EQ routing
    out.push_back (F (dreqXoverLoHz, "DREQ XO Lo", NormalisableRange<float>(80.f, 400.f, 0.1f, 0.35f), 160.f));
    out.push_back (F (dreqXoverHiHz, "DREQ XO Hi", NormalisableRange<float>(1000.f, 6000.f, 0.1f, 0.35f), 3000.f));
    out.push_back (C (dreqApply, "EQ Apply", ReverbParameters::dreqApplyChoices(), 1)); // default Post

    // Ducking (floating)
    out.push_back (B (duckOn, "Duck On", false));
    out.push_back (C (duckMode, "Duck Mode", ReverbParameters::duckModeChoices(), 0));
    out.push_back (F (duckDepthDb, "Duck Depth", NormalisableRange<float>(0.f, 24.f, 0.1f), 6.f));
    out.push_back (F (duckAtkMs, "Attack", NormalisableRange<float>(1.f, 100.f, 0.1f), 10.f));
    out.push_back (F (duckRelMs, "Release", NormalisableRange<float>(50.f, 2000.f, 0.1f, 0.35f), 300.f));
    out.push_back (F (duckThrDb, "Threshold", NormalisableRange<float>(-60.f, -6.f, 0.1f), -24.f));
    out.push_back (F (duckRatio, "Ratio", NormalisableRange<float>(1.f, 8.f, 0.01f), 3.f));
    out.push_back (F (duckKneeDb, "Knee", NormalisableRange<float>(0.f, 24.f, 0.1f), 6.f));
    out.push_back (F (duckBandHz, "Focus Hz", NormalisableRange<float>(50.f, 8000.f, 0.1f, 0.35f), 2000.f));
    out.push_back (F (duckBandQ, "Focus Q", NormalisableRange<float>(0.3f, 4.f, 0.01f, 0.35f), 1.0f));
    out.push_back (C (duckDetectorSrc, "Detector", ReverbParameters::duckDetectorChoices(), 3)); // Wet Sum

    // ================================================================
    // 🎯 DECAY RATE CONTROL PARAMETERS (JANUARY 2025)
    // ================================================================
    // CRITICAL: Musical decay-rate control for reverb tails
    // These parameters enable frequency-dependent T60 shaping
    // ================================================================
    out.push_back (F (decayLoMult, "Low Decay", NormalisableRange<float>(0.25f, 4.0f, 0.01f, 0.35f), 1.0f));
    out.push_back (F (decayHiMult, "High Decay", NormalisableRange<float>(0.25f, 4.0f, 0.01f, 0.35f), 1.0f));
    out.push_back (F (decayMidDb, "Mid Bell", NormalisableRange<float>(-12.f, 12.f, 0.1f), 0.f));
    out.push_back (F (decayMidFreqHz, "Mid Freq", NormalisableRange<float>(20.f, 20000.f, 0.1f, 0.35f), 1200.f));
    out.push_back (F (decayMidQ, "Mid Q", NormalisableRange<float>(0.3f, 6.0f, 0.01f, 0.35f), 0.9f));
    out.push_back (F (decayTiltDb, "Decay Tilt", NormalisableRange<float>(-12.f, 12.f, 0.1f), 0.f));
    out.push_back (C (decaySmoothing, "Smoothing", ReverbParameters::decaySmoothingChoices(), 1)); // Med
    out.push_back (C (decayMode, "Decay Mode", ReverbParameters::decayModeChoices(), 0)); // Simple
}