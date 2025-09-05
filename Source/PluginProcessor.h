#pragma once

#include <JuceHeader.h>
#include "dsp/Ducker.h"
#include "dsp/DelayEngine.h"

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
        cutoff = juce::jlimit<Sample> ((Sample)20, (Sample)300, hz);
        updateCoeffs();
    }

    void setSlopeDbPerOct (int slope)
    {
        slopeDbPerOct = juce::jlimit (6, 24, slope);
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
    void updateTiltEQ   (Sample tiltDb, Sample pivotHz);
    void applyTiltEQ    (Block, Sample tiltDb, Sample pivotHz);
    void applyScoopEQ   (Block, Sample scoopDb, Sample scoopFreq);
    void applyBassShelf (Block, Sample bassDb, Sample bassFreq);
    void applyAirBand   (Block, Sample airDb, Sample airFreq);

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

    // ----- state -----
    double sr { 48000.0 };

    // Oversampling (created on demand)
    std::unique_ptr<juce::dsp::Oversampling<Sample>> oversampling;
    int lastOsMode { -1 };

    // Core filters / EQ
    juce::dsp::StateVariableTPTFilter<Sample> hpFilter, lpFilter, depthLPF;
    MonoLowpassBank<Sample>                   monoLP;                   // mono-maker lows with variable slope
    // Imaging band split filters (3-band via LP@lo and HP@hi)
    juce::dsp::LinkwitzRileyFilter<Sample>    bandLowLP_L, bandLowLP_R;
    juce::dsp::LinkwitzRileyFilter<Sample>    bandHighHP_L, bandHighHP_R;
    // Shuffler split (2-band LP@xover; HP via subtraction)
    juce::dsp::LinkwitzRileyFilter<Sample>    shuffLP_L, shuffLP_R;
    juce::dsp::IIR::Filter<Sample>            lowShelf, highShelf, airFilter, bassFilter, scoopFilter;

    // Reverb (float adapter for double chain)
    std::unique_ptr<FloatReverbAdapter>  reverbD;  // only used when Sample == double
    std::unique_ptr<juce::dsp::Reverb>   reverbF;  // used when Sample == float
    juce::dsp::Reverb::Parameters        rvParams;
    // Preallocated buses to avoid per-block allocations
    juce::AudioBuffer<Sample>            dryBusBuf;
    juce::AudioBuffer<Sample>            wetBusBuf;
    // Smoothed wet mix (per-sample ramp)
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> wetMixSmoothed;
    // Smoothed reverb macro params
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> roomSizeSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> dampingSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> widthSmoothed;

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
    
    // Delay engine (per-Sample instance)
    DelayEngine<Sample>                  delayEngine;

    // Per-block params converted to Sample domain
    struct FieldParams
    {
        Sample gainLin{}, pan{}, panL{}, panR{}, depth{}, width{};
        Sample tiltDb{}, scoopDb{}, monoHz{}, hpHz{}, lpHz{};
        // New EQ shape/Q
        Sample shelfShapeS{};     // 0.25..1.50
        Sample filterQ{};         // 0.50..1.20
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
    } params;
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
    double getTailLengthSeconds() const override               { return 0.0; } // change to 2.0 if you want auto-tail renders
    bool supportsDoublePrecisionProcessing() const override    { return true; }

    // Programs (single program)
    int getNumPrograms() override                              { return 1; }
    int getCurrentProgram() override                           { return 0; }
    void setCurrentProgram (int) override                      {}
    const juce::String getProgramName (int) override           { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    // Lifecycle
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override                           {}

    // Layout
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    // Processing (float & double paths)
    void processBlock (juce::AudioBuffer<float>&,  juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;

    // Editor
    juce::AudioProcessorEditor* createEditor() override;

    // State
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Parameters
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Optional: waveform display / metering callback (not required by DSP)
    std::function<void(double, double)> onAudioSample;
    // Optional: block-level visualization callback (post-DSP, processed signal)
    std::function<void (const float* L, const float* R, int n)> onAudioBlock;
    // Optional: pre-DSP block callback (input signal)
    std::function<void (const float* L, const float* R, int n)> onAudioBlockPre;
    // Meters
    float getCorrelation() const { return meterCorrelation.load(); }
    float getRmsL() const { return meterRmsL.load(); }
    float getRmsR() const { return meterRmsR.load(); }
    float getPeakL() const { return meterPeakL.load(); }
    float getPeakR() const { return meterPeakR.load(); }
    // Current ducking gain reduction in dB (>=0), from active precision chain
    float getCurrentDuckGrDb() const
    {
        if (isDoublePrecEnabled && chainD) return chainD->getCurrentDuckGrDb();
        if (chainF) return chainF->getCurrentDuckGrDb();
        return 0.0f;
    }

private:
    // APVTS listener
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    // Optional host sync hooks (stubs in .cpp)
    void syncWithHostParameters();
    void updateHostParameters();

    // Chains (float & double)
    std::unique_ptr<FieldChain<float>>  chainF;
    std::unique_ptr<FieldChain<double>> chainD;
    bool isDoublePrecEnabled { false };

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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPluginAudioProcessor)
};
