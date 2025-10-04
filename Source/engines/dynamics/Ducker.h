#pragma once
#include <cmath>
#include <vector>
#include <algorithm>

namespace fielddsp {

struct DuckParams
{
    float thresholdDb   = -18.0f;
    float kneeDb        = 6.0f;
    float ratio         = 6.0f;
    float attackMs      = 12.0f;
    float releaseMs     = 180.0f;
    float lookaheadMs   = 5.0f;
    float rmsMs         = 15.0f;
    float maxDepthDb    = 0.0f;
    bool  bypass        = false;
};

template <typename Sample>
class Ducker
{
public:
    void prepare (double sampleRate, int maxBlock, int maxLookaheadMs = 24)
    {
        sr = sampleRate; blockMax = std::max (1, maxBlock);
        const int maxDelaySamps = (int) std::ceil (sr * maxLookaheadMs * 0.001);
        delayLen = std::max (1, maxDelaySamps);
        writeIdx = 0;

        delayL.assign ((size_t) delayLen + (size_t) blockMax + 8, (Sample)0);
        delayR.assign ((size_t) delayL.size(),                   (Sample)0);

        env = 0.0; grDbSmoothed = 0.0;

        updateTimeConstants();
        setParams (params);
        reset();
    }

    // Returns the most recent smoothed gain reduction in dB (>= 0)
    Sample getCurrentGainReductionDb() const noexcept { return grDbSmoothed; }

    // Returns the last lookahead-smeared linear gain applied (0..1)
    Sample getCurrentLinearGain() const noexcept { return db2lin ((Sample) -grDbSmoothed); }

    void reset()
    {
        std::fill (delayL.begin(), delayL.end(), (Sample)0);
        std::fill (delayR.begin(), delayR.end(), (Sample)0);
        env = 0.0; grDbSmoothed = 0.0;
    }

    void setParams (const DuckParams& p)
    {
        params = p;
        updateTimeConstants();
        setLookahead (params.lookaheadMs);
    }

    void setLookahead (float ms)
    {
        params.lookaheadMs = std::max (0.0f, ms);
        lookaheadSamps = (int) std::round (params.lookaheadMs * (float) sr * 0.001f);
        lookaheadSamps = std::clamp (lookaheadSamps, 0, (int) delayL.size() - 1);
    }

    // Process wet audio in-place with look-ahead; sidechain is stereo (or mono if sidechainR=null)
    void processWet (Sample* wetL, Sample* wetR, const Sample* sidechainL, const Sample* sidechainR, int numSamples)
    {
        if (params.bypass || params.maxDepthDb <= (Sample)0.0001)
            return writeThrough (wetL, wetR, numSamples);

        // Push wet into delay lines; read delayed version to apply gain on.
        delayedBlockL.clear(); delayedBlockR.clear();
        delayedBlockL.reserve ((size_t) numSamples);
        delayedBlockR.reserve ((size_t) numSamples);
        for (int i = 0; i < numSamples; ++i)
        {
            const size_t w = (size_t) (writeIdx % (int) delayL.size());
            delayL[w] = wetL[i];
            delayR[w] = wetR[i];
            ++writeIdx;

            const int idx = (int) w - lookaheadSamps;
            const size_t r = (size_t) (idx >= 0 ? idx : idx + (int) delayL.size());
            delayedBlockL.push_back (delayL[r]);
            delayedBlockR.push_back (delayR[r]);
        }

        for (int i = 0; i < numSamples; ++i)
        {
            const Sample scL = sidechainL ? sidechainL[i] : (Sample)0;
            const Sample scR = sidechainR ? sidechainR[i] : scL;
            const Sample mono = (std::abs (scL) + std::abs (scR)) * (Sample)0.5;

            const Sample x2 = mono * mono;
            env = (Sample)rms_a * env + (Sample)(1.0 - rms_a) * x2;
            const Sample rms = (Sample) std::sqrt ((double) env + 1e-20);

            const Sample inDb = lin2db (rms);
            Sample grDb = compGainReductionDb (inDb);
            grDb = std::min (grDb, (Sample) params.maxDepthDb);

            const double a = (grDb > grDbSmoothed) ? atk_a : rel_a;
            grDbSmoothed = (Sample)(a * grDbSmoothed + (1.0 - a) * grDb);

            const Sample g = db2lin ((Sample) -grDbSmoothed);

            delayedBlockL[i] *= g;
            delayedBlockR[i] *= g;
        }

        for (int i = 0; i < numSamples; ++i)
        {
            wetL[i] = delayedBlockL[i];
            wetR[i] = delayedBlockR[i];
        }
    }

private:
    DuckParams params;

    double sr = 48000.0;
    int    blockMax = 0;

    // Detector smoothing
    double rms_a = 0.0;
    double atk_a = 0.0;
    double rel_a = 0.0;

    // Lookahead delay
    std::vector<Sample> delayL, delayR;
    int delayLen = 0;
    int writeIdx = 0;
    int lookaheadSamps = 0;

    // Working buffers
    std::vector<Sample> delayedBlockL, delayedBlockR;

    // Detector state
    Sample env = 0;
    Sample grDbSmoothed = 0;

    void updateTimeConstants()
    {
        auto ms2a = [this](double ms)
        {
            const double T = std::max (1.0, ms) * 0.001;
            return std::exp (-1.0 / (T * sr));
        };
        rms_a = ms2a (params.rmsMs);
        atk_a = ms2a (params.attackMs);
        rel_a = ms2a (params.releaseMs);
        delayedBlockL.reserve ((size_t) blockMax + 8);
        delayedBlockR.reserve ((size_t) blockMax + 8);
    }

    inline static Sample db2lin (Sample db) noexcept
    {
        return (Sample) std::pow (10.0, (double) db * 0.05);
    }
    inline static Sample lin2db (Sample lin) noexcept
    {
        const double x = std::max (1e-12, (double) lin);
        return (Sample) (20.0 * std::log10 (x));
    }

    inline static Sample lerp (Sample a, Sample b, Sample t) noexcept { return a + (b - a) * t; }

    // Soft-knee downward compressor curve: returns GR dB ≥ 0
    Sample compGainReductionDb (Sample inDb) const noexcept
    {
        const Sample thr = (Sample) params.thresholdDb;
        const Sample R   = (Sample) std::max (1.0f, params.ratio);
        const Sample knee= (Sample) std::max (0.0f, params.kneeDb);

        if (knee <= (Sample) 1e-6)
        {
            if (inDb <= thr) return (Sample) 0;
            const Sample over = inDb - thr;
            const Sample outDb = thr + over / R;
            return inDb - outDb;
        }
        const Sample kneeHalf = knee * (Sample)0.5;
        if (inDb <= thr - kneeHalf) return (Sample) 0;
        if (inDb >= thr + kneeHalf)
        {
            const Sample over = inDb - thr;
            const Sample outDb = thr + over / R;
            return inDb - outDb;
        }
        const Sample x = inDb - (thr - kneeHalf);            // 0..knee
        const Sample y = x * x / (knee * (Sample)4.0);       // 0..1 approx
        const Sample softIn  = (thr - kneeHalf) + x;         // = inDb
        const Sample hardOut = thr + (softIn - thr) / R;
        const Sample softOut = lerp (softIn, hardOut, y);
        return inDb - softOut;
    }

    void writeThrough (Sample* wetL, Sample* wetR, int n)
    {
        for (int i = 0; i < n; ++i)
        {
            const size_t w = (size_t) (writeIdx % (int) delayL.size());
            delayL[w] = wetL[i];
            delayR[w] = wetR[i];
            ++writeIdx;

            const int idx = (int) w - lookaheadSamps;
            const size_t r = (size_t) (idx >= 0 ? idx : idx + (int) delayL.size());
            wetL[i] = delayL[r];
            wetR[i] = delayR[r];
        }
    }
};

} // namespace fielddsp


