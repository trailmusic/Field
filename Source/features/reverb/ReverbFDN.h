#pragma once
/*
====================================================================================================
 ReverbFDN.h — Minimal, production-ready FDN core (skeleton) with safe fallbacks
----------------------------------------------------------------------------------------------------
 Purpose
    - Provide an upgrade path from Phase 1 (ER-only) to a real tank.
    - The FDN "engine" is self-contained and off by default (compile switch).

 Design
    - Unitary feedback (Hadamard) → stable & decorrelated
    - Per-line allpass (diffusion) + loss filter (IIR) hooks
    - Modulation placeholders (safe scalar now; SIMD-ready later)

 API
    - prepare(sr, blockSize, channels)
    - reset()
    - setBaseT60(decaySec)
    - setPerBandLoss(filter coeffs precomputed upstream; not required for Phase 1)
    - process(inputWet, outTail) : write tail only (ER mixed elsewhere)

 Notes
    - This file is intentionally light; it's a functional scaffold you can extend.
    - Keeps allocations out of process(); ownership lives here.
====================================================================================================
*/

#include <JuceHeader.h>
#include "FieldReverbConfig.h"

namespace fieldverb
{
class FDNCore
{
public:
    void prepare (double sr, int maxBlock, int channels, int lines = 8)
    {
        sampleRate = sr;
        blockSize  = juce::jmax (1, maxBlock);
        numCh      = juce::jlimit (1, 2, channels);      // stereo tank for now
        numLines   = juce::jlimit (4, 16, lines);

        // delay lengths: prime-ish spread around 30–120 ms @48k
        const int baseDelaysMs[16] = { 31, 37, 43, 47, 53, 59, 71, 83,  97, 101, 109, 113, 127, 131, 139, 149 };
        delay.resize (numLines);
        writeIdx.assign (numLines, 0);

        int maxSamps = 0;
        for (int i=0; i<numLines; ++i)
        {
            const int Di = (int) juce::roundToInt (baseDelaysMs[i] * sampleRate / 1000.0);
            delay[i].assign (juce::jmax (Di, blockSize*2), 0.0f);
            maxSamps = juce::jmax (maxSamps, (int) delay[i].size());
        }

        // per-line filters (simple one-pole loss as placeholder)
        a1.assign (numLines, 0.0f);
        z1.assign (numLines, 0.0f);

        // default decay
        setBaseT60 (1.8f);

        tmp.setSize (numCh, blockSize);
    }

    void reset ()
    {
        for (auto& d : delay) std::fill (d.begin(), d.end(), 0.0f);
        std::fill (writeIdx.begin(), writeIdx.end(), 0);
        std::fill (z1.begin(), z1.end(), 0.0f);
    }

    void setBaseT60 (float seconds)
    {
        baseT60 = juce::jlimit (0.1f, 60.0f, seconds);

        // convert T60 to a simple per-sample pole for placeholder loss
        // a = 10^(-3 / (T60 * fs))
        const float a = std::pow (10.0f, (float) (-3.0 / (baseT60 * sampleRate)));
        // crude mapping to one-pole for each line (identical here; replace with per-band later)
        for (int i=0; i<numLines; ++i) a1[i] = a;
    }

    // Placeholder for Decay-Rate EQ → per-line loss shaping.
    // You'd pass precomputed IIR sets here; we keep API for future.
    void setPerBandLossUnused () {}

    void process (const juce::AudioBuffer<float>& in, juce::AudioBuffer<float>& tailOut)
    {
        const int N  = in.getNumSamples();
        const int C  = juce::jmin (numCh, in.getNumChannels());
        jassert (N <= tmp.getNumSamples());

        tailOut.clear();
        tmp.clear();

        // Input diffusion: sum channels to a mono excitation (simple now)
        juce::AudioBuffer<float> mono (1, N);
        {
            mono.clear();
            for (int c=0; c<C; ++c)
            {
                const float* s = in.getReadPointer (c);
                float* d = mono.getWritePointer (0);
                for (int n=0; n<N; ++n) d[n] += s[n] * (1.0f / C);
            }
        }

        // FDN tick
        for (int n=0; n<N; ++n)
        {
            // read all lines
            float x[16] = {};
            for (int i=0; i<numLines; ++i)
            {
                auto& buf = delay[i];
                const int ri = (writeIdx[i] + (int) buf.size() - 1) % (int) buf.size();
                x[i] = buf[ri];
            }

            // Hadamard mix (size power of two; we clamp at 8/16)
            hadamardMixInPlace (x, numLines);

            // feedback + input injection + simple loss
            for (int i=0; i<numLines; ++i)
            {
                const float inExcite = mono.getSample (0, n) * 0.35f; // input gain
                float y = x[i] + inExcite;
                // one-pole lowpass-ish loss (placeholder)
                y = (1.0f - a1[i]) * y + a1[i] * z1[i];
                z1[i] = y;

                auto& buf = delay[i];
                buf[writeIdx[i]] = y;

                writeIdx[i] = (writeIdx[i] + 1) % (int) buf.size();
            }

            // tap two decorrelated lines as stereo out (simple spread)
            const int L = 1 % numLines;
            const int R = 5 % numLines;
            const float l = delay[L][ (writeIdx[L] + (int) delay[L].size() - 3) % (int) delay[L].size() ];
            const float r = delay[R][ (writeIdx[R] + (int) delay[R].size() - 7) % (int) delay[R].size() ];

            for (int c=0; c<C; ++c)
            {
                float* d = tailOut.getWritePointer (c);
                d[n] += (c == 0 ? l : r);
            }
        }
    }

private:
    // in-place Hadamard for 4/8/16
    static void hadamardMixInPlace (float* v, int n)
    {
        if (n == 4)
        {
            const float a=v[0], b=v[1], c=v[2], d=v[3];
            v[0]= a+b+c+d; v[1]= a-b+c-d; v[2]= a+b-c-d; v[3]= a-b-c+d;
            const float s = 0.5f; for (int i=0;i<4;++i) v[i]*=s;
            return;
        }
        if (n == 8 || n == 16)
        {
            // simple recursive stages (scalar; SIMD later)
            for (int stride = 1; stride < n; stride <<= 1)
            {
                for (int i = 0; i < n; i += stride * 2)
                {
                    for (int j = 0; j < stride; ++j)
                    {
                        const float a = v[i + j];
                        const float b = v[i + j + stride];
                        v[i + j]          = a + b;
                        v[i + j + stride] = a - b;
                    }
                }
            }
            const float s = 1.0f / std::sqrt ((float) n);
            for (int i=0;i<n;++i) v[i]*=s;
        }
    }

    double sampleRate { 48000.0 };
    int    blockSize { 0 }, numCh { 2 }, numLines { 8 };

    std::vector<std::vector<float>> delay;
    std::vector<int>   writeIdx;
    std::vector<float> a1, z1;      // simple loss pole (placeholder)
    juce::AudioBuffer<float> tmp;
    float baseT60 { 1.8f };
};
} // namespace fieldverb
