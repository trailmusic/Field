
#pragma once
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include "MotionIDs.h"
namespace motion {
struct Params {
    std::atomic<float>* pannerSelect{};  // Choice parameters return float*
    std::atomic<float>* path{};          // Choice parameters return float*
    std::atomic<float>* rateHz{};
    std::atomic<float>* depth{};
    std::atomic<float>* phaseDeg{};
    std::atomic<float>* spread{};
    std::atomic<float>* elevBias{};
    std::atomic<float>* bounce{};
    std::atomic<float>* jitter{};
    std::atomic<float>* quantizeDiv{};   // Choice parameters return float*
    std::atomic<float>* mode{};          // Choice parameters return float*
    std::atomic<float>* retrig{};        // Bool parameters return float*
    std::atomic<float>* holdMs{};
    std::atomic<float>* sens{};
    std::atomic<float>* offsetDeg{};
    std::atomic<float>* frontBias{};
    std::atomic<float>* doppler{};
    std::atomic<float>* motionSend{};
    std::atomic<float>* anchor{};        // Bool parameters return float*
    std::atomic<float>* bassFloorHz{};
    std::atomic<float>* headphoneSafe{}; // Bool parameters return float*
};
struct Snapshot {
    int   pannerSelect{};  // Converted from float
    int   path{};          // Converted from float
    float rateHz{};
    float depth{};
    float phaseDeg{};
    float spread{};
    float elevBias{};
    float bounce{};
    float jitter{};
    int   quantizeDiv{};   // Converted from float
    int   mode{};          // Converted from float
    bool  retrig{};        // Converted from float
    float holdMs{};
    float sens{};
    float offsetDeg{};
    float frontBias{};
    float doppler{};
    float motionSend{};
    bool  anchor{};        // Converted from float
    float bassFloorHz{};
    bool  headphoneSafe{}; // Converted from float
};
inline Snapshot take (const Params& p) {
    Snapshot s;
    s.pannerSelect = p.pannerSelect? (int)p.pannerSelect->load() : 0;
    s.path         = p.path?         (int)p.path->load()         : 0;
    s.rateHz       = p.rateHz?       p.rateHz->load()       : 0.5f;
    s.depth        = p.depth?        p.depth->load()        : 0.5f;
    s.phaseDeg     = p.phaseDeg?     p.phaseDeg->load()     : 0.0f;
    s.spread       = p.spread?       p.spread->load()       : 1.0f;
    s.elevBias     = p.elevBias?     p.elevBias->load()     : 0.0f;
    s.bounce       = p.bounce?       p.bounce->load()       : 0.0f;
    s.jitter       = p.jitter?       p.jitter->load()       : 0.0f;
    s.quantizeDiv  = p.quantizeDiv?  (int)p.quantizeDiv->load()  : 0;
    s.mode         = p.mode?         (int)p.mode->load()         : 0;
    s.retrig       = p.retrig?       (p.retrig->load() > 0.5f)   : false;
    s.holdMs       = p.holdMs?       p.holdMs->load()       : 0.0f;
    s.sens         = p.sens?         p.sens->load()         : 0.5f;
    s.offsetDeg    = p.offsetDeg?    p.offsetDeg->load()    : 0.0f;
    s.frontBias    = p.frontBias?    p.frontBias->load()    : 0.0f;
    s.doppler      = p.doppler?      p.doppler->load()      : 0.0f;
    s.motionSend   = p.motionSend?   p.motionSend->load()   : 0.0f;
    s.anchor       = p.anchor?       (p.anchor->load() > 0.5f)   : false;
    s.bassFloorHz  = p.bassFloorHz?  p.bassFloorHz->load()  : 120.0f;
    s.headphoneSafe= p.headphoneSafe? (p.headphoneSafe->load() > 0.5f): false;
    return s;
}
}
