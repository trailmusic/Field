#error "Legacy path. Use engines/reverb/DSP/... and core/params/ParamIDs.h"
// Retired by WO-22: Use modules/FieldNodes/Node_Reverb (or Adapter_ReverbGlue) instead.
#error "ReverbProcessorGlue is retired. Replace includes with modules/FieldNodes/Node_Reverb or a minimal Adapter_ReverbGlue."
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
