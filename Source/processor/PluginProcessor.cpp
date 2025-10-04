#include "../shared/Core/PluginProcessor.cpp"
#include "core/runtime/LiveSwapPlanner.h"
#include "core/runtime/ParamChangeBus.h"
#include "core/params/Snapshot.h"
#include "core/runtime/OSPhaseResolver.h"
#include "modules/FieldParamHooks.h"
#include "processor/LatencyTailCompute.h"
void MyPluginAudioProcessor::messageThreadTickForLiveSwap (double sampleRate, int maxBlock)
{
#include "core/runtime/DevHudFlag.h"
#if FIELD_DEV_HUD_ON
# if defined(FIELD_LIVE_SWAP_AVAILABLE)
    if (paramBus_.consumeVoicingChanged() &&
        !paramBus_.peekTopologyChanged() && !paramBus_.peekLatencyChanged())
    {
        const auto snap = field::params::buildSnapshot(*this);
        auto res = field::core::runtime::LiveSwapPlanner::armIfSameLatency(
                       dualF_, snap, sampleRate, maxBlock, getTotalNumOutputChannels(),
                       2, 64);
        if (res.armed && res.sameLatency) hud_.setArmed(); else hud_.setDeferredLatency();
    }
# endif
    hud_.tick(50);
#else
    (void) sampleRate; (void) maxBlock;
#endif
}
