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
    
    // SR-aware oversampling
    double sampleRate = 48000.0;
    int osRealtime = 1;    // 0=Auto, 1-5=Off,2x,4x,8x,16x (force Off)
    int osOffline = 1;     // 0=Auto, 1-5=Off,2x,4x,8x,16x (force Off)
    int osFilterType = 0;  // 0=Linear, 1=Minimum
    bool tpSafe = false;   // True-peak safe mode (disabled for baseline)
    
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
    
    // SR-aware oversampling resolver (Gold Clip parity)
    static int resolveOSFactor(double sr, int qualityTier) {
        const bool loSR = (sr <= 48000.0);
        const bool midSR = (sr > 48000.0 && sr <= 96000.0);
        
        switch (qualityTier) {
            case 0: // Eco/High
                if (loSR)  return 4;  // 4× @ 44.1/48
                if (midSR) return 2;  // 2× @ 88.2/96
                return 1;             // 1× @ 192+
            case 1: // Standard/Pristine
                if (loSR)  return 8;  // 8× @ 44.1/48
                if (midSR) return 4;  // 4× @ 88.2/96
                return 2;             // 2× @ 192+
            case 2: // High/Extra Pristine
                if (loSR)  return 16; // 16× @ 44.1/48
                if (midSR) return 8;  // 8× @ 88.2/96
                return 4;             // 4× @ 192+
            default: return 1;
        }
    }
    
    // Get active OS factor based on realtime/offline mode
    int getActiveOSFactor(bool isOffline = false) const {
        int targetOS = isOffline ? osOffline : osRealtime;
        
        if (targetOS == 0) { // Auto by Quality
            return resolveOSFactor(sampleRate, quality);
        }
        return targetOS; // Manual override
    }
};
