#pragma once
#include <juce_core/juce_core.h>
#include <atomic>

namespace motion {

// Individual panner visual state
struct PannerViz {
    float azimuth = 0.0f;    // -1..+1
    float elevation = 0.0f;  // -1..+1
    float radius = 0.0f;     // 0..1
    float rateHz = 0.0f;
    float depth = 0.0f;
    float spread = 1.0f;
    int pathType = 0;
    float phaseDeg = 0.0f;
    float elevBias = 0.0f;
    float bounce = 0.0f;
    float jitter = 0.0f;
    float swing = 0.0f;
    int quantizeDiv = 0;
    int mode = 0;
    bool retrig = false;
    float holdMs = 0.0f;
    float sens = 0.0f;
    float inertia = 0.0f;
    float frontBias = 0.0f;
    float doppler = 0.0f;
    float motionSend = 0.0f;
    bool anchor = false;
    
    // Screen-space coordinates for drawing
    float x = 0.0f;  // normalized [-1..+1]
    float y = 0.0f;  // normalized [-1..+1]
};

// Panner selection enum
enum class ActiveSel { P1 = 0, P2 = 1, Link = 2 };

// Complete visual state for the motion panel
struct VisualState {
    PannerViz p1, p2;
    ActiveSel active = ActiveSel::P1;
    bool link = false;
    bool enable = true;
    float occlusion = 0.4f;
    bool headphoneSafe = false;
    float bassFloorHz = 120.0f;
    uint32_t seq = 0;   // monotonic sequence for thread safety
};

// Lock-free double-buffer mailbox for visual state handoff
struct VisualMailbox {
    VisualState buf[2];
    std::atomic<int> idx{0};      // which buffer is current
    std::atomic<uint32_t> seq{0}; // published sequence number

    inline void publish(const VisualState& v) noexcept {
        const int w = 1 - idx.load(std::memory_order_relaxed);
        buf[w] = v;                                    // copy to inactive buffer
        buf[w].seq = seq.load(std::memory_order_relaxed) + 1;
        seq.store(buf[w].seq, std::memory_order_relaxed);
        idx.store(w, std::memory_order_release);       // flip to new buffer
    }
    
    inline uint32_t read(VisualState& out) const noexcept {
        const int r = idx.load(std::memory_order_acquire);
        out = buf[r];                                  // cheap copy
        return out.seq;
    }
};

} // namespace motion
