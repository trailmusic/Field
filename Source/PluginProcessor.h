#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// Forward declaration of templated chain
template <typename SampleType>
struct FieldChain;

class MyPluginAudioProcessor : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
{
public:
    MyPluginAudioProcessor();
    ~MyPluginAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;
    bool supportsDoublePrecisionProcessing() const override { return true; }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    
    // Waveform display callback
    std::function<void(double, double)> onAudioSample;

    const juce::String getName() const override { return "Field"; }
    double getTailLengthSeconds() const override { return 2.0; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Parameter listener callback
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Templated DSP chains for float and double precision
    std::unique_ptr<FieldChain<float>> chainF;
    std::unique_ptr<FieldChain<double>> chainD;
    bool isDoublePrecEnabled = false;

    // Parameter smoothers (shared between chains)
    juce::SmoothedValue<float> panSmoothed, panLSmoothed, panRSmoothed, depthSmoothed, widthSmoothed, gainSmoothed, tiltSmoothed;
    juce::SmoothedValue<float> hpHzSmoothed, lpHzSmoothed, monoHzSmoothed;
    juce::SmoothedValue<float> satDriveLin, satMixSmoothed, airSmoothed, bassSmoothed, duckingSmoothed;
    juce::SmoothedValue<float> scoopSmoothed; // NEW: Scoop EQ smoother
    juce::SmoothedValue<float> tiltFreqSmoothed, scoopFreqSmoothed, bassFreqSmoothed, airFreqSmoothed; // NEW: Frequency control smoothers

    double currentSR = 48000.0;
    float lastTilt = 1e9f; // Instance-specific tilt tracking

    // Helpers
    void updateTiltEQ (float tiltDb);
    void syncWithHostParameters();
    void updateHostParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPluginAudioProcessor)
};

// Float Reverb Adapter for double precision processing
struct FloatReverbAdapter
{
    juce::dsp::Reverb r;
    juce::AudioBuffer<float> scratch;
    juce::dsp::AudioBlock<float> scratchBlock { scratch };

    void prepare (double sr, int block, int chans)
    {
        scratch.setSize (chans, block, false, false, true);
        juce::dsp::ProcessSpec spec { sr, (juce::uint32)block, (juce::uint32)chans };
        r.prepare (spec);
    }

    void processReplacing (juce::AudioBuffer<double>& buf)
    {
        const int ch = buf.getNumChannels(), n = buf.getNumSamples();
        scratch.setSize (ch, n, false, false, true);

        // Convert double -> float
        for (int c = 0; c < ch; ++c)
        {
            auto* dst = scratch.getWritePointer (c);
            auto* src = buf.getReadPointer (c);
            juce::FloatVectorOperations::convert (dst, src, n);
        }

        // Reverb on float scratch
        juce::dsp::ProcessContextReplacing<float> ctx (scratchBlock);
        r.process (ctx);

        // Convert back float -> double
        for (int c = 0; c < ch; ++c)
        {
            auto* dst = buf.getWritePointer (c);
            auto* src = scratch.getReadPointer  (c);
            juce::FloatVectorOperations::convert (dst, src, n);
        }
    }

    void processWetOnly (juce::AudioBuffer<double>& wet, const juce::AudioBuffer<double>& src, float wetLevel)
    {
        wet.makeCopyOf (src, true);
        processReplacing (wet);
        wet.applyGain (wetLevel);
    }
};

// Templated DSP Chain
template <typename SampleType>
struct FieldChain
{
    using Block = juce::dsp::AudioBlock<SampleType>;
    using CtxRep = juce::dsp::ProcessContextReplacing<SampleType>;
    using IIR = juce::dsp::IIR::Filter<SampleType>;
    using Smooth = juce::SmoothedValue<SampleType>;

    // Filters
    juce::dsp::StateVariableTPTFilter<SampleType> hpFilter, lpFilter;
    juce::dsp::LinkwitzRileyFilter<SampleType> lowSplitL, lowSplitR;
    juce::dsp::StateVariableTPTFilter<SampleType> depthLPF;

    // Tilt EQ
    juce::dsp::IIR::Filter<SampleType> lowShelf, highShelf, airFilter, bassFilter, scoopFilter;

    // Oversampling
    std::unique_ptr<juce::dsp::Oversampling<SampleType>> oversampling;
    SampleType lastOsMode = -1.0;

    // Reverb (float adapter for double precision)
    std::conditional_t<std::is_same_v<SampleType, double>, FloatReverbAdapter, juce::dsp::Reverb> reverb;
    juce::dsp::Reverb::Parameters rvParams;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void process (Block block);
    void reset();
    
    // Processing functions
    void applyHP_LP (Block block, SampleType hpHz, SampleType lpHz);
    void applyTiltEQ (Block block, SampleType tiltDb, SampleType tiltFreq);
    void applyScoopEQ (Block block, SampleType scoopDb, SampleType scoopFreq);
    void applyBassShelf (Block block, SampleType bassDb, SampleType bassFreq);
    void applyWidthMS (Block block, SampleType width);
    void applyMonoMaker (Block block, SampleType monoHz);
    void applySaturation (Block block, SampleType driveLin, SampleType mix01, int osModeIndex);
    void applySpaceAlgorithm (Block block, SampleType depth, int algorithm);
    void applyDucking (Block block, SampleType ducking);
    void applyAirBand (Block block, SampleType airDb, SampleType airFreq);
    void applyDepthShaping (Block block, SampleType depth);
    void applyPan (Block block, SampleType pan);
    void applySplitPan (Block block, SampleType panL, SampleType panR);
    void updateTiltEQ (SampleType tiltDb);
    void ensureOversampling (int osModeIndex);
}; 