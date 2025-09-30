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
// DR-EQ Crossovers (new)
inline constexpr const char* dreqXoverLoHz = "reverb_decay_xover_lo_hz";
inline constexpr const char* dreqXoverHiHz = "reverb_decay_xover_hi_hz";

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
// Macro / shaping (new)
inline constexpr const char* sizePct     = "reverb_size_pct";
inline constexpr const char* bloomPct    = "reverb_bloom_pct";
inline constexpr const char* distancePct = "reverb_distance_pct";

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

// Dynamic EQ (wet-only) — up to 4 bands (bell/low-shelf/high-shelf)
// Band 1
inline constexpr const char* dyneq1_on      = "reverb_dyneq1_on";
inline constexpr const char* dyneq1_mode    = "reverb_dyneq1_mode";      // 0=bell,1=lowShelf,2=highShelf
inline constexpr const char* dyneq1_freqHz  = "reverb_dyneq1_freq_hz";
inline constexpr const char* dyneq1_gainDb  = "reverb_dyneq1_gain_db";   // makeup/static tilt
inline constexpr const char* dyneq1_Q       = "reverb_dyneq1_q";
inline constexpr const char* dyneq1_thrDb   = "reverb_dyneq1_thr_db";
inline constexpr const char* dyneq1_ratio   = "reverb_dyneq1_ratio";     // >=1.0
inline constexpr const char* dyneq1_attMs   = "reverb_dyneq1_attack_ms";
inline constexpr const char* dyneq1_relMs   = "reverb_dyneq1_release_ms";
inline constexpr const char* dyneq1_rangeDb = "reverb_dyneq1_range_db";  // max reduction (positive number)
// Band 2
inline constexpr const char* dyneq2_on      = "reverb_dyneq2_on";
inline constexpr const char* dyneq2_mode    = "reverb_dyneq2_mode";
inline constexpr const char* dyneq2_freqHz  = "reverb_dyneq2_freq_hz";
inline constexpr const char* dyneq2_gainDb  = "reverb_dyneq2_gain_db";
inline constexpr const char* dyneq2_Q       = "reverb_dyneq2_q";
inline constexpr const char* dyneq2_thrDb   = "reverb_dyneq2_thr_db";
inline constexpr const char* dyneq2_ratio   = "reverb_dyneq2_ratio";
inline constexpr const char* dyneq2_attMs   = "reverb_dyneq2_attack_ms";
inline constexpr const char* dyneq2_relMs   = "reverb_dyneq2_release_ms";
inline constexpr const char* dyneq2_rangeDb = "reverb_dyneq2_range_db";
// Band 3
inline constexpr const char* dyneq3_on      = "reverb_dyneq3_on";
inline constexpr const char* dyneq3_mode    = "reverb_dyneq3_mode";
inline constexpr const char* dyneq3_freqHz  = "reverb_dyneq3_freq_hz";
inline constexpr const char* dyneq3_gainDb  = "reverb_dyneq3_gain_db";
inline constexpr const char* dyneq3_Q       = "reverb_dyneq3_q";
inline constexpr const char* dyneq3_thrDb   = "reverb_dyneq3_thr_db";
inline constexpr const char* dyneq3_ratio   = "reverb_dyneq3_ratio";
inline constexpr const char* dyneq3_attMs   = "reverb_dyneq3_attack_ms";
inline constexpr const char* dyneq3_relMs   = "reverb_dyneq3_release_ms";
inline constexpr const char* dyneq3_rangeDb = "reverb_dyneq3_range_db";
// Band 4
inline constexpr const char* dyneq4_on      = "reverb_dyneq4_on";
inline constexpr const char* dyneq4_mode    = "reverb_dyneq4_mode";
inline constexpr const char* dyneq4_freqHz  = "reverb_dyneq4_freq_hz";
inline constexpr const char* dyneq4_gainDb  = "reverb_dyneq4_gain_db";
inline constexpr const char* dyneq4_Q       = "reverb_dyneq4_q";
inline constexpr const char* dyneq4_thrDb   = "reverb_dyneq4_thr_db";
inline constexpr const char* dyneq4_ratio   = "reverb_dyneq4_ratio";
inline constexpr const char* dyneq4_attMs   = "reverb_dyneq4_attack_ms";
inline constexpr const char* dyneq4_relMs   = "reverb_dyneq4_release_ms";
inline constexpr const char* dyneq4_rangeDb = "reverb_dyneq4_range_db";

// Mix / Out
inline constexpr const char* wetMix01   = "reverb_wet_mix";
inline constexpr const char* outTrimDb  = "reverb_output_trim_db";
} // namespace ReverbIDs


