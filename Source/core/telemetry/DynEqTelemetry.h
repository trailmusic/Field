#pragma once
#include <atomic>

namespace field { namespace core { namespace telemetry {

struct DynEqTelemetry
{
	inline static std::atomic<float> grDb[24] = { 0.0f }; // per-band gain reduction in dB (negative for downward)
};

inline float getDynEqGrDb (int bandIdx) noexcept
{
	if (bandIdx < 0 || bandIdx >= 24) return 0.0f;
	return DynEqTelemetry::grDb[bandIdx].load (std::memory_order_relaxed);
}

inline void setDynEqGrDb (int bandIdx, float grDb) noexcept
{
	if (bandIdx < 0 || bandIdx >= 24) return;
	DynEqTelemetry::grDb[bandIdx].store (grDb, std::memory_order_relaxed);
}

}}} // namespace field::core::telemetry
