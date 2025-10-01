#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

class BandCounter : private juce::AudioProcessorValueTreeState::Listener
{
public:
    BandCounter (juce::AudioProcessorValueTreeState& s,
                 const juce::StringArray& ids,
                 std::function<void(int)> onCountChanged)
        : apvts (s), paramIds (ids), onChanged (std::move(onCountChanged))
    {
        for (auto& id : paramIds) apvts.addParameterListener (id, this);
        refresh(); // initial
    }

    ~BandCounter() override
    {
        for (auto& id : paramIds) apvts.removeParameterListener (id, this);
    }

    void refresh()
    {
        int c = 0;
        for (auto& id : paramIds)
            if (auto* v = apvts.getRawParameterValue (id); v && v->load() > 0.5f) ++c;

        if (count.exchange (c) != c && onChanged) onChanged (c);
    }

private:
    void parameterChanged (const juce::String&, float) override { refresh(); }

    juce::AudioProcessorValueTreeState& apvts;
    juce::StringArray paramIds;
    std::function<void(int)> onChanged;
    std::atomic<int> count { 0 };
};
