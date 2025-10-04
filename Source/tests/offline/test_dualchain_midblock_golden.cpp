#include "modules/FieldDualChain.h"
#include "modules/FieldChain.h"
#include "core/signal/Warmup.h"
#include "tests/offline/TestUtils_Golden.h"
#include <juce_dsp/juce_dsp.h>
#include <cassert>
#include <cstring>

using namespace field::tests;
using field::modules::DualChain;
using field::modules::FieldChain;

static void render(DualChain& dual, double sr, int block, int chans,
                   const std::vector<float>& inL, const std::vector<float>& inR,
                   std::vector<float>& outL, std::vector<float>& outR)
{
    const size_t N = inL.size();
    outL.assign(N, 0.f); outR.assign(N, 0.f);
    dual.buildStaging(sr, block, chans);

    size_t i = 0;
    while (i < N) {
        const int n = (int) std::min<size_t>(block, N - i);
        std::vector<float> blkL(n), blkR(n);
        for (int k=0;k<n;++k) { blkL[k] = inL[i+k]; blkR[k] = inR[i+k]; }
        float* chansPtr[2] = { blkL.data(), blkR.data() };
        juce::dsp::AudioBlock<float> buf(chansPtr, (size_t)chans, (size_t)n);

        dual.process(buf);

        for (int k=0;k<n;++k) { outL[i+k] = blkL[k]; outR[i+k] = blkR[k]; }
        i += n;
    }
}

int main()
{
    const double sr = 44100.0;
    const int chans = 2;
    const size_t total = 44100 + 100;

    auto mono = makeDeterministicInput<float>(total, 0x51APu);
    std::vector<float> inL, inR; monoToStereo(mono, inL, inR);

    DualChain dual;
    FieldChain::Config cfg{};
    dual.activeChain().setConfig(cfg);   dual.activeChain().buildFromConfig();
    dual.stagingChain().setConfig(cfg);  dual.stagingChain().buildFromConfig();

    const int blockSizes[] = { 96, 144, 192 };
    const int offsetSamples = 37;
    const int warmupBlocks  = 2;

    std::vector<uint64_t> hashes;
    std::vector<float> outL, outR;

    for (int b : blockSizes)
    {
        (void) dual.armLiveSwapAtSameLatency(offsetSamples, warmupBlocks);
        render(dual, sr, b, chans, inL, inR, outL, outR);

        assert(std::memcmp(outL.data(), inL.data(), outL.size()*sizeof(float)) == 0);
        assert(std::memcmp(outR.data(), inR.data(), outR.size()*sizeof(float)) == 0);
        hashes.push_back(hashAudio(outL));
    }

    assert(hashes[0] == hashes[1] && hashes[1] == hashes[2]);
    return 0;
}


