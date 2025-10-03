#include "../shared/Core/PluginProcessor.cpp"
#include "core/runtime/LiveSwapPlanner.h"
#include "core/runtime/ParamChangeBus.h"
#include "core/params/Snapshot.h"
void MyPluginAudioProcessor::messageThreadTickForLiveSwap (double sampleRate, int maxBlock)
{
#if JUCE_DEBUG
    if (paramBus_.consumeVoicingChanged() &&
        !paramBus_.peekTopologyChanged() && !paramBus_.peekLatencyChanged())
    {
        const auto snap = field::params::buildSnapshot(*this);
        auto res = field::core::runtime::LiveSwapPlanner::armIfSameLatency(
                       dualF_, snap, sampleRate, maxBlock, getTotalNumOutputChannels(),
                       2, 64);
        if (res.armed && res.sameLatency) hud_.setArmed(); else hud_.setDeferredLatency();
    }
    hud_.tick(50);
#else
    (void) sampleRate; (void) maxBlock;
#endif
}
