#include "modules/FieldChain.h"
#include <juce_dsp/juce_dsp.h>
#include <cassert>

using field::modules::FieldChain;

static void runOnce()
{
    FieldChain chain;
    FieldChain::Config cfg{}; // all stages disabled by default
    chain.setConfig(cfg);
    chain.buildFromConfig();
    chain.prepare<float>(48000.0, 512, 2);

    juce::AudioBuffer<float> buf(2, 512);
    buf.clear();
    juce::dsp::AudioBlock<float> blk(buf);

    chain.process<float>(blk);

    // Expect no crash and buffer unchanged (still silent)
    for (int c = 0; c < buf.getNumChannels(); ++c)
    {
        const float* d = buf.getReadPointer(c);
        for (int i = 0; i < buf.getNumSamples(); ++i)
            assert(d[i] == 0.0f);
    }
}

int main()
{
    runOnce();
    return 0;
}


