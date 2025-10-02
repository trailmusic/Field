#pragma once
#include <atomic>

// Runtime DSP configuration - atomic copy-on-write for thread safety
struct DspRuntimeConfig
{
    int os = 0;                   // oversampling index (0..4)
    int phase = 0;                // 0=Zero, 2=Hybrid, 3=FullLinear
    bool userOverrodeOS = false;
    bool userOverrodePhase = false;
    int quality = 1;              // 0..2
    int latencySamples = 0;       // reported latency (phase/OS dependent)
    
    // Quality mapping
    struct QualityMap { int os; int phase; };
    static constexpr QualityMap kQMap[] = {
        /*Eco*/     { 0, 0 },   // OS off, Zero phase
        /*Standard*/{ 1, 2 },   // 2x, Hybrid
        /*High*/    { 2, 3 },   // 4x, Full linear
    };
    
    // Helper methods
    bool needsRebuild(const DspRuntimeConfig& other) const {
        return os != other.os || phase != other.phase;
    }
    
    void resetOverrides() {
        userOverrodeOS = false;
        userOverrodePhase = false;
        os = kQMap[quality].os;
        phase = kQMap[quality].phase;
    }
    
    void applyQualityDefaults() {
        if (!userOverrodeOS) os = kQMap[quality].os;
        if (!userOverrodePhase) phase = kQMap[quality].phase;
    }
};
