#pragma once
#include <JuceHeader.h>
#include "DynamicEqParamIDs.h"

namespace dynEq
{
struct BandState
{
    bool   active = false; int type=0, slope=3, channel=0, phase=1;
    float  freqHz=1000.f, gainDb=0.f, q=0.707f;
    bool   dynOn=false; int dynMode=0; float dynRangeDb=-3.f, dynThreshDb=-24.f, dynAtkMs=10.f, dynRelMs=120.f, dynHoldMs=0.f; float dynLookAheadMs=0.f;
    int    dynDetectorSrc=1; float dynDetHPHz=20.f, dynDetLPHz=20000.f, dynDetQ=0.7f; float dynTAmount=0.5f, dynSAmount=0.5f;
    bool   specOn=false; int specSelect=50, specResol=1; bool specAdaptive=true;
    int    character=0; float charAmt=0.f;
    bool   constOn=false; int constRoot=1; float constHz=110.f; int constNote=57; int constCount=6; float constSpread=25.f; juce::Array<float> constWeights; int constOddEven=0; int constTrack=1; float constGlideMs=50.f; bool constFormant=true;

    juce::ValueTree toVT (int idx) const;
    static BandState fromVT (const juce::ValueTree&);
};

struct State
{
    static constexpr int kMaxBands = 24;
    bool enabled=true; int qualityMode=1; int oversample=1; bool analyzerOn=true; int analyzerPrePost=1; bool unmaskEnable=false; int unmaskTargetBus=-1;
    BandState bands[kMaxBands];

    juce::ValueTree toVT() const;
    static State fromVT (const juce::ValueTree&);
};
}


