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
    p.push_back (B (ReverbIDs::enabled, "Reverb Enable", false));
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
    p.push_back (F (ReverbIDs::dreqXoverLoHz, "Decay Xover Lo (Hz)", {80.f, 800.f, 0.f, 0.5f}, 250.f));
    p.push_back (F (ReverbIDs::dreqXoverHiHz, "Decay Xover Hi (Hz)", {1000.f, 10000.f, 0.f, 0.5f}, 4500.f));

    // Motion
    p.push_back (F (ReverbIDs::widthPct,      "Width (%)",         {0.f, 120.f, 0.01f}, 100.f));
    p.push_back (F (ReverbIDs::widthStartPct, "Width Start (%)",   {0.f, 120.f, 0.01f}, 100.f));
    p.push_back (F (ReverbIDs::widthEndPct,   "Width End (%)",     {0.f, 120.f, 0.01f}, 70.f));
    p.push_back (F (ReverbIDs::widthEnvCurve, "Width Curve",       {0.2f, 5.f, 0.001f}, 1.f));
    p.push_back (F (ReverbIDs::rotStartDeg,   "Rotate Start (deg)", {-30.f, 30.f, 0.01f}, 0.f));
    p.push_back (F (ReverbIDs::rotEndDeg,     "Rotate End (deg)",   {-30.f, 30.f, 0.01f}, 8.f));
    p.push_back (F (ReverbIDs::rotEnvCurve,   "Rotate Curve",       {0.2f, 5.f, 0.001f}, 1.f));
    p.push_back (F (ReverbIDs::sizePct,       "Size (%)",          {0.f, 100.f, 0.01f}, 50.f));
    p.push_back (F (ReverbIDs::bloomPct,      "Bloom (%)",         {0.f, 100.f, 0.01f}, 35.f));
    p.push_back (F (ReverbIDs::distancePct,   "Distance (%)",      {0.f, 100.f, 0.01f}, 35.f));

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

  // DynEQ (wet-only) — 4 bands
  auto modeChoices = StringArray{ "Bell", "LowShelf", "HighShelf" };
  auto addDyn = [&] (int idx, const char* onId, const char* modeId, const char* fId, const char* gId, const char* qId, const char* thrId, const char* ratioId, const char* attId, const char* relId, const char* rangeId)
  {
      p.push_back (B (onId,  String ("DynEQ ") + String (idx) + " On", false));
      p.push_back (C (modeId, String ("DynEQ ") + String (idx) + " Mode", modeChoices, 0));
      p.push_back (F (fId,   String ("DynEQ ") + String (idx) + " Freq (Hz)", {20.f, 20000.f, 1.f, 0.35f}, idx==1?220.f: idx==2?1200.f: idx==3?3500.f:8000.f));
      p.push_back (F (gId,   String ("DynEQ ") + String (idx) + " Makeup (dB)", {-12.f, 12.f, 0.01f}, 0.f));
      p.push_back (F (qId,   String ("DynEQ ") + String (idx) + " Q", {0.3f, 8.0f, 0.001f}, 1.0f));
      p.push_back (F (thrId, String ("DynEQ ") + String (idx) + " Thr (dB)", {-60.f, 0.f, 0.01f}, -24.f));
      p.push_back (F (ratioId, String ("DynEQ ") + String (idx) + " Ratio", {1.f, 20.f, 0.01f}, 4.f));
      p.push_back (F (attId, String ("DynEQ ") + String (idx) + " Attack (ms)", {1.f, 60.f, 0.01f}, 10.f));
      p.push_back (F (relId, String ("DynEQ ") + String (idx) + " Release (ms)", {10.f, 800.f, 0.1f}, 200.f));
      p.push_back (F (rangeId, String ("DynEQ ") + String (idx) + " Range (dB)", {0.f, 24.f, 0.01f}, 6.f));
  };
  addDyn (1, ReverbIDs::dyneq1_on, ReverbIDs::dyneq1_mode, ReverbIDs::dyneq1_freqHz, ReverbIDs::dyneq1_gainDb, ReverbIDs::dyneq1_Q, ReverbIDs::dyneq1_thrDb, ReverbIDs::dyneq1_ratio, ReverbIDs::dyneq1_attMs, ReverbIDs::dyneq1_relMs, ReverbIDs::dyneq1_rangeDb);
  addDyn (2, ReverbIDs::dyneq2_on, ReverbIDs::dyneq2_mode, ReverbIDs::dyneq2_freqHz, ReverbIDs::dyneq2_gainDb, ReverbIDs::dyneq2_Q, ReverbIDs::dyneq2_thrDb, ReverbIDs::dyneq2_ratio, ReverbIDs::dyneq2_attMs, ReverbIDs::dyneq2_relMs, ReverbIDs::dyneq2_rangeDb);
  addDyn (3, ReverbIDs::dyneq3_on, ReverbIDs::dyneq3_mode, ReverbIDs::dyneq3_freqHz, ReverbIDs::dyneq3_gainDb, ReverbIDs::dyneq3_Q, ReverbIDs::dyneq3_thrDb, ReverbIDs::dyneq3_ratio, ReverbIDs::dyneq3_attMs, ReverbIDs::dyneq3_relMs, ReverbIDs::dyneq3_rangeDb);
  addDyn (4, ReverbIDs::dyneq4_on, ReverbIDs::dyneq4_mode, ReverbIDs::dyneq4_freqHz, ReverbIDs::dyneq4_gainDb, ReverbIDs::dyneq4_Q, ReverbIDs::dyneq4_thrDb, ReverbIDs::dyneq4_ratio, ReverbIDs::dyneq4_attMs, ReverbIDs::dyneq4_relMs, ReverbIDs::dyneq4_rangeDb);

    // Mix / Out
    p.push_back (F (ReverbIDs::wetMix01,  "Reverb Wet 0..1", {0.f, 1.f, 0.001f}, 0.25f));
    p.push_back (F (ReverbIDs::outTrimDb, "Reverb Out (dB)", { -24.f, 12.f, 0.01f}, 0.f));
}


