#pragma once

namespace ReverbIDs {
// Top / Algo
inline constexpr const char* enabled   = "reverb_enabled";
inline constexpr const char* killDry   = "reverb_kill_dry";
inline constexpr const char* algo      = "reverb_algo";

// Space / Time
inline constexpr const char* preDelayMs    = "reverb_predelay_ms";
inline constexpr const char* decaySec      = "reverb_decay_s";
inline constexpr const char* densityPct    = "reverb_density_pct";
inline constexpr const char* diffusionPct  = "reverb_diffusion_pct";
inline constexpr const char* modDepthCents = "reverb_mod_depth_cents";
inline constexpr const char* modRateHz     = "reverb_mod_rate_hz";

// ER
inline constexpr const char* erLevelDb    = "reverb_er_level_db";
inline constexpr const char* erTimeMs     = "reverb_er_time_ms";
inline constexpr const char* erDensityPct = "reverb_er_density_pct";
inline constexpr const char* erWidthPct   = "reverb_er_width_pct";
inline constexpr const char* erToTailPct  = "reverb_er_to_tail_pct";

// Tone (wet path)
inline constexpr const char* hpfHz  = "reverb_hpf_hz";
inline constexpr const char* lpfHz  = "reverb_lpf_hz";
inline constexpr const char* tiltDb = "reverb_tilt_db";

// DR-EQ
inline constexpr const char* dreqLowX  = "reverb_decay_low_mult";
inline constexpr const char* dreqMidX  = "reverb_decay_mid_mult";
inline constexpr const char* dreqHighX = "reverb_decay_high_mult";

// Motion
inline constexpr const char* widthPct      = "reverb_width_pct";
inline constexpr const char* widthStartPct = "reverb_width_start_pct";
inline constexpr const char* widthEndPct   = "reverb_width_end_pct";
inline constexpr const char* widthEnvCurve = "reverb_width_env_curve";
inline constexpr const char* rotStartDeg   = "reverb_rotate_start_deg";
inline constexpr const char* rotEndDeg     = "reverb_rotate_end_deg";
inline constexpr const char* rotEnvCurve   = "reverb_rotate_env_curve";

// Ducking
inline constexpr const char* duckMode    = "reverb_duck_mode"; // WetOnly/Center/Band
inline constexpr const char* duckDepthDb = "reverb_duck_depth_db";
inline constexpr const char* duckThrDb   = "reverb_duck_threshold_db";
inline constexpr const char* duckKneeDb  = "reverb_duck_knee_db";
inline constexpr const char* duckRatio   = "reverb_duck_ratio";
inline constexpr const char* duckAtkMs   = "reverb_duck_attack_ms";
inline constexpr const char* duckRelMs   = "reverb_duck_release_ms";
inline constexpr const char* duckLaMs    = "reverb_duck_lookahead_ms";
inline constexpr const char* duckRmsMs   = "reverb_duck_rms_ms";
inline constexpr const char* duckBandHz  = "reverb_duck_band_center_hz";
inline constexpr const char* duckBandQ   = "reverb_duck_band_q";

// Specials
inline constexpr const char* freeze        = "reverb_freeze";
inline constexpr const char* gateAmtPct    = "reverb_gate_amount_pct";
inline constexpr const char* shimmerAmtPct = "reverb_shimmer_amt_pct";
inline constexpr const char* shimmerInt    = "reverb_shimmer_interval"; // +12/+7/-12

// Post-EQ (wet only)
inline constexpr const char* eqOn         = "reverb_eq_on";
inline constexpr const char* postEqMixPct = "reverb_post_eq_mix_pct";
inline constexpr const char* eqLowFreqHz  = "reverb_eq_low_freq_hz";
inline constexpr const char* eqLowGainDb  = "reverb_eq_low_gain_db";
inline constexpr const char* eqLowQ       = "reverb_eq_low_q";
inline constexpr const char* eqMidFreqHz  = "reverb_eq_mid_freq_hz";
inline constexpr const char* eqMidGainDb  = "reverb_eq_mid_gain_db";
inline constexpr const char* eqMidQ       = "reverb_eq_mid_q";
inline constexpr const char* eqHighFreqHz = "reverb_eq_high_freq_hz";
inline constexpr const char* eqHighGainDb = "reverb_eq_high_gain_db";
inline constexpr const char* eqHighQ      = "reverb_eq_high_q";

// Mix / Out
inline constexpr const char* wetMix01   = "reverb_wet_mix";
inline constexpr const char* outTrimDb  = "reverb_output_trim_db";
} // namespace ReverbIDs


