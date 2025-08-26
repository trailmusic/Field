#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    static constexpr const char* spaceAlgo  = "space_algo";   // 0=Inner 1=Outer 2=Deep
    static constexpr const char* airDb      = "air_db";
    static constexpr const char* bassDb     = "bass_db";
    static constexpr const char* ducking    = "ducking";
    static constexpr const char* osMode     = "os_mode";      // 0 Off, 1=2x, 2=4x
    static constexpr const char* splitMode  = "split_mode";   // 0 normal, 1 split
    // EQ start freqs
    static constexpr const char* tiltFreq   = "tilt_freq";
    static constexpr const char* scoopFreq  = "scoop_freq";
    static constexpr const char* bassFreq   = "bass_freq";
    static constexpr const char* airFreq    = "air_freq";
}

// ================================================================
// Helper: safe access to APVTS raw values
// ================================================================
static inline float getParam (juce::AudioProcessorValueTreeState& apvts, const char* id)
{
    if (auto* p = apvts.getRawParameterValue (id)) return p->load();
    jassertfalse; return 0.0f;
}

// HostParams is declared in PluginProcessor.h

// ================================================================
// Float-only reverb adapter for double chains (parallel, wet return)
// ================================================================
struct FloatReverbAdapter
{
    juce::dsp::Reverb                      reverbF;
    juce::dsp::Reverb::Parameters          params;
    juce::AudioBuffer<float>               scratch;

    void prepare (double sr, int maxBlock, int chans)
    {
        scratch.setSize (chans, maxBlock, false, false, true);
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) maxBlock, (juce::uint32) chans };
        reverbF.prepare (spec);
    }

    void setParameters (const juce::dsp::Reverb::Parameters& p) { params = p; reverbF.setParameters (params); }

    // Renders a wet-only float buffer from a double AudioBlock and mixes back with wetLevel
    void processParallelMix (juce::dsp::AudioBlock<double> block, float wetLevel)
    {
        if (wetLevel <= 0.0001f) return;
        const int ch = (int) block.getNumChannels();
        const int n  = (int) block.getNumSamples();
        scratch.setSize (ch, n, false, false, true);

        // Copy double -> float (manual conversion)
        for (int c = 0; c < ch; ++c)
        {
            auto* dst = scratch.getWritePointer (c);
            auto* src = block.getChannelPointer (c);
            for (int i = 0; i < n; ++i)
                dst[i] = static_cast<float> (src[i]);
        }

        juce::dsp::AudioBlock<float> fblk (scratch);
        juce::dsp::ProcessContextReplacing<float> fctx (fblk);
        reverbF.process (fctx);

        // Mix wet back into double
        for (int c = 0; c < ch; ++c)
        {
            auto* d = block.getChannelPointer (c);
            auto* s = scratch.getReadPointer (c);
            for (int i = 0; i < n; ++i) d[i] += (double) s[i] * (double) wetLevel;
        }
    }
};

// ================================================================
// MyPluginAudioProcessor
// ================================================================
MyPluginAudioProcessor::MyPluginAudioProcessor()
: AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
, apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    // Templated chain instances
    chainF = std::make_unique<FieldChain<float>>();
    chainD = std::make_unique<FieldChain<double>>();

    // Keep existing smoothers if declared in the header (no harm if unused here)
    for (auto* s : { &panSmoothed, &panLSmoothed, &panRSmoothed, &depthSmoothed, &widthSmoothed, &gainSmoothed, &tiltSmoothed,
                     &hpHzSmoothed, &lpHzSmoothed, &monoHzSmoothed,
                     &satDriveLin, &satMixSmoothed, &airSmoothed, &bassSmoothed, &duckingSmoothed })
        s->reset (currentSR, 0.005); // 5 ms smoothing

    apvts.addParameterListener (IDs::pan,  this);
    apvts.addParameterListener (IDs::gain, this);
}

bool MyPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto in  = layouts.getMainInputChannelSet();
    auto out = layouts.getMainOutputChannelSet();
    if (in != out || out.isDisabled()) return false;
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void MyPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSR = sampleRate;

    for (auto* s : { &panSmoothed, &panLSmoothed, &panRSmoothed, &depthSmoothed, &widthSmoothed, &gainSmoothed, &tiltSmoothed,
                     &hpHzSmoothed, &lpHzSmoothed, &monoHzSmoothed,
                     &satDriveLin, &satMixSmoothed, &airSmoothed, &bassSmoothed, &duckingSmoothed })
        s->reset (sampleRate, 0.005);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock,
                                  (juce::uint32) getTotalNumOutputChannels() };

    chainF->prepare (spec);
    chainD->prepare (spec);
}

// Utility: build a HostParams snapshot each block
static HostParams makeHostParams (juce::AudioProcessorValueTreeState& apvts)
{
    HostParams p{};
    p.gainDb   = getParam(apvts, IDs::gain);
    p.pan      = getParam(apvts, IDs::pan);
    p.panL     = getParam(apvts, IDs::panL);
    p.panR     = getParam(apvts, IDs::panR);
    p.depth    = getParam(apvts, IDs::depth);
    p.width    = getParam(apvts, IDs::width);
    p.tiltDb   = getParam(apvts, IDs::tilt);
    p.scoopDb  = getParam(apvts, IDs::scoop);
    p.monoHz   = getParam(apvts, IDs::monoHz);
    p.hpHz     = getParam(apvts, IDs::hpHz);
    p.lpHz     = getParam(apvts, IDs::lpHz);
    p.satDriveDb = getParam(apvts, IDs::satDriveDb);
    p.satMix     = getParam(apvts, IDs::satMix);
    p.bypass     = (getParam(apvts, IDs::bypass) >= 0.5f);
    p.spaceAlgo  = (int) apvts.getParameterAsValue (IDs::spaceAlgo).getValue();
    p.airDb    = getParam(apvts, IDs::airDb);
    p.bassDb   = getParam(apvts, IDs::bassDb);
    p.ducking  = getParam(apvts, IDs::ducking);
    p.osMode   = (int) apvts.getParameterAsValue (IDs::osMode).getValue();
    p.splitMode= (bool) apvts.getParameterAsValue (IDs::splitMode).getValue();
    p.tiltFreq = getParam(apvts, IDs::tiltFreq);
    p.scoopFreq= getParam(apvts, IDs::scoopFreq);
    p.bassFreq = getParam(apvts, IDs::bassFreq);
    p.airFreq  = getParam(apvts, IDs::airFreq);
    return p;
}

// Float path
void MyPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    isDoublePrecEnabled = false;

    auto hp = makeHostParams (apvts);
    chainF->setParameters (hp);     // cast/copy inside chain

    juce::dsp::AudioBlock<float> block (buffer);
    chainF->process (block);
}

// Double path
void MyPluginAudioProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    isDoublePrecEnabled = true;

    auto hp = makeHostParams (apvts);
    chainD->setParameters (hp);

    juce::dsp::AudioBlock<double> block (buffer);
    chainD->process (block);
}

// =========================
// State
// =========================
juce::AudioProcessorEditor* MyPluginAudioProcessor::createEditor() { return new MyPluginAudioProcessorEditor (*this); }

void MyPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (state.isValid()) if (auto xml = state.createXml()) copyXmlToBinary (*xml, destData);
}

void MyPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes)) apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

void MyPluginAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    juce::ignoreUnused (parameterID, newValue);
}

// =========================
// Parameter Layout
// =========================
juce::AudioProcessorValueTreeState::ParameterLayout MyPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::gain, 1 }, "Gain +", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::pan, 1 }, "Pan", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::panL, 1 }, "Pan L", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::panR, 1 }, "Pan R", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::depth, 1 }, "Depth", juce::NormalisableRange<float> (0.0f, 1.0f, 0.0001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::width, 1 }, "Width", juce::NormalisableRange<float> (0.5f, 2.0f, 0.00001f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::tilt, 1 }, "Tone (Tilt)", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::scoop, 1 }, "Scoop", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::monoHz, 1 }, "Mono Maker (Hz)", juce::NormalisableRange<float> (0.0f, 300.0f, 0.01f, 0.5f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::hpHz, 1 }, "HP (Hz)", juce::NormalisableRange<float> (20.0f, 1000.0f, 0.01f, 0.5f), 20.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::lpHz, 1 }, "LP (Hz)", juce::NormalisableRange<float> (2000.0f, 20000.0f, 0.01f, 0.5f), 20000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::satDriveDb, 1 }, "Saturation Drive (dB)", juce::NormalisableRange<float> (0.0f, 36.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::satMix, 1 }, "Saturation Mix", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::bypass, 1 }, "Bypass", juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::spaceAlgo, 1 }, "Space Algorithm", juce::StringArray { "Inner", "Outer", "Deep" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::airDb, 1 }, "Air", juce::NormalisableRange<float> (0.0f, 6.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::bassDb, 1 }, "Bass", juce::NormalisableRange<float> (-6.0f, 6.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::ducking, 1 }, "Ducking", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::osMode, 1 }, "Oversampling", juce::StringArray { "Off", "2x", "4x" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ IDs::splitMode, 1 }, "Split Mode", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::tiltFreq, 1 },  "Tilt Frequency", juce::NormalisableRange<float> (100.0f, 1000.0f, 1.0f, 0.5f), 500.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::scoopFreq, 1 }, "Scoop Frequency", juce::NormalisableRange<float> (200.0f, 2000.0f, 1.0f, 0.5f), 800.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::bassFreq, 1 },  "Bass Frequency", juce::NormalisableRange<float> (50.0f, 500.0f, 1.0f, 0.5f), 150.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::airFreq, 1 },   "Air Frequency",  juce::NormalisableRange<float> (2000.0f, 20000.0f, 10.0f, 0.5f), 8000.0f));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MyPluginAudioProcessor(); }

// =====================================================================
// ==================  Templated FieldChain Implementation  =============
// =====================================================================

template <typename Sample>
static inline Sample softClipT (Sample x)
{
    // atan clip scaled for ~unity at +/-1
    return std::atan (x) * (Sample) (1.0 / 1.2533141373155);
}

// --------- prepare/reset ---------

template <typename Sample>
void FieldChain<Sample>::prepare (const juce::dsp::ProcessSpec& spec)
{
    sr = spec.sampleRate;

    hpFilter.prepare (spec);
    lpFilter.prepare (spec);
    lowSplitL.prepare (spec);
    lowSplitR.prepare (spec);
    depthLPF.prepare (spec);
    lowShelf.prepare (spec);
    highShelf.prepare (spec);
    airFilter.prepare (spec);
    bassFilter.prepare (spec);
    scoopFilter.prepare (spec);

    hpFilter.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    lpFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    lowSplitL.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    lowSplitR.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    depthLPF.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    depthLPF.setCutoffFrequency ((Sample) 20000);
    depthLPF.setResonance ((Sample) 0.707);

    if constexpr (std::is_same_v<Sample, double>)
    {
        reverbD = std::make_unique<FloatReverbAdapter>();
        reverbD->prepare (spec.sampleRate, (int) spec.maximumBlockSize, (int) spec.numChannels);
    }
    else
    {
        juce::dsp::ProcessSpec rspec { spec.sampleRate, spec.maximumBlockSize, spec.numChannels };
        reverbF = std::make_unique<juce::dsp::Reverb>();
        reverbF->prepare (rspec);
    }

    // Default reverb params
    rvParams.roomSize   = 0.45f;
    rvParams.width      = 1.0f;
    rvParams.damping    = 0.35f;
    rvParams.dryLevel   = 1.0f;
    rvParams.wetLevel   = 0.0f;
    rvParams.freezeMode = 0.0f;

    // No OS by default
    oversampling.reset();
    lastOsMode = -1;
}

template <typename Sample>
void FieldChain<Sample>::reset()
{
    hpFilter.reset(); lpFilter.reset(); lowSplitL.reset(); lowSplitR.reset(); depthLPF.reset();
    lowShelf.reset(); highShelf.reset(); airFilter.reset(); bassFilter.reset(); scoopFilter.reset();
    if (oversampling) oversampling->reset();
    if constexpr (std::is_same_v<Sample, double>) { if (reverbD) reverbD->reverbF.reset(); }
    else                                           { if (reverbF) reverbF->reset(); }
}

// --------- parameter ingress ---------

template <typename Sample>
void FieldChain<Sample>::setParameters (const HostParams& hp)
{
    // Cast/copy once per block into Sample domain
    params.gainLin   = juce::Decibels::decibelsToGain ((Sample) hp.gainDb);
    params.pan       = (Sample) hp.pan;
    params.panL      = (Sample) hp.panL;
    params.panR      = (Sample) hp.panR;
    params.depth     = (Sample) juce::jlimit (0.0, 1.0, hp.depth);
    params.width     = (Sample) juce::jlimit (0.5, 2.0, hp.width);
    params.tiltDb    = (Sample) hp.tiltDb;
    params.scoopDb   = (Sample) hp.scoopDb;
    params.monoHz    = (Sample) hp.monoHz;
    params.hpHz      = (Sample) hp.hpHz;
    params.lpHz      = (Sample) hp.lpHz;
    params.satDriveLin = (Sample) juce::Decibels::decibelsToGain (hp.satDriveDb);
    params.satMix    = (Sample) juce::jlimit (0.0, 1.0, hp.satMix);
    params.bypass    = hp.bypass;
    params.spaceAlgo = hp.spaceAlgo;
    params.airDb     = (Sample) hp.airDb;
    params.bassDb    = (Sample) hp.bassDb;
    params.ducking   = (Sample) juce::jlimit (0.0, 1.0, hp.ducking);
    params.osMode    = hp.osMode;
    params.splitMode = hp.splitMode;
    params.tiltFreq  = (Sample) hp.tiltFreq;
    params.scoopFreq = (Sample) hp.scoopFreq;
    params.bassFreq  = (Sample) hp.bassFreq;
    params.airFreq   = (Sample) hp.airFreq;
}

// --------- processing utilities ---------

template <typename Sample>
void FieldChain<Sample>::ensureOversampling (int osModeIndex)
{
    if (lastOsMode == osModeIndex && oversampling) return;
    lastOsMode = osModeIndex;

    if (osModeIndex <= 0) { oversampling.reset(); return; }

    const int factor = (osModeIndex == 1 ? 2 : 4);
    const int stages = juce::roundToInt (std::log2 (factor));

    oversampling = std::make_unique<juce::dsp::Oversampling<Sample>> (
        (int) 2, stages, juce::dsp::Oversampling<Sample>::filterHalfBandPolyphaseIIR);
    oversampling->reset();
}

// --------- per-module DSP (Sample domain) ---------

template <typename Sample>
void FieldChain<Sample>::applyHP_LP (Block block, Sample hpHz, Sample lpHz)
{
    hpHz = juce::jlimit ((Sample)20,  (Sample)1000,  hpHz);
    lpHz = juce::jlimit ((Sample)1000,(Sample)20000, lpHz);
    hpFilter.setCutoffFrequency (hpHz);
    lpFilter.setCutoffFrequency (lpHz);
    CtxRep ctx (block);
    hpFilter.process (ctx);
    lpFilter.process (ctx);
}

template <typename Sample>
void FieldChain<Sample>::updateTiltEQ (Sample tiltDb, Sample pivotHz)
{
    const double fs = sr;
    // Map pivot to complementary shelves
    const double lowFc  = juce::jlimit (50.0,  1000.0, (double) pivotHz * 0.30);
    const double highFc = juce::jlimit (1500.0, 20000.0, (double) pivotHz * 12.0);
    const Sample lowGain  = (Sample) juce::Decibels::decibelsToGain ( juce::jlimit (-12.0, 12.0, (double) tiltDb));
    const Sample highGain = (Sample) juce::Decibels::decibelsToGain (-juce::jlimit (-12.0, 12.0, (double) tiltDb));

    auto lowCoef  = juce::dsp::IIR::Coefficients<Sample>::makeLowShelf  (fs, lowFc,  (Sample)1.0, lowGain);
    auto highCoef = juce::dsp::IIR::Coefficients<Sample>::makeHighShelf (fs, highFc, (Sample)1.0, highGain);
    lowShelf.coefficients  = lowCoef;
    highShelf.coefficients = highCoef;
}

template <typename Sample>
void FieldChain<Sample>::applyTiltEQ (Block block, Sample tiltDb, Sample pivotHz)
{
    updateTiltEQ (tiltDb, pivotHz);
    CtxRep ctx (block);
    lowShelf.process (ctx);
    highShelf.process (ctx);
}

template <typename Sample>
void FieldChain<Sample>::applyScoopEQ (Block block, Sample scoopDb, Sample scoopFreq)
{
    if (std::abs ((double) scoopDb) < 0.1) return;
    auto coef = juce::dsp::IIR::Coefficients<Sample>::makePeakFilter (sr, scoopFreq, (Sample)1.0, (Sample) juce::Decibels::decibelsToGain ((double) scoopDb));
    scoopFilter.coefficients = coef;
    CtxRep ctx (block); scoopFilter.process (ctx);
}

template <typename Sample>
void FieldChain<Sample>::applyBassShelf (Block block, Sample bassDb, Sample bassFreq)
{
    if (std::abs ((double) bassDb) < 0.1) return;
    auto coef = juce::dsp::IIR::Coefficients<Sample>::makeLowShelf (sr, bassFreq, (Sample)0.7, (Sample) juce::Decibels::decibelsToGain ((double) bassDb));
    bassFilter.coefficients = coef; CtxRep ctx (block); bassFilter.process (ctx);
}

template <typename Sample>
void FieldChain<Sample>::applyAirBand (Block block, Sample airDb, Sample airFreq)
{
    if (airDb <= (Sample)0.05) return; // positive-only air
    auto coef = juce::dsp::IIR::Coefficients<Sample>::makeHighShelf (sr, airFreq, (Sample)0.3, (Sample) juce::Decibels::decibelsToGain ((double) airDb));
    airFilter.coefficients = coef; CtxRep ctx (block); airFilter.process (ctx);
}

template <typename Sample>
void FieldChain<Sample>::applyWidthMS (Block block, Sample width)
{
    if (block.getNumChannels() < 2) return;
    width = juce::jlimit ((Sample)0.5, (Sample)2.0, width);
    auto* L = block.getChannelPointer (0);
    auto* R = block.getChannelPointer (1);
    const int N = (int) block.getNumSamples();
    const Sample k = (Sample)0.7071067811865476;
    
    for (int i = 0; i < N; ++i)
    {
        const Sample l = L[i], r = R[i];
        Sample M = k * (l + r);
        Sample S = k * (l - r);
        if (width < (Sample)1)      S *= width;
        else if (width > (Sample)1) S *= (Sample) (1.0 + (double)(width - (Sample)1.0) * 0.6);
        L[i] = k * (M + S);
        R[i] = k * (M - S);
    }
}

template <typename Sample>
void FieldChain<Sample>::applyMonoMaker (Block block, Sample monoHz)
{
    if (block.getNumChannels() < 2) return;
    if (monoHz <= (Sample)0) return;

    monoHz = juce::jlimit ((Sample)20, (Sample)300, monoHz);
    lowSplitL.setCutoffFrequency (monoHz);
    lowSplitR.setCutoffFrequency (monoHz);

    // Create temp low buffer (copy)
    juce::AudioBuffer<Sample> low (2, (int) block.getNumSamples());
    for (int c = 0; c < 2; ++c)
        low.copyFrom (c, 0, block.getChannelPointer (c), (int) block.getNumSamples());

    // Filter lows per channel
    {
        juce::dsp::AudioBlock<Sample> lb (low);
        auto cl = lb.getSingleChannelBlock (0);
        auto cr = lb.getSingleChannelBlock (1);
        juce::dsp::ProcessContextReplacing<Sample> ctxL (cl), ctxR (cr);
        lowSplitL.process (ctxL);
        lowSplitR.process (ctxR);
    }

    auto* L = block.getChannelPointer (0);
    auto* R = block.getChannelPointer (1);
    const auto* LL = low.getReadPointer (0);
    const auto* RR = low.getReadPointer (1);
    const int N = (int) block.getNumSamples();

    for (int i = 0; i < N; ++i)
    {
        const Sample monoBass = (Sample)0.5 * (LL[i] + RR[i]);
        const Sample lHigh = L[i] - LL[i];
        const Sample rHigh = R[i] - RR[i];
        L[i] = lHigh + monoBass;
        R[i] = rHigh + monoBass;
    }
}

template <typename Sample>
void FieldChain<Sample>::applyPan (Block block, Sample pan)
{
    if (block.getNumChannels() < 2) return;
    pan = juce::jlimit ((Sample)-1, (Sample)1, pan);
    auto positionToGains = [] (Sample x, Sample& gL, Sample& gR)
    {
        const Sample theta = (juce::jlimit ((Sample)-1, (Sample)1, x) + (Sample)1) * (Sample) (juce::MathConstants<double>::pi * 0.25);
        gL = std::cos (theta); gR = std::sin (theta);
    };
    const Sample pL = juce::jlimit ((Sample)-1, (Sample)1, (Sample)-1 + pan);
    const Sample pR = juce::jlimit ((Sample)-1, (Sample)1, (Sample)+1 + pan);
    Sample gLL, gLR, gRL, gRR; positionToGains (pL, gLL, gRL); positionToGains (pR, gLR, gRR);

    auto* L = block.getChannelPointer (0);
    auto* R = block.getChannelPointer (1);
    const int N = (int) block.getNumSamples();
    for (int i = 0; i < N; ++i)
    {
        const Sample l = L[i], r = R[i];
        L[i] = gLL * l + gLR * r;
        R[i] = gRL * l + gRR * r;
    }
}

template <typename Sample>
void FieldChain<Sample>::applySplitPan (Block block, Sample panL, Sample panR)
{
    if (block.getNumChannels() < 2) return;
    panL = juce::jlimit ((Sample)-1, (Sample)1, panL);
    panR = juce::jlimit ((Sample)-1, (Sample)1, panR);
    auto positionToGains = [] (Sample x, Sample& gL, Sample& gR)
    {
        const Sample theta = (juce::jlimit ((Sample)-1, (Sample)1, x) + (Sample)1) * (Sample) (juce::MathConstants<double>::pi * 0.25);
        gL = std::cos (theta); gR = std::sin (theta);
    };
    Sample gLL, gLR, gRL, gRR; positionToGains (panL, gLL, gRL); positionToGains (panR, gLR, gRR);

    auto* L = block.getChannelPointer (0);
    auto* R = block.getChannelPointer (1);
    const int N = (int) block.getNumSamples();
    for (int i = 0; i < N; ++i)
    {
        const Sample l = L[i], r = R[i];
        L[i] = gLL * l + gLR * r;
        R[i] = gRL * l + gRR * r;
    }
}

template <typename Sample>
void FieldChain<Sample>::applySaturationOnBlock (juce::dsp::AudioBlock<Sample> b, Sample driveLin)
{
    const int ch = (int) b.getNumChannels();
    const int n  = (int) b.getNumSamples();
    for (int c = 0; c < ch; ++c)
    {
        auto* d = b.getChannelPointer (c);
        for (int i = 0; i < n; ++i)
        {
            const Sample input = d[i] * driveLin;
            Sample y = softClipT (input);
            if ((double) driveLin > 2.0)
                y += (Sample) (std::sin ((double) input * 2.0) * 0.1 * ((double) driveLin - 2.0));
            d[i] = y;
        }
    }
}

template <typename Sample>
void FieldChain<Sample>::applySaturation (Block block, Sample driveLin, Sample mix01, int osModeIndex)
{
    if (mix01 <= (Sample)0.0001 || driveLin <= (Sample)1.0001) return;
    ensureOversampling (osModeIndex);

    juce::AudioBuffer<Sample> dry ((int) block.getNumChannels(), (int) block.getNumSamples());
    for (int c = 0; c < (int) block.getNumChannels(); ++c)
        dry.copyFrom (c, 0, block.getChannelPointer (c), (int) block.getNumSamples());

    if (oversampling)
    {
        auto up = oversampling->processSamplesUp (block);
        applySaturationOnBlock (up, driveLin);
        oversampling->processSamplesDown (block);
    }
    else
    {
        applySaturationOnBlock (block, driveLin);
    }

    // Mix dry back in: manual loop over AudioBlock
    const Sample dryGain = (Sample)1 - mix01;
    for (int c = 0; c < (int) block.getNumChannels(); ++c)
    {
        auto* dst = block.getChannelPointer (c);
        auto* src = dry.getReadPointer (c);
        for (int i = 0; i < (int) block.getNumSamples(); ++i)
            dst[i] += src[i] * dryGain;
    }
}

template <typename Sample>
void FieldChain<Sample>::applyDucking (Block block, Sample ducking)
{
    if (ducking <= (Sample)0.01) return;
    auto* L = block.getChannelPointer (0);
    auto* R = (block.getNumChannels() > 1 ? block.getChannelPointer (1) : L);
    const int N = (int) block.getNumSamples();
    for (int i = 0; i < N; ++i)
    {
        const Sample lvl = std::sqrt (L[i]*L[i] + R[i]*R[i]);
        Sample duckAmt = (Sample) juce::jmap ((double) lvl, 0.0, 1.0, 0.0, (double) ducking);
        duckAmt = std::pow (duckAmt, (Sample)0.5);
        const Sample g = (Sample)1 - duckAmt;
        L[i] *= g; R[i] *= g;
    }
}

// Space algorithms (simplified, parallel reverb + light tone)

template <typename Sample>
void FieldChain<Sample>::applySpaceAlgorithm (Block block, Sample depth01, int algo)
{
    depth01 = juce::jlimit ((Sample)0, (Sample)1, depth01);

    float wet = 0.0f, damp = 0.35f, room = 0.45f, width = 1.0f;
    if (algo == 0) // Inner (EQ sculpt + subtle comp)
    {
        const Sample midCutDb   = (Sample) juce::jmap ((double) depth01, 0.0, 1.0, 0.0, -8.0);
        const Sample highShelf  = (Sample) juce::jmap ((double) depth01, 0.0, 1.0, 0.0,  4.0);
        const Sample lowShelfDb = (Sample) juce::jmap ((double) depth01, 0.0, 1.0, 0.0,  2.0);

        // Low shelf -> mid cut -> high shelf
        auto lowC  = juce::dsp::IIR::Coefficients<Sample>::makeLowShelf  (sr, 120.0, (Sample)0.8, (Sample) juce::Decibels::decibelsToGain ((double) lowShelfDb));
        auto midC  = juce::dsp::IIR::Coefficients<Sample>::makePeakFilter (sr, 350.0, (Sample)0.8, (Sample) juce::Decibels::decibelsToGain ((double) midCutDb));
        auto highC = juce::dsp::IIR::Coefficients<Sample>::makeHighShelf (sr, 6000.0,(Sample)0.7, (Sample) juce::Decibels::decibelsToGain ((double) highShelf));
        juce::dsp::IIR::Filter<Sample> fL, fM, fH;
        fL.prepare ({ sr, (juce::uint32) block.getNumSamples(), (juce::uint32) block.getNumChannels() });
        fM.prepare ({ sr, (juce::uint32) block.getNumSamples(), (juce::uint32) block.getNumChannels() });
        fH.prepare ({ sr, (juce::uint32) block.getNumSamples(), (juce::uint32) block.getNumChannels() });
        fL.coefficients = lowC; fM.coefficients = midC; fH.coefficients = highC;
        CtxRep ctx (block); fL.process (ctx); fM.process (ctx); fH.process (ctx);

        // light soft comp (static)
        const Sample k = (Sample) juce::jmap ((double) depth01, 0.0, 1.0, 1.0, 0.85);
        for (int c = 0; c < (int) block.getNumChannels(); ++c)
        {
            auto* d = block.getChannelPointer (c);
            for (int i = 0; i < (int) block.getNumSamples(); ++i)
                d[i] = softClipT (d[i] * k) / k;
        }
        return; // no reverb here
    }
    else if (algo == 1) // Outer (natural)
    {
        wet  = (float) juce::jmap ((double) depth01, 0.0, 1.0, 0.0, 0.35);
        damp = (float) juce::jmap ((double) depth01, 0.0, 1.0, 0.15, 0.45);
        room = (float) juce::jmap ((double) depth01, 0.0, 1.0, 0.25, 0.65);
        width= 0.8f;
    }
    else // Deep (warm + more wet)
    {
        wet  = (float) juce::jmap ((double) depth01, 0.0, 1.0, 0.0, 0.50);
        damp = (float) juce::jmap ((double) depth01, 0.0, 1.0, 0.25, 0.85);
        room = (float) juce::jmap ((double) depth01, 0.0, 1.0, 0.40, 0.85);
        width= 1.0f;
    }

    // Configure params
    rvParams.damping  = damp;
    rvParams.roomSize = room;
    rvParams.wetLevel = wet;
    rvParams.width    = width;

    if constexpr (std::is_same_v<Sample, double>)
    {
        jassert (reverbD);
        reverbD->setParameters (rvParams);
        juce::dsp::AudioBlock<double> dblk (block);
        reverbD->processParallelMix (dblk, rvParams.wetLevel);
    }
    else
    {
        jassert (reverbF);
        reverbF->setParameters (rvParams);
        // make wet copy
        juce::AudioBuffer<float> wetBuf ((int) block.getNumChannels(), (int) block.getNumSamples());
        for (int c = 0; c < (int) block.getNumChannels(); ++c)
            wetBuf.copyFrom (c, 0, block.getChannelPointer (c), (int) block.getNumSamples());
        juce::dsp::AudioBlock<float> fblk (wetBuf);
        juce::dsp::ProcessContextReplacing<float> fctx (fblk);
        reverbF->process (fctx);
        // mix back (manual add to AudioBlock)
        for (int c = 0; c < (int) block.getNumChannels(); ++c)
        {
            auto* dst = block.getChannelPointer (c);
            auto* src = wetBuf.getReadPointer (c);
            for (int i = 0; i < (int) block.getNumSamples(); ++i)
                dst[i] += src[i] * (Sample) rvParams.wetLevel;
        }
    }
}

// --------- main process (Sample) ---------

template <typename Sample>
void FieldChain<Sample>::process (Block block)
{
    if (params.bypass) return;

    // Input gain
    block.multiplyBy (params.gainLin);

    // Clean filters
    applyHP_LP (block, params.hpHz, params.lpHz);

    // Imaging & placement
    if (params.splitMode) applySplitPan (block, params.panL, params.panR);
    else                  applyPan     (block, params.pan);
    applyWidthMS (block, params.width);

    // Core tone
    applyTiltEQ  (block, params.tiltDb,  params.tiltFreq);
    applyScoopEQ (block, params.scoopDb, params.scoopFreq);
    applyBassShelf (block, params.bassDb, params.bassFreq);
    applyAirBand   (block, params.airDb,  params.airFreq);

    // Space
    applySpaceAlgorithm (block, params.depth, params.spaceAlgo);

    // LF mono
    applyMonoMaker (block, params.monoHz);

    // Nonlinear
    applySaturation (block, params.satDriveLin, params.satMix, params.osMode);

    // Duck last
    applyDucking (block, params.ducking);
}

// Explicit instantiation
template struct FieldChain<float>;
template struct FieldChain<double>;
