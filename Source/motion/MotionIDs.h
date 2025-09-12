
#pragma once
#include <juce_core/juce_core.h>
namespace motion {
static constexpr const char* kPrefix = "motion.";
enum class PannerSelect : int { P1 = 0, P2 = 1, Link = 2 };
enum class PathType : int { Circle = 0, Figure8, Bounce, Arc, Spiral, Polygon, RandomWalk, UserShape };
enum class MotionMode : int { Free = 0, Sync, InputEnv, Sidechain, OneShot };
enum class QuantizeDiv : int { Off = 0, N1, N1_2, N1_4, N1_8, N1_16, N1_32, N1_4T, N1_8T, N1_16T, N1_4D, N1_8D, N1_16D };
namespace id {
    static constexpr const char* panner_select   = "motion.panner_select";
    static constexpr const char* path            = "motion.path";
    static constexpr const char* rate_hz         = "motion.rate_hz";
    static constexpr const char* depth_pct       = "motion.depth_pct";
    static constexpr const char* phase_deg       = "motion.phase_deg";
    static constexpr const char* spread_pct      = "motion.spread_pct";
    static constexpr const char* elev_bias       = "motion.elev_bias";
    static constexpr const char* shape_bounce    = "motion.shape_bounce";
    static constexpr const char* jitter_amt      = "motion.jitter_amt";
    static constexpr const char* quantize_div    = "motion.quantize_div";
    static constexpr const char* mode            = "motion.mode";
    static constexpr const char* retrig          = "motion.retrig";
    static constexpr const char* hold_ms         = "motion.hold_ms";
    static constexpr const char* sens            = "motion.sens";
    static constexpr const char* offset_deg      = "motion.offset_deg";
    static constexpr const char* front_bias      = "motion.front_bias";
    static constexpr const char* doppler_amt     = "motion.doppler_amt";
    static constexpr const char* motion_send     = "motion.motion_send";
    static constexpr const char* anchor_enable   = "motion.anchor_enable";
    static constexpr const char* bass_floor_hz   = "motion.bass_floor_hz";
    static constexpr const char* headphone_safe  = "motion.headphone_safe";
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
