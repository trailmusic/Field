#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// ReverbParamIDs — Canonical parameter ID strings for the Reverb module
// ----------------------------------------------------------------------------
// DEV NOTES
// - Keep IDs stable; hosts persist automation by string ID.
// - Choice index namespaces below are purely for readability (no magic nums).
// ─────────────────────────────────────────────────────────────────────────────

namespace ReverbParamIDs
{
    // Core routing
    inline constexpr const char* enabled         = "reverb_enabled";
    inline constexpr const char* killDry         = "reverb_kill_dry";

    // Structure / space
    inline constexpr const char* preDelayMs      = "preDelayMs";     // 0..200 ms
    inline constexpr const char* decaySec        = "decaySec";       // 0.2..20 s
    inline constexpr const char* sizePct         = "sizePct";        // 10..200 %

    // Early reflections
    inline constexpr const char* erLevelDb       = "erLevelDb";      // -24..+6 dB
    inline constexpr const char* erDensityPct    = "erDensityPct";   // 0..100 %
    inline constexpr const char* erWidthPct      = "erWidthPct";     // 0..200 %
    inline constexpr const char* erTimeMs        = "erTimeMs";       // 50..500 ms
    inline constexpr const char* erToTailPct     = "erToTailPct";    // 0..100 %

    // Diffusion / density
    inline constexpr const char* diffusionPct    = "diffusionPct";   // 0..100 %
    inline constexpr const char* densityPct      = "densityPct";     // 0..100 %

    // Modulation
    inline constexpr const char* modDepthCents   = "modDepthCents";  // 0..25 cents
    inline constexpr const char* modRateHz       = "modRateHz";      // 0.05..5 Hz

    // Stereo + rotation
    inline constexpr const char* widthPct        = "widthPct";       // 0..200 %
    inline constexpr const char* rotationDeg     = "rotationDeg";    // -45..+45 deg

    // Motion follow (from global Motion Engine)
    inline constexpr const char* followWidth     = "followWidth";
    inline constexpr const char* followWidthAmt  = "followWidthAmt"; // 0..100 %
    inline constexpr const char* followRot       = "followRot";
    inline constexpr const char* followRotAmt    = "followRotAmt";   // 0..100 %

    // Mix & specials
    inline constexpr const char* wetMix01        = "wetMix01";       // 0..1
    inline constexpr const char* bloomPct        = "bloomPct";       // 0..100 %
    inline constexpr const char* distancePct     = "distancePct";    // 0..100 %
    inline constexpr const char* freeze          = "freeze";
    inline constexpr const char* shimmerAmtPct   = "shimmerAmtPct";  // 0..100 %
    inline constexpr const char* shimmerInt      = "shimmerInt";     // 0..100 %
    inline constexpr const char* gateAmtPct      = "gateAmtPct";     // 0..100 %
    inline constexpr const char* outTrimDb       = "outTrimDb";      // -24..+12 dB

    // Reverb EQ routing
    inline constexpr const char* dreqXoverLoHz   = "dreqXoverLoHz";  // 80..400 Hz
    inline constexpr const char* dreqXoverHiHz   = "dreqXoverHiHz";  // 1k..6k Hz
    inline constexpr const char* dreqApply       = "dreqApply";      // 0=Pre,1=Post,2=ER,3=Tail

    // Ducking (floating module)
    inline constexpr const char* duckOn          = "duckOn";
    inline constexpr const char* duckMode        = "duckMode";       // 0=General,1=Vocal,2=DrumBus,3=Guitar,4=Keys
    inline constexpr const char* duckDepthDb     = "duckDepthDb";    // 0..24 dB
    inline constexpr const char* duckAtkMs       = "duckAtkMs";      // 1..100 ms
    inline constexpr const char* duckRelMs       = "duckRelMs";      // 50..2000 ms
    inline constexpr const char* duckThrDb       = "duckThrDb";      // -60..-6 dB
    inline constexpr const char* duckRatio       = "duckRatio";      // 1..8
    inline constexpr const char* duckKneeDb      = "duckKneeDb";     // 0..24 dB
    inline constexpr const char* duckBandHz      = "duckBandHz";     // 50..8000 Hz
    inline constexpr const char* duckBandQ       = "duckBandQ";      // 0.3..4
    inline constexpr const char* duckDetectorSrc = "duckDetectorSrc";// 0=Dry,1=ER,2=Tail,3=Wet

    // ================================================================
    // 🎯 DECAY RATE CONTROL PARAMETERS (JANUARY 2025)
    // ================================================================
    // CRITICAL: Musical decay-rate control for reverb tails
    // These parameters enable frequency-dependent T60 shaping
    // ================================================================
    inline constexpr const char* decayLoMult     = "decayLoMult";     // 0.25..4.0 (Low T60×)
    inline constexpr const char* decayHiMult     = "decayHiMult";     // 0.25..4.0 (High T60×)
    inline constexpr const char* decayMidDb      = "decayMidDb";      // -12..+12 dB (Mid bell gain)
    inline constexpr const char* decayMidFreqHz  = "decayMidFreqHz";  // 20..20000 Hz (Mid bell freq)
    inline constexpr const char* decayMidQ       = "decayMidQ";       // 0.3..6.0 (Mid bell Q)
    inline constexpr const char* decayTiltDb     = "decayTiltDb";     // -12..+12 dB (Decay tilt bias)
    inline constexpr const char* decaySmoothing  = "decaySmoothing";  // 0=Fast,1=Med,2=Slow (Smoothing speed)
    inline constexpr const char* decayMode       = "decayMode";       // 0=Simple,1=Advanced (UI mode)
    
    // ================================================================
    // 🎯 DECAY PROFILE SYSTEM (JANUARY 2025)
    // ================================================================
    // CRITICAL: Musical decay profile modes and coupling system
    // These parameters enable intelligent decay curve generation
    // ================================================================
    inline constexpr const char* decayProfileMode     = "decay_profile_mode";     // 0..7 (Profile generation mode)
    inline constexpr const char* decayProfileCoupling = "decay_profile_coupling"; // 0..4 (External coupling mode)
    
    // ================================================================
    // 🎯 SIDECHAIN LEARN SYSTEM (JANUARY 2025)
    // ================================================================
    // CRITICAL: Auto-learn decay profiles from external signals
    // These parameters enable intelligent decay curve learning
    // ================================================================
    inline constexpr const char* decayLearn          = "decay_learn";           // Learn button (momentary)
    inline constexpr const char* decayLearnReset     = "decay_learn_reset";     // Reset learned profile
    inline constexpr const char* decayLearnStrength  = "decay_learn_strength";  // Blend strength (0..1)
    inline constexpr const char* decayLearnWindow    = "decay_learn_window_s";  // Capture window (2..8s)

    // ─────────────────────────────────────────────────────────────────────────
    // Choice index helpers (keep in sync with ReverbParameters choice arrays)
    // ─────────────────────────────────────────────────────────────────────────

    namespace DreqApplyIdx {
        inline constexpr int Pre   = 0;
        inline constexpr int Post  = 1; // default in layout
        inline constexpr int Early = 2;
        inline constexpr int Tail  = 3;
    }

    namespace DuckModeIdx {
        inline constexpr int General = 0;
        inline constexpr int Vocal   = 1;
        inline constexpr int DrumBus = 2;
        inline constexpr int Guitar  = 3;
        inline constexpr int Keys    = 4;
    }

    namespace DuckDetectorIdx {
        inline constexpr int Dry  = 0;
        inline constexpr int ER   = 1;
        inline constexpr int Tail = 2;
        inline constexpr int Wet  = 3; // default in layout
    }

    namespace DecaySmoothingIdx {
        inline constexpr int Fast = 0;
        inline constexpr int Med  = 1; // default
        inline constexpr int Slow = 2;
    }

    namespace DecayModeIdx {
        inline constexpr int Simple   = 0; // default
        inline constexpr int Advanced = 1;
    }

    namespace DecayProfileModeIdx {
        inline constexpr int Manual3Band = 0; // default
        inline constexpr int TiltCoupled = 1;
        inline constexpr int Plate = 2;
        inline constexpr int Hall = 3;
        inline constexpr int Room = 4;
        inline constexpr int Chamber = 5;
        inline constexpr int Cathedral = 6;
        inline constexpr int Nonlinear = 7;
    }

    namespace DecayProfileCouplingIdx {
        inline constexpr int Independent = 0; // default
        inline constexpr int FollowToneTilt = 1;
        inline constexpr int FollowHPLP = 2;
        inline constexpr int FollowWidthDesigner = 3;
        inline constexpr int SidechainLearn = 4; // Now implemented!
    }
}