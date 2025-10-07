#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include "NodeLatency.h"
#include "../../core/telemetry/DynEqTelemetry.h"

namespace field { namespace modules { namespace nodes {
struct Node_DynEq : NodeLatencyMixin<Node_DynEq>
{
    struct DynEqParams {
        bool enabled{false}; int lookaheadSamples{0}; uint8_t globalMode{0}; uint8_t link{0};
        struct Band { uint8_t enabled{0}, type{0}, direction{0}, sidechain{0};
            float freqHz{}, q{}, staticGainLin{}, rangeDb{}, ratio{}, threshDbfs{}, kneeDb{};
            float atkSec{}, relSec{}, holdSec{}, makeupLin{}, scHP_Hz{}, scLP_Hz{}, wet01{}; } band[24];
    } params_{};

    void setParameters (const DynEqParams& p) noexcept { params_ = p; }

    template <typename Sample>
    void prepare (double sr, int /*maxBlock*/, int chans) noexcept {
        sr_ = (sr > 0.0 ? sr : 48000.0);
        chans_ = (chans > 0 ? chans : 2);
        for (int i = 0; i < 24; ++i) {
            scBP_L_[i].reset(); scBP_R_[i].reset();
            scBP_L_[i].coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass (sr_, 1000.0, 1.2).get();
            scBP_R_[i].coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass (sr_, 1000.0, 1.2).get();
            scHPF_L_[i].reset(); scHPF_R_[i].reset();
            scLPF_L_[i].reset(); scLPF_R_[i].reset();
            scHPF_L_[i].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr_, 20.0).get();
            scHPF_R_[i].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr_, 20.0).get();
            scLPF_L_[i].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass  (sr_, 20000.0).get();
            scLPF_R_[i].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass  (sr_, 20000.0).get();
            env_[i] = 0.0f;
            grDbZ_[i] = 0.0f;
            holdCount_[i] = 0;
            prevFreq_[i] = 0.0f; prevQ_[i] = 0.0f;
            prevScHP_[i] = 0.0f; prevScLP_[i] = 0.0f;
        }
    }

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& io) const noexcept
	{
		if (!params_.enabled) return;
		const int C = (int) io.getNumChannels();
		const int N = (int) io.getNumSamples();
		if (C == 0 || N == 0) return;
        const float eps = 1e-12f;
        // Per-band sidechain + envelope, then apply block-level GR via peaking EQ
		for (int b = 0; b < 24; ++b)
		{
			const auto& band = params_.band[b];
			if (!band.enabled) continue;
			if (band.wet01 <= 0.0001f) continue;
            const float freq = juce::jlimit (20.0f, 20000.0f, (float) band.freqHz);
            const float Q    = juce::jlimit (0.1f, 24.0f, (float) band.q);
            const float staticDb = 20.0f * std::log10 (std::max (eps, (float) band.staticGainLin));
            const float makeupDb = 20.0f * std::log10 (std::max (eps, (float) band.makeupLin));
            // Update sidechain filters to band center/Q only if meaningfully changed
            auto approxChanged = [](float oldV, float newV){
                if (oldV == 0.0f) return true;
                float rel = std::abs((newV - oldV) / oldV);
                return rel > 0.005f; // >0.5%
            };
            if (approxChanged(prevFreq_[b], freq) || approxChanged(prevQ_[b], Q))
            {
                prevFreq_[b] = freq; prevQ_[b] = Q;
                scBP_L_[b].coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass (sr_, freq, Q).get();
                scBP_R_[b].coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass (sr_, freq, Q).get();
            }
            const float scHP = juce::jlimit (20.0f, 2000.0f, (float) band.scHP_Hz);
            const float scLP = juce::jlimit (1000.0f, 20000.0f, (float) band.scLP_Hz);
            if (approxChanged(prevScHP_[b], scHP))
            {
                prevScHP_[b] = scHP;
                scHPF_L_[b].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr_, scHP).get();
                scHPF_R_[b].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr_, scHP).get();
            }
            if (approxChanged(prevScLP_[b], scLP))
            {
                prevScLP_[b] = scLP;
                scLPF_L_[b].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass  (sr_, scLP).get();
                scLPF_R_[b].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass  (sr_, scLP).get();
            }
            // Envelope smoothing (attack/release)
            float env = env_[b];
            const float atk = std::max (1e-4f, (float) band.atkSec);
            const float rel = std::max (1e-4f, (float) band.relSec);
            const float aA  = std::exp (-1.0f / (atk * (float) sr_));
            const float aRbase  = std::exp (-1.0f / (rel * (float) sr_));
            for (int i = 0; i < N; ++i)
            {
                // Detector source selection: 0=Pre, 1=Post (fallback to Pre), 2=Ext1, 3=Ext2
                float l = (float) io.getSample (0, i);
                float r = (C > 1 ? (float) io.getSample (1, i) : l);
                if (band.sidechain == 2 || band.sidechain == 3) { l = 0.0f; r = 0.0f; } // Ext feeds inactive → silence
                float sc;
                // Band-focused detection (BP) before HP/LP yields musical per-band sensing
                float lb = scBP_L_[b].processSample (l);
                float rb = (C > 1 ? scBP_R_[b].processSample (r) : lb);
                lb = scLPF_L_[b].processSample (scHPF_L_[b].processSample (lb));
                rb = (C > 1 ? scLPF_R_[b].processSample (scHPF_R_[b].processSample (rb)) : lb);
                sc = 0.5f * (std::abs(lb) + std::abs(rb));
                const float a = (sc > env ? aA : aRbase);
                env = sc + a * (env - sc);
            }
            env_[b] = env;
            const float levelDbfs = 20.0f * std::log10 (std::max (eps, env));
			// Compute GR (down/up/both) with cap by range
            float grDbTarget = 0.0f;
            const float over = levelDbfs - (float) band.threshDbfs;
            // Adaptive detector behavior: bias knee and ratio by program level
            float knee = (float) band.kneeDb;
            float ratioEff = (float) band.ratio;
            if (levelDbfs < -36.0f) {
                knee = juce::jmax (0.0f, knee - 6.0f);
                ratioEff = juce::jmax (1.0f, ratioEff * 0.85f);
            } else if (levelDbfs < -24.0f) {
                knee = juce::jmax (0.0f, knee - 3.0f);
            } else if (levelDbfs > -12.0f) {
                ratioEff = juce::jmax (1.0f, ratioEff - 0.5f);
            }
            auto softKneeDown = [&](float x){
                if (knee <= 1e-6f) return juce::jmax (0.0f, x);
                if (x <= 0.0f) return 0.0f;
                if (x >= knee) return x - knee * 0.5f;
                // quadratic easing in knee region
                return 0.5f * (x * x) / juce::jmax (1e-6f, knee);
            };
            auto softKneeUp = [&](float x){
                if (knee <= 1e-6f) return juce::jmax (0.0f, x);
                if (x <= 0.0f) return 0.0f;
                if (x >= knee) return x - knee * 0.5f;
                return 0.5f * (x * x) / juce::jmax (1e-6f, knee);
            };
			if (band.direction == 0 /*down*/ || band.direction == 2 /*both*/)
			{
				if (over > 0.0f) {
                    const float oEff = softKneeDown (over);
					const float comp = oEff - (oEff / juce::jmax (1.0f, ratioEff));
					grDbTarget -= std::min ((float) band.rangeDb, comp);
				}
			}
			if (band.direction == 1 /*up*/ || band.direction == 2 /*both*/)
			{
				if (over < 0.0f) {
                    const float below = -over;
                    const float bEff = softKneeUp (below);
                    const float expn = bEff - (bEff / juce::jmax (1.0f, ratioEff));
					grDbTarget += std::min ((float) band.rangeDb, expn);
				}
			}
            // Smooth GR with hold and program-dependent release
            float grZ = grDbZ_[b];
            const float absT = std::abs (grDbTarget), absZ = std::abs (grZ);
            const bool increasing = (absT > absZ + 1e-5f);
            if (increasing)
            {
                const float a = aA;
                grZ = grDbTarget + a * (grZ - grDbTarget);
                // Reset hold counter when engaging
                const int holdSamps = (int) juce::jmax (0.0f, (float) (band.holdSec * sr_));
                holdCount_[b] = holdSamps;
            }
            else
            {
                if (holdCount_[b] > 0)
                {
                    --holdCount_[b];
                }
                else
                {
                    // Program-dependent release: larger GR => slower
                    const float rangeAbs = juce::jmax (1e-4f, (float) std::abs (band.rangeDb));
                    const float frac = juce::jlimit (0.0f, 1.0f, absZ / rangeAbs);
                    const float scale = 0.5f + 1.5f * frac; // 0.5x .. 2.0x
                    const float relEff = std::max (1e-4f, (float) band.relSec * scale);
                    const float aRel = std::exp (-1.0f / (relEff * (float) sr_));
                    grZ = grDbTarget + aRel * (grZ - grDbTarget);
                }
            }
            grDbZ_[b] = grZ;
            // publish GR for UI (negative for downward)
            field::core::telemetry::setDynEqGrDb (b, grZ);
            // Apply makeup and wet scaling of dynamic gain
            const float tgtDb = staticDb + makeupDb + (grZ * juce::jlimit (0.0f, 1.0f, (float) band.wet01));
			const float tgtLin = std::pow (10.0f, tgtDb * 0.05f);
			auto coeff = juce::dsp::IIR::Coefficients<Sample>::makePeakFilter (sr_, (Sample)freq, (Sample)Q, (Sample)tgtLin);
			juce::dsp::IIR::Filter<Sample> filtL, filtR;
			filtL.coefficients = coeff;
			if (C > 1) filtR.coefficients = coeff;
			for (int ch = 0; ch < juce::jmin(C, chans_); ++ch)
			{
				auto blockCh = io.getSingleChannelBlock ((size_t) ch);
				juce::dsp::ProcessContextReplacing<Sample> ctx (blockCh);
				if (ch == 0) filtL.process (ctx); else filtR.process (ctx);
			}
		}
	}

	void reset() noexcept {}
private:
    double sr_ { 48000.0 };
    int    chans_ { 2 };
    mutable std::array<juce::dsp::IIR::Filter<float>, 24> scBP_L_{};
    mutable std::array<juce::dsp::IIR::Filter<float>, 24> scBP_R_{};
    mutable std::array<juce::dsp::IIR::Filter<float>, 24> scHPF_L_{};
    mutable std::array<juce::dsp::IIR::Filter<float>, 24> scHPF_R_{};
    mutable std::array<juce::dsp::IIR::Filter<float>, 24> scLPF_L_{};
    mutable std::array<juce::dsp::IIR::Filter<float>, 24> scLPF_R_{};
    mutable std::array<float, 24> env_{};
    mutable std::array<float, 24> grDbZ_{};
    mutable std::array<int,   24> holdCount_{};
    // Cached values to avoid unnecessary filter coefficient rebuilds
    mutable std::array<float, 24> prevFreq_{};
    mutable std::array<float, 24> prevQ_{};
    mutable std::array<float, 24> prevScHP_{};
    mutable std::array<float, 24> prevScLP_{};
};
}}} // namespace field::modules::nodes
