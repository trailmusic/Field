
#pragma once
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include "MotionIDs.h"
namespace motion {
// Individual panner parameter set
struct PannerParams {
    std::atomic<float>* path{};          // Choice parameters return float*
    std::atomic<float>* rateHz{};
    std::atomic<float>* depth{};
    std::atomic<float>* phaseDeg{};
    std::atomic<float>* spread{};
    std::atomic<float>* elevBias{};
    std::atomic<float>* bounce{};
    std::atomic<float>* jitter{};
    std::atomic<float>* quantizeDiv{};   // Choice parameters return float*
    std::atomic<float>* swing{};         // Swing percentage
    std::atomic<float>* mode{};          // Choice parameters return float*
    std::atomic<float>* retrig{};        // Bool parameters return float*
    std::atomic<float>* holdMs{};
    std::atomic<float>* sens{};
    std::atomic<float>* inertia{};       // Inertia in milliseconds
    std::atomic<float>* frontBias{};
    std::atomic<float>* doppler{};
    std::atomic<float>* motionSend{};
    std::atomic<float>* anchor{};        // Bool parameters return float*
};

struct Params {
    // Global parameters (shared)
    std::atomic<float>* enable{};        // Bool parameters return float*
    std::atomic<float>* pannerSelect{};  // Choice parameters return float*
    std::atomic<float>* headphoneSafe{}; // Bool parameters return float*
    std::atomic<float>* bassFloorHz{};
    std::atomic<float>* occlusion{};     // Occlusion percentage
    
    // P1 and P2 parameter sets
    PannerParams p1;
    PannerParams p2;
    
    // Legacy parameter pointers (for backward compatibility - point to P1)
    std::atomic<float>* path{};          // Points to p1.path
    std::atomic<float>* rateHz{};        // Points to p1.rateHz
    std::atomic<float>* depth{};         // Points to p1.depth
    std::atomic<float>* phaseDeg{};      // Points to p1.phaseDeg
    std::atomic<float>* spread{};        // Points to p1.spread
    std::atomic<float>* elevBias{};      // Points to p1.elevBias
    std::atomic<float>* bounce{};        // Points to p1.bounce
    std::atomic<float>* jitter{};        // Points to p1.jitter
    std::atomic<float>* quantizeDiv{};   // Points to p1.quantizeDiv
    std::atomic<float>* swing{};         // Points to p1.swing
    std::atomic<float>* mode{};          // Points to p1.mode
    std::atomic<float>* retrig{};        // Points to p1.retrig
    std::atomic<float>* holdMs{};        // Points to p1.holdMs
    std::atomic<float>* sens{};          // Points to p1.sens
    std::atomic<float>* offsetDeg{};     // Points to p1.phaseDeg (legacy)
    std::atomic<float>* inertia{};       // Points to p1.inertia
    std::atomic<float>* frontBias{};     // Points to p1.frontBias
    std::atomic<float>* doppler{};       // Points to p1.doppler
    std::atomic<float>* motionSend{};    // Points to p1.motionSend
    std::atomic<float>* anchor{};        // Points to p1.anchor
};
// Individual panner snapshot
struct PannerSnapshot {
    int   path{};          // Converted from float
    float rateHz{};
    float depth{};
    float phaseDeg{};
    float spread{};
    float elevBias{};
    float bounce{};
    float jitter{};
    int   quantizeDiv{};   // Converted from float
    float swing{};         // Swing percentage
    int   mode{};          // Converted from float
    bool  retrig{};        // Converted from float
    float holdMs{};
    float sens{};
    float inertia{};       // Inertia in milliseconds
    float frontBias{};
    float doppler{};
    float motionSend{};
    bool  anchor{};        // Converted from float
};

struct Snapshot {
    // Global parameters
    bool  enable{};        // Converted from float
    int   pannerSelect{};  // Converted from float
    bool  headphoneSafe{}; // Converted from float
    float bassFloorHz{};
    float occlusion{};     // Occlusion percentage
    
    // P1 and P2 snapshots
    PannerSnapshot p1;
    PannerSnapshot p2;
    
    // Legacy fields (for backward compatibility - reference P1)
    int   path{};          // References p1.path
    float rateHz{};        // References p1.rateHz
    float depth{};         // References p1.depth
    float phaseDeg{};      // References p1.phaseDeg
    float spread{};        // References p1.spread
    float elevBias{};      // References p1.elevBias
    float bounce{};        // References p1.bounce
    float jitter{};        // References p1.jitter
    int   quantizeDiv{};   // References p1.quantizeDiv
    float swing{};         // References p1.swing
    int   mode{};          // References p1.mode
    bool  retrig{};        // References p1.retrig
    float holdMs{};        // References p1.holdMs
    float sens{};          // References p1.sens
    float offsetDeg{};     // References p1.phaseDeg (legacy)
    float inertia{};       // References p1.inertia
    float frontBias{};     // References p1.frontBias
    float doppler{};       // References p1.doppler
    float motionSend{};    // References p1.motionSend
    bool  anchor{};        // References p1.anchor
};
// Helper function to take a panner snapshot
inline PannerSnapshot takePanner(const PannerParams& pp) {
    PannerSnapshot ps;
    ps.path         = pp.path?         (int)pp.path->load()         : 0;
    ps.rateHz       = pp.rateHz?       pp.rateHz->load()       : 0.5f;
    ps.depth        = pp.depth?        pp.depth->load()        : 0.5f;
    ps.phaseDeg     = pp.phaseDeg?     pp.phaseDeg->load()     : 0.0f;
    ps.spread       = pp.spread?       pp.spread->load()       : 1.0f;
    ps.elevBias     = pp.elevBias?     pp.elevBias->load()     : 0.0f;
    ps.bounce       = pp.bounce?       pp.bounce->load()       : 0.0f;
    ps.jitter       = pp.jitter?       pp.jitter->load()       : 0.0f;
    ps.quantizeDiv  = pp.quantizeDiv?  (int)pp.quantizeDiv->load()  : 0;
    ps.swing        = pp.swing?        pp.swing->load()        : 0.2f;
    ps.mode         = pp.mode?         (int)pp.mode->load()         : 0;
    ps.retrig       = pp.retrig?       (pp.retrig->load() > 0.5f)   : false;
    ps.holdMs       = pp.holdMs?       pp.holdMs->load()       : 0.0f;
    ps.sens         = pp.sens?         pp.sens->load()         : 0.5f;
    ps.inertia      = pp.inertia?      pp.inertia->load()      : 120.0f;
    ps.frontBias    = pp.frontBias?    pp.frontBias->load()    : 0.0f;
    ps.doppler      = pp.doppler?      pp.doppler->load()      : 0.0f;
    ps.motionSend   = pp.motionSend?   pp.motionSend->load()   : 0.0f;
    ps.anchor       = pp.anchor?       (pp.anchor->load() > 0.5f)   : false;
    return ps;
}

inline Snapshot take (const Params& p) {
    Snapshot s;
    
    // Global parameters
    s.enable       = p.enable?       (p.enable->load() > 0.5f)   : true;
    s.pannerSelect = p.pannerSelect? (int)p.pannerSelect->load() : 0;
    s.headphoneSafe= p.headphoneSafe? (p.headphoneSafe->load() > 0.5f): false;
    s.bassFloorHz  = p.bassFloorHz?  p.bassFloorHz->load()  : 120.0f;
    s.occlusion    = p.occlusion?    p.occlusion->load()    : 0.4f;
    
    // P1 and P2 snapshots
    s.p1 = takePanner(p.p1);
    s.p2 = takePanner(p.p2);
    
    // Legacy fields (reference P1 for backward compatibility)
    s.path         = s.p1.path;
    s.rateHz       = s.p1.rateHz;
    s.depth        = s.p1.depth;
    s.phaseDeg     = s.p1.phaseDeg;
    s.spread       = s.p1.spread;
    s.elevBias     = s.p1.elevBias;
    s.bounce       = s.p1.bounce;
    s.jitter       = s.p1.jitter;
    s.quantizeDiv  = s.p1.quantizeDiv;
    s.swing        = s.p1.swing;
    s.mode         = s.p1.mode;
    s.retrig       = s.p1.retrig;
    s.holdMs       = s.p1.holdMs;
    s.sens         = s.p1.sens;
    s.offsetDeg    = s.p1.phaseDeg;  // Legacy mapping
    s.inertia      = s.p1.inertia;
    s.frontBias    = s.p1.frontBias;
    s.doppler      = s.p1.doppler;
    s.motionSend   = s.p1.motionSend;
    s.anchor       = s.p1.anchor;
    
    return s;
}
}
