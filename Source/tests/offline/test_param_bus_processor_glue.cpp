#include "processor/PluginProcessor.h"
#include "core/params/ParamIDs.h"
#include "core/runtime/ParamChangeBus.h"
#include "tests/offline/TestUtils_APVTS.h"
#include <cassert>

using field::params::kChainDelayEnable;
using field::params::kChainDynEqEnable;
using field::params::kChainReverbEnable;
using field::params::kReverbLinearPhase;
using field::params::kReverbFIRHalfLen;
using field::params::kDynEqLookAheadMs;
using field::params::kDelayLookAheadMs;

int main()
{
    MyPluginAudioProcessor proc;
    auto& apvts = proc.getAPVTS();

    // Local bus bound to APVTS for this headless test
    field::core::runtime::ParamChangeBus bus(apvts);

    proc.prepareToPlay(48000.0, 512);
    assert(proc.getLatencySamples() == 0);

    // 1) Topology flag
    field::tests::setBool(apvts, kChainReverbEnable, true);
    assert(bus.consumeTopologyChanged());
    assert(!bus.consumeLatencyChanged());

    proc.releaseResources();
    proc.prepareToPlay(48000.0, 512);
    assert(!bus.consumeTopologyChanged());

    // 2) Latency-only flags
    field::tests::setBool (apvts, kReverbLinearPhase, true);
    field::tests::setInt  (apvts, kReverbFIRHalfLen,  128);
    field::tests::setFloat(apvts, kDynEqLookAheadMs,  1.0f);
    field::tests::setFloat(apvts, kDelayLookAheadMs,  0.5f);

    assert(!bus.consumeTopologyChanged());
    assert(bus.consumeLatencyChanged());

    // PDC unchanged while "playing"
    assert(proc.getLatencySamples() == 0);

    proc.releaseResources();
    proc.prepareToPlay(48000.0, 512);
    assert(!bus.consumeLatencyChanged());

    // 3) Voicing flag (only if registered; we skip actual voicing ID set here)
    // This test ensures no crash and separation of flags by exercising topology/latency above.

    proc.releaseResources();
    return 0;
}


