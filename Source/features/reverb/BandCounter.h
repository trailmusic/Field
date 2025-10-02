#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

/**
 * BandCounter
 * ------------
 * Counts how many boolean "active" params are ON among a provided list of IDs.
 * IMPORTANT: parameterChanged can be called from the audio thread.
 * We marshal UI callbacks to the message thread using AsyncUpdater.
 */
class BandCounter
    : private juce::AudioProcessorValueTreeState::Listener
    , private juce::AsyncUpdater
{
public:
    BandCounter (juce::AudioProcessorValueTreeState& state,
                 juce::StringArray ids,
                 std::function<void (int)> onCountChanged)
        : apvts (state),
          paramIds (std::move (ids)),
          onChanged (std::move (onCountChanged))
    {
        for (const auto& id : paramIds)
            apvts.addParameterListener (id, this);

        // Start in a neutral state; caller can call prime() after UI is ready.
        count.store (0);
        pending.store (0);
    }

    ~BandCounter() override
    {
        cancelPendingUpdate(); // ensure no queued callbacks fire post-destruction
        for (const auto& id : paramIds)
            apvts.removeParameterListener (id, this);
    }

    /** Force an initial publish (e.g., 0) on the message thread. */
    void prime()
    {
        // Push current computed value (or 0 if not ready) to UI even if unchanged
        const int c = computeCount();
        pending.store (c, std::memory_order_release);
        hasEverPublished.store (true, std::memory_order_release);
        triggerAsyncUpdate();
    }

    /** Manual recompute; safe on any thread. */
    void refresh()
    {
        const int c = computeCount();
        const int prev = count.exchange (c, std::memory_order_acq_rel);
        if (prev != c || !hasEverPublished.load (std::memory_order_acquire))
        {
            pending.store (c, std::memory_order_release);
            hasEverPublished.store (true, std::memory_order_release);
            triggerAsyncUpdate();
        }
    }

private:
    // APVTS listener (may be audio thread)
    void parameterChanged (const juce::String&, float) override
    {
        refresh();
    }

    // AsyncUpdater (message thread)
    void handleAsyncUpdate() override
    {
        if (onChanged)
        {
            const int c = pending.load (std::memory_order_acquire);
            onChanged (c); // safe to touch UI here
        }
    }

    int computeCount() const
    {
        int c = 0;
        for (const auto& id : paramIds)
        {
            // Treat missing params as OFF
            if (auto* v = apvts.getRawParameterValue (id))
                if (v->load() > 0.5f) ++c;
        }
        return c;
    }

    juce::AudioProcessorValueTreeState& apvts;
    juce::StringArray paramIds;
    std::function<void (int)> onChanged;

    std::atomic<int>  count   { 0 }; // last published
    std::atomic<int>  pending { 0 }; // next to publish
    std::atomic<bool> hasEverPublished { false };
};
