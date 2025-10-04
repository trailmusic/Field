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
}} // namespace field::params
