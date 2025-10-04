// ─────────────────────────────────────────────────────────────────────────────
// ReverbParameters.cpp — APVTS layout + choice lists (labels + pretty strings)
// ----------------------------------------------------------------------------

#include "ReverbParameters.h"
#include "core/params/ParamIDs.h"
#include <juce_audio_processors/juce_audio_processors.h>

using namespace juce;
using namespace field::params;

namespace
{
    // JUCE ParameterID version — bump only if you truly need to remap
    constexpr int kParamVersion = 1;

    // Helpers for parameter creation
    inline std::unique_ptr<AudioParameterFloat> makeFloat (const String& id,
                                                           const String& name,
                                                           NormalisableRange<float> range,
                                                           float def,
                                                           const String& label = {},
                                                           int ver = kParamVersion)
    {
        return std::make_unique<AudioParameterFloat> (ParameterID { id, ver }, name, range, def, label);
    }

    inline std::unique_ptr<AudioParameterBool> makeBool (const String& id,
                                                         const String& name,
                                                         bool def,
                                                         int ver = kParamVersion)
    {
        return std::make_unique<AudioParameterBool> (ParameterID { id, ver }, name, def);
    }

    inline std::unique_ptr<AudioParameterChoice> makeChoice (const String& id,
                                                             const String& name,
                                                             const StringArray& choices,
                                                             int defIndex,
                                                             int ver = kParamVersion)
    {
        return std::make_unique<AudioParameterChoice> (ParameterID { id, ver }, name, choices, defIndex);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Choice lists
// ─────────────────────────────────────────────────────────────────────────────

StringArray ReverbParameters::dreqApplyChoices ()
{
    return { "Pre", "Post", "Early", "Tail" };
}

StringArray ReverbParameters::duckModeChoices ()
{
    return { "General", "Vocal", "Drum Bus", "Guitar", "Keys" };
}

StringArray ReverbParameters::duckDetectorChoices ()
{
    return { "Dry", "ER", "Tail", "Wet" };
}

StringArray ReverbParameters::decaySmoothingChoices ()
{
    return { "Fast", "Med", "Slow" };
}

StringArray ReverbParameters::decayModeChoices ()
{
    return { "Simple", "Advanced" };
}

StringArray ReverbParameters::decayProfileModeChoices ()
{
    return { "Manual 3-Band", "Tilt-Coupled", "Plate", "Hall", 
             "Room", "Chamber", "Cathedral", "Nonlinear" };
}

StringArray ReverbParameters::decayProfileCouplingChoices ()
{
    return { "Independent", "Follow Tone Tilt", "Follow HP/LP", 
             "Follow Width Designer", "Sidechain Learn" };
}

StringArray ReverbParameters::decayLearnChoices ()
{
    return { "Idle", "Capturing", "Solving", "Ready", "Error" };
}

// ─────────────────────────────────────────────────────────────────────────────
// Factory — APVTS layout
// ─────────────────────────────────────────────────────────────────────────────

void ReverbParameters::addParameters (AudioProcessorValueTreeState::ParameterLayout& layout)
{
    // Core routing
    layout.add (makeBool (enabled,  "Reverb Enable", true));
    layout.add (makeBool (killDry,  "Wet Only",      false));

    // Structure / space
    layout.add (makeFloat (preDelayMs, "Pre",
                           { 0.f, 200.f, 0.1f },                         0.f,   "ms"));
    layout.add (makeFloat (decaySec, "Decay",
                           { 0.2f, 20.f, 0.001f, 0.4f },                 2.4f,  "s"));
    layout.add (makeFloat (sizePct,    "Size",
                           { 10.f, 200.f, 0.1f },                        100.f, "%"));

    // Early reflections
    layout.add (makeFloat (erLevelDb,   "ER Lvl",
                           { -24.f, 6.f, 0.1f },                         -6.f,  "dB"));
    layout.add (makeFloat (erDensityPct,"ER Den",
                           { 0.f, 100.f, 0.1f },                         60.f,  "%"));
    layout.add (makeFloat (erWidthPct,  "ER Wid",
                           { 0.f, 200.f, 0.1f },                         110.f, "%"));
    layout.add (makeFloat (erTimeMs,    "ER Time",
                           { 50.f, 500.f, 0.1f },                        120.f, "ms"));
    layout.add (makeFloat (erToTailPct, "ER→T",
                           { 0.f, 100.f, 0.1f },                         40.f,  "%"));

    // Diffusion / density
    layout.add (makeFloat (diffusionPct,"Diff",
                           { 0.f, 100.f, 0.1f },                         70.f,  "%"));
    layout.add (makeFloat (densityPct,  "Dens",
                           { 0.f, 100.f, 0.1f },                         65.f,  "%"));

    // Modulation
    layout.add (makeFloat (modDepthCents,"Mod Dep",
                           { 0.f, 25.f, 0.01f },                         3.5f,  "ct")); // cents
    layout.add (makeFloat (modRateHz,   "Mod Rate",
                           { 0.05f, 5.f, 0.0001f, 0.4f },                0.35f, "Hz"));

    // Stereo + rotation
    layout.add (makeFloat (widthPct,   "Width",
                           { 0.f, 200.f, 0.1f },                         100.f, "%"));
    layout.add (makeFloat (rotationDeg,"Rot",
                           { -45.f, 45.f, 0.1f },                        0.f,   "°"));

    // Motion follow
    layout.add (makeBool  (followWidth,    "Follow W", false));
    layout.add (makeFloat (followWidthAmt, "W Amt",
                           { 0.f, 100.f, 0.1f },                         0.f,   "%"));
    layout.add (makeBool  (followRot,      "Follow R", false));
    layout.add (makeFloat (followRotAmt,   "R Amt",
                           { 0.f, 100.f, 0.1f },                         0.f,   "%"));

    // Mix & specials
    layout.add (makeFloat (wetMix01, "Wet",
                           { 0.f, 1.f, 0.001f },                         0.25f, "%"));
    layout.add (makeFloat (bloomPct,     "Bloom",
                           { 0.f, 100.f, 0.1f },                         0.f,   "%"));
    layout.add (makeFloat (distancePct,  "Distance",
                           { 0.f, 100.f, 0.1f },                         50.f,  "%"));
    layout.add (makeBool  (freeze,       "Freeze",                        false));
    layout.add (makeFloat (shimmerAmtPct,"Shim Amt",
                           { 0.f, 100.f, 0.1f },                         0.f,   "%"));
    layout.add (makeFloat (shimmerInt,   "Shim Int",
                           { 0.f, 100.f, 0.1f },                         50.f,  "%"));
    layout.add (makeFloat (gateAmtPct,   "Gate",
                           { 0.f, 100.f, 0.1f },                         0.f,   "%"));
    layout.add (makeFloat (outTrimDb,    "Trim",
                           { -24.f, 12.f, 0.01f },                       0.f,   "dB"));

    // Reverb EQ routing
    layout.add (makeFloat (dreqXoverLoHz, "DREQ XO Lo",
                           { 80.f, 400.f, 0.1f, 0.35f },                  160.f, "Hz"));
    layout.add (makeFloat (dreqXoverHiHz, "DREQ XO Hi",
                           { 1000.f, 6000.f, 0.1f, 0.35f },               3000.f,"Hz"));
    layout.add (makeChoice (dreqApply, "EQ Apply",
                            ReverbParameters::dreqApplyChoices (),        1)); // default Post

    // Ducking (floating)
    layout.add (makeBool  (duckOn,       "Duck On",                        false));
    layout.add (makeChoice(duckMode,     "Duck Mode",
                           ReverbParameters::duckModeChoices (),          0));
    layout.add (makeFloat (duckDepthDb,  "Duck Depth",
                           { 0.f, 24.f, 0.1f },                           6.f,   "dB"));
    layout.add (makeFloat (duckAtkMs,    "Attack",
                           { 1.f, 100.f, 0.1f },                          10.f,  "ms"));
    layout.add (makeFloat (duckRelMs,    "Release",
                           { 50.f, 2000.f, 0.1f, 0.35f },                 300.f, "ms"));
    layout.add (makeFloat (duckThrDb,    "Threshold",
                           { -60.f, -6.f, 0.1f },                         -24.f, "dB"));
    layout.add (makeFloat (duckRatio,    "Ratio",
                           { 1.f, 8.f, 0.01f },                           3.f,   ":1"));
    layout.add (makeFloat (duckKneeDb,   "Knee",
                           { 0.f, 24.f, 0.1f },                           6.f,   "dB"));
    layout.add (makeFloat (duckBandHz,   "Focus Hz",
                           { 50.f, 8000.f, 0.1f, 0.35f },                 2000.f,"Hz"));
    layout.add (makeFloat (duckBandQ,    "Focus Q",
                           { 0.3f, 4.f, 0.01f, 0.35f },                   1.0f  /* unitless */));
    layout.add (makeChoice (duckDetectorSrc, "Detector",
                            ReverbParameters::duckDetectorChoices (),     3)); // default Wet

    // ================================================================
    // 🎯 DECAY RATE CONTROL PARAMETERS (JANUARY 2025)
    // ================================================================
    // CRITICAL: Musical decay-rate control for reverb tails
    // These parameters enable frequency-dependent T60 shaping
    // ================================================================
    layout.add (makeFloat (decayLoMult,     "Low Decay",
                           { 0.25f, 4.0f, 0.01f, 0.35f },                   1.0f,  "×"));
    layout.add (makeFloat (decayHiMult,     "High Decay",
                           { 0.25f, 4.0f, 0.01f, 0.35f },                   1.0f,  "×"));
    layout.add (makeFloat (decayMidDb,      "Mid Bell",
                           { -12.f, 12.f, 0.1f },                           0.f,   "dB"));
    layout.add (makeFloat (decayMidFreqHz,  "Mid Freq",
                           { 20.f, 20000.f, 0.1f, 0.35f },                  1200.f,"Hz"));
    layout.add (makeFloat (decayMidQ,      "Mid Q",
                           { 0.3f, 6.0f, 0.01f, 0.35f },                    0.9f  /* unitless */));
    layout.add (makeFloat (decayTiltDb,     "Decay Tilt",
                           { -12.f, 12.f, 0.1f },                           0.f,   "dB"));
    layout.add (makeChoice (decaySmoothing, "Smoothing",
                            ReverbParameters::decaySmoothingChoices (),     1)); // default Med
    layout.add (makeChoice (decayMode,      "Decay Mode",
                            ReverbParameters::decayModeChoices (),         0)); // default Simple
    
    // ================================================================
    // 🎯 DECAY PROFILE SYSTEM (JANUARY 2025)
    // ================================================================
    // CRITICAL: Musical decay profile modes and coupling system
    // These parameters enable intelligent decay curve generation
    // ================================================================
    layout.add (makeChoice (decayProfileMode,     "Decay Profile",
                            ReverbParameters::decayProfileModeChoices (),     0)); // default Manual 3-Band
    layout.add (makeChoice (decayProfileCoupling, "Decay Coupling",
                            ReverbParameters::decayProfileCouplingChoices (),  0)); // default Independent
    
    // ================================================================
    // 🎯 SIDECHAIN LEARN SYSTEM (JANUARY 2025)
    // ================================================================
    // CRITICAL: Auto-learn decay profiles from external signals
    // These parameters enable intelligent decay curve learning
    // ================================================================
    layout.add (makeBool  (decayLearn,       "Learn (One-Shot)", false));
    layout.add (makeBool  (decayLearnReset,  "Reset Learned",    false));
    layout.add (makeFloat (decayLearnStrength, "Learn Strength",
                           { 0.0f, 1.0f, 0.01f },                0.5f,  "%"));
    layout.add (makeFloat (decayLearnWindow, "Learn Window (s)",
                           { 2.0f, 8.0f, 0.1f },                 4.0f,  "s"));
}