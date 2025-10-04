#pragma once
#include <algorithm>
#warning "DspRuntimeConfig is deprecated. Use core/params/Snapshot.h + core/runtime/OSPhaseResolver.h. Scheduled removal in WO-38."
// Minimal shim to keep legacy includes compiling. Do NOT read on audio thread.
struct DspRuntimeConfig {
    int    quality            = 1;      // 0..2
    int    osRealtime         = 1;      // 0=auto, else {1,2,4,8,16}
    int    osOffline          = 1;      // 0=auto, else {1,2,4,8,16}
    int    phaseMode          = 0;      // 0/2/3
    bool   userOverrodePhase  = false;
    bool   userOverrodeOS     = false;
    double sampleRate         = 48000.0;

    // Legacy fields referenced in glue (no longer authoritative)
    int    os                 = 1;      // effective OS factor (legacy)
    int    phase              = 0;      // effective phase (legacy)
    int    osFilterType       = 0;      // 0=Linear Phase (legacy no-op)
    bool   tpSafe             = true;   // true-peak safe (legacy no-op)
    int    latencySamples     = 0;      // legacy placeholder

    struct QMapEntry { int phase; };
    static inline QMapEntry kQMap[3] = { {0}, {2}, {3} };

    static inline int clampQuality(int q) { return std::clamp(q, 0, 2); }

    // Legacy helper kept as a no-op conservative resolver
    static inline int resolveOSFactor(double sr, int qualityTier){
        const bool lo  = (sr <= 48000.0);
        const bool mid = (sr > 48000.0 && sr <= 96000.0);
        switch (clampQuality(qualityTier)){
            case 0: return 1;
            case 1: return lo?2 : mid?2 : 1;
            case 2: return lo?4 : mid?2 : 2;
        }
        return 1;
    }
};
