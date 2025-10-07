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
	return s;
}

}} // namespace field::params
