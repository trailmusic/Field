#include "ReverbEngine.h"

using namespace juce;

void ReverbEngine::prepare (double sr, int maxBlock, int channels)
{
    sampleRate = sr; maxSamples = maxBlock; chans = jmax (1, channels);
    erBuf.setSize (chans, maxSamples);
    tailBuf.setSize (chans, maxSamples);
    tmpBuf.setSize (chans, maxSamples);
    dynEq.resize (chans);
    dynEqGrDb.reset (0.f);
}

void ReverbEngine::reset ()
{
}

void ReverbEngine::setParams (const ReverbParams& p)
{
    // Configure DynEQ filters per band (simple peaking for now; mode mapping todo)
    for (size_t i=0;i<p.dyneq.size();++i)
    {
        const auto& b = p.dyneq[i];
        if (!b.on) { dyneqGrDb[i].store(0.f); continue; }
        dyneqFilters[i] = Biquad::makePeaking (sampleRate, jlimit (20.0, 20000.0, (double) b.freq), jmax (0.1, (double) b.Q), b.gainDb);
        dyneqFilters[i].resize (chans);
    }
}

void ReverbEngine::processWet (AudioBuffer<float>& wet, const AudioBuffer<float>& sidechain)
{
    ignoreUnused (sidechain);
    // Stub: pass-through for now so UI can integrate; replace with ER+FDN rendering
    tailBuf.makeCopyOf (wet);
    erBuf.clear();
    // Meters
    auto rms = [] (const AudioBuffer<float>& b)
    {
        long double s = 0.0; const int ch = b.getNumChannels(), n = b.getNumSamples();
        for (int c=0;c<ch;++c){ const float* d=b.getReadPointer(c); for (int i=0;i<n;++i) s += (long double) d[i]*d[i]; }
        const double v = std::sqrt ((double) s / jmax (1, ch*n));
        return (float) v;
    };
    erRms .store (rms (erBuf));
    tailRms.store (rms (tailBuf));
    // --- Wet dynamic EQ (multi-band placeholder detector) -----------------------
    const int N = tailBuf.getNumSamples();
    const int C = tailBuf.getNumChannels();
    AudioBuffer<float> work (C, N);
    work.makeCopyOf (tailBuf);

    // Very rough per-band energy estimate: split with simple peaking filters and measure RMS
    for (size_t i=0;i<dyneqFilters.size(); ++i)
    {
        auto& f = dyneqFilters[i];
        if (f.z1.empty()) continue; // not configured
        AudioBuffer<float> band (C, N);
        band.makeCopyOf (work);
        f.processInPlace (band);
        long double s = 0.0; for (int c=0;c<C;++c){ const float* d=band.getReadPointer(c); for (int n=0;n<N;++n) s += (long double) d[n]*d[n]; }
        const float rms = std::sqrt ((double) s / jmax (1, C*N));
        // Map RMS to a crude GR for UI; engine GR computer to be refined with thr/ratio/atk/rel
        const float gr = juce::jlimit (0.f, 12.f, juce::Decibels::gainToDecibels (rms + 1.0e-6f, -120.0f) > -20.f ? 3.f : 0.f);
        dyneqGrDb[i].store (gr);
    }

    // Apply cascaded filters (static gainDb already included; dynamic GR not yet applied to audio in this scaffold)
    for (size_t i=0;i<dyneqFilters.size(); ++i)
        if (!dyneqFilters[i].z1.empty()) dyneqFilters[i].processInPlace (tailBuf);

    // Sum ER+Tail into wet
    wet.makeCopyOf (tailBuf);
    duckGrDb.store (0.f);
}


