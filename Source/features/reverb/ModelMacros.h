#pragma once
#include <string>
#include <algorithm>
#include "ReverbEngine.h"

struct ModelDefaults { float sizePct, decaySec, erTimeMs, erDensityPct, diffusionPct, modDepthCents, modRateHz, dreqLoMult, dreqHiMult; };
inline ModelDefaults modelFor(const std::string& name) {
    auto s=name; std::transform(s.begin(),s.end(),s.begin(),::tolower);
    if(s=="plate")   return {55,2.2f,45,65,80,6.0f,0.35f,1.00f,0.85f};
    if(s=="hall")    return {85,4.5f,65,55,90,8.0f,0.25f,1.05f,0.80f};
    if(s=="chamber") return {50,1.8f,55,70,78,5.0f,0.30f,0.95f,0.90f};
    return {35,0.9f,35,75,65,3.0f,0.40f,0.90f,0.95f}; // room
}
inline void applyModel(ReverbParams& p, const std::string& model){
    const auto m = modelFor(model);
    // Note: ReverbParams doesn't have sizePct, erDensity, diffusion fields
    // These would need to be mapped to the actual ReverbParams fields
    p.decaySec = m.decaySec;
    p.erTimeMs = m.erTimeMs;
    p.modDepthCents = m.modDepthCents;
    p.modRateHz = m.modRateHz;
    // Additional mapping would be needed for other fields
}
