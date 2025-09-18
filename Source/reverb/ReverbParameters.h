#pragma once
#include <JuceHeader.h>
#include "ReverbParamIDs.h"

inline void addReverbParameters (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& p)
{
    using namespace juce;
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

    // Top / Algo
    p.push_back (B (ReverbIDs::enabled, "Reverb Enable", true));
    p.push_back (B (ReverbIDs::killDry, "Wet Only", false));
    p.push_back (C (ReverbIDs::algo, "Reverb Algo", StringArray{ "Modern FDN", "Chamber", "Platey", "Vintage" }, 0));

    // Space / Time
    p.push_back (F (ReverbIDs::preDelayMs,    "Pre-Delay (ms)",    {0.f, 120.f, 0.01f}, 12.f));
    p.push_back (F (ReverbIDs::decaySec,      "Decay (s)",         {0.2f, 20.f, 0.001f, 0.4f}, 2.2f));
    p.push_back (F (ReverbIDs::densityPct,    "Density (%)",       {0.f,100.f,0.01f}, 75.f));
    p.push_back (F (ReverbIDs::diffusionPct,  "Diffusion (%)",     {0.f,100.f,0.01f}, 65.f));
    p.push_back (F (ReverbIDs::modDepthCents, "Mod Depth (c)",     {0.f,50.f,0.01f}, 8.f));
    p.push_back (F (ReverbIDs::modRateHz,     "Mod Rate (Hz)",     {0.05f,2.f,0.0001f,0.5f}, 0.30f));

    // ER
    p.push_back (F (ReverbIDs::erLevelDb,    "ER Level (dB)",     {-24.f, 0.f, 0.01f}, -6.f));
    p.push_back (F (ReverbIDs::erTimeMs,     "ER Time (ms)",      {5.f, 80.f, 0.01f},  22.f));
    p.push_back (F (ReverbIDs::erDensityPct, "ER Density (%)",    {0.f,100.f,0.01f},   60.f));
    p.push_back (F (ReverbIDs::erWidthPct,   "ER Width (%)",      {0.f,100.f,0.01f},   90.f));
    p.push_back (F (ReverbIDs::erToTailPct,  "ER→Tail (%)",       {0.f,100.f,0.01f},   70.f));

    // Tone
    p.push_back (F (ReverbIDs::hpfHz,  "HPF (Hz)", {20.f, 500.f, 0.01f, 0.3f}, 120.f));
    p.push_back (F (ReverbIDs::lpfHz,  "LPF (Hz)", {3000.f, 20000.f, 1.f, 0.3f}, 9500.f));
    p.push_back (F (ReverbIDs::tiltDb, "Tilt (dB)", {-6.f, 6.f, 0.01f}, 0.f));

    // DR-EQ
    p.push_back (F (ReverbIDs::dreqLowX,  "Decay Low ×",  {0.3f, 2.0f, 0.001f}, 0.8f));
    p.push_back (F (ReverbIDs::dreqMidX,  "Decay Mid ×",  {0.5f, 1.5f, 0.001f}, 1.0f));
    p.push_back (F (ReverbIDs::dreqHighX, "Decay High ×", {0.3f, 2.0f, 0.001f}, 0.9f));

    // Motion
    p.push_back (F (ReverbIDs::widthPct,      "Width (%)",         {0.f, 120.f, 0.01f}, 100.f));
    p.push_back (F (ReverbIDs::widthStartPct, "Width Start (%)",   {0.f, 120.f, 0.01f}, 100.f));
    p.push_back (F (ReverbIDs::widthEndPct,   "Width End (%)",     {0.f, 120.f, 0.01f}, 70.f));
    p.push_back (F (ReverbIDs::widthEnvCurve, "Width Curve",       {0.2f, 5.f, 0.001f}, 1.f));
    p.push_back (F (ReverbIDs::rotStartDeg,   "Rotate Start (deg)", {-30.f, 30.f, 0.01f}, 0.f));
    p.push_back (F (ReverbIDs::rotEndDeg,     "Rotate End (deg)",   {-30.f, 30.f, 0.01f}, 8.f));
    p.push_back (F (ReverbIDs::rotEnvCurve,   "Rotate Curve",       {0.2f, 5.f, 0.001f}, 1.f));

    // Ducking
    p.push_back (C (ReverbIDs::duckMode, "Duck Mode", StringArray{ "WetOnly", "Center", "Band" }, 0));
    p.push_back (F (ReverbIDs::duckDepthDb,  "Duck Depth (dB)",     {0.f, 36.f, 0.01f}, 12.f));
    p.push_back (F (ReverbIDs::duckThrDb,    "Duck Threshold (dB)", {-60.f, 0.f, 0.01f}, -18.f));
    p.push_back (F (ReverbIDs::duckKneeDb,   "Duck Knee (dB)",      {0.f, 18.f, 0.01f}, 6.f));
    p.push_back (F (ReverbIDs::duckRatio,    "Duck Ratio",          {1.f, 20.f, 0.01f}, 6.f));
    p.push_back (F (ReverbIDs::duckAtkMs,    "Duck Attack (ms)",    {1.f, 80.f, 0.01f}, 12.f));
    p.push_back (F (ReverbIDs::duckRelMs,    "Duck Release (ms)",   {20.f, 800.f, 0.1f}, 180.f));
    p.push_back (F (ReverbIDs::duckLaMs,     "Duck Lookahead (ms)", {0.f, 20.f, 0.01f}, 5.f));
    p.push_back (F (ReverbIDs::duckRmsMs,    "Duck RMS (ms)",       {2.f, 50.f, 0.01f}, 15.f));
    p.push_back (F (ReverbIDs::duckBandHz,   "Duck Band (Hz)",      {100.f, 8000.f, 0.01f, 0.3f}, 600.f));
    p.push_back (F (ReverbIDs::duckBandQ,    "Duck Band Q",         {0.3f, 4.f, 0.001f}, 1.f));

    // Specials
    p.push_back (B (ReverbIDs::freeze,        "Freeze", false));
    p.push_back (F (ReverbIDs::gateAmtPct,    "Gate (%)",        {0.f, 100.f, 0.01f}, 0.f));
    p.push_back (F (ReverbIDs::shimmerAmtPct, "Shimmer (%)",     {0.f, 100.f, 0.01f}, 0.f));
    p.push_back (C (ReverbIDs::shimmerInt,    "Shimmer Interval", StringArray{ "+12", "+7", "-12" }, 0));

    // Post-EQ
    p.push_back (B (ReverbIDs::eqOn,         "EQ On", true));
    p.push_back (F (ReverbIDs::postEqMixPct, "EQ Mix (%)",       {0.f, 100.f, 0.01f}, 50.f));
    p.push_back (F (ReverbIDs::eqLowFreqHz,  "EQ Low Freq (Hz)", {20.f, 500.f, 0.01f, 0.3f}, 120.f));
    p.push_back (F (ReverbIDs::eqLowGainDb,  "EQ Low Gain (dB)", { -18.f, 18.f, 0.01f}, 0.f));
    p.push_back (F (ReverbIDs::eqLowQ,       "EQ Low Q",         {0.3f, 2.0f, 0.001f}, 0.71f));
    p.push_back (F (ReverbIDs::eqMidFreqHz,  "EQ Mid Freq (Hz)", {200.f, 8000.f, 0.01f, 0.3f}, 1500.f));
    p.push_back (F (ReverbIDs::eqMidGainDb,  "EQ Mid Gain (dB)", { -18.f, 18.f, 0.01f}, 0.f));
    p.push_back (F (ReverbIDs::eqMidQ,       "EQ Mid Q",         {0.3f, 5.0f, 0.001f}, 1.0f));
    p.push_back (F (ReverbIDs::eqHighFreqHz, "EQ High Freq (Hz)",{2000.f, 20000.f, 1.f, 0.3f}, 9500.f));
    p.push_back (F (ReverbIDs::eqHighGainDb, "EQ High Gain (dB)",{ -18.f, 18.f, 0.01f}, 0.f));
    p.push_back (F (ReverbIDs::eqHighQ,      "EQ High Q",        {0.3f, 2.0f, 0.001f}, 0.71f));

    // Mix / Out
    p.push_back (F (ReverbIDs::wetMix01,  "Reverb Wet 0..1", {0.f, 1.f, 0.001f}, 0.25f));
    p.push_back (F (ReverbIDs::outTrimDb, "Reverb Out (dB)", { -24.f, 12.f, 0.01f}, 0.f));
}


