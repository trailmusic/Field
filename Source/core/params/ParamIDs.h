#pragma once
namespace field { namespace params {
static constexpr const char* kChainDelayEnable  = "chain.delay.enable";
static constexpr const char* kChainDynEqEnable  = "chain.dyneq.enable";
static constexpr const char* kChainReverbEnable = "chain.reverb.enable";

static constexpr const char* kQualityOSFactor   = "quality.os.factor"; // 1,2,4,8

static constexpr const char* kReverbLinearPhase = "reverb.linearPhase"; // bool
static constexpr const char* kReverbFIRHalfLen  = "reverb.fir.halfLenSamples"; // int samples

static constexpr const char* kDynEqLookAheadMs  = "dyneq.lookahead.ms"; // float ms
static constexpr const char* kDelayLookAheadMs  = "delay.lookahead.ms"; // float ms

// Dev HUD (guarded in layout)
static constexpr const char* kDevHudEnable      = "dev.hud.enable"; // bool

// Mix / Output
static constexpr const char* kMixWet01          = "mix.wet01";            // float 0..1
static constexpr const char* kOutGainDb         = "gain.output.db";       // float dB
static constexpr const char* kInGainDb          = "gain.input.db";        // float dB
static constexpr const char* kPanBalance        = "pan.balance";          // float -1..+1 (optional)

// Tone
static constexpr const char* kToneTiltDbPerOct  = "tone.tilt.dbPerOct";   // float dB/oct
static constexpr const char* kToneBassDb        = "tone.bass.db";         // float dB

// Reverb voicing
static constexpr const char* kRvPreDelayMs      = "reverb.preDelay.ms";   // float ms
static constexpr const char* kRvSizeNorm        = "reverb.size.norm";     // float 0..1
static constexpr const char* kRvDampingHz       = "reverb.damping.hz";    // float Hz

// Imager
static constexpr const char* kImagerWidth       = "imager.width";         // float 0..2

// Global
static constexpr const char* kGlobalBypass      = "global.bypass";        // bool
}} // namespace field::params
