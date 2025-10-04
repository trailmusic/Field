#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <cassert>

namespace field { namespace tests {

inline juce::RangedAudioParameter* findParam(juce::AudioProcessorValueTreeState& apvts,
                                             const juce::String& paramID)
{
    if (auto* p = apvts.getParameter (paramID))
        return dynamic_cast<juce::RangedAudioParameter*>(p);
    return nullptr;
}

inline void setBool(juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& id, bool v)
{
    auto* p = findParam(apvts, id);
    assert(p && "param not found");
    const auto& r = p->getNormalisableRange();
    const float norm = p->convertTo0to1 (v ? r.end : r.start);
    p->beginChangeGesture();
    p->setValueNotifyingHost(norm);
    p->endChangeGesture();
}

inline void setInt(juce::AudioProcessorValueTreeState& apvts,
                   const juce::String& id, int v)
{
    auto* p = findParam(apvts, id);
    assert(p && "param not found");
    const float norm = p->convertTo0to1 ((float) v);
    p->beginChangeGesture();
    p->setValueNotifyingHost(norm);
    p->endChangeGesture();
}

inline void setFloat(juce::AudioProcessorValueTreeState& apvts,
                     const juce::String& id, float v)
{
    auto* p = findParam(apvts, id);
    assert(p && "param not found");
    const float norm = p->convertTo0to1 (v);
    p->beginChangeGesture();
    p->setValueNotifyingHost(norm);
    p->endChangeGesture();
}

}} // namespace field::tests


