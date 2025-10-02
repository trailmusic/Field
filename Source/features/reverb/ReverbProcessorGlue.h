#pragma once
/*
====================================================================================================
 ReverbProcessorGlue — APVTS ⇄ Engine bridge (prepareToPlay/processBlock)
----------------------------------------------------------------------------------------------------
 Role
    - Pulls parameters from APVTS → packs ReverbParams → calls ReverbEngine.
    - Handles sidechain feed (post-FX dry) and channel layout consistency.
    - Lives outside your AudioProcessor to keep that class slender.

 Integration
    - Construct in your processor ctor: glue = std::make_unique<ReverbProcessorGlue>(apvts, engine);
    - Call glue->prepareToPlay(sr, block, inOutChans);
    - Call glue->processBlock(buffer, midi);

 Notes
    - Sidechain: we pass a copy of input pre-wet as "dry" (adjust to your routing).
====================================================================================================
*/

#include <JuceHeader.h>
#include "ReverbEngine.h"
#include "FieldReverbConfig.h"

class ReverbProcessorGlue
{
public:
    ReverbProcessorGlue (juce::AudioProcessorValueTreeState& s, ReverbEngine& e)
        : apvts (s), engine (e) {}

    void prepareToPlay (double sr, int blockSize, int inOutChannels)
    {
        sampleRate = sr;
        block      = blockSize;
        chans      = inOutChannels > 0 ? inOutChannels : 2;

        sidechain.setSize (chans, block);
        wet.setSize (chans, block);

        engine.prepare (sr, blockSize, chans);
        cacheAllParams(); // seed engine
    }

    void releaseResources() {}

    void processBlock (juce::AudioBuffer<float>& io, juce::MidiBuffer&)
    {
        const int N = io.getNumSamples();
        const int C = juce::jmin (chans, io.getNumChannels());

        // pull params (cheap, but we could throttle/smooth externally)
        cacheAllParams();

        // sidechain = a copy of input (post preFX dry, adjust if you have dedicated SC bus)
        sidechain.makeCopyOf (io);

        // wet path render (engine expects 100% wet)
        wet.setSize (C, N, false, false, true);
        for (int c=0;c<C;++c) wet.clear (c, 0, N);

        engine.processWet (wet, sidechain);

        // simple wet/dry mix — for Phase 1 use outTrim/wetMix if you have them in APVTS
        // Here we just add wet on top; your top-level mixer likely applies wet/dry elsewhere.
        for (int c=0;c<C;++c)
        {
            float* dst = io.getWritePointer (c);
            const float* w = wet.getReadPointer (c);
            for (int i=0;i<N;++i) dst[i] += w[i];
        }
    }

private:
    void cacheAllParams()
    {
        ReverbParams p{};

        // Core (adjust IDs to match your ReverbParamIDs)
        auto getF = [&](const juce::String& id, float def){ if (auto* v = apvts.getRawParameterValue(id)) return v->load(); return def; };
        auto getB = [&](const juce::String& id, bool def){ if (auto* v = apvts.getRawParameterValue(id)) return v->load() > 0.5f; return def; };
        auto getI = [&](const juce::String& id, int  def){ if (auto* v = apvts.getRawParameterValue(id)) return (int) std::lround (v->load()); return def; };

        // Fill p (map to your actual IDs)
        p.preDelayMs   = getF ("pre_ms", 0.f);
        p.decaySec     = getF ("decay_sec", 1.8f);
        p.density      = getF ("density_pct", 50.f);
        p.diffusion    = getF ("diffusion_pct", 50.f);
        p.modDepthCents= getF ("mod_depth_cent", 0.f);
        p.modRateHz    = getF ("mod_rate_hz", 0.2f);

        p.erLevelDb    = getF ("er_level_db", -6.f);
        p.erTimeMs     = getF ("er_time_ms", 40.f);
        p.erDensity    = getF ("er_density_pct", 50.f);
        p.erWidthPct   = getF ("er_width_pct", 75.f);
        p.erToTailPct  = getF ("er_to_tail_pct", 50.f);

        p.widthPct     = getF ("width_pct", 100.f);

        p.duckMode     = getI ("duck_mode", 0);
        p.duckDetector = getI ("duck_detector", 0);
        p.duckDepthDb  = getF ("duck_depth_db", 6.f);
        p.duckThrDb    = getF ("duck_threshold_db", -18.f);
        p.duckKneeDb   = getF ("duck_knee_db", 2.f);
        p.duckRatio    = getF ("duck_ratio", 3.f);
        p.duckAtkMs    = getF ("duck_attack_ms", 10.f);
        p.duckRelMs    = getF ("duck_release_ms", 120.f);
        p.duckBandHz   = getF ("duck_band_hz", 1000.f);
        p.duckBandQ    = getF ("duck_band_q", 1.0f);
        p.duckOn       = getB ("duck_on", false);

        p.freeze       = getB ("freeze", false);
        p.gateAmtPct   = getF ("gate_amt_pct", 0.f);
        p.shimmerAmtPct= getF ("shimmer_amt_pct", 0.f);
        p.shimmerIntervalMode = getI ("shimmer_mode", 0);

        engine.setParams (p);
    }

    juce::AudioProcessorValueTreeState& apvts;
    ReverbEngine& engine;

    double sampleRate = 48000.0;
    int block = 0, chans = 2;

    juce::AudioBuffer<float> sidechain, wet;
};
