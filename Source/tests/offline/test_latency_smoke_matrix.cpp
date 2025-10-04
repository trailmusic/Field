#include "modules/FieldChain.h"
#include "modules/FieldParamHooks.h"
#include "core/telemetry/LatencyProbe.h"
#include "tests/offline/TestUtils_Golden.h"
#include "tests/offline/TestUtils_Params.h"
#include <juce_dsp/juce_dsp.h>
#include <cassert>
#include <vector>
#include <cstring>

using field::modules::FieldChain;
using field::modules::applyLatencyFromSnapshot;
using field::tests::makeDeterministicInput;
using field::tests::monoToStereo;
using field::tests::hashAudio;
using field::params::ChainParamSnapshot;

template<typename Sample>
static int probeLatency(FieldChain& chain, double sr, int maxBlock, int chans)
{
    chain.prepare(sr, maxBlock, chans);
    return field::core::telemetry::LatencyProbe::measure<Sample>(
        maxBlock,
        [&chain](juce::dsp::AudioBlock<Sample>& in, juce::dsp::AudioBlock<Sample>& out)
        {
            chain.process(in);
            out.copyFrom(in);
        });
}

int main()
{
    const int sampleRates[] = { 44100, 48000, 96000 };
    const int blockSizes[]  = { 64, 128, 512 };
    const int firHalfLens[] = { 0, 64, 256 };
    const float lookAheadMs[] = { 0.f, 1.5f };
    const int osFactors[] = { 1, 2 };

    const size_t N = 48000;
    auto mono = makeDeterministicInput<float>(N, 0x5M0KEu);
    std::vector<float> inL, inR; monoToStereo(mono, inL, inR);
    const uint64_t inHash = hashAudio(inL);

    for (int sr : sampleRates)
    for (int bs : blockSizes)
    for (int firHL : firHalfLens)
    for (float la : lookAheadMs)
    for (int os : osFactors)
    {
        ChainParamSnapshot snap = field::tests::makeSnap(
            /*delay*/ true, /*dynEQ*/ true, /*reverb*/ true,
            os, /*linear*/ true, firHL, /*dyneq*/ la, /*delay*/ la
        );

        FieldChain chain;
        FieldChain::Config cfg{}; cfg.enableDelay = cfg.enableDynEq = cfg.enableReverb = true;
        chain.setConfig(cfg); chain.buildFromConfig();
        applyLatencyFromSnapshot(chain, snap, (double)sr);

        const int laSamples = (la <= 0.f) ? 0 : (int) ((la/1000.0) * sr + 0.5);
        const int expected = firHL + laSamples + laSamples;
        assert(chain.latencySamples() == expected);

        const int measured = probeLatency<float>(chain, (double)sr, bs, 2);
        assert(measured == expected);

        FieldChain verify; verify.setConfig(cfg); verify.buildFromConfig(); applyLatencyFromSnapshot(verify, snap, (double)sr);
        verify.prepare((double)sr, bs, 2);

        std::vector<float> outL(N), outR(N);
        size_t i = 0;
        while (i < N)
        {
            const int n = (int) std::min<size_t>(bs, N - i);
            std::vector<float> blkL(n), blkR(n);
            for (int k=0;k<n;++k){ blkL[k]=inL[i+k]; blkR[k]=inR[i+k]; }
            float* chansPtr[2] = { blkL.data(), blkR.data() };
            juce::dsp::AudioBlock<float> buf(chansPtr, 2, (size_t)n);
            verify.process(buf);
            for (int k=0;k<n;++k){ outL[i+k]=blkL[k]; outR[i+k]=blkR[k]; }
            i += n;
        }
        assert(hashAudio(outL) == inHash);
        assert(hashAudio(outR) == inHash);
    }

    return 0;
}


