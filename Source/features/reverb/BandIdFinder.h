#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

struct BandIdFinder
{
    static juce::StringArray findEnabledIds (juce::AudioProcessorValueTreeState& apvts,
                                             const juce::String& prefix,
                                             const juce::String& suffix)
    {
        juce::StringArray out;
        
        // Get all parameters from the APVTS
        auto& tree = apvts.state;
        for (auto child : tree)
        {
            if (child.hasType("PARAM"))
            {
                auto id = child.getProperty("id").toString();
                if (id.startsWith (prefix) && id.endsWith (suffix))
                    out.add (id);
            }
        }
        
        out.removeDuplicates (true);
        out.sort (true);
        return out;
    }
};
