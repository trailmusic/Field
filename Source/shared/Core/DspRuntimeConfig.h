#pragma once
#warning "DspRuntimeConfig is deprecated. Use core/params/Snapshot.h + core/runtime/OSPhaseResolver.h. Scheduled removal in WO-38."
// Minimal shim to keep legacy includes compiling. Do NOT read on audio thread.
struct DspRuntimeConfig {
    int   quality           = 1;      // 0..2
    int   osRealtime        = 1;      // 0=auto, else {1,2,4,8,16}
    int   osOffline         = 1;      // 0=auto, else {1,2,4,8,16}
    int   phaseMode         = 0;      // 0/2/3
    bool  userOverrodePhase = false;
    double sampleRate       = 48000.0;
};
