#pragma once
#include <cmath>
#include "core/runtime/SafeParamGate.h"
#include "core/params/ParamIDs.h"

namespace field { namespace params {

struct ChainParamSnapshot
{
    bool bypass { false };
	bool enableDelay  {false};
	bool enableDynEq  {false};
	bool enableReverb {false};
	int  osFactor {1};
	bool  reverbLinearPhase {false};
	int   reverbFIRHalfLen  {0};
	float dynEqLookAheadMs  {0.f};
	float delayLookAheadMs  {0.f};

	// Mix / Output
	float wet01 {0.33f};
	float dry01 {0.67f};
    float outGainLin {1.0f};
    float inGainLin  {1.0f};
	float panBalance {-0.0f}; // -1..+1

	// Tone
	float toneTilt_dB_per_oct {+1.5f};
	float toneBass_dB         {+2.0f};

	// Reverb voicing
	float rvPreDelaySec {0.020f};
	float rvSizeNorm    {0.62f};
	float rvDampingHz   {6000.0f};

	// Imager
	float imagerWidth   {1.15f};

	// DynEQ (global + 24 bands stub)
	struct DynEqBandSnap {
		uint8_t enabled{0}, type{0}, direction{0}, sidechain{0};
		float   freqHz{1000.f}, q{1.2f}, staticGainLin{1.f}, rangeDb{6.f}, ratio{3.f};
		float   threshDbfs{-24.f}, kneeDb{6.f}, atkSec{0.008f}, relSec{0.120f}, holdSec{0.f};
		float   makeupLin{1.f}, scHP_Hz{20.f}, scLP_Hz{20000.f}, wet01{1.f};
	};
	struct DynEqSnap { bool enabled{false}; uint8_t globalMode{0}, link{0}; int lookaheadSamples{0}; DynEqBandSnap band[24]; } dyneq;
};

template <typename ProcessorLike>
static inline ChainParamSnapshot buildSnapshot (ProcessorLike& proc)
{
	ChainParamSnapshot s;
	using SG = field::core::runtime::SafeParamGate;
	s.enableDelay   = SG::getBool  (proc, kChainDelayEnable,  false);
	s.enableDynEq   = SG::getBool  (proc, kChainDynEqEnable,  false);
	s.enableReverb  = SG::getBool  (proc, kChainReverbEnable, false);
	s.osFactor          = SG::getInt   (proc, kQualityOSFactor,   1);
	s.reverbLinearPhase = SG::getBool  (proc, kReverbLinearPhase, false);
	s.reverbFIRHalfLen  = SG::getInt   (proc, kReverbFIRHalfLen,  0);
	s.dynEqLookAheadMs  = SG::getFloat (proc, kDynEqLookAheadMs,  0.f);
	s.delayLookAheadMs  = SG::getFloat (proc, kDelayLookAheadMs,  0.f);

	// Mix / Output
    s.bypass     = SG::getBool (proc, kGlobalBypass, false);
    s.wet01      = juce::jlimit (0.0f, 1.0f, SG::getFloat (proc, kMixWet01, 0.33f));
	s.dry01      = 1.0f - s.wet01;
	{
        const float inDb  = SG::getFloat (proc, kInGainDb,  0.0f);
        const float outDb = SG::getFloat (proc, kOutGainDb, 0.0f);
        s.inGainLin  = (inDb  <= -80.0f ? 0.0f : std::pow (10.0f, inDb  * 0.05f));
		s.outGainLin = (outDb <= -80.0f ? 0.0f : std::pow (10.0f, outDb * 0.05f));
	}
	s.panBalance = juce::jlimit (-1.0f, 1.0f, SG::getFloat (proc, kPanBalance, 0.0f));

	// Tone
	s.toneTilt_dB_per_oct = juce::jlimit (-6.0f, 6.0f, SG::getFloat (proc, kToneTiltDbPerOct, +1.5f));
	s.toneBass_dB         = juce::jlimit (-12.0f, 12.0f, SG::getFloat (proc, kToneBassDb, +2.0f));

	// Reverb voicing
	{
		const float ms = SG::getFloat (proc, kRvPreDelayMs, 20.0f);
		s.rvPreDelaySec = juce::jlimit (0.0f, 0.5f, ms * 0.001f);
	}
	s.rvSizeNorm  = juce::jlimit (0.0f, 1.0f, SG::getFloat (proc, kRvSizeNorm, 0.62f));
	s.rvDampingHz = juce::jlimit (1000.0f, 16000.0f, SG::getFloat (proc, kRvDampingHz, 6000.0f));

    // Imager
	s.imagerWidth = juce::jlimit (0.0f, 2.0f, SG::getFloat (proc, kImagerWidth, 1.15f));
    
    // DynEQ (global + 24 bands)
    {
        s.dyneq.enabled = SG::getBool (proc, juce::String { "dyneq.enabled" }, false);
        // These are choice indices in UI; keep as small integers
        s.dyneq.globalMode = (uint8_t) juce::jlimit (0, 3, SG::getInt (proc, juce::String { "dyneq.global.mode" }, 0));
        s.dyneq.link       = (uint8_t) juce::jlimit (0, 3, SG::getInt (proc, juce::String { "dyneq.global.link" }, 0));
        const float laMs   = SG::getFloat (proc, juce::String { "dyneq.global.lookahead.ms" }, 1.5f);
        const double sr    = ((juce::AudioProcessor*)&proc)->getSampleRate();
        s.dyneq.lookaheadSamples = (int) juce::roundToInt (juce::jlimit (0.0f, 10.0f, laMs) * 0.001f * (float) (sr > 0.0 ? sr : 48000.0));
        for (int i = 0; i < 24; ++i)
        {
            auto& b = s.dyneq.band[i];
            const juce::String base = "dyneq.b[" + juce::String(i) + "].";
            b.enabled   = (uint8_t) (SG::getBool  (proc, base + "enabled", false) ? 1 : 0);
            b.type      = (uint8_t) juce::jlimit (0, 5, SG::getInt (proc, base + "type", 0));
            b.direction = (uint8_t) juce::jlimit (0, 2, SG::getInt (proc, base + "direction", 0));
            b.sidechain = (uint8_t) juce::jlimit (0, 2, SG::getInt (proc, base + "sidechain", 0));
            b.freqHz    = juce::jlimit (20.0f, 20000.0f, SG::getFloat (proc, base + "freq.hz", 1000.0f));
            b.q         = juce::jlimit (0.1f, 24.0f,   SG::getFloat (proc, base + "q", 1.2f));
            b.staticGainLin = (SG::getFloat (proc, base + "gain.db", 0.0f) <= -80.0f ? 0.0f
                               : std::pow (10.0f, SG::getFloat (proc, base + "gain.db", 0.0f) * 0.05f));
            b.rangeDb   = juce::jlimit (0.0f, 24.0f,   SG::getFloat (proc, base + "range.db", 6.0f));
            b.ratio     = juce::jlimit (1.0f, 10.0f,   SG::getFloat (proc, base + "ratio", 3.0f));
            b.threshDbfs= juce::jlimit (-72.0f, 0.0f,  SG::getFloat (proc, base + "threshold.dbfs", -24.0f));
            b.kneeDb    = juce::jlimit (0.0f, 24.0f,   SG::getFloat (proc, base + "knee.db", 6.0f));
            b.atkSec    = juce::jlimit (0.0001f, 0.200f, SG::getFloat (proc, base + "attack.ms", 8.0f) / 1000.0f);
            b.relSec    = juce::jlimit (0.005f, 2.000f, SG::getFloat (proc, base + "release.ms", 120.0f) / 1000.0f);
            b.holdSec   = juce::jlimit (0.0f, 0.200f, SG::getFloat (proc, base + "hold.ms", 0.0f) / 1000.0f);
            b.makeupLin = (SG::getFloat (proc, base + "makeup.db", 0.0f) <= -80.0f ? 0.0f
                               : std::pow (10.0f, SG::getFloat (proc, base + "makeup.db", 0.0f) * 0.05f));
            b.scHP_Hz   = juce::jlimit (20.0f,  2000.0f,  SG::getFloat (proc, base + "sc.hp.hz", 20.0f));
            b.scLP_Hz   = juce::jlimit (2000.0f, 20000.0f, SG::getFloat (proc, base + "sc.lp.hz", 20000.0f));
            b.wet01     = juce::jlimit (0.0f, 1.0f, SG::getFloat (proc, base + "mix.wet01", 1.0f));
        }
    }
	return s;
}

}} // namespace field::params
