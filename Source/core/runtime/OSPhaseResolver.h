#pragma once
#include <algorithm>

namespace field::runtime {

inline int clampQuality(int q)            { return std::clamp(q, 0, 2); }
inline int clampOSFactor(int f)           { switch (f){case 1:case 2:case 4:case 8:case 16:return f; default:return 1;} }
inline int clampPhase(int p)              { return (p==0||p==2||p==3) ? p : 0; }

inline int autoOSFactor(double sr, int qualityTier){
    const bool lo  = (sr <= 48000.0);
    const bool mid = (sr > 48000.0 && sr <= 96000.0);
    switch (clampQuality(qualityTier)){
        case 0: return 1;
        case 1: return lo?2 : mid?2 : 1;
        case 2: return lo?4 : mid?2 : 2;
    }
    return 1;
}

inline int effectiveOSFactor(double sr, int quality, int overrideFactor /*0=auto*/, bool /*isOffline*/){
    return overrideFactor == 0 ? autoOSFactor(sr, quality) : clampOSFactor(overrideFactor);
}

inline int effectivePhase(int quality, bool userOverrodePhase, int phaseMode){
    if (!userOverrodePhase){
        switch (clampQuality(quality)){ case 0: return 0; case 1: return 2; case 2: return 3; }
    }
    return clampPhase(phaseMode);
}

} // namespace field::runtime
