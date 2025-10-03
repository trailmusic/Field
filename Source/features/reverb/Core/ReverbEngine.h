/*
====================================================================================================
 ReverbEngine — ER + Ducking (Phase 1) with FDN / Decay-Rate EQ / Tone EQ scaffolding
----------------------------------------------------------------------------------------------------
What's implemented now (Phase 1)
  • Early Reflections (ER): single-write/multi-read ring per channel, per-tap gains, equal-power pan,
    and per-tap biquads (per-channel state). Proper energy normalization and width mapping.
  • Ducking: pointer-selected detector (Dry/ER/Tail/Wet), windowed RMS (ring of mean-squares),
    soft-knee transfer, look-ahead FIFO, depth as a *cap* in dB, attack/release smoothing.
  • Meters: atomic ER/Tail RMS and current ducking GR in dB.

Scaffolding (ready for next PRs)
  • FDN tank: typed shell with prepare/reset/setParams/process and hooks for Decay-Rate profile and Tone EQ.
  • Decay-Rate EQ: small vector<Band> (Bell/TiltLo/TiltHi) accepted via setDecayRateProfile().
  • Tone EQ: static IIR set; Apply routing enum { Pre, Post, EROnly, TailOnly }.

Performance & safety
  • No allocations in processWet(); all buffers pre-sized in prepare().
  • Coefficients/state sized in setParams()/prepare(). Thread-safe meters via std::atomic.
  • ER uses per-tap filters with per-channel state; no denormals in typical JUCE builds.

Wiring plan (Phase 2+)
  • Tail = FDN(input + ER); Decay-Rate profile becomes frequency-dependent loss inside feedback path.
  • Tone EQ honors routing: Pre (before ER), EROnly, TailOnly, Post (after ducking).

Author's notes
  • API is stable for UI/processor integration: setParams(), setDecayRateProfile(), setToneEq().
  • This header is intentionally self-contained (biquad type shared across modules).
====================================================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <atomic>
#include "FieldReverbConfig.h"
#include "ReverbTypes.h"

#if FIELD_REVERB_PHASE2
#include "../DSP/ReverbFDN.h"
#endif

// ===================== Shared biquad (IIR) ==================================
struct ERFilter
{
    double b0{}, b1{}, b2{}, a0{1.0}, a1{}, a2{};
    std::vector<float> z1, z2; // per-channel states

    void resize (int channels) { z1.assign (channels, 0.f); z2.assign (channels, 0.f); }

    static ERFilter makePeaking (double fs, double f0, double Q, double gainDb);
    static ERFilter makeLowShelf (double fs, double f0, double Q, double gainDb);
    static ERFilter makeHighShelf(double fs, double f0, double Q, double gainDb);

    void  processInPlace (juce::AudioBuffer<float>& buf);
    float processSample (float x, int ch);
};

// ===================== Engine parameter bundle ==============================
struct ReverbParams
{
    // Core / Tank
    float preDelayMs{}, decaySec{}, density{}, diffusion{}, modDepthCents{}, modRateHz{};

    // ER
    float erLevelDb{}, erTimeMs{}, erDensity{}, erWidthPct{}, erToTailPct{};

    // Imaging (future)
    float widthPct{}, widthStartPct{}, widthEndPct{}, widthCurve{};
    float rotStartDeg{}, rotEndDeg{}, rotCurve{};

    // Ducking
    int   duckMode{}, duckDetector{};
    float duckDepthDb{}, duckThrDb{}, duckKneeDb{}, duckRatio{};
    float duckAtkMs{},  duckRelMs{},  duckBandHz{},  duckBandQ{};
    bool  duckOn{};

    // FX (future; placeholders kept for completeness)
    bool  freeze{};
    float gateAmtPct{}, shimmerAmtPct{};
    int   shimmerIntervalMode{};
};

// ===================== ReverbEngine =========================================
class ReverbEngine
{
public:
    void prepare (double sr, int maxBlock, int channels);
    void reset   ();

    void setParams (const ReverbParams& p);
    void setDecayRateProfile (const DecayRateProfile& p);
    void setToneEq (const ToneEq& eq);

    // Render 100% wet into 'wet'. 'sidechain' is post-FX dry (for ducking).
    void processWet (juce::AudioBuffer<float>& wet,
                     const juce::AudioBuffer<float>& sidechain);

    float  getCurrentDuckGrDb() const noexcept { return duckGrDb.load(); }
    float  getErRms()           const noexcept { return erRms.load(); }
    float  getTailRms()         const noexcept { return tailRms.load(); }
    double getTailSeconds()     const noexcept { return 4.0; }
    
    // Latency reporting for PDC
    int  getReportedLatencySamples() const noexcept
    {
        // Only the ducking look-ahead contributes latency in our graph.
        int latency = duck.getLookAheadSamples();
        
#if FIELD_REVERB_PHASE2
        // FDN itself adds no latency; if you later add oversampling, include it here.
        // latency += oversamplingLatencySamp; // Future: add OS latency when implemented
#endif
        return latency;
    }
    bool isDuckingEnabled() const noexcept { return duck.isEnabled(); }

private:
    // -------- Early Reflections ---------------------------------------------
    struct EarlyReflections
    {
        static constexpr int MAX_ER_TAPS = 32;

        struct Tap {
            int   delaySamp = 0;
            float gain      = 0.0f; // scalar pre-pan
            float pan       = 0.0f; // -1..+1 equal-power
            float f0        = 1200.f;
            float Q         = 0.707f;
        };

        double sampleRate = 48000.0;
        int numTaps = 0;
        std::array<Tap, MAX_ER_TAPS> taps;

        // per-channel rings and indices
        std::vector<std::vector<float>> ring;
        std::vector<int> writeIdx;

        // per-tap filters (shared coeffs) with per-channel state
        std::vector<ERFilter> tapFilters;

        void prepare   (int channels, double sr, int maxDelayMs);
        void reset     ();
        void setParams (float erTimeMs, float erDensity, float erWidthPct, float erLevelDb);
        void process   (const juce::AudioBuffer<float>& in, juce::AudioBuffer<float>& out);
    } er;

    // -------- FDN tank (Phase-2 implementation) -----------------------------
    struct FdnTank
    {
        void prepare (double sr, int maxBlock, int channels)
        {
#if FIELD_REVERB_PHASE2
            fdnCore.prepare(sr, maxBlock, channels, 8); // 8 delay lines
#endif
        }
        
        void reset()
        {
#if FIELD_REVERB_PHASE2
            fdnCore.reset();
#endif
        }
        
        void setParams (float decaySec, float diffusion, float modDepthCents, float modRateHz)
        {
#if FIELD_REVERB_PHASE2
            fdnCore.setBaseT60(decaySec);
            // TODO: Add modulation and diffusion parameters
#endif
        }
        
        void setDecayProfile (const DecayRateProfile& profile)
        {
#if FIELD_REVERB_PHASE2
            fdnCore.setDecayProfile(profile);
#endif
        }
        
        void setToneEq (const ToneEq& eq)
        {
            // TODO: Implement tone EQ routing
        }
        
        void process (const juce::AudioBuffer<float>& in, juce::AudioBuffer<float>& out)
        {
#if FIELD_REVERB_PHASE2
            fdnCore.process(in, out);
#else
            // Phase 1: copy ER to tail
            out.makeCopyOf(in);
#endif
        }
        
    private:
#if FIELD_REVERB_PHASE2
        fieldverb::FDNCore fdnCore;
#endif
    } fdn;

    // -------- Ducking --------------------------------------------------------
    struct DuckingSystem
    {
        struct Mode { float lookAheadMs, rmsMs; };
        static constexpr std::array<Mode,5> modes = {{
            {8.f, 20.f}, {16.f, 25.f}, {4.5f, 9.f}, {7.5f, 20.f}, {7.5f, 20.f}
        }};

        void prepare (double sr, int maxBlock, int channels);
        void reset   ();
        void setParams (int mode, int detector, float depthDb, float thrDb, float ratio,
                        float kneeDb, float atkMs, float relMs, float bandHz, float bandQ, bool enabled);

        void process (juce::AudioBuffer<float>& wet,
                      const juce::AudioBuffer<float>& dry,
                      const juce::AudioBuffer<float>& er,
                      const juce::AudioBuffer<float>& tail);

        bool  enabled   = false;
        float envelope  = 1.0f;
        float lastGrDb  = 0.0f;
        
        // Latency reporting accessors
        int  getLookAheadSamples() const noexcept { return enabled ? juce::jmax(0, gaAhead) : 0; }
        bool isEnabled()          const noexcept { return enabled; }

    private:
        // detector selection (no copies)
        const juce::AudioBuffer<float>* selectDetector (int src,
                        const juce::AudioBuffer<float>& dry,
                        const juce::AudioBuffer<float>& er,
                        const juce::AudioBuffer<float>& tail,
                        const juce::AudioBuffer<float>& wet) const;

        // optional band-filter on detector
        ERFilter band;
        juce::AudioBuffer<float> work; // used only if bandHz>0

        // windowed RMS (ring of mean-squares)
        std::vector<float> rmsRing;
        int    rmsWindowSamp = 1;
        int    rmsHead = 0;
        double sumSq   = 0.0;

        // look-ahead FIFO for target gain
        std::vector<float> gaRing;
        int gaAhead = 0, gaW = 0, gaR = 0;

        // parameters & fs
        double fs = 48000.0;
        int   detectorSrc = 0;
        float depthCapDb = 6.0f;
        float thrDb = -12.0f, kneeDb = 2.0f, rat = 3.0f;
        float atkMs = 10.0f, relMs = 100.0f;
        float bandHz = 0.0f, bandQ = 0.707f;

        // helpers
        float computeRms (const juce::AudioBuffer<float>& buf);
        float computeTargetGain (float rms);
    } duck;

    // -------- Tone EQ runtime (static IIR) ----------------------------------
    struct ToneEqRuntime
    {
        ToneEq::Apply apply = ToneEq::Post;
        std::vector<ERFilter> biqs;
        int channels = 2;

        void prepare (double fs, int channelsIn, const ToneEq& eq);
        void process (juce::AudioBuffer<float>& buf);
        bool active() const noexcept { return !biqs.empty(); }
    } tone;

    // engine state/buffers
    double sampleRate { 48000.0 };
    int    maxSamples { 0 };
    int    chans      { 2 };

    juce::AudioBuffer<float> erBuf, tailBuf, tmpBuf;

    std::atomic<float> duckGrDb { 0.f }, erRms { 0.f }, tailRms { 0.f };

    // cached profiles (for FDN in Phase-2)
    DecayRateProfile decayProfile;
    ToneEq           toneEq;
};