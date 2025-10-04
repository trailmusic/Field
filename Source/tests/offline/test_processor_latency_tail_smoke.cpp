#include "processor/PluginProcessor.h"
#include "core/params/ParamIDs.h"
#include "tests/offline/TestUtils_APVTS.h"
#include <juce_dsp/juce_dsp.h>
#include <cassert>

using field::params::kReverbLinearPhase;
using field::params::kReverbFIRHalfLen;
using field::params::kDynEqLookAheadMs;
using field::params::kDelayLookAheadMs;
using field::params::kChainReverbEnable;
using field::params::kChainDynEqEnable;
using field::params::kChainDelayEnable;

static void renderSilence(MyPluginAudioProcessor& proc, double /*sr*/, int block, int numBlocks)
{
    juce::AudioBuffer<float> buf(2, block);
    juce::MidiBuffer midi;
    for (int i = 0; i < numBlocks; ++i)
    {
        buf.clear();
        proc.processBlock(buf, midi);
    }
}

int main()
{
    MyPluginAudioProcessor proc;

    const double sr = 48000.0;
    const int maxBlock = 512;
    proc.prepareToPlay(sr, maxBlock);

    assert(proc.getLatencySamples() == 0);
    assert(proc.getTailLengthSeconds() == 0.0);

    auto& apvts = proc.getAPVTS();

    field::tests::setBool(apvts, kChainReverbEnable, true);
    field::tests::setBool(apvts, kChainDynEqEnable, true);
    field::tests::setBool(apvts, kChainDelayEnable,  true);

    const int firHalfLen = 256;
    const float lookAheadMs = 1.5f;

    field::tests::setBool (apvts, kReverbLinearPhase, true);
    field::tests::setInt  (apvts, kReverbFIRHalfLen,  firHalfLen);
    field::tests::setFloat(apvts, kDynEqLookAheadMs,  lookAheadMs);
    field::tests::setFloat(apvts, kDelayLookAheadMs,  lookAheadMs);

    renderSilence(proc, sr, 128, 8);

    const int latDuring = proc.getLatencySamples();
    const double tailDuring = proc.getTailLengthSeconds();
    assert(latDuring == 0);
    assert(tailDuring == 0.0);

    proc.releaseResources();
    proc.prepareToPlay(sr, maxBlock);

    const int laSamples = (int) ((lookAheadMs / 1000.0) * sr + 0.5);
    const int expectedLatency = firHalfLen + laSamples + laSamples;

    const int latAfter = proc.getLatencySamples();
    assert(latAfter == expectedLatency);

    const double tailAfter = proc.getTailLengthSeconds();
    assert(tailAfter >= 0.0);

    renderSilence(proc, sr, 256, 4);
    const int latStable = proc.getLatencySamples();
    assert(latStable == expectedLatency);

    proc.releaseResources();
    return 0;
}


