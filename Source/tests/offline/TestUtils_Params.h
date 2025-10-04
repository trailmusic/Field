#pragma once
#include "core/params/Snapshot.h"

namespace field::tests {

inline field::params::ChainParamSnapshot makeSnap(
    bool enableDelay = false, bool enableDynEq = false, bool enableReverb = false,
    int osFactor = 1,
    bool reverbLinearPhase = false, int reverbFIRHalfLen = 0,
    float dynEqLookAheadMs = 0.f, float delayLookAheadMs = 0.f)
{
    field::params::ChainParamSnapshot s;
    s.enableDelay = enableDelay;
    s.enableDynEq = enableDynEq;
    s.enableReverb = enableReverb;
    s.osFactor = osFactor;
    s.reverbLinearPhase = reverbLinearPhase;
    s.reverbFIRHalfLen = reverbFIRHalfLen;
    s.dynEqLookAheadMs = dynEqLookAheadMs;
    s.delayLookAheadMs = delayLookAheadMs;
    return s;
}

} // namespace field::tests


