#include "ReverbEngine.h"

using namespace juce;

void ReverbEngine::prepare (double sr, int maxBlock, int channels)
{
    sampleRate = sr; maxSamples = maxBlock; chans = jmax (1, channels);
    erBuf.setSize (chans, maxSamples);
    tailBuf.setSize (chans, maxSamples);
    tmpBuf.setSize (chans, maxSamples);
}

void ReverbEngine::reset ()
{
}

void ReverbEngine::setParams (const ReverbParams& p)
{
    ignoreUnused (p);
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
    // Sum ER+Tail into wet
    wet.makeCopyOf (tailBuf);
    duckGrDb.store (0.f);
}


