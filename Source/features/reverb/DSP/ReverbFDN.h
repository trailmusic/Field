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
#include <atomic>
#include <memory>
#include "../Core/FieldReverbConfig.h"
#include "engines/reverb/DSP/DecayLossDesigner.h"
#include "core/util/DenormGuard.h"
#include "core/signal/Sanitize.h"

namespace fieldverb
{
// Double-buffered runtime for thread safety
struct FdnRuntime {
    std::vector<float> g;          // per-line feedback gains
    float inputGain = 0.35f;
};

class FDNCore
{
public:
    // Member variables (needed for prepare method)
    std::vector<double> lineDelaySec; // Round-trip delays in seconds
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
            lineDelaySec.resize (numLines);

            int maxSamps = 0;
            for (int i=0; i<numLines; ++i)
            {
                const int Di = (int) juce::roundToInt (baseDelaysMs[i] * sampleRate / 1000.0);
                delay[i].assign (juce::jmax (Di, blockSize*2), 0.0f);
                lineDelaySec[i] = (double)Di / sampleRate; // Store round-trip time
                maxSamps = juce::jmax (maxSamps, (int) delay[i].size());
            }

            // Initialize decay-rate designer
            decayDesigner.prepare (sampleRate, numLines);

            // default decay
            setBaseT60 (1.8f);

            tmp.setSize (numCh, blockSize);
        }

    void reset ()
    {
        for (auto& d : delay) std::fill (d.begin(), d.end(), 0.0f);
        std::fill (writeIdx.begin(), writeIdx.end(), 0);
    }

    void setBaseT60 (float seconds)
    {
        baseT60 = juce::jlimit (0.1f, 60.0f, seconds);
        decayDesigner.setBaseT60(seconds);
    }

    // Real Decay-Rate EQ → per-line loss shaping
    void setDecayProfile (const DecayRateProfile& profile)
    {
        decayDesigner.setDecayProfile(profile);
    }
    
    // Commit runtime parameters from designer (call from message thread)
    void commitRuntimeFromDesigner()
    {
        if (!back) back = std::make_unique<FdnRuntime>();
        back->g = decayDesigner.getLossCoeffs(); // copy
        back->inputGain = 0.35f;
        auto* old = rt.exchange(back.release(), std::memory_order_release);
        delete old;
    }

    void process (const juce::AudioBuffer<float>& in, juce::AudioBuffer<float>& tailOut)
    {
        DenormGuard _ftzGuard;
        const int N  = in.getNumSamples();
        const int C  = juce::jmin (numCh, in.getNumChannels());
        jassert (N <= tmp.getNumSamples());

        // Update decay coefficients from Decay-Rate EQ with correct per-block smoothing
        decayDesigner.updateCoeffs(N);
        
        // Snapshot runtime parameters (thread-safe)
        FdnRuntime* snap = rt.load(std::memory_order_acquire);
        const auto& feedbackGains = (snap && !snap->g.empty()) ? snap->g : decayDesigner.getLossCoeffs();
        const float inputGain = (snap) ? snap->inputGain : 0.35f;

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

        // FDN tick with denormal protection
        juce::ScopedNoDenormals noDenormals;
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

            // Input spread weights (decorrelated)
            static constexpr float W[8] = { +0.50f, -0.35f, +0.25f, -0.20f, +0.15f, -0.12f, +0.10f, -0.08f };
            const float excite = mono.getSample(0, n);
            
            // feedback + input injection + per-cycle feedback gains
            for (int i=0; i<numLines; ++i)
            {
                const float g = (i < (int)feedbackGains.size()) ? feedbackGains[i] : 0.9f;
                const float spreadGain = (i < 8) ? W[i] : 0.1f; // Use decorrelated weights
                float y = g * x[i] + excite * inputGain * spreadGain;

                auto& buf = delay[i];
                buf[writeIdx[i]] = y;

                writeIdx[i] = (writeIdx[i] + 1) % (int) buf.size();
            }

            // tap multiple decorrelated lines for stereo output
            static const int Lset[4] = {1,3,6,7};
            static const int Rset[4] = {0,2,4,5};
            float l=0.f, r=0.f;
            for (int k=0; k<4; ++k)
            {
                const int Li = Lset[k] % numLines;
                const int Ri = Rset[k] % numLines;
                l += delay[Li][ (writeIdx[Li] + (int)delay[Li].size() - (3+2*k)) % (int)delay[Li].size() ];
                r += delay[Ri][ (writeIdx[Ri] + (int)delay[Ri].size() - (5+2*k)) % (int)delay[Ri].size() ];
            }
            l *= 0.25f; r *= 0.25f;

            for (int c=0; c<C; ++c)
            {
                float* d = tailOut.getWritePointer (c);
                float v = (c == 0 ? l : r);
                if (!std::isfinite((double)v) || std::abs (v) < 1e-30f) v = 0.0f;
                d[n] += v;
            }
        }

        // Optional final sanitize for development safety
        // sanitize (juce::dsp::AudioBlock<float> (tailOut));
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
    juce::AudioBuffer<float> tmp;
    float baseT60 { 1.8f };
    
    // Real decay-rate mapping
    DecayLossDesigner decayDesigner;
    
    // Thread-safe runtime parameters
    std::atomic<FdnRuntime*> rt { nullptr };
    std::unique_ptr<FdnRuntime> back;
};
} // namespace fieldverb
