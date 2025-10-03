#pragma once
#include "core/runtime/SafeParamGate.h"
#include "core/params/ParamIDs.h"

namespace field { namespace params {

struct ChainParamSnapshot
{
	bool enableDelay  {false};
	bool enableDynEq  {false};
	bool enableReverb {false};
	int  osFactor {1};
	bool  reverbLinearPhase {false};
	int   reverbFIRHalfLen  {0};
	float dynEqLookAheadMs  {0.f};
	float delayLookAheadMs  {0.f};
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
	return s;
}

}} // namespace field::params
