
#pragma once
#include <juce_core/juce_core.h>
namespace motion {
static constexpr const char* kPrefix = "motion.";
enum class PannerSelect : int { P1 = 0, P2 = 1, Link = 2 };
enum class PathType : int { Circle = 0, Figure8, Bounce, Arc, Spiral, Polygon, RandomWalk, UserShape };
enum class MotionMode : int { Free = 0, Sync, InputEnv, Sidechain, OneShot };
enum class QuantizeDiv : int { Off = 0, N1, N1_2, N1_4, N1_8, N1_16, N1_32, N1_4T, N1_8T, N1_16T, N1_4D, N1_8D, N1_16D };
namespace id {
    // Global parameters (shared)
    static constexpr const char* enable          = "motion.enable";
    static constexpr const char* panner_select   = "motion.panner_select";
    static constexpr const char* headphone_safe  = "motion.headphone_safe";
    static constexpr const char* bass_floor_hz   = "motion.bass_floor_hz";
    static constexpr const char* occlusion       = "motion.occlusion";
    
    // P1-specific parameters
    static constexpr const char* p1_path         = "motion.p1.path";
    static constexpr const char* p1_rate_hz      = "motion.p1.rate_hz";
    static constexpr const char* p1_depth_pct    = "motion.p1.depth_pct";
    static constexpr const char* p1_phase_deg    = "motion.p1.phase_deg";
    static constexpr const char* p1_spread_pct   = "motion.p1.spread_pct";
    static constexpr const char* p1_elev_bias    = "motion.p1.elev_bias";
    static constexpr const char* p1_shape_bounce = "motion.p1.shape_bounce";
    static constexpr const char* p1_jitter_amt   = "motion.p1.jitter_amt";
    static constexpr const char* p1_quantize_div = "motion.p1.quantize_div";
    static constexpr const char* p1_swing_pct    = "motion.p1.swing_pct";
    static constexpr const char* p1_mode         = "motion.p1.mode";
    static constexpr const char* p1_retrig       = "motion.p1.retrig";
    static constexpr const char* p1_hold_ms      = "motion.p1.hold_ms";
    static constexpr const char* p1_sens         = "motion.p1.sens";
    static constexpr const char* p1_inertia_ms   = "motion.p1.inertia_ms";
    static constexpr const char* p1_front_bias   = "motion.p1.front_bias";
    static constexpr const char* p1_doppler_amt  = "motion.p1.doppler_amt";
    static constexpr const char* p1_motion_send  = "motion.p1.motion_send";
    static constexpr const char* p1_anchor_enable= "motion.p1.anchor_enable";
    
    // P2-specific parameters
    static constexpr const char* p2_path         = "motion.p2.path";
    static constexpr const char* p2_rate_hz      = "motion.p2.rate_hz";
    static constexpr const char* p2_depth_pct    = "motion.p2.depth_pct";
    static constexpr const char* p2_phase_deg    = "motion.p2.phase_deg";
    static constexpr const char* p2_spread_pct   = "motion.p2.spread_pct";
    static constexpr const char* p2_elev_bias    = "motion.p2.elev_bias";
    static constexpr const char* p2_shape_bounce = "motion.p2.shape_bounce";
    static constexpr const char* p2_jitter_amt   = "motion.p2.jitter_amt";
    static constexpr const char* p2_quantize_div = "motion.p2.quantize_div";
    static constexpr const char* p2_swing_pct    = "motion.p2.swing_pct";
    static constexpr const char* p2_mode         = "motion.p2.mode";
    static constexpr const char* p2_retrig       = "motion.p2.retrig";
    static constexpr const char* p2_hold_ms      = "motion.p2.hold_ms";
    static constexpr const char* p2_sens         = "motion.p2.sens";
    static constexpr const char* p2_inertia_ms   = "motion.p2.inertia_ms";
    static constexpr const char* p2_front_bias   = "motion.p2.front_bias";
    static constexpr const char* p2_doppler_amt  = "motion.p2.doppler_amt";
    static constexpr const char* p2_motion_send  = "motion.p2.motion_send";
    static constexpr const char* p2_anchor_enable= "motion.p2.anchor_enable";
    
    // Legacy parameter IDs for backward compatibility (mapped to P1)
    static constexpr const char* path            = "motion.p1.path";
    static constexpr const char* rate_hz         = "motion.p1.rate_hz";
    static constexpr const char* depth_pct       = "motion.p1.depth_pct";
    static constexpr const char* phase_deg       = "motion.p1.phase_deg";
    static constexpr const char* spread_pct      = "motion.p1.spread_pct";
    static constexpr const char* elev_bias       = "motion.p1.elev_bias";
    static constexpr const char* shape_bounce    = "motion.p1.shape_bounce";
    static constexpr const char* jitter_amt      = "motion.p1.jitter_amt";
    static constexpr const char* quantize_div    = "motion.p1.quantize_div";
    static constexpr const char* swing_pct       = "motion.p1.swing_pct";
    static constexpr const char* mode            = "motion.p1.mode";
    static constexpr const char* retrig          = "motion.p1.retrig";
    static constexpr const char* hold_ms         = "motion.p1.hold_ms";
    static constexpr const char* sens            = "motion.p1.sens";
    static constexpr const char* offset_deg      = "motion.p1.phase_deg"; // Map to P1 phase
    static constexpr const char* inertia_ms      = "motion.p1.inertia_ms";
    static constexpr const char* front_bias      = "motion.p1.front_bias";
    static constexpr const char* doppler_amt     = "motion.p1.doppler_amt";
    static constexpr const char* motion_send     = "motion.p1.motion_send";
    static constexpr const char* anchor_enable   = "motion.p1.anchor_enable";
}
inline juce::StringArray choiceListPath() {
    return { "Circle", "Figure-8", "Bounce", "Arc", "Spiral", "Polygon", "Random Walk", "User Shape…" };
}
inline juce::StringArray choiceListMode() {
    return { "Free", "Sync", "Input Env", "Sidechain", "One-Shot" };
}
inline juce::StringArray choiceListPanner() {
    return { "P1", "P2", "Link" };
}
inline juce::StringArray choiceListQuant() {
    return { "Off","1/1","1/2","1/4","1/8","1/16","1/32","1/4T","1/8T","1/16T","1/4D","1/8D","1/16D" };
}
}
