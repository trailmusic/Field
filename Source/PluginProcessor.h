#pragma once

#include <JuceHeader.h>

// Forward decls used by the templated chain / processor snapshot
struct HostParams;           // Double-domain snapshot built each block in the processor
struct FloatReverbAdapter;   // Float-only reverb wrapper for the double chain

// ===============================
// Templated DSP Chain (declaration)
// ===============================

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

private:
    // ----- helpers -----
    void ensureOversampling (int osModeIndex);

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
    void applyDucking (Block, Sample ducking);
    void applySpaceAlgorithm (Block, Sample depth01, int algo);

    // ----- state -----
    double sr { 48000.0 };

    // Oversampling (created on demand)
    std::unique_ptr<juce::dsp::Oversampling<Sample>> oversampling;
    int lastOsMode { -1 };

    // Core filters / EQ
    juce::dsp::StateVariableTPTFilter<Sample> hpFilter, lpFilter, depthLPF;
    juce::dsp::LinkwitzRileyFilter<Sample>    lowSplitL, lowSplitR;     // mono-maker lows
    juce::dsp::IIR::Filter<Sample>            lowShelf, highShelf, airFilter, bassFilter, scoopFilter;

    // Reverb (float adapter for double chain)
    std::unique_ptr<FloatReverbAdapter>  reverbD;  // only used when Sample == double
    std::unique_ptr<juce::dsp::Reverb>   reverbF;  // used when Sample == float
    juce::dsp::Reverb::Parameters        rvParams;

    // Per-block params converted to Sample domain
    struct FieldParams
    {
        Sample gainLin{}, pan{}, panL{}, panR{}, depth{}, width{};
        Sample tiltDb{}, scoopDb{}, monoHz{}, hpHz{}, lpHz{};
        Sample satDriveLin{}, satMix{}; bool bypass{}; int spaceAlgo{};
        Sample airDb{}, bassDb{}, ducking{}; int osMode{}; bool splitMode{};
        Sample tiltFreq{}, scoopFreq{}, bassFreq{}, airFreq{};
    } params;
};

// ===============================
// HostParams (double snapshot)
// ===============================

struct HostParams
{
    double gainDb{}, pan{}, panL{}, panR{}, depth{}, width{};
    double tiltDb{}, scoopDb{}, monoHz{}, hpHz{}, lpHz{};
    double satDriveDb{}, satMix{}; bool bypass{}; int spaceAlgo{};
    double airDb{}, bassDb{}, ducking{}; int osMode{}; bool splitMode{};
    double tiltFreq{}, scoopFreq{}, bassFreq{}, airFreq{};
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPluginAudioProcessor)
};
