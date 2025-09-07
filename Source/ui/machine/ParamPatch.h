#pragma once
#include <JuceHeader.h>

struct ParamPatch
{
    juce::String paramID;
    float current { 0.0f };
    float target  { 0.0f };
    float confidence { 1.0f }; // 0..1
    juce::String rationale;
};


