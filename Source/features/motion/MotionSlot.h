#pragma once

#include <JuceHeader.h>

// Canonical 32-slot Motion control registry
// Single source of truth for both PluginEditor.cpp and MotionControlsPane.h

namespace MotionSlot {
    // Canonical 32-slot enum
    enum Slot : int {
        kEnable = 1,        // Master on/off for motion engine
        kPanner = 2,        // Algorithm: Orbit, Figure-8, Arc, Free, etc.
        kPath = 3,          // Geometric path preset / user path select
        kRate = 4,          // Motion speed (Hz / synced)
        kDepth = 5,         // Orbit radius / amplitude
        kPhase = 6,         // Phase offset of motion
        kSpread = 7,        // Stereo spread multiplier
        kElev = 8,          // Z/elevation bias for 3D feel
        kBounce = 9,        // Edge reflectivity (bouncy vs wrap)
        kJitter = 10,       // Random micro-timing/position perturbation
        kQuant = 11,        // Lock motion to grid (note values)
        kSwing = 12,        // Groove swing for quantized steps
        kMode = 13,         // Free / Sync / Input Env / Sidechain / One-shot
        kRetrig = 14,       // Re-trigger motion on transport/beat
        kHold = 15,         // Freeze time when input holds (gate hold)
        kSens = 16,         // Envelope/sidechain sensitivity
        kOffset = 17,       // Constant pan offset (pre-motion)
        kInertia = 18,      // Slew/smoothing—mass of the orbiter
        kFront = 19,        // Front-image bias (front vs rear)
        kDoppler = 20,      // Spectral/level doppler depth
        kSend = 21,         // Motion-controlled send amount
        kAnchor = 22,       // Pin a position (pause motion at anchor)
        kBass = 23,         // Low-band mono/width emphasis while moving
        kOcclusion = 24,   // HRTF/LP+attenuation when "behind"
        kStartAngle = 25,  // Starting angle of the path (deg)
        kPathScale = 26,   // Uniform scaling of the chosen path
        kPathMorph = 27,   // Morph between path shapes (0–1)
        kCenterBias = 28,  // Attraction to center (auto-recenter)
        kStereoLink = 29,  // Link L/R motion phases (toggle)
        kRandomSeed = 30,  // Reroll jitter randomization deterministically
        kMotionSmooth = 31, // Global smoothing (post-calc, pre-render)
        kMotionMix = 32    // Dry/Wet for the motion stage only
    };

    // Control type enum
    enum Type : int {
        kButton = 0,
        kComboBox = 1,
        kKnob = 2
    };

    // Parameter registry entry
    struct ParamRef {
        const char* id;
        const char* name;
        Type type;
        float min, max, defaultVal;
        const char* tooltip;
    };

    // Canonical 32-slot parameter registry
    constexpr ParamRef kMotionSlots[32] = {
        // 1-16: Core controls (using actual MotionIDs.h parameter names)
        {"motion.enable", "Enable", kButton, 0, 1, 0, "Master on/off for motion engine"},
        {"motion.panner_select", "Panner", kComboBox, 0, 4, 0, "Algorithm: Orbit, Figure-8, Arc, Free, etc."},
        {"motion.p1.path", "Path", kComboBox, 0, 3, 0, "Geometric path preset / user path select"},
        {"motion.p1.rate_hz", "Rate", kKnob, 0.1f, 20.0f, 1.0f, "Motion speed (Hz / synced)"},
        {"motion.p1.depth_pct", "Depth", kKnob, 0, 100, 50, "Orbit radius / amplitude"},
        {"motion.p1.phase_deg", "Phase", kKnob, 0, 360, 0, "Phase offset of motion"},
        {"motion.p1.spread_pct", "Spread", kKnob, 0, 200, 100, "Stereo spread multiplier"},
        {"motion.p1.elev_bias", "Elev", kKnob, -1, 1, 0, "Z/elevation bias for 3D feel"},
        {"motion.p1.shape_bounce", "Bounce", kKnob, 0, 1, 0.5f, "Edge reflectivity (bouncy vs wrap)"},
        {"motion.p1.jitter_amt", "Jitter", kKnob, 0, 1, 0, "Random micro-timing/position perturbation"},
        {"motion.p1.quantize_div", "Quant", kComboBox, 0, 7, 0, "Lock motion to grid (note values)"},
        {"motion.p1.swing_pct", "Swing", kKnob, 0, 100, 0, "Groove swing for quantized steps"},
        {"motion.p1.mode", "Mode", kComboBox, 0, 4, 1, "Free / Sync / Input Env / Sidechain / One-shot"},
        {"motion.p1.retrig", "Retrig", kButton, 0, 1, 0, "Re-trigger motion on transport/beat"},
        {"motion.p1.hold_ms", "Hold", kKnob, 0, 1000, 0, "Freeze time when input holds (gate hold)"},
        {"motion.p1.sens", "Sens", kKnob, 0, 1, 0.5f, "Envelope/sidechain sensitivity"},
        
        // 17-24: Extended controls (using actual parameter names)
        {"motion.p1.offset_pct", "Offset", kKnob, -100, 100, 0, "Constant pan offset (pre-motion)"},
        {"motion.p1.inertia_ms", "Inertia", kKnob, 0, 1000, 100, "Slew/smoothing—mass of the orbiter"},
        {"motion.p1.front_bias", "Front", kKnob, -1, 1, 0, "Front-image bias (front vs rear)"},
        {"motion.p1.doppler_amt", "Doppler", kKnob, 0, 1, 0, "Spectral/level doppler depth"},
        {"motion.p1.motion_send", "Send", kKnob, 0, 1, 0, "Motion-controlled send amount"},
        {"motion.p1.anchor_enable", "Anchor", kButton, 0, 1, 0, "Pin a position (pause motion at anchor)"},
        {"motion.bass_floor_hz", "Bass", kKnob, 20, 200, 80, "Low-band mono/width emphasis while moving"},
        {"motion.occlusion", "Occlusion", kKnob, 0, 1, 0, "HRTF/LP+attenuation when \"behind\""},
        
        // 25-32: New high-impact controls (placeholder IDs for now)
        {"motion.start_angle_deg", "Start Angle", kKnob, 0, 360, 0, "Starting angle of the path (deg)"},
        {"motion.path_scale", "Path Scale", kKnob, 0.25f, 4.0f, 1.0f, "Uniform scaling of the chosen path"},
        {"motion.path_morph", "Path Morph", kKnob, 0, 1, 0, "Morph between path shapes (0–1)"},
        {"motion.center_bias", "Center Bias", kKnob, 0, 1, 0.25f, "Attraction to center (auto-recenter)"},
        {"motion.stereo_link", "Stereo Link", kButton, 0, 1, 1, "Link L/R motion phases (toggle)"},
        {"motion.random_seed", "Random Seed", kButton, 0, 1, 0, "Reroll jitter randomization deterministically"},
        {"motion.smooth", "Motion Smooth", kKnob, 0, 1, 0.3f, "Global smoothing (post-calc, pre-render)"},
        {"motion.mix", "Motion Mix", kKnob, 0, 1, 1.0f, "Dry/Wet for the motion stage only"}
    };

    // Helper functions
    inline const ParamRef& getSlot(int slot) {
        return kMotionSlots[slot - 1]; // Convert 1-based to 0-based
    }
    
    inline const char* getParamID(int slot) {
        return getSlot(slot).id;
    }
    
    inline const char* getDisplayName(int slot) {
        return getSlot(slot).name;
    }
    
    inline Type getType(int slot) {
        return getSlot(slot).type;
    }
}
