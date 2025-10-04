#pragma once
// Moved from features/reverb/DSP/ReverbFDN.h
// Single source of truth lives here.

#include <JuceHeader.h>
#include <atomic>
#include <memory>
#include "features/reverb/Core/FieldReverbConfig.h"
#include "engines/reverb/DSP/DecayLossDesigner.h"
#include "core/util/DenormGuard.h"
#include "core/signal/Sanitize.h"
#include <algorithm>

namespace fieldverb
{
struct FdnRuntime { std::vector<float> g; float inputGain = 0.35f; };

class FDNCore
{
public:
    template <typename T>
    static inline void normalizeMatrixL1 (T** A, int M, double safety = 0.98) noexcept
    {
        double maxRowSum = 0.0;
        for (int r=0; r<M; ++r)
        {
            double s = 0.0;
            for (int c=0; c<M; ++c) s += std::abs((double)A[r][c]);
            if (s > maxRowSum) maxRowSum = s;
        }
        if (maxRowSum > 0.0 && maxRowSum > safety)
        {
            const double scale = safety / maxRowSum;
            for (int r=0; r<M; ++r)
                for (int c=0; c<M; ++c)
                    A[r][c] = (T)((double)A[r][c] * scale);
        }
    }

    std::vector<double> lineDelaySec;

    void prepare (double sr, int maxBlock, int channels, int lines = 8)
    {
        sampleRate = sr;
        blockSize  = juce::jmax (1, maxBlock);
        numCh      = juce::jlimit (1, 2, channels);
        numLines   = juce::jlimit (4, 16, lines);

        const int baseDelaysMs[16] = { 31, 37, 43, 47, 53, 59, 71, 83,  97, 101, 109, 113, 127, 131, 139, 149 };
        delay.resize (numLines);
        writeIdx.assign (numLines, 0);
        lineDelaySec.resize (numLines);

        for (int i=0; i<numLines; ++i)
        {
            const int Di = (int) juce::roundToInt (baseDelaysMs[i] * sampleRate / 1000.0);
            const int L = juce::jmax (Di, blockSize*2);
            delay[i].assign (L + kPad, 0.0f);
            lineDelaySec[i] = (double)Di / sampleRate;
        }

        decayDesigner.prepare (sampleRate, numLines);
        setBaseT60 (1.8f);
        tmp.setSize (numCh, blockSize);
        feedback_.prepare(sampleRate, 10.0);
        feedback_.setTarget(1.0);
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

    void setDecayProfile (const DecayRateProfile& profile) { decayDesigner.setDecayProfile(profile); }

    void commitRuntimeFromDesigner()
    {
        if (!back) back = std::make_unique<FdnRuntime>();
        back->g = decayDesigner.getLossCoeffs();
        back->inputGain = 0.35f;
        auto* old = rt.exchange(back.release(), std::memory_order_release);
        delete old;
    }

    void process (const juce::AudioBuffer<float>& in, juce::AudioBuffer<float>& tailOut)
    {
#if JUCE_DEBUG
        auto dbgOnce = [](bool cond, const char* msg){ if (!cond) { DBG(msg); jassertfalse; } };
#endif
        DenormGuard _ftzGuard;
        const int N  = in.getNumSamples();
        const int C  = juce::jmin (numCh, in.getNumChannels());
        jassert (N <= tmp.getNumSamples());

        decayDesigner.updateCoeffs(N);
        FdnRuntime* snap = rt.load(std::memory_order_acquire);
        const auto& feedbackGains = (snap && !snap->g.empty()) ? snap->g : decayDesigner.getLossCoeffs();
        const float inputGain = (snap) ? snap->inputGain : 0.35f;

        tailOut.clear(); tmp.clear();

        juce::AudioBuffer<float> mono (1, N);
        mono.clear();
        for (int c=0; c<C; ++c)
        {
            const float* s = in.getReadPointer (c);
            float* d = mono.getWritePointer (0);
            for (int n=0; n<N; ++n) d[n] += s[n] * (1.0f / C);
        }

        juce::ScopedNoDenormals noDenormals;
        for (int n=0; n<N; ++n)
        {
            float x[16] = {};
            for (int i=0; i<numLines; ++i)
            {
                auto& buf = delay[i];
                const int logical = logicalLen(buf);
                const int ri = wrappedRead(writeIdx[i], 1, logical);
#if JUCE_DEBUG
                dbgOnce(ri >= 0 && ri < logical, "FDN: read index out of range");
#endif
                x[i] = buf[(size_t)ri];
            }

            hadamardMixInPlace (x, numLines);
            static constexpr float W[8] = { +0.50f, -0.35f, +0.25f, -0.20f, +0.15f, -0.12f, +0.10f, -0.08f };
            const float excite = mono.getSample(0, n);
            const float fbScalar = feedback_.tick();

            for (int i=0; i<numLines; ++i)
            {
                const float g = (i < (int)feedbackGains.size()) ? feedbackGains[i] : 0.9f;
                const float spreadGain = (i < 8) ? W[i] : 0.1f;
                float y = (fbScalar * g) * x[i] + excite * inputGain * spreadGain;

                auto& buf = delay[i];
                const int logical = logicalLen(buf);
#if JUCE_DEBUG
                dbgOnce(writeIdx[i] >= 0 && writeIdx[i] < logical, "FDN: write index out of range (pre)");
#endif
                buf[(size_t)writeIdx[i]] = y;
                const int prev = writeIdx[i];
                incWrite(writeIdx[i], logical);
                if (writeIdx[i] == 0 && prev != 0) postWrapPad(buf);
#if JUCE_DEBUG
                dbgOnce(writeIdx[i] >= 0 && writeIdx[i] < logical, "FDN: write index out of range (post)");
#endif
            }

            static const int Lset[4] = {1,3,6,7};
            static const int Rset[4] = {0,2,4,5};
            float l=0.f, r=0.f;
            for (int k=0; k<4; ++k)
            {
                const int Li = Lset[k] % numLines;
                const int Ri = Rset[k] % numLines;
                const int lenL = logicalLen(delay[Li]);
                const int lenR = logicalLen(delay[Ri]);
                l += delay[Li][ wrappedRead(writeIdx[Li], (3+2*k), lenL) ];
                r += delay[Ri][ wrappedRead(writeIdx[Ri], (5+2*k), lenR) ];
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

        juce::dsp::AudioBlock<float> wetBlock (tailOut);
        postWetBus (wetBlock);
    }

private:
    static constexpr int kSimdWidth = 4;
    static constexpr int kPad       = kSimdWidth - 1;

    static inline int logicalLen (const std::vector<float>& v) noexcept { return (int)v.size() - kPad; }
    static inline void postWrapPad (std::vector<float>& v) noexcept
    {
        const int len = logicalLen(v);
        if (len <= 0) return;
        for (int i = 0; i < kPad; ++i) v[(size_t)len + (size_t)i] = v[(size_t)i];
    }
    static inline void incWrite (int& w, int len) noexcept { if (++w >= len) w = 0; }
    static inline int  wrappedRead (int w, int tap, int len) noexcept { int r = w - tap; if (r < 0) r += len; return r; }

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
    std::vector<std::vector<float>> delay; std::vector<int> writeIdx; juce::AudioBuffer<float> tmp; float baseT60 { 1.8f };
    DecayLossDesigner decayDesigner;
    std::atomic<FdnRuntime*> rt { nullptr }; std::unique_ptr<FdnRuntime> back;
    struct SmoothedScalar { void prepare(double sr,double ms){sr_=sr;setTimeMs(ms);z_=target_;} void setTimeMs(double ms){alpha_=std::exp(-1.0/(std::max(1.0,sr_)*(ms*0.001)));} void setTarget(double v){ target_=juce::jlimit(0.0,0.999,v);} float tick(){ z_= (float)(target_ + alpha_ * (z_ - target_)); return z_; } double sr_{48000.0},alpha_{0.0},target_{1.0},z_{1.0}; } feedback_;

    template <typename Sample>
    inline void postWetBus (juce::dsp::AudioBlock<Sample>& block)
    {
        // Optional DC block / soft clip hooks go here (disabled by default)
        juce::ignoreUnused(block);
    }
};
}
