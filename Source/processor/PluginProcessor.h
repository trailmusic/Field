#pragma once

#include <JuceHeader.h>
#include "core/telemetry/LiveSwapHUD.h"
#include "engines/dynamics/Ducker.h"
#include "engines/delay/DelayEngine.h"
#include "engines/phase/PhaseModes.h"
#include "engines/phase/PhaseAlignmentEngine.h"
#include "engines/phase/MinPhaseBankIntegration.h"
#include "engines/motion/MotionEngine.h"
#include "core/params/ParamIDs.h"
#include "features/reverb/Core/ReverbEngine.h"
#include "core/signal/SignalGraph.h"
#include "core/runtime/LatencyManager.h"
#include "core/runtime/TailManager.h"

// Forward declaration for Biquad struct
struct Biquad;

// Dynamic EQ band structure
struct DynEqBand {
    bool active = false;
    int type = 0;        // 0=Bell,1=LS,2=HS,3=HP,4=LP,5=Notch,6=BP,7=AllPass
    float freqHz = 1000.0f;
    float gainDb = 0.0f;
    float Q = 0.707f;
    int channel = 0;      // 0=Stereo,1=M,2=S,3=L,4=R
    
    // Dynamic processing
    bool dynOn = false;
    int dynMode = 0;     // 0=Down,1=Up
    float dynRangeDb = -3.0f;
    float dynThreshDb = -24.0f;
    float dynAtkMs = 10.0f;
    float dynRelMs = 120.0f;
    float dynRatio = 4.0f;  // Compression/expansion ratio
    
    // Spectral processing
    bool specOn = false;
    float specRangeDb = 3.0f;
    float specSelect = 50.0f;
    
    // Constellation processing
    bool constOn = false;
    int constRoot = 1;   // 0=Auto,1=Pitch,2=Note,3=Hz
    float constHz = 110.0f;
    int constCount = 6;
    float constSpread = 25.0f;
};

// =========================
// Parameter IDs
// =========================
namespace IDs {
    static constexpr const char* gain   = "gain_db";
    static constexpr const char* inputGain = "input_gain_db";
    static constexpr const char* outputGain = "output_gain_db";
    static constexpr const char* pan    = "pan";
    static constexpr const char* panL   = "pan_l";   // Split L
    static constexpr const char* panR   = "pan_r";   // Split R
    static constexpr const char* depth  = "depth";
    static constexpr const char* width  = "width";
    static constexpr const char* tilt   = "tilt";
    static constexpr const char* scoop  = "scoop";
    static constexpr const char* monoHz = "mono_hz";
    static constexpr const char* hpHz   = "hp_hz";
    static constexpr const char* lpHz   = "lp_hz";
    static constexpr const char* satDriveDb = "sat_drive_db";
    static constexpr const char* satMix     = "sat_mix";
    static constexpr const char* bypass     = "bypass";
    static constexpr const char* airDb      = "air_db";
    static constexpr const char* bassDb     = "bass_db";
    static constexpr const char* ducking    = "ducking";
    static constexpr const char* duckThrDb  = "duck_threshold_db";
    static constexpr const char* duckKneeDb = "duck_knee_db";
    static constexpr const char* duckRatio  = "duck_ratio";
    static constexpr const char* duckAtkMs  = "duck_attack_ms";
    static constexpr const char* duckRelMs  = "duck_release_ms";
    static constexpr const char* duckLAms   = "duck_lookahead_ms";
    static constexpr const char* duckTarget = "duck_target"; // 0 WetOnly, 1 Global
    static constexpr const char* osMode     = "os_mode";      // 0 Off, 1=2x, 2=4x
    static constexpr const char* splitMode  = "split_mode";   // 0 normal, 1 split
    // Quality / Precision controls
    static constexpr const char* quality    = "quality";      // 0 Eco, 1 Standard, 2 High
    static constexpr const char* precision  = "precision";    // 0 Auto(Host), 1 Force32, 2 Force64
    // EQ start freqs
    static constexpr const char* tiltFreq   = "tilt_freq";
    static constexpr const char* scoopFreq  = "scoop_freq";
    static constexpr const char* bassFreq   = "bass_freq";
    static constexpr const char* airFreq    = "air_freq";
    // Imaging
    static constexpr const char* xoverLoHz  = "xover_lo_hz";
    static constexpr const char* xoverHiHz  = "xover_hi_hz";
    static constexpr const char* widthLo    = "width_lo";
    static constexpr const char* widthMid   = "width_mid";
    static constexpr const char* widthHi    = "width_hi";
    static constexpr const char* rotationDeg= "rotation_deg";
    static constexpr const char* asymmetry  = "asymmetry";
    static constexpr const char* shufLoPct  = "shuffler_lo_pct";
    static constexpr const char* shufHiPct  = "shuffler_hi_pct";
    static constexpr const char* shufXHz    = "shuffler_xover_hz";
    static constexpr const char* monoSlope  = "mono_slope_db_oct";
    static constexpr const char* monoAud    = "mono_audition";
    
    // EQ shaping/Q link additions
    static constexpr const char* eqShelfShape = "eq_shelf_shape";  // S: 0.25..1.50
    static constexpr const char* eqFilterQ    = "eq_filter_q";     // global Q: 0.50..1.20
    static constexpr const char* mix          = "mix";              // Mix: 0..100%
    static constexpr const char* tiltLinkS    = "tilt_link_s";     // link Tilt shelves to S
    static constexpr const char* eqQLink      = "eq_q_link";       // link HP/LP Q to global
    static constexpr const char* hpQ          = "hp_q";            // per-filter Q
    static constexpr const char* lpQ          = "lp_q";            // per-filter Q
    
    // Delay parameters
    static constexpr const char* delayEnabled = "delay_enabled";
    static constexpr const char* delayMode = "delay_mode";
    static constexpr const char* delaySync = "delay_sync";
    static constexpr const char* delayGridFlavor = "delay_grid_flavor";
    static constexpr const char* delayTimeMs = "delay_time_ms";
    static constexpr const char* delayTimeDiv = "delay_time_div";
    static constexpr const char* delayFeedbackPct = "delay_feedback_pct";
    static constexpr const char* delayWet = "delay_wet";
    static constexpr const char* delayKillDry = "delay_kill_dry";
    static constexpr const char* delayFreeze = "delay_freeze";
    static constexpr const char* delayPingpong = "delay_pingpong";
    static constexpr const char* delayCrossfeedPct = "delay_crossfeed_pct";
    static constexpr const char* delayStereoSpreadPct = "delay_stereo_spread_pct";
    static constexpr const char* delayWidth = "delay_width";
    static constexpr const char* delayModRateHz = "delay_mod_rate_hz";
    static constexpr const char* delayModDepthMs = "delay_mod_depth_ms";
    static constexpr const char* delayWowflutter = "delay_wowflutter";
    static constexpr const char* delayJitterPct = "delay_jitter_pct";
    static constexpr const char* delayHpHz = "delay_hp_hz";
    static constexpr const char* delayLpHz = "delay_lp_hz";
    static constexpr const char* delayTiltDb = "delay_tilt_db";
    static constexpr const char* delaySat = "delay_sat";
    static constexpr const char* delayDiffusion = "delay_diffusion";
    static constexpr const char* delayDiffuseSizeMs = "delay_diffuse_size_ms";
    static constexpr const char* delayDuckSource = "delay_duck_source";
    static constexpr const char* delayDuckPost = "delay_duck_post";
    static constexpr const char* delayDuckDepth = "delay_duck_depth";
    static constexpr const char* delayDuckAttackMs = "delay_duck_attack_ms";
    static constexpr const char* delayDuckReleaseMs = "delay_duck_release_ms";
    static constexpr const char* delayDuckThresholdDb = "delay_duck_threshold_db";
    static constexpr const char* delayDuckRatio = "delay_duck_ratio";
    static constexpr const char* delayDuckLookaheadMs = "delay_duck_lookahead_ms";
    static constexpr const char* delayDuckLinkGlobal = "delay_duck_link_global";
    static constexpr const char* delayPreDelayMs = "delay_pre_delay_ms";
    static constexpr const char* delayFilterType = "delay_filter_type";
    // Phase Mode for EQ/Tone filtering (NOT Phase Alignment Engine)
    static constexpr const char* phaseMode = "phase_mode";  // 0=Zero, 1=Natural, 2=Hybrid, 3=Full Linear
    // Width Designer additions
    static constexpr const char* widthMode          = "width_mode";            // 0=Classic, 1=Designer
    static constexpr const char* widthSideTiltDbOct = "width_side_tilt_db_oct";
    static constexpr const char* widthTiltPivotHz   = "width_tilt_pivot_hz";
    static constexpr const char* widthAutoDepth     = "width_auto_depth";
    static constexpr const char* widthAutoThrDb     = "width_auto_thr_db";
    static constexpr const char* widthAutoAtkMs     = "width_auto_atk_ms";
    static constexpr const char* widthAutoRelMs     = "width_auto_rel_ms";
    static constexpr const char* widthMax           = "width_max";
    // Phase Alignment System (32 parameters)
    // Global/Routing (3 params)
    static constexpr const char* phase_ref_source    = "phase_ref_source";     // enum: AtoB, BtoA
    static constexpr const char* phase_channel_mode  = "phase_channel_mode";     // enum: Stereo, MS, DualMono
    static constexpr const char* phase_follow_xo     = "phase_follow_xo";       // bool: Off, On
    
    // Capture/Align (3 params)
    static constexpr const char* phase_capture_len   = "phase_capture_len";     // enum: 2s, 5s
    static constexpr const char* phase_align_mode    = "phase_align_mode";      // enum: Manual, Semi, Auto
    static constexpr const char* phase_align_goal    = "phase_align_goal";      // enum: MonoPunch, BassTight, StereoFocus
    
    // Polarity/Delay (6 params)
    static constexpr const char* phase_polarity_a    = "phase_polarity_a";     // bool: Normal, Invert
    static constexpr const char* phase_polarity_b    = "phase_polarity_b";     // bool: Normal, Invert
    static constexpr const char* phase_delay_ms_coarse = "phase_delay_ms_coarse"; // float: -20.00 to +20.00
    static constexpr const char* phase_delay_ms_fine  = "phase_delay_ms_fine";   // float: -1.000 to +1.000
    static constexpr const char* phase_delay_units    = "phase_delay_units";     // enum: ms, samples
    static constexpr const char* phase_link_mode     = "phase_link_mode";       // enum: Off, TimeOnly, AllBands
    
    // Engine/Latency/Commands (4 params)
    static constexpr const char* phase_engine         = "phase_engine";         // enum: Live, Studio
    static constexpr const char* phase_latency_ro    = "phase_latency_ro";      // readout: 0-200ms
    static constexpr const char* phase_reset_cmd     = "phase_reset_cmd";      // enum: Time, Phase, All
    static constexpr const char* phase_commit_cmd    = "phase_commit_cmd";      // button: Trigger
    
    // Banding/Crossovers (2 params)
    static constexpr const char* phase_xo_lo_hz      = "phase_xo_lo_hz";       // float: 40-400
    static constexpr const char* phase_xo_hi_hz      = "phase_xo_hi_hz";       // float: 800-6000
    
    // Per-Band All-Pass (6 params)
    static constexpr const char* phase_lo_ap_deg     = "phase_lo_ap_deg";     // float: 0-180
    static constexpr const char* phase_lo_q          = "phase_lo_q";           // float: 0.30-4.00
    static constexpr const char* phase_mid_ap_deg    = "phase_mid_ap_deg";    // float: 0-180
    static constexpr const char* phase_mid_q         = "phase_mid_q";          // float: 0.30-6.00
    static constexpr const char* phase_hi_ap_deg     = "phase_hi_ap_deg";     // float: 0-180
    static constexpr const char* phase_hi_q          = "phase_hi_q";          // float: 0.30-8.00
    
    // FIR Phase Match (1 param)
    static constexpr const char* phase_fir_len       = "phase_fir_len";        // int: 64, 128, 256, 512, 1024, 2048, 4096
    
    // Dynamic Phase (1 param)
    static constexpr const char* phase_dynamic_mode  = "phase_dynamic_mode";    // enum: Off, Light, Med, Hard
    
    // Monitoring/Output (4 params)
    static constexpr const char* phase_monitor_mode  = "phase_monitor_mode";   // enum: Stereo, MonoMinus6, Mid, Side, A, B
    static constexpr const char* phase_metric_mode   = "phase_metric_mode";     // enum: Corr, Coherence, DeltaPhiRMS, MonoLFRMS
    static constexpr const char* phase_audition_blend = "phase_audition_blend"; // enum: Apply100, Blend50
    static constexpr const char* phase_trim_db       = "phase_trim_db";         // float: -12 to +12
    
    // Logging/Preset Behavior (2 params)
    static constexpr const char* phase_rec_enable    = "phase_rec_enable";     // bool: Off, On
    static constexpr const char* phase_apply_on_load  = "phase_apply_on_load"; // bool: Off, On

    // Center Group (Rows 3-4)
    static constexpr const char* centerPromDb        = "center_prom_db";        // -9..+9 dB
    static constexpr const char* centerFocusLoHz     = "center_f_lo_hz";        // 40..1000 Hz (log)
    static constexpr const char* centerFocusHiHz     = "center_f_hi_hz";        // 1000..12000 Hz (log)
    static constexpr const char* centerPunchAmt01    = "center_punch_amt";      // 0..1
    static constexpr const char* centerPunchMode     = "center_punch_mode";     // 0 toSides, 1 toCenter
    static constexpr const char* centerLockOn        = "center_lock_on";        // bool
    static constexpr const char* centerLockDb        = "center_lock_db";        // 0..6 dB cap
    
    // SR-aware oversampling parameters
    static constexpr const char* osRealtime         = "oversampling_realtime";
    static constexpr const char* osOffline          = "oversampling_offline";
    static constexpr const char* osFilterType        = "oversampling_filter_type";
    static constexpr const char* tpSafe              = "true_peak_safe";
    static constexpr const char* forceOffline        = "force_offline_mode";
    // (Removed temporary Safe Bypass param; use existing header Bypass)
}
// Delay UI bridge
#include "features/delay/DelayUiBridge.h"
#include "shared/Core/FloatShim.h"
#include "shared/Core/PhaseBanks.h"
// ==================================
// Visualization Bus (lock-free SPSC)
// ==================================
struct VisBus
{
    static constexpr int kChannels = 2;
    static constexpr int kCapacity = 1 << 15; // storage capacity (frames after decimation)
    static constexpr int kDecim    = 8;       // keep 1 of every 8 samples
    juce::AbstractFifo fifo { kCapacity };
    juce::AudioBuffer<float> buf { kChannels, kCapacity };

    // Decimating push: reduces copy volume and UI work
    inline void push (const float* L, const float* R, int n) noexcept
    {
        if (n <= 0) return;
        const int frames = (n + kDecim - 1) / kDecim; // decimated frames
        int start1, size1, start2, size2;
        fifo.prepareToWrite (frames, start1, size1, start2, size2);
        auto* wL = buf.getWritePointer(0);
        auto* wR = buf.getWritePointer(1);
        auto write = [&](int start, int count, int offset)
        {
            for (int i = 0; i < count; ++i)
            {
                const int src = (offset + i) * kDecim;
                const float l = (L != nullptr && src < n) ? L[src] : 0.0f;
                wL[start + i] = l;
                wR[start + i] = (R != nullptr && src < n) ? R[src] : l;
            }
        };
        if (size1 > 0) write (start1, size1, 0);
        if (size2 > 0) write (start2, size2, size1);
        fifo.finishedWrite (size1 + size2);
    }

    inline int pull (juce::AudioBuffer<float>& out, int maxSamples)
    {
        if (maxSamples <= 0) return 0;
        int start1, size1, start2, size2;
        fifo.prepareToRead (maxSamples, start1, size1, start2, size2);
        const int total = size1 + size2;
        if (total <= 0) return 0;
        out.setSize (kChannels, total, false, false, true);
        for (int ch = 0; ch < kChannels; ++ch)
        {
            auto* dst = out.getWritePointer (ch);
            auto* src = buf.getReadPointer (ch);
            if (size1 > 0) memcpy (dst,        src + start1, sizeof(float)*size1);
            if (size2 > 0) memcpy (dst+size1,  src + start2, sizeof(float)*size2);
        }
        fifo.finishedRead (total);
        return total;
    }

    inline void clearAll() { fifo.reset(); }
};

// Forward decls used by the templated chain / processor snapshot
struct HostParams;           // Double-domain snapshot built each block in the processor

// ===============================
// Templated DSP Chain (declaration)
// ===============================

template <typename Sample>
struct MonoLowpassBank
{
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        reset();
        updateCoeffs();
    }

    void reset()
    {
        lp1L.reset(); lp1R.reset();
        lp2aL.reset(); lp2aR.reset();
        lp2bL.reset(); lp2bR.reset();
    }

    void setCutoff (Sample hz)
    {
        hz = juce::jlimit<Sample> ((Sample)20, (Sample)300, hz);
        if (std::abs (hz - cutoff) < (Sample) 0.5) return;
        cutoff = hz;
        updateCoeffs();
    }

    void setSlopeDbPerOct (int slope)
    {
        const int newSlope = juce::jlimit (6, 24, slope);
        if (newSlope == slopeDbPerOct) return;
        slopeDbPerOct = newSlope;
        if (slopeDbPerOct == 18) slopeDbPerOct = 12;
        updateCoeffs();
    }

    void processToLow (juce::dsp::AudioBlock<Sample> lowBlock)
    {
        jassert (lowBlock.getNumChannels() >= 2);
        auto L = lowBlock.getSingleChannelBlock (0);
        auto R = lowBlock.getSingleChannelBlock (1);

        juce::dsp::ProcessContextReplacing<Sample> ctxL (L);
        juce::dsp::ProcessContextReplacing<Sample> ctxR (R);

        if (slopeDbPerOct == 6)
        {
            lp1L.process (ctxL); lp1R.process (ctxR);
        }
        else if (slopeDbPerOct == 12)
        {
            lp2aL.process (ctxL); lp2aR.process (ctxR);
        }
        else
        {
            lp2aL.process (ctxL); lp2aR.process (ctxR);
            lp2bL.process (ctxL); lp2bR.process (ctxR);
        }
    }

    double sr = 48000.0;
    Sample cutoff = (Sample)120;
    int    slopeDbPerOct = 12;

    juce::dsp::IIR::Filter<Sample> lp1L,  lp1R;
    juce::dsp::IIR::Filter<Sample> lp2aL, lp2aR, lp2bL, lp2bR;

private:
    void updateCoeffs()
    {
        if (sr <= 0.0) return;
        lp1L.coefficients  = juce::dsp::IIR::Coefficients<Sample>::makeFirstOrderLowPass (sr, cutoff);
        lp1R.coefficients  = juce::dsp::IIR::Coefficients<Sample>::makeFirstOrderLowPass (sr, cutoff);
        auto c2 = juce::dsp::IIR::Coefficients<Sample>::makeLowPass (sr, cutoff);
        lp2aL.coefficients = c2; lp2aR.coefficients = c2; lp2bL.coefficients = c2; lp2bR.coefficients = c2;
    }
};

template <typename Sample>
struct FieldChain
{
    using Block  = juce::dsp::AudioBlock<Sample>;
    using CtxRep = juce::dsp::ProcessContextReplacing<Sample>;
    
    static constexpr int kMaxPreparedAudioBlock = 8192;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setParameters (const HostParams& hp);
    void process (Block);
    float getCurrentDuckGrDb() const;
    float getReverbErRms() const;
    float getReverbTailRms() const;
    float getDelayWetRmsL() const;
    float getDelayWetRmsR() const;
    double getDelayLastSamplesL() const;
    double getDelayLastSamplesR() const;
    int   getLinearPhaseLatencySamples() const { return (linConvolver ? linConvolver->getLatencySamples() : 0); }
    int   getFullLinearLatencySamples() const { return (fullLinearConvolver ? fullLinearConvolver->getLatencySamples() : 0); }
    int   getReverbLatencySamples() const { return reverbEnginePrepared ? reverbEngine.getReportedLatencySamples() : 0; }
    
    int   getPreparedBlockSize() const { return preparedBlockSize; }

private:
    void ensureOversampling (int osModeIndex);
    void applyThreeBandWidth (Block block,
                              Sample loHz, Sample hiHz,
                              Sample wLo, Sample wMid, Sample wHi);
    void applyShufflerWidth (Block block, Sample xoverHz, Sample wLow, Sample wHigh);
    void applyRotationAsym (Block block, Sample rotationRad, Sample asym);

    void applyHP_LP     (Block, Sample hpHz, Sample lpHz);
    void ensureLinearPhaseKernel (double sr, Sample hpHz, Sample lpHz, int maxBlock, int numChannels);
    void requestLinearPhaseRedesign (double sr, Sample hpHz, Sample lpHz, int maxBlock, int numChannels);
    void updateTiltEQ   (Sample tiltDb, Sample pivotHz);
    void applyTiltEQ    (Block, Sample tiltDb, Sample pivotHz);
    void applyScoopEQ   (Block, Sample scoopDb, Sample scoopFreq);
    void applyBassShelf (Block, Sample bassDb, Sample bassFreq);
    void applyAirBand   (Block, Sample airDb, Sample airFreq);
    void applyFullLinearFIR (Block block);

    void applyWidthMS (Block, Sample width);
    void applyMonoMaker (Block, Sample monoHz);
    void applyPan (Block, Sample pan);
    void applySplitPan (Block, Sample panL, Sample panR);

    void applySaturationOnBlock (juce::dsp::AudioBlock<Sample> b, Sample driveLin);
    void applySaturation (Block, Sample driveLin, Sample mix01, int osModeIndex);
    
    void applyDynamicEq (Block audioBlock);
    void processBandChannel (Block audioBlock, int band, int channel, const Biquad& filter);

    double sr { 48000.0 };
    int preparedBlockSize { 0 };

    std::unique_ptr<juce::dsp::Oversampling<Sample>> oversampling;
    int lastOsMode { -1 };

    juce::dsp::StateVariableTPTFilter<Sample> hpFilter, lpFilter, depthLPF;
    juce::dsp::StateVariableTPTFilter<Sample> hpFilterB, lpFilterB;
    juce::dsp::LinkwitzRileyFilter<Sample> lrHpL, lrHpR;
    juce::dsp::LinkwitzRileyFilter<Sample> lrLpL, lrLpR;
    MonoLowpassBank<Sample>                   monoLP;
    juce::dsp::LinkwitzRileyFilter<Sample>    bandLowLP_L, bandLowLP_R;
    juce::dsp::LinkwitzRileyFilter<Sample>    bandHighHP_L, bandHighHP_R;
    juce::dsp::LinkwitzRileyFilter<Sample>    shuffLP_L, shuffLP_R;
    juce::dsp::IIR::Filter<Sample>            lowShelf, highShelf, airFilter, bassFilter, scoopFilter;
    juce::dsp::StateVariableTPTFilter<Sample> dcBlocker;

    Sample lastTiltDb { (Sample) 1e9 }, lastTiltHz { (Sample) 1e9 };
    Sample lastScoopDb{ (Sample) 1e9 }, lastScoopHz{ (Sample) 1e9 };
    Sample lastBassDb { (Sample) 1e9 }, lastBassHz { (Sample) 1e9 };
    Sample lastAirDb  { (Sample) 1e9 }, lastAirHz  { (Sample) 1e9 };
    int toneCoeffCooldownSamples { 0 };
    int toneXfadeSamplesLeft { 0 };
    int toneXfadeTotal       { 0 };
    juce::AudioBuffer<Sample> toneDryBuf;
    juce::dsp::IIR::Filter<Sample>            sTiltLow, sTiltHigh;

    juce::AudioBuffer<Sample>            dryBusBuf;
    juce::AudioBuffer<Sample>            wetBusBuf;
    juce::AudioBuffer<Sample>            delayWetBuf;
    juce::AudioBuffer<float>             wetFloatScratch;
    juce::AudioBuffer<float>             dryFloatScratch;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> wetMixSmoothed;

    std::vector<Sample> rvDelayL, rvDelayR;
    int rvIdxL { 0 }, rvIdxR { 0 };
    int rvLenL { 0 }, rvLenR { 0 };

    std::unique_ptr<OverlapSaveConvolver<Sample>> linConvolver;
    int   linKernelLen { 4097 };
    float lastHpHzLP   { -1.0f };
    float lastLpHzLP   { -1.0f };
    int   linKernelCooldownSamples { 0 };
    float lastDesignedHpHzLP { -1.0f };
    float lastDesignedLpHzLP { -1.0f };
    int   linPreparedBlockLen { 0 };
    int   linPreparedChannels { 0 };

    std::unique_ptr<OverlapSaveConvolver<Sample>> fullLinearConvolver;
    int   fullKernelLen { 4097 };
    struct ToneKey { double tiltDb, bassDb, airDb, scoopDb, hpHz, lpHz, tiltFreq, scoopFreq, bassFreq, airFreq; int mode; } lastToneKey{};
    bool  fullLinearKernelDirty { true };
    int    autoLinearSamplesLeft { 0 };
    double autoLinearHoldSec     { 0.0 };
    bool   paramsPrimed { false };
    std::shared_ptr<std::vector<float>> activeFullKernel;
    std::shared_ptr<std::vector<float>> pendingFullKernel;
    int   fullPreparedBlockLen { 0 };
    int   fullPreparedChannels { 0 };
    
    std::unique_ptr<PhaseAlignmentEngine> phaseAlignmentEngine;
    juce::AudioBuffer<Sample> phaseDryBuffer;

    juce::SmoothedValue<Sample> tiltDbSm, tiltFreqSm;
    juce::SmoothedValue<Sample> bassDbSm, bassFreqSm;
    juce::SmoothedValue<Sample> airDbSm,  airFreqSm;
    juce::SmoothedValue<Sample> scoopDbSm, scoopFreqSm;
    juce::SmoothedValue<Sample> hpHzSm, lpHzSm;
    Sample lastAppliedHpHz { (Sample) -1 };
    Sample lastAppliedLpHz { (Sample) -1 };
    int    iirCoeffCooldownSamples { 0 };
    int    hpLpPinchState { 0 };
    juce::SmoothedValue<Sample> hpLpEngage;
    int  hpLpXfadeSamplesLeft { 0 };
    int  hpLpXfadeTotal       { 0 };
    bool hpLpUseBankB         { false };
    juce::AudioBuffer<Sample> hpLpTemp;
    int  hpLpTempPreparedCh   { 0 };
    int  hpLpTempPreparedNs   { 0 };
    Sample bankA_hpHz { (Sample) 20 }, bankA_lpHz { (Sample) 20000 };
    Sample bankB_hpHz { (Sample) 20 }, bankB_lpHz { (Sample) 20000 };

    template <typename S>
    static inline S readHermite4 (const S* d, int N, float rp)
    {
        int i1 = (int) rp; float t = rp - (float) i1;
        int i0 = (i1 - 1 + N) % N, i2 = (i1 + 1) % N, i3 = (i1 + 2) % N;
        S y0 = d[i0], y1 = d[i1], y2 = d[i2], y3 = d[i3];
        S c0 = y1;
        S c1 = (S) 0.5 * (y2 - y0);
        S c2 = y0 - (S) 2.5 * y1 + (S) 2.0 * y2 - (S) 0.5 * y3;
        S c3 = (S) 0.5 * (y3 - y0) + (S) 1.5 * (y1 - y2);
        return ((c3 * t + c2) * t + c1) * t + c0;
    }

    fielddsp::Ducker<Sample>             ducker;
    
    struct CustomDelayLine {
        void prepare(double sampleRate, double maxSeconds) {
            this->sampleRate = sampleRate;
            const size_t N = (size_t)std::ceil(maxSeconds * sampleRate) + 8;
            buffer.assign(N, Sample{});
            write = 0; size = N;
            active.read = 0; target.read = 0;
            xfadeSamples = 0; xfadePos = 0;
        }
        
        void setDelaySamples(double delaySamp) {
            delaySamp = juce::jlimit(1.0, (double)size - 4.0, delaySamp);
            const double newRead = wrap(write - delaySamp);
            const double delta = std::abs(newRead - active.read);
            if (delta > 32.0) {
                target.read = newRead;
                xfadeSamples = (int)std::round(0.02 * sampleRate);
                xfadePos = 0;
            } else {
                active.read = newRead;
            }
        }
        
        inline void push(Sample x) {
            buffer[write] = x;
            write = (write + 1) % size;
        }
        
        inline Sample read() {
            auto sampleAt = [&](double pos) -> Sample {
                const int i0 = (int)pos;
                const double frac = pos - i0;
                const int i1 = (i0 + 1) % size;
                const int i2 = (i0 + 2) % size;
                const int i3 = (i0 + 3) % size;
                
                const double um1 = frac - 1.0, up1 = frac + 1.0;
                const double a0 = -(frac * um1 * (frac - 2.0)) / 6.0;
                const double a1 = (up1 * um1 * (frac - 2.0)) / 2.0;
                const double a2 = -(up1 * frac * (frac - 2.0)) / 2.0;
                const double a3 = (up1 * frac * um1) / 6.0;
                
                return (Sample)(a0 * buffer[i0] + a1 * buffer[i1] + a2 * buffer[i2] + a3 * buffer[i3]);
            };
            
            if (xfadeSamples > 0) {
                const double xf = (double)xfadePos / (double)xfadeSamples;
                const Sample activeSample = sampleAt(active.read);
                const Sample targetSample = sampleAt(target.read);
                ++xfadePos;
                if (xfadePos >= xfadeSamples) {
                    active.read = target.read;
                    xfadeSamples = 0;
                }
                return (Sample)((1.0 - xf) * activeSample + xf * targetSample);
            }
            return sampleAt(active.read);
        }
        
        void reset() {
            std::fill(buffer.begin(), buffer.end(), Sample{});
            write = 0; active.read = 0; target.read = 0;
            xfadeSamples = 0; xfadePos = 0;
        }
        
    private:
        double sampleRate = 48000.0;
        std::vector<Sample> buffer;
        int write = 0, size = 0;
        struct { double read = 0; } active, target;
        int xfadeSamples = 0, xfadePos = 0;
        
        inline double wrap(double pos) {
            while (pos < 0) pos += size;
            while (pos >= size) pos -= size;
            return pos;
        }
    };
    
    CustomDelayLine delayLineL, delayLineR;
    bool delayPrepared { false };
    
    motion::MotionEngine                 motionEngine;
    motion::Params                       motionParams;
    bool motionEnginePrepared { false };
    int lastMotionBlockSize { 0 };
    
    ReverbEngine                         reverbEngine;
    bool reverbEnginePrepared { false };
    int lastReverbBlockSize { 0 };
    int lastReverbChannels { 0 };
    
    struct OnePoleBlockSmoother
    {
        void prepare(double newFs) noexcept { fs = (newFs > 0.0 ? newFs : 48000.0); }
        void setTimeMs(double ms) noexcept  { tau = juce::jlimit(1.0, 2000.0, ms) * 0.001; }
        void reset(float v) noexcept        { y = target = v; }
        void setTarget(float v) noexcept    { target = v; }
        void processBlock(int N) noexcept
        {
            if (N <= 0) return;
            if (tau <= 1e-6) { y = target; return; }
            const double a  = std::exp(-1.0 / (tau * fs));
            const double an = std::pow(a, (double) N);
            y = (float) (target + (y - target) * an);
        }
        float value() const noexcept { return y; }
    private:
        double fs   = 48000.0;
        double tau  = 0.080;
        float  y    = 0.0f;
        float  target = 0.0f;
    };
    
    struct DecayProfileSmoother
    {
        void prepare(double fs, float initTiltDb = 0.0f,
                     float initLoMult = 1.0f, float initHiMult = 1.0f,
                     float initMidBellDb = 0.0f) noexcept
        {
            tilt.prepare(fs);    lo.prepare(fs);     hi.prepare(fs);    midBell.prepare(fs);
            tilt.setTimeMs(tiltMs); lo.setTimeMs(multMs); hi.setTimeMs(multMs); midBell.setTimeMs(midMs);
            tilt.reset(initTiltDb);
            lo.reset(clampMult(initLoMult));
            hi.reset(clampMult(initHiMult));
            midBell.reset(initMidBellDb);
        }
        void setTargets(float tiltDb, float loMult, float hiMult, float midBellDb = 0.0f) noexcept
        {
            tilt.setTarget(juce::jlimit(-24.0f, 24.0f, tiltDb));
            lo.setTarget  (clampMult(loMult));
            hi.setTarget  (clampMult(hiMult));
            midBell.setTarget(juce::jlimit(-12.0f, 12.0f, midBellDb));
        }
        void processBlock(int N) noexcept
        {
            tilt.processBlock(N);
            lo  .processBlock(N);
            hi  .processBlock(N);
            midBell.processBlock(N);
        }
        float tiltDb()     const noexcept { return tilt.value(); }
        float loMult()     const noexcept { return lo.value(); }
        float hiMult()     const noexcept { return hi.value(); }
        float midBellDb()  const noexcept { return midBell.value(); }
        void setTimesMs(double tiltTimeMs, double multTimeMs, double midTimeMs) noexcept
        {
            tiltMs = tiltTimeMs; multMs = multTimeMs; midMs = midTimeMs;
            tilt.setTimeMs(tiltMs); lo.setTimeMs(multMs); hi.setTimeMs(multMs); midBell.setTimeMs(midMs);
        }
    private:
        static inline float clampMult(float m) noexcept { return juce::jlimit(0.25f, 4.0f, m); }
        OnePoleBlockSmoother tilt, lo, hi, midBell;
        double tiltMs = 80.0, multMs = 80.0, midMs = 100.0; 
    };
    
    DecayProfileSmoother rvDecaySm;

    juce::dsp::IIR::Filter<Sample> aliasGuardHP;
    juce::dsp::IIR::Filter<Sample> aliasGuardLP;
    bool aliasGuardsPrepared { false };
public:
    void prepareAliasGuards(double sampleRate)
    {
        const double fs = sampleRate;
        const double hp = juce::jlimit (5.0, 80.0, 40.0);
        const double lp = juce::jlimit (3000.0, juce::jmin (20000.0, fs * 0.49), 18000.0);
        aliasGuardHP.coefficients = juce::dsp::IIR::Coefficients<Sample>::makeHighPass (fs, hp);
        aliasGuardLP.coefficients = juce::dsp::IIR::Coefficients<Sample>::makeLowPass  (fs, lp);
        aliasGuardsPrepared = true;
    }
private:
    int osXfadeSamplesLeft { 0 };
    int osXfadeTotal       { 0 };

    Sample rv_hpStateL{}; Sample rv_hpStateR{};
    Sample rv_lpStateL{}; Sample rv_lpStateR{};
    Sample rv_tiltLP_L{}; Sample rv_tiltLP_R{};

    struct FieldParams
    {
        Sample gainLin{}, inputGainLin{}, outputGainLin{}, pan{}, panL{}, panR{}, depth{}, width{};
        Sample tiltDb{}, scoopDb{}, monoHz{}, hpHz{}, lpHz{};
        Sample shelfShapeS{}; 
        Sample filterQ{}; 
        Sample mixPct{}; 
        Sample hpQ{}; 
        Sample lpQ{}; 
        bool   tiltLinkS{}; 
        bool   eqQLink{}; 
        Sample satDriveLin{}, satMix{}; bool bypass{};
        Sample airDb{}, bassDb{}, ducking{}; int osMode{}; bool splitMode{};
        Sample duckThresholdDb{};
        Sample duckKneeDb{};
        Sample duckRatio{};
        Sample duckAttackMs{};
        Sample duckReleaseMs{};
        Sample duckLookaheadMs{};
        int    duckTarget{};
        Sample tiltFreq{}, scoopFreq{}, bassFreq{}, airFreq{};
        Sample xoverLoHz{}, xoverHiHz{};
        Sample widthLo{}, widthMid{}, widthHi{};
        int    widthMode{}; 
        Sample widthSideTiltDbOct{}; 
        Sample widthTiltPivotHz{}; 
        Sample widthAutoDepth{}; 
        Sample widthAutoThrDb{}; 
        Sample widthAutoAtkMs{}; 
        Sample widthAutoRelMs{}; 
        Sample widthMax{}; 
        Sample rotationRad{};
        Sample asymmetry{};
        Sample shufflerLo{}, shufflerHi{};
        Sample shufflerXoverHz{};
        int    monoSlopeDbOct{};
        bool   monoAudition{};
        
        bool   delayEnabled{};
        int    delayMode{};
        bool   delaySync{};
        Sample delayTimeMs{};
        int    delayTimeDiv{};
        Sample delayFeedbackPct{};
        Sample delayWet{};
        bool   delayKillDry{};
        bool   delayFreeze{};
        bool   delayPingpong{};
        Sample delayCrossfeedPct{};
        Sample delayStereoSpreadPct{};
        Sample delayWidth{};
        Sample delayModRateHz{};
        Sample delayModDepthMs{};
        Sample delayWowflutter{};
        Sample delayJitterPct{};
        Sample delayHpHz{};
        Sample delayLpHz{};
        Sample delayTiltDb{};
        Sample delaySat{};
        Sample delayDiffusion{};
        Sample delayDiffuseSizeMs{};
        int    delayDuckSource{};
        bool   delayDuckPost{};
        Sample delayDuckDepth{};
        Sample delayDuckAttackMs{};
        Sample delayDuckReleaseMs{};
        Sample delayDuckThresholdDb{};
        Sample delayDuckRatio{};
        Sample delayDuckLookaheadMs{};
        bool   delayDuckLinkGlobal{};
        int    delayGridFlavor{};   
        double tempoBpm{120.0};
        int    phaseMode{};

        bool   rvEnabled{};
        bool   rvKillDry{};
        bool   rvDuckOn{};
        Sample rvPreDelayMs{};
        Sample rvDecaySec{};
        Sample rvDensityPct{};
        Sample rvDiffusionPct{};
        Sample rvModDepthCents{};
        Sample rvModRateHz{};
        Sample rvErLevelDb{};
        Sample rvErTimeMs{};
        Sample rvErDensityPct{};
        Sample rvErWidthPct{};
        Sample rvErToTailPct{};
        Sample rvHpfHz{};
        Sample rvLpfHz{};
        Sample rvTiltDb{};
        Sample rvWidthPct{};
        Sample rvWet01{};
        Sample rvOutTrimDb{};
        int    rvDuckMode{};
        int    rvDuckDetector{};
        Sample rvDuckDepthDb{};
        Sample rvDuckThrDb{};
        Sample rvDuckKneeDb{};
        Sample rvDuckRatio{};
        Sample rvDuckAtkMs{};
        Sample rvDuckRelMs{};
        Sample rvDuckBandHz{};
        Sample rvDuckBandQ{};
        
        Sample rvDuckDepth{};
        Sample rvDuckAttackMs{};
        Sample rvDuckReleaseMs{};
        Sample rvDuckThresholdDb{};
        Sample rvDuckLookaheadMs{};
        bool   rvDuckLinkGlobal{};
        
        Sample rvDecayLoMult{};      
        Sample rvDecayHiMult{};      
        Sample rvDecayMidDb{};       
        Sample rvDecayMidFreqHz{};  
        Sample rvDecayMidQ{};        
        Sample rvDecayTiltDb{};      
        Sample rvDecaySmoothing{};   
        Sample rvDecayMode{};        
        
        int rvDecayProfileMode{};     
        int rvDecayProfileCoupling{}; 
        
        bool rvDecayLearn{};          
        bool rvDecayLearnReset{};     
        Sample rvDecayLearnStrength{}; 
        Sample rvDecayLearnWindow{};   
        
        bool   motionEnabled{};
        int    motionPannerSelect{};
        bool   motionOcclusion{};
        bool   motionHeadphoneSafe{};
        Sample motionBassFloorHz{};
        
        bool   dynEqEnabled{};
        DynEqBand dynEqBands[24];
    } params;

    Sample aw_env = (Sample) 1.0;
    Sample aw_alphaAtk = (Sample) 0.0, aw_alphaRel = (Sample) 0.0;

    float rv_erRms { 0.0f };
    float rv_tailRms { 0.0f };
    float delay_wetRmsL { 0.0f };
    float delay_wetRmsR { 0.0f };

    Sample filterStates[24][2][4] = {{{0}}};
    Sample envelopeStates[24][2] = {{0}};
    uint32_t rng = 0x1234567u;
    Sample lastHpLR = (Sample) -1, lastLpLR = (Sample) -1;

public:
    bool isLinkOn() const;
};

struct HostParams
{
    double gainDb{}, inputGainDb{}, outputGainDb{}, pan{}, panL{}, panR{}, depth{}, width{};
    double tiltDb{}, scoopDb{}, monoHz{}, hpHz{}, lpHz{};
    double eqShelfShapeS{};
    double eqFilterQ{};     
    double mixPct{};        
    double hpQ{};           
    double lpQ{};           
    bool   tiltLinkS{};
    bool   eqQLink{};
    double satDriveDb{}, satMix{}; bool bypass{};
    double airDb{}, bassDb{}, ducking{}; int osMode{}; bool splitMode{};
    double duckThresholdDb{};
    double duckKneeDb{};
    double duckRatio{};
    double duckAttackMs{};
    double duckReleaseMs{};
    double duckLookaheadMs{};
    int    duckTarget{}; 
    double tiltFreq{}, scoopFreq{}, bassFreq{}, airFreq{};
    double xoverLoHz{};       
    double xoverHiHz{};       
    double widthLo{};         
    double widthMid{};        
    double widthHi{};         
    double rotationDeg{};     
    double asymmetry{};       
    double shufflerLoPct{};   
    double shufflerHiPct{};   
    double shufflerXoverHz{}; 
    int    monoSlopeDbOct{};  
    bool   monoAudition{};    
    int    widthMode{};            
    double widthSideTiltDbOct{};   
    double widthTiltPivotHz{};     
    double widthAutoDepth{};       
    double widthAutoThrDb{};       
    double widthAutoAtkMs{};       
    double widthAutoRelMs{};       
    double widthMax{};             
    bool   rvEnabled{};            
    bool   rvKillDry{};            
    bool   rvDuckOn{};             
    double rvPreDelayMs{};
    double rvDecaySec{};
    double rvDensityPct{};
    double rvDiffusionPct{};
    double rvModDepthCents{};
    double rvModRateHz{};
    double rvErLevelDb{};
    double rvErTimeMs{};
    double rvErDensityPct{};
    double rvErWidthPct{};
    double rvErToTailPct{};
    double rvHpfHz{};
    double rvLpfHz{};
    double rvTiltDb{};
    double rvWidthPct{};
    double rvWet01{};
    double rvOutTrimDb{};
    int    rvDuckMode{};
    int    rvDuckDetector{};
    double rvDuckDepthDb{};
    double rvDuckThrDb{};
    double rvDuckKneeDb{};
    double rvDuckRatio{};
    double rvDuckAtkMs{};
    double rvDuckRelMs{};
    double rvDuckBandHz{};
    double rvDuckBandQ{};
    
    double rvDecayLoMult{};      
    double rvDecayHiMult{};      
    double rvDecayMidDb{};       
    double rvDecayMidFreqHz{};  
    double rvDecayMidQ{};        
    double rvDecayTiltDb{};      
    double rvDecaySmoothing{};   
    double rvDecayMode{};        
    
    int rvDecayProfileMode{};     
    int rvDecayProfileCoupling{}; 
    
    bool rvDecayLearn{};          
    bool rvDecayLearnReset{};     
    double rvDecayLearnStrength{}; 
    double rvDecayLearnWindow{};   
        
    bool   delayEnabled{};
    int    delayMode{};       
    bool   delaySync{};
    double delayTimeMs{};
    int    delayTimeDiv{};
    double delayFeedbackPct{};
    double delayWet{};
    bool   delayKillDry{};
    bool   delayFreeze{};
    bool   delayPingpong{};
    double delayCrossfeedPct{};
    double delayStereoSpreadPct{};
    double delayWidth{};
    double delayModRateHz{};
    double delayModDepthMs{};
    double delayWowflutter{};
    double delayJitterPct{};
    double delayHpHz{};
    double delayLpHz{};
    double delayTiltDb{};
    double delaySat{};
    double delayDiffusion{};
    double delayDiffuseSizeMs{};
    int    delayDuckSource{};
    bool   delayDuckPost{};
    double delayDuckDepth{};
    double delayDuckAttackMs{};
    double delayDuckReleaseMs{};
    double delayDuckThresholdDb{};
    double delayDuckRatio{};
    double delayDuckLookaheadMs{};
    bool   delayDuckLinkGlobal{};
    int    delayGridFlavor{};   
    double tempoBpm{120.0};
    int    phaseMode{}; 
    
    bool   motionEnabled{};
    int    motionPannerSelect{};
    bool   motionOcclusion{};
    bool   motionHeadphoneSafe{};
    double motionBassFloorHz{};
    
    bool   dynEqEnabled{};
    DynEqBand dynEqBands[24];
};

class MyPluginAudioProcessor : public juce::AudioProcessor,
                               private juce::AudioProcessorValueTreeState::Listener
{
public:
    MyPluginAudioProcessor();
    ~MyPluginAudioProcessor() override = default;

    const juce::String getName() const override                { return "Field"; }
    bool hasEditor() const override                            { return true; }
    bool acceptsMidi() const override                          { return false; }
    bool producesMidi() const override                         { return false; }
    bool isMidiEffect() const override                         { return false; }
    int getLatencySamples() const                          { return latency.getApplied(); }
    double getTailLengthSeconds() const override           { return tail_.getSeconds(); }
    bool supportsDoublePrecisionProcessing() const override    { return true; }

    int getNumPrograms() override                              { return 1; }
    int getCurrentProgram() override                           { return 0; }
    void setCurrentProgram (int) override                      {}
    const juce::String getProgramName (int) override           { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
    void refreshReportedLatency();
    void prepareEnginesWithBufferSize(const juce::AudioBuffer<float>& buffer);
    void prepareEnginesWithBufferSize(const juce::AudioBuffer<double>& buffer);

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&,  juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

    // Message-thread tick to plan/arm live swap (dev-only usage)
    void messageThreadTickForLiveSwap (double sampleRate, int maxBlock);
    // HUD accessor for potential overlay (dev-only)
    const field::core::telemetry::LiveSwapHUD& hud() const noexcept { return hud_; }
    field::core::telemetry::LiveSwapHUD&       hud()       noexcept { return hud_; }

    // Expose APVTS for SafeParamGate prepare-time reads
    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }

    void setIsEditing (bool b) { isEditing.store (b, std::memory_order_release); }
    bool getIsEditing() const { return isEditing.load (std::memory_order_acquire); }

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void updateLatencyForPhaseMode();
    bool latencyLocked { false };

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::atomic<bool> safePassthrough { false };
    void setSafePassthrough (bool on) { safePassthrough.store (on, std::memory_order_release); }
    bool getSafePassthrough() const { return safePassthrough.load (std::memory_order_acquire); }

    VisBus visPre, visPost;
    LatencyManager latency;
    field::core::runtime::TailManager tail_;
    DelayUiBridge& getDelayUiBridge() { return delayUiBridge; }

    std::function<void(double, double)> onAudioSample;
    std::function<void (const float* L, const float* R, int n)> onAudioBlock;
    std::function<void (const float* L, const float* R, int n)> onAudioBlockPre;

    float getCorrelation() const { return meterCorrelation.load(); }
    float getRmsL() const { return meterRmsL.load(); }
    float getRmsR() const { return meterRmsR.load(); }
    float getPeakL() const { return meterPeakL.load(); }
    float getPeakR() const { return meterPeakR.load(); }
    float getInRms() const { return meterInRms.load(); }
    float getOutRms() const { return meterOutRms.load(); }

    float getCurrentDuckGrDb() const
    {
        if (isDoublePrecEnabled && chainD) return chainD->getCurrentDuckGrDb();
        if (chainF) return chainF->getCurrentDuckGrDb();
        return 0.0f;
    }
    float getReverbDuckGrDb() const {
        if (isDoublePrecEnabled && chainD) return chainD->getCurrentDuckGrDb();
        if (chainF) return chainF->getCurrentDuckGrDb();
        return 0.0f;
    }
    float getReverbErRms() const {
        if (isDoublePrecEnabled && chainD) return chainD->getReverbErRms();
        if (chainF) return chainF->getReverbErRms();
        return 0.0f;
    }
    float getReverbTailRms() const {
        if (isDoublePrecEnabled && chainD) return chainD->getReverbTailRms();
        if (chainF) return chainF->getReverbTailRms();
        return 0.0f;
    }
    float getReverbWidthNow() const { return 100.0f; }
    
    double getTransportTimeSeconds() const { return transportTimeSeconds.load(); }
    bool   isTransportPlaying() const      { return transportIsPlaying.load(); }
    
    bool isLinkOn() const {
        if (auto* rp = apvts.getParameter ("panner_select"))
            if (const auto* cp = dynamic_cast<const juce::AudioParameterChoice*> (rp))
                return cp->getIndex() == 2;
        return false;
    }

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    void applyQualityFromParams();
    
    void onQualityChanged(int quality);
    void onOSChanged(int os);
    void onPhaseChanged(int phase);
    
    void onOSRealtimeChanged(int os);
    void onOSOfflineChanged(int os);  
    void onOSFilterTypeChanged(int type);
    void onTPSafeChanged(bool enabled);
    
    void resetManualOverrides();
    void scheduleDspRebuildIfNeeded();
    
    template <typename Sample>
    void rebuildDspForConfig(juce::AudioBuffer<Sample>& buffer);
    inline int osLatencySamples(int factor);
    void startTopologyCrossfadeMs(float ms);
    
    bool isNonRealtime() const;
    
    template <typename Sample>
    void applyTruePeakProtection(juce::AudioBuffer<Sample>& buffer, bool enabled);

    void syncWithHostParameters();
    void updateHostParameters();

    juce::ThreadPool backgroundPool { 1 };

    std::unique_ptr<FieldChain<float>>  chainF;
    std::unique_ptr<FieldChain<double>> chainD;
    std::unique_ptr<SignalGraph> graphF, graphD;
    int preparedMax_ { 0 };
    bool isDoublePrecEnabled { false };
    std::atomic<int> precisionMode { 0 };
    std::atomic<int> qualityMode   { 1 };
    std::vector<double> scratch64;
public:
    std::atomic<bool> isEditing { false };
private:
    std::atomic<bool> qualityApplyingGuard { false };
    std::atomic<bool> userOsOverride       { false };
    std::atomic<bool> userPhaseOverride    { false };
    std::atomic<bool> osFollowQuality      { true };
    std::atomic<bool> phaseFollowQuality   { true };
    std::atomic<bool> reverbAutoGuard      { false };
    DelayUiBridge delayUiBridge;
    field::core::telemetry::LiveSwapHUD hud_;
    
    std::atomic<bool> mirrorGuard{false};

    juce::SmoothedValue<double> panSmoothed, panLSmoothed, panRSmoothed,
                                depthSmoothed, widthSmoothed, gainSmoothed, tiltSmoothed,
                                hpHzSmoothed, lpHzSmoothed, monoHzSmoothed,
                                satDriveLin, satMixSmoothed, airSmoothed, bassSmoothed,
                                duckingSmoothed;

    double currentSR { 48000.0 };
    std::atomic<float> meterCorrelation { 0.0f };
    std::atomic<float> meterRmsL { 0.0f }, meterRmsR { 0.0f };
    std::atomic<float> meterPeakL { 0.0f }, meterPeakR { 0.0f };
    std::atomic<float> meterInRms { 0.0f }, meterOutRms { 0.0f };

    std::atomic<double> transportTimeSeconds { 0.0 };
    std::atomic<bool>   transportIsPlaying   { false };

    double lastTransportTimeSeconds { 0.0 };
    bool   lastTransportWasPlaying  { false };
    int    fadeInSamplesLeft        { 0 };
    int    fadeInTotal              { 0 };
    float  recentInRmsAvg           { 0.0f };
    float  recentOutRmsAvg          { 0.0f };
    int    watchdogSamplesAcc       { 0 };
    int    watchdogWindowSamples    { 0 };

public:
    struct ClockSnapshot {
        bool  playing = false;
        bool  looping = false;
        int64_t samplePos = 0;
        double sampleRate = 48000.0;
        double bpm = 120.0;
        int numerator = 4, denominator = 4;
        int64_t latencySamples = 0;
    };

public:
    juce::AbstractFifo clockFifo { 256 };
    std::array<ClockSnapshot, 256> clockRing;
    std::atomic<ClockSnapshot*> lastClockForUI { nullptr };

    std::atomic<bool> needsDspRebuild { false };
    
    std::unique_ptr<juce::dsp::Oversampling<float>>  osF;
    std::unique_ptr<juce::dsp::Oversampling<double>> osD;
    
    PhaseBanks phaseBanksF, phaseBanksD;
    
    int topologyXfadeSamplesLeft { 0 };
    int topologyXfadeTotal { 0 };

private:
    std::unique_ptr<PhaseAlignmentEngine> phaseAlignmentEngine;
    juce::AudioBuffer<float> phaseDryBuffer;
    
    std::unique_ptr<MinPhaseBankIntegration> minPhaseBank;

    inline void pushClockSnapshot (const ClockSnapshot& s)
    {
        int s1, n1, s2, n2;
        clockFifo.prepareToWrite (1, s1, n1, s2, n2);
        if (n1 > 0) { 
            clockRing[(size_t)s1] = s; 
            lastClockForUI.store(&clockRing[(size_t)s1], std::memory_order_release); 
        }
        clockFifo.finishedWrite (n1 + n2);
    }

    inline double visSeconds(double samplePos, double sr, int latencySamples) const {
        return std::max(0.0, (samplePos - latencySamples) / std::max(1.0, sr));
    }
    
    inline int getCurrentLatencySamples() const { return latency.getApplied(); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPluginAudioProcessor)
};

// DspRuntimeConfig fully removed; no forward declarations remain
