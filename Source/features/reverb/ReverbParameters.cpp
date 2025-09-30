#include "ReverbParameters.h"
using namespace juce;
using namespace ReverbParamIDs;

StringArray ReverbParameters::dreqApplyChoices()      { return {"Pre","Post","ER-Only","Tail-Only"}; }
StringArray ReverbParameters::duckModeChoices()       { return {"General","Vocal","DrumBus","Guitar","Keys"}; }
StringArray ReverbParameters::duckDetectorChoices()   { return {"Dry In","ER Only","Tail Only","Wet Sum"}; }

void ReverbParameters::addParameters (AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto norm = [](float v) { return NormalisableRange<float> (v, v, 0.0f); }; // helper when creating bools or lists

    layout.add (std::make_unique<AudioParameterBool>   (enabled,     "Enable", true));
    layout.add (std::make_unique<AudioParameterBool>   (killDry,     "Wet Only", false));

    layout.add (std::make_unique<AudioParameterFloat>  (preDelayMs,  "Pre",     NormalisableRange<float>(0.f, 200.f, 0.1f), 0.f));
    layout.add (std::make_unique<AudioParameterFloat>  (decaySec,    "Decay",   NormalisableRange<float>(0.2f, 20.f, 0.001f, 0.4f), 2.4f));
    layout.add (std::make_unique<AudioParameterFloat>  (sizePct,     "Size",    NormalisableRange<float>(10.f, 200.f, 0.1f), 100.f));

    layout.add (std::make_unique<AudioParameterFloat>  (erLevelDb,   "ER Lvl",  NormalisableRange<float>(-24.f, 6.f, 0.1f), -6.f));
    layout.add (std::make_unique<AudioParameterFloat>  (erDensityPct,"ER Den",  NormalisableRange<float>(0.f, 100.f, 0.1f), 60.f));
    layout.add (std::make_unique<AudioParameterFloat>  (erWidthPct,  "ER Wid",  NormalisableRange<float>(0.f, 200.f, 0.1f), 110.f));
    layout.add (std::make_unique<AudioParameterFloat>  (erTimeMs,    "ER Time", NormalisableRange<float>(50.f, 500.f, 0.1f), 120.f));
    layout.add (std::make_unique<AudioParameterFloat>  (erToTailPct, "ER→T",    NormalisableRange<float>(0.f, 100.f, 0.1f), 40.f));

    layout.add (std::make_unique<AudioParameterFloat>  (diffusionPct,"Diff",    NormalisableRange<float>(0.f, 100.f, 0.1f), 70.f));
    layout.add (std::make_unique<AudioParameterFloat>  (densityPct,  "Dens",    NormalisableRange<float>(0.f, 100.f, 0.1f), 65.f));

    layout.add (std::make_unique<AudioParameterFloat>  (modDepthCents,"Mod Dep",NormalisableRange<float>(0.f, 25.f, 0.01f), 3.5f));
    layout.add (std::make_unique<AudioParameterFloat>  (modRateHz,   "Mod Rate",NormalisableRange<float>(0.05f, 5.f, 0.0001f, 0.4f), 0.35f));

    layout.add (std::make_unique<AudioParameterFloat>  (widthPct,    "Width",   NormalisableRange<float>(0.f, 200.f, 0.1f), 100.f));
    layout.add (std::make_unique<AudioParameterFloat>  (rotationDeg, "Rot",     NormalisableRange<float>(-45.f, 45.f, 0.1f), 0.f));

    layout.add (std::make_unique<AudioParameterBool>   (followWidth,   "Follow W", false));
    layout.add (std::make_unique<AudioParameterFloat>  (followWidthAmt,"W Amt",   NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));
    layout.add (std::make_unique<AudioParameterBool>   (followRot,     "Follow R", false));
    layout.add (std::make_unique<AudioParameterFloat>  (followRotAmt,  "R Amt",   NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));

    layout.add (std::make_unique<AudioParameterFloat>  (wetMix01,    "Wet",     NormalisableRange<float>(0.f, 1.f, 0.001f), 0.25f));
    layout.add (std::make_unique<AudioParameterFloat>  (bloomPct,    "Bloom",   NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));
    layout.add (std::make_unique<AudioParameterFloat>  (distancePct, "Distance",NormalisableRange<float>(0.f, 100.f, 0.1f), 50.f));
    layout.add (std::make_unique<AudioParameterBool>   (freeze,      "Freeze",  false));
    layout.add (std::make_unique<AudioParameterFloat>  (shimmerAmtPct,"Shim Amt",NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));
    layout.add (std::make_unique<AudioParameterFloat>  (shimmerInt,   "Shim Int",NormalisableRange<float>(0.f, 100.f, 0.1f), 50.f));
    layout.add (std::make_unique<AudioParameterFloat>  (gateAmtPct,   "Gate",    NormalisableRange<float>(0.f, 100.f, 0.1f), 0.f));
    layout.add (std::make_unique<AudioParameterFloat>  (outTrimDb,    "Trim",    NormalisableRange<float>(-24.f, 12.f, 0.01f), 0.f));

    layout.add (std::make_unique<AudioParameterFloat>  (dreqXoverLoHz,"DREQ XO Lo", NormalisableRange<float>(80.f, 400.f, 0.1f, 0.35f), 160.f));
    layout.add (std::make_unique<AudioParameterFloat>  (dreqXoverHiHz,"DREQ XO Hi", NormalisableRange<float>(1000.f, 6000.f, 0.1f, 0.35f), 3000.f));
    layout.add (std::make_unique<AudioParameterChoice> (dreqApply,    "EQ Apply", dreqApplyChoices(), 1)); // default Post

    // Ducking (floating)
    layout.add (std::make_unique<AudioParameterBool>   (duckOn,       "Duck On",   false));
    layout.add (std::make_unique<AudioParameterChoice> (duckMode,     "Duck Mode", duckModeChoices(), 0));
    layout.add (std::make_unique<AudioParameterFloat>  (duckDepthDb,  "Duck Depth", NormalisableRange<float>(0.f, 24.f, 0.1f), 6.f));
    layout.add (std::make_unique<AudioParameterFloat>  (duckAtkMs,    "Attack",     NormalisableRange<float>(1.f, 100.f, 0.1f), 10.f));
    layout.add (std::make_unique<AudioParameterFloat>  (duckRelMs,    "Release",    NormalisableRange<float>(50.f, 2000.f, 0.1f, 0.35f), 300.f));
    layout.add (std::make_unique<AudioParameterFloat>  (duckThrDb,    "Threshold",  NormalisableRange<float>(-60.f, -6.f, 0.1f), -24.f));
    layout.add (std::make_unique<AudioParameterFloat>  (duckRatio,    "Ratio",      NormalisableRange<float>(1.f, 8.f, 0.01f), 3.f));
    layout.add (std::make_unique<AudioParameterFloat>  (duckKneeDb,   "Knee",       NormalisableRange<float>(0.f, 24.f, 0.1f), 6.f));
    layout.add (std::make_unique<AudioParameterFloat>  (duckBandHz,   "Focus Hz",   NormalisableRange<float>(50.f, 8000.f, 0.1f, 0.35f), 2000.f));
    layout.add (std::make_unique<AudioParameterFloat>  (duckBandQ,    "Focus Q",    NormalisableRange<float>(0.3f, 4.f, 0.01f, 0.35f), 1.0f));
    layout.add (std::make_unique<AudioParameterChoice> (duckDetectorSrc, "Detector", duckDetectorChoices(), 3)); // Wet Sum
}
