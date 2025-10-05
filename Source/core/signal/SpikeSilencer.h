#pragma once
#include <juce_dsp/juce_dsp.h>
#include <algorithm>

// Dev-only one-shot micro-fade that arms on spikes and applies a short envelope.
// Compiled into Debug builds; no-ops in Release if call sites are guarded.
struct SpikeSilencer
{
    // Config
    float  thresh      = 0.98f;  // arm when max|x| exceeds this
    int    fadeDown    = 8;      // samples to fade 1->0
    int    hold        = 8;      // samples to hold at 0
    int    fadeUp      = 16;     // samples to fade 0->1
    int    coolDown    = 64;     // min distance between arms (samples)

    // State
    enum Phase { Idle, Down, Hold, Up, Cooling } phase = Idle;
    int pos = 0; // sample counter within current phase

    void configure(float t, int fd, int h, int fu, int cd) noexcept
    {
        thresh = t; fadeDown = fd; hold = h; fadeUp = fu; coolDown = cd;
        reset();
    }

    void reset() noexcept { phase = Idle; pos = 0; }

    template <typename Sample>
    void process(juce::dsp::AudioBlock<Sample>& block) noexcept
    {
        const int n   = (int) block.getNumSamples();
        const int chs = (int) block.getNumChannels();

        for (int i = 0; i < n; ++i)
        {
            if (phase == Idle)
            {
                Sample m = 0;
                for (int ch = 0; ch < chs; ++ch)
                    m = std::max(m, (Sample) std::abs((double) block.getChannelPointer(ch)[i]));
                if (m > (Sample) thresh) { phase = Down; pos = 0; }
            }

            float g = 1.0f;
            switch (phase)
            {
                case Down:
                    g = 1.0f - (fadeDown > 0 ? (float) pos / (float) fadeDown : 1.0f);
                    if (++pos >= fadeDown) { phase = Hold; pos = 0; }
                    break;
                case Hold:
                    g = 0.0f;
                    if (++pos >= hold) { phase = Up; pos = 0; }
                    break;
                case Up:
                    g = (fadeUp > 0 ? (float) pos / (float) fadeUp : 1.0f);
                    if (++pos >= fadeUp) { phase = Cooling; pos = 0; g = 1.0f; }
                    break;
                case Cooling:
                    g = 1.0f;
                    if (++pos >= coolDown) { phase = Idle; pos = 0; }
                    break;
                case Idle: default:
                    g = 1.0f; break;
            }

            if (g != 1.0f)
                for (int ch = 0; ch < chs; ++ch)
                {
                    auto* p = block.getChannelPointer(ch);
                    p[i] = (Sample) ((double) p[i] * g);
                }
        }
    }
};


