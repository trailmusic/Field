#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

inline void copyDoubleToFloat (juce::AudioBuffer<double>& src, juce::AudioBuffer<float>& dst)
{
    jassert (dst.getNumChannels() >= src.getNumChannels());
    jassert (dst.getNumSamples()  >= src.getNumSamples());
    const int chs = src.getNumChannels();
    const int n   = src.getNumSamples();
    for (int ch = 0; ch < chs; ++ch)
    {
        auto* s = src.getReadPointer (ch);
        auto* d = dst.getWritePointer (ch);
        for (int i = 0; i < n; ++i)
            d[i] = static_cast<float>(s[i]);
    }
}

inline void copyFloatToDouble (juce::AudioBuffer<float>& src, juce::AudioBuffer<double>& dst)
{
    jassert (dst.getNumChannels() >= src.getNumChannels());
    jassert (dst.getNumSamples()  >= src.getNumSamples());
    const int chs = src.getNumChannels();
    const int n   = src.getNumSamples();
    for (int ch = 0; ch < chs; ++ch)
    {
        auto* s = src.getReadPointer (ch);
        auto* d = dst.getWritePointer (ch);
        for (int i = 0; i < n; ++i)
            d[i] = static_cast<double>(s[i]);
    }
}
