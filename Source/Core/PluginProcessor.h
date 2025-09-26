#pragma once

#include <JuceHeader.h>
#include "dsp/Ducker.h"
#include "dsp/DelayEngine.h"
#include "dsp/PhaseModes.h"
#include "motion/MotionEngine.h"
#include "reverb/ReverbParamIDs.h"
#include "reverb/ReverbEngine.h"

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
    static constexpr const char* spaceAlgo  = "space_algo";   // 0=Room 1=Plate 2=Hall
    static constexpr const char* airDb      = "air_db";
    static constexpr const char* bassDb     = "bass_db";
    static constexpr const char* ducking    = "ducking";
    static constexpr const char* duckThrDb  = "duck_threshold_db";
    static constexpr const char* duckKneeDb = "duck_knee_db";
    static constexpr const char* duckRatio  = "duck_ratio";
    static constexpr const char* duckAtkMs  = "duck_attack_ms";
    static constexpr const char* duckRelMs  = "duck_release_ms";
    static constexpr const char* duckLAms   = "duck_lookahead_ms";
    static constexpr const char* duckRmsMs  = "duck_rms_ms";
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
    static constexpr const char* eqGain       = "eq_gain";          // EQ gain: -12..+12 dB
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
    // Width Designer additions
    static constexpr const char* widthMode          = "width_mode";            // 0=Classic, 1=Designer
    static constexpr const char* widthSideTiltDbOct = "width_side_tilt_db_oct";
    static constexpr const char* widthTiltPivotHz   = "width_tilt_pivot_hz";
    static constexpr const char* widthAutoDepth     = "width_auto_depth";
    static constexpr const char* widthAutoThrDb     = "width_auto_thr_db";
    static constexpr const char* widthAutoAtkMs     = "width_auto_atk_ms";
    static constexpr const char* widthAutoRelMs     = "width_auto_rel_ms";
    static constexpr const char* widthMax           = "width_max";
    // Phase Modes
    static constexpr const char* phaseMode  = "phase_mode"; // 0 Zero, 1 Natural, 2 Hybrid Linear

    // Center Group (Rows 3-4)
    static constexpr const char* centerPromDb        = "center_prom_db";        // -9..+9 dB
    static constexpr const char* centerFocusLoHz     = "center_f_lo_hz";        // 40..1000 Hz (log)
    static constexpr const char* centerFocusHiHz     = "center_f_hi_hz";        // 1000..12000 Hz (log)
    static constexpr const char* centerPunchAmt01    = "center_punch_amt";      // 0..1
    static constexpr const char* centerPunchMode     = "center_punch_mode";     // 0 toSides, 1 toCenter
    static constexpr const char* centerPhaseRecOn    = "center_phase_rec_on";   // bool
    static constexpr const char* centerPhaseAmt01    = "center_phase_rec_amt";  // 0..1
    static constexpr const char* centerLockOn        = "center_lock_on";        // bool
    static constexpr const char* centerLockDb        = "center_lock_db";        // 0..6 dB cap
}
// Delay UI bridge
#include "ui/delay/DelayUiBridge.h"
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
struct FloatReverbAdapter;   // Float-only reverb wrapper for the double chain

// ===============================
// Templated DSP Chain (declaration)
// ===============================

// MonoLowpassBank: 6/12/24 dB/oct via 1st/2nd/4th-order Butterworth
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
        // Epsilon gate to avoid thrashing coeff rebuilds
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

    // Process in-place on a 2ch low-band copy
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

    juce::dsp::IIR::Filter<Sample> lp1L,  lp1R;                  // 1st order
    juce::dsp::IIR::Filter<Sample> lp2aL, lp2aR, lp2bL, lp2bR;   // 2nd order sections

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

    // Lifecycle
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setParameters (const HostParams& hp);   // per-block ingress (double -> Sample)
    void process (Block);                        // main process
    float getCurrentDuckGrDb() const;            // meter: current GR dB
    float getReverbErRms() const;                // meter: ER RMS (approx)
    float getReverbTailRms() const;              // meter: Tail RMS
    float getDelayWetRmsL() const;                // meter: Delay wet RMS L
    float getDelayWetRmsR() const;                // meter: Delay wet RMS R
    double getDelayLastSamplesL() const;          // telemetry: last effective delay samples L
    double getDelayLastSamplesR() const;          // telemetry: last effective delay samples R
    int   getLinearPhaseLatencySamples() const { return (linConvolver ? linConvolver->getLatencySamples() : 0); }
    int   getFullLinearLatencySamples() const { return (fullLinearConvolver ? fullLinearConvolver->getLatencySamples() : 0); }

private:
    // ----- helpers -----
    void ensureOversampling (int osModeIndex);
    // Imaging helpers
    void applyThreeBandWidth (Block block,
                              Sample loHz, Sample hiHz,
                              Sample wLo, Sample wMid, Sample wHi);
    void applyShufflerWidth (Block block, Sample xoverHz, Sample wLow, Sample wHigh);
    void applyRotationAsym (Block block, Sample rotationRad, Sample asym);

    // Filters / tone
    void applyHP_LP     (Block, Sample hpHz, Sample lpHz);
    void ensureLinearPhaseKernel (double sr, Sample hpHz, Sample lpHz, int maxBlock, int numChannels);
    void requestLinearPhaseRedesign (double sr, Sample hpHz, Sample lpHz, int maxBlock, int numChannels);
    void updateTiltEQ   (Sample tiltDb, Sample pivotHz);
    void applyTiltEQ    (Block, Sample tiltDb, Sample pivotHz);
    void applyScoopEQ   (Block, Sample scoopDb, Sample scoopFreq);
    void applyBassShelf (Block, Sample bassDb, Sample bassFreq);
    void applyAirBand   (Block, Sample airDb, Sample airFreq);
    void applyFullLinearFIR (Block block); // composite linear-phase tone (Phase Mode = Full Linear)

    // Imaging / placement
    void applyWidthMS (Block, Sample width);
    void applyMonoMaker (Block, Sample monoHz);
    void applyPan (Block, Sample pan);
    void applySplitPan (Block, Sample panL, Sample panR);

    // Nonlinear / dynamics / FX
    void applySaturationOnBlock (juce::dsp::AudioBlock<Sample> b, Sample driveLin);
    void applySaturation (Block, Sample driveLin, Sample mix01, int osModeIndex);
    void applySpaceAlgorithm (Block, Sample depth01, int algo);
    void renderSpaceWet (juce::AudioBuffer<Sample>& wet);
    
    // Dynamic EQ
    void applyDynamicEq (Block audioBlock);
    void processBandChannel (Block audioBlock, int band, int channel, const Biquad& filter);

    // ----- state -----
    double sr { 48000.0 };

    // Oversampling (created on demand)
    std::unique_ptr<juce::dsp::Oversampling<Sample>> oversampling;
    int lastOsMode { -1 };

    // Core filters / EQ
    juce::dsp::StateVariableTPTFilter<Sample> hpFilter, lpFilter, depthLPF;
    juce::dsp::StateVariableTPTFilter<Sample> hpFilterB, lpFilterB;
    // Main zero-latency IIR path (non-resonant): Linkwitz–Riley per channel
    juce::dsp::LinkwitzRileyFilter<Sample> lrHpL, lrHpR;
    juce::dsp::LinkwitzRileyFilter<Sample> lrLpL, lrLpR;
    MonoLowpassBank<Sample>                   monoLP;                   // mono-maker lows with variable slope
    // Imaging band split filters (3-band via LP@lo and HP@hi)
    juce::dsp::LinkwitzRileyFilter<Sample>    bandLowLP_L, bandLowLP_R;
    juce::dsp::LinkwitzRileyFilter<Sample>    bandHighHP_L, bandHighHP_R;
    // Shuffler split (2-band LP@xover; HP via subtraction)
    juce::dsp::LinkwitzRileyFilter<Sample>    shuffLP_L, shuffLP_R;
    juce::dsp::IIR::Filter<Sample>            lowShelf, highShelf, airFilter, bassFilter, scoopFilter;
    juce::dsp::StateVariableTPTFilter<Sample> dcBlocker;
    // Last applied tone targets to gate coefficient rebuilds
    Sample lastTiltDb { (Sample) 1e9 }, lastTiltHz { (Sample) 1e9 };
    Sample lastScoopDb{ (Sample) 1e9 }, lastScoopHz{ (Sample) 1e9 };
    Sample lastBassDb { (Sample) 1e9 }, lastBassHz { (Sample) 1e9 };
    Sample lastAirDb  { (Sample) 1e9 }, lastAirHz  { (Sample) 1e9 };
    int toneCoeffCooldownSamples { 0 };
    // Tone crossfade to mask IIR coefficient swaps
    int toneXfadeSamplesLeft { 0 };
    int toneXfadeTotal       { 0 };
    juce::AudioBuffer<Sample> toneDryBuf; // per-block dry snapshot for tone wet crossfade
    // Width Designer: side-only tilt filters
    juce::dsp::IIR::Filter<Sample>            sTiltLow, sTiltHigh;

    // Reverb (float adapter for double chain)
    std::unique_ptr<FloatReverbAdapter>  reverbD;  // only used when Sample == double
    std::unique_ptr<juce::dsp::Reverb>   reverbF;  // used when Sample == float
    juce::dsp::Reverb::Parameters        rvParams;
    // Preallocated buses to avoid per-block allocations
    juce::AudioBuffer<Sample>            dryBusBuf;
    juce::AudioBuffer<Sample>            wetBusBuf;
    juce::AudioBuffer<Sample>            delayWetBuf; // delay wet-only bus
    // Smoothed wet mix (per-sample ramp)
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> wetMixSmoothed;
    // Smoothed reverb macro params
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> roomSizeSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> dampingSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> widthSmoothed;

    // New lightweight reverb state (no JUCE Reverb): simple dual delay feedback per channel
    std::vector<Sample> rvDelayL, rvDelayR;
    int rvIdxL { 0 }, rvIdxR { 0 };
    int rvLenL { 0 }, rvLenR { 0 };

    // Linear HP/LP (Hybrid Linear mode)
    std::unique_ptr<OverlapSaveConvolver<Sample>> linConvolver;
    int   linKernelLen { 4097 };
    float lastHpHzLP   { -1.0f };
    float lastLpHzLP   { -1.0f };
    // Debounce / hysteresis for FIR redesigns
    int   linKernelCooldownSamples { 0 };
    float lastDesignedHpHzLP { -1.0f };
    float lastDesignedLpHzLP { -1.0f };
    // Cache last prepared sizes for linConvolver to avoid per-block prepare
    int   linPreparedBlockLen { 0 };
    int   linPreparedChannels { 0 };
    // Full Linear cache
    std::unique_ptr<OverlapSaveConvolver<Sample>> fullLinearConvolver;
    int   fullKernelLen { 4097 };
    struct ToneKey { double tiltDb, bassDb, airDb, scoopDb, hpHz, lpHz, tiltFreq, scoopFreq, bassFreq, airFreq; int mode; } lastToneKey{};
    bool  fullLinearKernelDirty { true };     // redesign FIR only when tone changes
    // Auto-linear during edits: when macro tone is changing, temporarily use Full Linear FIR
    int    autoLinearSamplesLeft { 0 };     // countdown in samples
    double autoLinearHoldSec     { 0.0 };   // disabled to prevent FIR/IIR swaps during edits
    bool   paramsPrimed { false };          // avoid triggering auto-linear on first ingress
    std::shared_ptr<std::vector<float>> activeFullKernel; // RT-hot
    std::shared_ptr<std::vector<float>> pendingFullKernel; // bg-built
    // Cache last prepared sizes for fullLinearConvolver to avoid per-block prepare
    int   fullPreparedBlockLen { 0 };
    int   fullPreparedChannels { 0 };

    // Smoothed macro tone parameters (to reduce zipper/crackle)
    juce::SmoothedValue<Sample> tiltDbSm, tiltFreqSm;
    juce::SmoothedValue<Sample> bassDbSm, bassFreqSm;
    juce::SmoothedValue<Sample> airDbSm,  airFreqSm;
    juce::SmoothedValue<Sample> scoopDbSm, scoopFreqSm;
    // HP/LP cutoff smoothing (IIR mode) to avoid zipper during knob moves
    juce::SmoothedValue<Sample> hpHzSm, lpHzSm;
    // Cache last applied IIR HP/LP to avoid coefficient churn
    Sample lastAppliedHpHz { (Sample) -1 };
    Sample lastAppliedLpHz { (Sample) -1 };
    int    iirCoeffCooldownSamples { 0 };
    // HP/LP pinch hysteresis state: 0 = HP+LP, 1 = HP-only
    int    hpLpPinchState { 0 };
    // Smooth engage mix for HP/LP to avoid onset clicks
    juce::SmoothedValue<Sample> hpLpEngage;
    // Crossfade between A/B SVF banks on retune
    int  hpLpXfadeSamplesLeft { 0 };
    int  hpLpXfadeTotal       { 0 };
    bool hpLpUseBankB         { false }; // active bank: false=A, true=B
    juce::AudioBuffer<Sample> hpLpTemp;   // temp buffer for inactive-bank output during crossfade
    int  hpLpTempPreparedCh   { 0 };
    int  hpLpTempPreparedNs   { 0 };
    Sample bankA_hpHz { (Sample) 20 }, bankA_lpHz { (Sample) 20000 };
    Sample bankB_hpHz { (Sample) 20 }, bankB_lpHz { (Sample) 20000 };

    // High-order interpolation hooks (for future modulated delay lines)
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

    // Look-ahead ducker (per-Sample instance)
    fielddsp::Ducker<Sample>             ducker;
    
    // Delay - Direct custom algorithms (no complex engine wrapper)
    // Custom cubic Lagrange interpolation delay line
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
                
                // Cubic Lagrange interpolation
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
    
    // Motion Engine (moved from main processor)
    motion::MotionEngine                 motionEngine;
    motion::Params                       motionParams;
    bool motionEnginePrepared { false };
    
    // Reverb Engine (moved from main processor)
    ReverbEngine                         reverbEngine;
    bool reverbEnginePrepared { false };

    // Anti-alias/anti-imaging guards for OS Off around saturation
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
    // Short crossfade when OS factor changes
    int osXfadeSamplesLeft { 0 };
    int osXfadeTotal       { 0 };

    // Wet tone (HPF/LPF/Tilt) simple one-pole states for reverb bus
    Sample rv_hpStateL{}; Sample rv_hpStateR{};
    Sample rv_lpStateL{}; Sample rv_lpStateR{};
    Sample rv_tiltLP_L{}; Sample rv_tiltLP_R{};

    // Per-block params converted to Sample domain
    struct FieldParams
    {
        Sample gainLin{}, pan{}, panL{}, panR{}, depth{}, width{};
        Sample tiltDb{}, scoopDb{}, monoHz{}, hpHz{}, lpHz{};
        // New EQ shape/Q
        Sample shelfShapeS{};     // 0.25..1.50
        Sample filterQ{};         // 0.50..1.20
        Sample eqGainDb{};        // EQ gain
        Sample mixPct{};          // Mix percentage
        Sample hpQ{};             // per-filter Q
        Sample lpQ{};             // per-filter Q
        bool   tiltLinkS{};       // use shelfShapeS for tilt shelves
        bool   eqQLink{};         // link HP/LP Q to filterQ
        Sample satDriveLin{}, satMix{}; bool bypass{}; int spaceAlgo{};
        Sample airDb{}, bassDb{}, ducking{}; int osMode{}; bool splitMode{};
        // Ducking advanced params
        Sample duckThresholdDb{};
        Sample duckKneeDb{};
        Sample duckRatio{};
        Sample duckAttackMs{};
        Sample duckReleaseMs{};
        Sample duckLookaheadMs{};
        Sample duckRmsMs{};
        int    duckTarget{}; // 0=WetOnly, 1=Global
        Sample tiltFreq{}, scoopFreq{}, bassFreq{}, airFreq{};
        // Imaging additions (Sample domain)
        Sample xoverLoHz{}, xoverHiHz{};
        Sample widthLo{}, widthMid{}, widthHi{};
        // Width Designer params
        int    widthMode{};            // 0=Classic, 1=Designer
        Sample widthSideTiltDbOct{};   // dB/oct on S
        Sample widthTiltPivotHz{};     // pivot Hz
        Sample widthAutoDepth{};       // 0..1
        Sample widthAutoThrDb{};       // threshold in dB (S/M)
        Sample widthAutoAtkMs{};       // attack ms
        Sample widthAutoRelMs{};       // release ms
        Sample widthMax{};             // ceiling
        Sample rotationRad{};
        Sample asymmetry{};
        Sample shufflerLo{}, shufflerHi{};
        Sample shufflerXoverHz{};
        int    monoSlopeDbOct{};
        bool   monoAudition{};
        
        // Delay parameters
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
        // Sync helpers
        int    delayGridFlavor{};   // 0=S,1=D,2=T
        double tempoBpm{120.0};
        int    phaseMode{};

        // Reverb (Sample domain)
        bool   rvEnabled{};
        bool   rvKillDry{};
        int    rvAlgo{};
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
        Sample rvDreqLowX{};
        Sample rvDreqMidX{};
        Sample rvDreqHighX{};
        Sample rvWidthPct{};
        Sample rvWet01{};
        Sample rvOutTrimDb{};
        // Reverb ducking (Sample domain)
        Sample rvDuckDepthDb{};
        Sample rvDuckThrDb{};
        Sample rvDuckKneeDb{};
        Sample rvDuckRatio{};
        Sample rvDuckAtkMs{};
        Sample rvDuckRelMs{};
        Sample rvDuckLaMs{};
        Sample rvDuckRmsMs{};
        
        // Additional reverb ducking parameters
        Sample rvDuckDepth{};
        Sample rvDuckAttackMs{};
        Sample rvDuckReleaseMs{};
        Sample rvDuckThresholdDb{};
        Sample rvDuckLookaheadMs{};
        bool   rvDuckLinkGlobal{};
        
        // Motion parameters
        bool   motionEnabled{};
        int    motionPannerSelect{};
        bool   motionOcclusion{};
        bool   motionHeadphoneSafe{};
        Sample motionBassFloorHz{};
        
        // Dynamic EQ parameters
        bool   dynEqEnabled{};
        DynEqBand dynEqBands[24];
    } params;

    // Width Designer runtime state
    Sample aw_env = (Sample) 1.0; // auto-width smoothed gain
    Sample aw_alphaAtk = (Sample) 0.0, aw_alphaRel = (Sample) 0.0;

    // Reverb meters per-chain
    float rv_erRms { 0.0f };
    float rv_tailRms { 0.0f };
    float delay_wetRmsL { 0.0f };
    float delay_wetRmsR { 0.0f };

public:
    // removed eco flag
};

// ===============================
// HostParams (double snapshot)
// ===============================

struct HostParams
{
    double gainDb{}, pan{}, panL{}, panR{}, depth{}, width{};
    double tiltDb{}, scoopDb{}, monoHz{}, hpHz{}, lpHz{};
    // New EQ shape/Q
    double eqShelfShapeS{}; // S
    double eqFilterQ{};     // global Q
    double eqGainDb{};     // EQ gain
    double mixPct{};        // Mix percentage
    double hpQ{};           // per-filter
    double lpQ{};           // per-filter
    bool   tiltLinkS{};
    bool   eqQLink{};
    double satDriveDb{}, satMix{}; bool bypass{}; int spaceAlgo{};
    double airDb{}, bassDb{}, ducking{}; int osMode{}; bool splitMode{};
    // Ducking advanced
    double duckThresholdDb{};
    double duckKneeDb{};
    double duckRatio{};
    double duckAttackMs{};
    double duckReleaseMs{};
    double duckLookaheadMs{};
    double duckRmsMs{};
    int    duckTarget{}; // 0 wet, 1 global
    double tiltFreq{}, scoopFreq{}, bassFreq{}, airFreq{};
    // Imaging additions
    double xoverLoHz{};       // 40..400
    double xoverHiHz{};       // 800..6000
    double widthLo{};         // 0..2
    double widthMid{};        // 0..2
    double widthHi{};         // 0..2
    double rotationDeg{};     // -45..+45
    double asymmetry{};       // -1..+1
    double shufflerLoPct{};   // 0..200
    double shufflerHiPct{};   // 0..200
    double shufflerXoverHz{}; // 150..2000
    int    monoSlopeDbOct{};  // 6/12/24
    bool   monoAudition{};    // 0/1
    // Width Designer (Designer mode)
    int    widthMode{};            // 0=Classic, 1=Designer
    double widthSideTiltDbOct{};   // dB/oct on S
    double widthTiltPivotHz{};     // pivot Hz
    double widthAutoDepth{};       // 0..1
    double widthAutoThrDb{};       // threshold in dB (S/M)
    double widthAutoAtkMs{};       // attack ms
    double widthAutoRelMs{};       // release ms
    double widthMax{};             // ceiling
    // Reverb (new)
    bool   rvEnabled{};            // enabled
    bool   rvKillDry{};            // wet only
    int    rvAlgo{};               // algorithm index
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
    double rvDreqLowX{};
    double rvDreqMidX{};
    double rvDreqHighX{};
    double rvWidthPct{};
    double rvWet01{};
    double rvOutTrimDb{};
    // Reverb ducking
    double rvDuckDepthDb{};
    double rvDuckThrDb{};
    double rvDuckKneeDb{};
    double rvDuckRatio{};
    double rvDuckAtkMs{};
    double rvDuckRelMs{};
    double rvDuckLaMs{};
    double rvDuckRmsMs{};
        
    // Delay parameters
    bool   delayEnabled{};
    int    delayMode{};       // 0=Digital, 1=Analog, 2=Tape
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
    // Sync helpers
    int    delayGridFlavor{};   // 0=S,1=D,2=T
    double tempoBpm{120.0};
    int    phaseMode{}; // 0 Zero, 1 Natural, 2 Hybrid Linear
    
    // Motion parameters
    bool   motionEnabled{};
    int    motionPannerSelect{};
    bool   motionOcclusion{};
    bool   motionHeadphoneSafe{};
    double motionBassFloorHz{};
    
    // Dynamic EQ parameters
    bool   dynEqEnabled{};
    DynEqBand dynEqBands[24];
};

// ===============================
// Audio Processor
// ===============================

class MyPluginAudioProcessor : public juce::AudioProcessor,
                               private juce::AudioProcessorValueTreeState::Listener
{
public:
    MyPluginAudioProcessor();
    ~MyPluginAudioProcessor() override = default;

    // Capabilities / info
    const juce::String getName() const override                { return "Field"; }
    bool hasEditor() const override                            { return true; }
    bool acceptsMidi() const override                          { return false; }
    bool producesMidi() const override                         { return false; }
    bool isMidiEffect() const override                         { return false; }
    double getTailLengthSeconds() const override               {
        if (auto* p = apvts.getRawParameterValue (ReverbIDs::decaySec))
            return juce::jlimit (0.0, 20.0, (double) p->load() * 2.0);
        return 0.0;
    }
    bool supportsDoublePrecisionProcessing() const override    { return true; }

    // Undo system removed

    // Programs (single program)
    int getNumPrograms() override                              { return 1; }
    int getCurrentProgram() override                           { return 0; }
    void setCurrentProgram (int) override                      {}
    const juce::String getProgramName (int) override           { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    // Lifecycle
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override                           {
        // Clear UI visualization buses to avoid any pending reads on UI timers
        visPre.clearAll();
        visPost.clearAll();
        // Clear any legacy UI callbacks (editor should also null these in its dtor)
        onAudioSample = nullptr;
        onAudioBlock = nullptr;
        onAudioBlockPre = nullptr;
    }

    // Layout
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    // Processing (float & double paths)
    void processBlock (juce::AudioBuffer<float>&,  juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;

    // Editor
    juce::AudioProcessorEditor* createEditor() override;
    // Gesture gating for editor
    void setIsEditing (bool b) { isEditing.store (b, std::memory_order_release); }
    bool getIsEditing() const { return isEditing.load (std::memory_order_acquire); }

    // State
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void updateLatencyForPhaseMode();
    bool latencyLocked { false }; // prevent runtime latency changes

    // Parameters
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // removed eco mode

    // Safety passthrough: when true, processBlock returns input unmodified (hard bypass)
    std::atomic<bool> safePassthrough { false }; // OFF so audio processes; toggle if needed
    void setSafePassthrough (bool on) { safePassthrough.store (on, std::memory_order_release); }
    bool getSafePassthrough() const { return safePassthrough.load (std::memory_order_acquire); }

    // Undo manager removed

    // Visualization buses (audio thread → UI thread)
    VisBus visPre, visPost;
    // Delay visuals bridge
    DelayUiBridge& getDelayUiBridge() { return delayUiBridge; }

    // Deprecated: UI callback hooks (no longer used)
    std::function<void(double, double)> onAudioSample;
    std::function<void (const float* L, const float* R, int n)> onAudioBlock;
    std::function<void (const float* L, const float* R, int n)> onAudioBlockPre;
    // Meters
    float getCorrelation() const { return meterCorrelation.load(); }
    float getRmsL() const { return meterRmsL.load(); }
    float getRmsR() const { return meterRmsR.load(); }
    float getPeakL() const { return meterPeakL.load(); }
    float getPeakR() const { return meterPeakR.load(); }
    float getInRms() const { return meterInRms.load(); }
    float getOutRms() const { return meterOutRms.load(); }
    // Current ducking gain reduction in dB (>=0), from active precision chain
    float getCurrentDuckGrDb() const
    {
        if (isDoublePrecEnabled && chainD) return chainD->getCurrentDuckGrDb();
        if (chainF) return chainF->getCurrentDuckGrDb();
        return 0.0f;
    }
    // Reverb meters (UI polling)
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
    std::array<float,4> getReverbDynEqGrDb() const { 
        // Reverb Engine is now handled by FieldChain template
        return {0.0f, 0.0f, 0.0f, 0.0f}; 
    }
    
    // Transport info (UI polling)
    double getTransportTimeSeconds() const { return transportTimeSeconds.load(); }
    bool   isTransportPlaying() const      { return transportIsPlaying.load(); }
    
    // Motion Engine is now handled by FieldChain template
    
    // Link policy helpers
    bool isLinkOn() const {
        if (auto* param = apvts.getRawParameterValue(motion::id::panner_select)) {
            return (int)std::round(param->load()) == 2; // Link mode
        }
        return false;
    }

private:
    // APVTS listener
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    // Quality/precision application
    void applyQualityFromParams();

    // Optional host sync hooks (stubs in .cpp)
    void syncWithHostParameters();
    void updateHostParameters();

    // Background worker for heavy redesign (FIR, etc.)
    juce::ThreadPool backgroundPool { 1 };

    // Chains (float & double)
    std::unique_ptr<FieldChain<float>>  chainF;
    std::unique_ptr<FieldChain<double>> chainD;
    bool isDoublePrecEnabled { false };
    // Precision/quality state
    std::atomic<int> precisionMode { 0 }; // 0=Auto(Host), 1=Force32, 2=Force64
    std::atomic<int> qualityMode   { 1 }; // 0=Eco, 1=Standard, 2=High (reserved for future use)
    std::vector<double> scratch64;        // temporary double buffer for 64f-internal on 32f hosts
    // Edit gesture gating
public:
    std::atomic<bool> isEditing { false };
private:
    // Follow/override guards to protect existing UI logic
    std::atomic<bool> qualityApplyingGuard { false };
    std::atomic<bool> userOsOverride       { false };
    std::atomic<bool> userPhaseOverride    { false };
    std::atomic<bool> osFollowQuality      { true };
    std::atomic<bool> phaseFollowQuality   { true };
    // Auto-enable/disable Reverb based on Wet slider guard
    std::atomic<bool> reverbAutoGuard      { false };
    // Reverb engine (new)
    // Engines moved to FieldChain template for consistent architecture
    // Delay visuals bridge (owned by processor)
    DelayUiBridge delayUiBridge;
    
    // Link policy mirroring guard
    std::atomic<bool> mirrorGuard{false};

    // Smoothers (double-domain for precision)
    juce::SmoothedValue<double> panSmoothed, panLSmoothed, panRSmoothed,
                                depthSmoothed, widthSmoothed, gainSmoothed, tiltSmoothed,
                                hpHzSmoothed, lpHzSmoothed, monoHzSmoothed,
                                satDriveLin, satMixSmoothed, airSmoothed, bassSmoothed,
                                duckingSmoothed;

    // If you smooth the “start frequencies”, add them here as needed:
    // juce::SmoothedValue<double> tiltFreqSmoothed, scoopFreqSmoothed, bassFreqSmoothed, airFreqSmoothed;

    // Misc
    double currentSR { 48000.0 };
    // Metering
    std::atomic<float> meterCorrelation { 0.0f };
    std::atomic<float> meterRmsL { 0.0f }, meterRmsR { 0.0f };
    std::atomic<float> meterPeakL { 0.0f }, meterPeakR { 0.0f };
    std::atomic<float> meterInRms { 0.0f }, meterOutRms { 0.0f };

    // Host transport (updated in processBlock)
    std::atomic<double> transportTimeSeconds { 0.0 };
    std::atomic<bool>   transportIsPlaying   { false };

    // Loop/start detection + silence watchdog
    double lastTransportTimeSeconds { 0.0 };
    bool   lastTransportWasPlaying  { false };
    int    fadeInSamplesLeft        { 0 };      // short fade-in after loop/start
    int    fadeInTotal              { 0 };
    // Watchdog: simple sliding windows of recent RMS to detect stuck silence
    float  recentInRmsAvg           { 0.0f };
    float  recentOutRmsAvg          { 0.0f };
    int    watchdogSamplesAcc       { 0 };
    int    watchdogWindowSamples    { 0 };      // ~100 ms at current SR

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPluginAudioProcessor)
};
