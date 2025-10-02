#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * BandIdFinder
 * ------------
 * Scans APVTS.state for parameter ValueTree nodes and returns IDs that match
 * a (prefix, suffix) pattern. Robust to empty prefix/suffix.
 *
 * NOTE: This uses APVTS's public .state. While JUCE currently tags params as
 * "PARAM" with an "id" property, treat that as an implementation detail.
 * If you ever swap away from APVTS or JUCE changes internals, consider
 * maintaining an explicit registry of your band IDs instead.
 */
struct BandIdFinder
{
    // Deterministic builder: "tb_active_0..3" or "db_active_0..2"
    static juce::StringArray makeIndexedIds (const juce::String& base, int count)
    {
        juce::StringArray out;
        out.ensureStorageAllocated (count);
        for (int i = 0; i < count; ++i)
            out.add (base + "_" + juce::String (i));
        return out;
    }

    // Optional: keep your APVTS scan (but be strict about suffix/prefix).
    static juce::StringArray findEnabledIds (juce::AudioProcessorValueTreeState& apvts,
                                             const juce::String& prefix,
                                             const juce::String& suffix)
    {
        juce::StringArray out;

        const auto tree = apvts.state;
        const auto paramTag = juce::Identifier ("PARAM");
        const bool checkPrefix = prefix.isNotEmpty();
        const bool checkSuffix = suffix.isNotEmpty();

        // Iterate children of the APVTS state
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            const auto child = tree.getChild (i);
            if (! child.hasType (paramTag))
                continue;

            const auto idVar = child.getProperty ("id");
            if (! idVar.isString())
                continue;

            const juce::String id = idVar.toString();

            const bool prefixOK = (!checkPrefix) || id.startsWith (prefix);
            const bool suffixOK = (!checkSuffix) || id.endsWith (suffix);

            if (prefixOK && suffixOK)
                out.add (id);
        }

        out.removeDuplicates (true);
        out.sort (true);
        return out;
    }
};
