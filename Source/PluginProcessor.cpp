#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace IDs {
    static constexpr const char* gain   = "gain_db";
    static constexpr const char* pan    = "pan";
    static constexpr const char* panL   = "pan_l";  // Left channel pan for split mode
    static constexpr const char* panR   = "pan_r";  // Right channel pan for split mode
    static constexpr const char* depth  = "depth";
    static constexpr const char* width  = "width";
    static constexpr const char* tilt   = "tilt";
    static constexpr const char* scoop  = "scoop";  // NEW: Scoop EQ parameter
    static constexpr const char* monoHz     = "mono_hz";
    static constexpr const char* hpHz       = "hp_hz";
    static constexpr const char* lpHz       = "lp_hz";
    static constexpr const char* satDriveDb = "sat_drive_db";
    static constexpr const char* satMix     = "sat_mix";
    static constexpr const char* bypass     = "bypass";
    static constexpr const char* spaceAlgo  = "space_algo";
    static constexpr const char* airDb      = "air_db";
    static constexpr const char* bassDb     = "bass_db";
    static constexpr const char* ducking    = "ducking";
    static constexpr const char* osMode     = "os_mode"; // 0 Off, 1=2x, 2=4x
    static constexpr const char* splitMode  = "split_mode"; // 0=normal pan, 1=split pan

    // NEW: Frequency control parameters for EQ starting points
    static constexpr const char* tiltFreq   = "tilt_freq";   // Tilt EQ frequency
    static constexpr const char* scoopFreq  = "scoop_freq";  // Scoop EQ frequency  
    static constexpr const char* bassFreq   = "bass_freq";   // Bass shelf frequency
    static constexpr const char* airFreq    = "air_freq";    // Air shelf frequency
}

MyPluginAudioProcessor::MyPluginAudioProcessor()
: AudioProcessor (BusesProperties()
    .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
    .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
  apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    // Initialize DSP chains
    chainF = std::make_unique<FieldChain<float>>();
    chainD = std::make_unique<FieldChain<double>>();

    for (auto* s : { &panSmoothed, &panLSmoothed, &panRSmoothed, &depthSmoothed, &widthSmoothed, &gainSmoothed, &tiltSmoothed,
                     &hpHzSmoothed, &lpHzSmoothed, &monoHzSmoothed,
                     &satDriveLin, &satMixSmoothed, &airSmoothed, &bassSmoothed, &duckingSmoothed })
        s->reset (currentSR, 0.005); // Much faster response - 5ms instead of 30ms

    // Add parameter change listeners to sync with host
    apvts.addParameterListener(IDs::pan, this);
    apvts.addParameterListener(IDs::gain, this);
}

void MyPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSR = sampleRate;
    for (auto* s : { &panSmoothed, &panLSmoothed, &panRSmoothed, &depthSmoothed, &widthSmoothed, &gainSmoothed, &tiltSmoothed,
                     &hpHzSmoothed, &lpHzSmoothed, &monoHzSmoothed,
                     &satDriveLin, &satMixSmoothed, &airSmoothed, &bassSmoothed, &duckingSmoothed })
        s->reset (sampleRate, 0.005); // Much faster response - 5ms instead of 30ms

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock,
                                  (juce::uint32) getTotalNumOutputChannels() };

    // Prepare both float and double chains
    chainF->prepare(spec);
    chainD->prepare(spec);

    updateTiltEQ (0.0f);
    
    // Sync with host parameters when plugin is prepared
    syncWithHostParameters();
}

bool MyPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto in  = layouts.getMainInputChannelSet();
    auto out = layouts.getMainOutputChannelSet();
    if (in != out || out.isDisabled())
        return false;
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

static inline double softClip (double x) { return std::atan (x) * (1.0 / 1.2533141); }

void MyPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    isDoublePrecEnabled = false;
    juce::dsp::AudioBlock<float> block (buffer);
    chainF->process(block);
}

void MyPluginAudioProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi)
{
    isDoublePrecEnabled = true;
    juce::dsp::AudioBlock<double> block (buffer);
    chainD->process(block);
}

// Template implementation for FieldChain
template <typename SampleType>
void FieldChain<SampleType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    // Prepare filters
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

    // Set filter types
    hpFilter.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    lpFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    lowSplitL.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    lowSplitR.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    depthLPF.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    depthLPF.setCutoffFrequency (20000.0f);
    depthLPF.setResonance (0.707f);

    // Prepare reverb
    if constexpr (std::is_same_v<SampleType, double>)
    {
        reverb.prepare (spec.sampleRate, (int)spec.maximumBlockSize, (int)spec.numChannels);
    }
    else
    {
        reverb.prepare (spec);
    }

    // Initialize reverb parameters
    rvParams.roomSize  = 0.45f;
    rvParams.width     = 1.0f;
    rvParams.damping   = 0.35f;
    rvParams.dryLevel  = 1.0f;
    rvParams.wetLevel  = 0.0f;
    rvParams.freezeMode= 0.0f;
    
    if constexpr (std::is_same_v<SampleType, double>)
    {
        reverb.r.setParameters (rvParams);
    }
    else
    {
        reverb.setParameters (rvParams);
    }

    updateTiltEQ (0.0f);
}

template <typename SampleType>
void FieldChain<SampleType>::reset()
{
    hpFilter.reset();
    lpFilter.reset();
    lowSplitL.reset();
    lowSplitR.reset();
    depthLPF.reset();
    lowShelf.reset();
    highShelf.reset();
    airFilter.reset();
    bassFilter.reset();
    scoopFilter.reset();
    
    if constexpr (std::is_same_v<SampleType, double>)
    {
        reverb.r.reset();
    }
    else
    {
        reverb.reset();
    }
    
    if (oversampling)
        oversampling->reset();
}

template <typename SampleType>
void FieldChain<SampleType>::process (Block block)
{
    // Get parameters from the processor (this needs to be passed in or accessed differently)
    // For now, we'll use placeholder values - this needs to be connected to the actual parameter system
    
    // Apply all processing
    // applyHP_LP (block, hpHz, lpHz);
    // applyTiltEQ (block, tiltDb, tiltFreq);
    // applyScoopEQ (block, scoopDb, scoopFreq);
    // applyBassShelf (block, bassDb, bassFreq);
    // applyDucking (block, ducking);
    // applySpaceAlgorithm (block, depth, spaceAlgo);
    // applyAirBand (block, airDb, airFreq);
    // applyPan (block, pan);
    // applyWidthMS (block, width);
    // applyMonoMaker (block, monoHz);
    // applySaturation (block, driveLin, mix01, osIndex);
    
    // For now, just pass through the audio
    // This will be fully implemented once we connect the parameter system
}

// Explicit template instantiations
template struct FieldChain<float>;
template struct FieldChain<double>;

// Template implementations for processing functions
template <typename SampleType>
void FieldChain<SampleType>::applyWidthMS (Block block, SampleType width)
{
    if (block.getNumChannels() < 2) return;
    
    // Refined width control: narrower and gentler
    width = juce::jlimit (SampleType(0.5), SampleType(2.0), width);
    
    auto* L = block.getChannelPointer (0);
    auto* R = block.getChannelPointer (1);
    const int N = (int)block.getNumSamples();
    
    for (int i = 0; i < N; ++i)
    {
        const SampleType l = L[i], r = R[i];
        
        // Mid-Side processing
        SampleType M = SampleType(0.70710678) * (l + r);  // Mid (mono)
        SampleType S = SampleType(0.70710678) * (l - r);  // Side (stereo)
        
        // Enhanced width processing with extended range
        // Ableton suggestion: reduce width before hard panning for smoother transitions
        if (width < SampleType(1.0)) {
            // Narrowing: reduce side signal
            S *= width;
        } else if (width > SampleType(1.0)) {
            // Widening: gentle curve and mild limiting
            SampleType widthExcess = width - SampleType(1.0);          // 0..1
            SampleType scale = SampleType(1.0) + widthExcess * SampleType(0.6);   // up to 1.6x
            S *= scale;
            S = juce::jlimit (SampleType(-1.5), SampleType(1.5), S);         // prevent harshness
        }
        
        // Reconstruct stereo
        L[i] = SampleType(0.70710678) * (M + S);
        R[i] = SampleType(0.70710678) * (M - S);
    }
}

template <typename SampleType>
void FieldChain<SampleType>::applyHP_LP (Block block, SampleType hpHz, SampleType lpHz)
{
    hpHz = juce::jlimit (SampleType(20.0), SampleType(1000.0), hpHz);
    lpHz = juce::jlimit (SampleType(1000.0), SampleType(20000.0), lpHz);
    hpFilter.setCutoffFrequency (hpHz);
    lpFilter.setCutoffFrequency (lpHz);
    CtxRep ctx (block);
    hpFilter.process (ctx);
    lpFilter.process (ctx);
}

template <typename SampleType>
void FieldChain<SampleType>::applyTiltEQ (Block block, SampleType tiltDb, SampleType tiltFreq)
{
    // Update EQ coefficients if tilt changed
    updateTiltEQ (tiltDb);
    
    // Apply the tilt EQ
    CtxRep ctx (block);
    lowShelf.process (ctx);
    highShelf.process (ctx);
}

template <typename SampleType>
void FieldChain<SampleType>::updateTiltEQ (SampleType tiltDb)
{
    const double fs = 48000.0; // This should come from the processor
    const double lowFc  = 150.0;  // Lower frequency for more bass control
    const double highFc = 6000.0; // Higher frequency for more treble control
    const SampleType  lowGain  = juce::Decibels::decibelsToGain ( juce::jlimit (SampleType(-12.0), SampleType(12.0), tiltDb) );
    const SampleType  highGain = juce::Decibels::decibelsToGain (-juce::jlimit (SampleType(-12.0), SampleType(12.0), tiltDb));
    
    // More aggressive Q values for more pronounced effect
    auto lowCoef  = juce::dsp::IIR::Coefficients<SampleType>::makeLowShelf  (fs, lowFc,  SampleType(1.0), lowGain);
    auto highCoef = juce::dsp::IIR::Coefficients<SampleType>::makeHighShelf (fs, highFc, SampleType(1.0), highGain);
    lowShelf.coefficients = lowCoef;
    highShelf.coefficients = highCoef;
}

template <typename SampleType>
void FieldChain<SampleType>::ensureOversampling (int osModeIndex)
{
    if ((SampleType) osModeIndex == lastOsMode && oversampling) return;
    lastOsMode = (SampleType) osModeIndex;

    if (osModeIndex <= 0) { oversampling.reset(); return; }

    const int numCh = 2; // This should come from the processor
    const int factor = (osModeIndex == 1 ? 2 : 4);

    oversampling = std::make_unique<juce::dsp::Oversampling<SampleType>>(
        (size_t) numCh,
        (size_t) juce::roundToInt (std::log2 (factor)),
        juce::dsp::Oversampling<SampleType>::filterHalfBandPolyphaseIIR);
}

// Placeholder implementations for other functions
template <typename SampleType>
void FieldChain<SampleType>::applyScoopEQ (Block block, SampleType scoopDb, SampleType scoopFreq) {}
template <typename SampleType>
void FieldChain<SampleType>::applyBassShelf (Block block, SampleType bassDb, SampleType bassFreq) {}
template <typename SampleType>
void FieldChain<SampleType>::applyMonoMaker (Block block, SampleType monoHz) {}
template <typename SampleType>
void FieldChain<SampleType>::applySaturation (Block block, SampleType driveLin, SampleType mix01, int osModeIndex) {}
template <typename SampleType>
void FieldChain<SampleType>::applySpaceAlgorithm (Block block, SampleType depth, int algorithm) {}
template <typename SampleType>
void FieldChain<SampleType>::applyDucking (Block block, SampleType ducking) {}
template <typename SampleType>
void FieldChain<SampleType>::applyAirBand (Block block, SampleType airDb, SampleType airFreq) {}
template <typename SampleType>
void FieldChain<SampleType>::applyDepthShaping (Block block, SampleType depth) {}
template <typename SampleType>
void FieldChain<SampleType>::applyPan (Block block, SampleType pan) {}
template <typename SampleType>
void FieldChain<SampleType>::applySplitPan (Block block, SampleType panL, SampleType panR) {}

// Remove old processing functions - they are now in the templated chain
{
    hpHz = juce::jlimit (20.0, 1000.0, hpHz);
    lpHz = juce::jlimit (1000.0, 20000.0, lpHz);
    hpFilter.setCutoffFrequency (hpHz);
    lpFilter.setCutoffFrequency (lpHz);
    juce::dsp::AudioBlock<double> blk (buffer);
    juce::dsp::ProcessContextReplacing<double> ctx (blk);
    hpFilter.process (ctx);
    lpFilter.process (ctx);
}

void MyPluginAudioProcessor::applyTiltEQ (juce::AudioBuffer<double>& buffer, double tiltDb, double tiltFreq)
{
    // Update EQ coefficients if tilt changed
    updateTiltEQ (tiltDb);
    
    // Apply the tilt EQ
    juce::dsp::AudioBlock<double> blk (buffer);
    juce::dsp::ProcessContextReplacing<double> ctx (blk);
    lowShelf.process (ctx);
    highShelf.process (ctx);
}

void MyPluginAudioProcessor::applyScoopEQ (juce::AudioBuffer<double>& buffer, double scoopDb, double scoopFreq)
{
    if (std::abs(scoopDb) < 0.1) return; // Skip if no scoop processing
    
    // Scoop EQ: creates a gentle dip in the mid frequencies
    // This is a bell curve that cuts frequencies around the scoop frequency
    const double scoopQ = 1.0; // Moderate Q for musical scoop
    const double scoopGain = juce::Decibels::decibelsToGain (juce::jlimit (-12.0, 12.0, scoopDb)); // Full range for scoop
    
    auto scoopCoef = juce::dsp::IIR::Coefficients<double>::makePeakFilter (currentSR, scoopFreq, scoopQ, scoopGain);
    scoopFilter.coefficients = scoopCoef;
    
    juce::dsp::AudioBlock<double> blk (buffer);
    juce::dsp::ProcessContextReplacing<double> ctx (blk);
    scoopFilter.process (ctx);
}

void MyPluginAudioProcessor::applyBassShelf (juce::AudioBuffer<double>& buffer, double bassDb, double bassFreq)
{
    if (std::abs(bassDb) < 0.1) return; // Skip if no bass processing
    
    // Bass shelf: gentle low shelf centered around user-defined frequency
    const double bassQ = 0.7; // Gentle Q for musical bass
    const double bassGain = juce::Decibels::decibelsToGain (juce::jlimit (-6.0, 6.0, bassDb)); // Safe range for bass shelf
    
    auto bassCoef = juce::dsp::IIR::Coefficients<double>::makeLowShelf (currentSR, bassFreq, bassQ, bassGain);
    bassFilter.coefficients = bassCoef;
    
    juce::dsp::AudioBlock<double> blk (buffer);
    juce::dsp::ProcessContextReplacing<double> ctx (blk);
    bassFilter.process (ctx);
}

void MyPluginAudioProcessor::applyWidthMS (juce::AudioBuffer<double>& buffer, double width)
{
    if (buffer.getNumChannels() < 2) return;
    
    // Refined width control: narrower and gentler
    width = juce::jlimit (0.5, 2.0, width);
    
    auto* L = buffer.getWritePointer (0);
    auto* R = buffer.getWritePointer (1);
    const int N = buffer.getNumSamples();
    
    for (int i = 0; i < N; ++i)
    {
        const double l = L[i], r = R[i];
        
        // Mid-Side processing
        double M = 0.70710678 * (l + r);  // Mid (mono)
        double S = 0.70710678 * (l - r);  // Side (stereo)
        
        // Enhanced width processing with extended range
        // Ableton suggestion: reduce width before hard panning for smoother transitions
        if (width < 1.0) {
            // Narrowing: reduce side signal
            S *= width;
        } else if (width > 1.0) {
            // Widening: gentle curve and mild limiting
            double widthExcess = width - 1.0;          // 0..1
            double scale = 1.0 + widthExcess * 0.6;   // up to 1.6x
            S *= scale;
            S = juce::jlimit (-1.5, 1.5, S);         // prevent harshness
        }
        
        // Reconstruct stereo
        L[i] = 0.70710678 * (M + S);
        R[i] = 0.70710678 * (M - S);
    }
}

void MyPluginAudioProcessor::applyMonoMaker (juce::AudioBuffer<double>& buffer, double monoHz)
{
    if (buffer.getNumChannels() < 2) return;
    
    // Enhanced mono maker - more like Ableton's Utility
    if (monoHz <= 0.0) return; // No processing if set to 0
    
    monoHz = juce::jlimit (20.0, 300.0, monoHz);
    lowSplitL.setCutoffFrequency (monoHz);
    lowSplitR.setCutoffFrequency (monoHz);

    juce::AudioBuffer<double> low (2, buffer.getNumSamples());
    low.copyFrom (0, 0, buffer, 0, 0, buffer.getNumSamples());
    low.copyFrom (1, 0, buffer, 1, 0, buffer.getNumSamples());

    {
        juce::dsp::AudioBlock<double> lb (low);
        auto cl = lb.getSingleChannelBlock (0);
        auto cr = lb.getSingleChannelBlock (1);
        juce::dsp::ProcessContextReplacing<double> ctxL (cl);
        juce::dsp::ProcessContextReplacing<double> ctxR (cr);
        lowSplitL.process (ctxL);
        lowSplitR.process (ctxR);
    }

    auto* L = buffer.getWritePointer (0);
    auto* R = buffer.getWritePointer (1);
    auto* LL = low.getReadPointer (0);
    auto* RR = low.getReadPointer (1);
    const int N = buffer.getNumSamples();
    
    // Enhanced mono processing with better bass mono handling
    for (int i = 0; i < N; ++i)
    {
        // Extract low frequencies (bass)
        const double lLow = LL[i];
        const double rLow = RR[i];
        
        // Create mono bass (like Ableton's "Bass Mono")
        const double monoBass = 0.5 * (lLow + rLow);
        
        // Extract high frequencies (stereo)
        const double lHigh = L[i] - lLow;
        const double rHigh = R[i] - rLow;
        
        // Reconstruct with mono bass and stereo highs
        L[i] = lHigh + monoBass;
        R[i] = rHigh + monoBass;
    }
}

void MyPluginAudioProcessor::ensureOversampling (int osModeIndex)
{
    if ((double) osModeIndex == lastOsMode && oversampling) return;
    lastOsMode = (double) osModeIndex;

    if (osModeIndex <= 0) { oversampling.reset(); return; }

    const int numCh = juce::jmax (1, getTotalNumOutputChannels());
    const int factor = (osModeIndex == 1 ? 2 : 4);

    oversampling = std::make_unique<juce::dsp::Oversampling<double>>(
        (size_t) numCh,
        (size_t) juce::roundToInt (std::log2 (factor)),
        juce::dsp::Oversampling<double>::filterHalfBandPolyphaseIIR);
}

void MyPluginAudioProcessor::applySaturation (juce::AudioBuffer<double>& buffer, double driveLin, double mix01, int osModeIndex)
{
    if (mix01 <= 0.0001 || driveLin <= 1.0001) return;
    
    // Enhanced saturation with more dynamic character
    juce::AudioBuffer<double> dry; dry.makeCopyOf (buffer);

    if (oversampling && osModeIndex > 0)
    {
        juce::dsp::AudioBlock<float> blk (buffer);
        auto up = oversampling->processSamplesUp (blk);
        for (size_t ch = 0; ch < up.getNumChannels(); ++ch)
        {
            auto* d = up.getChannelPointer (ch);
            for (size_t i = 0; i < up.getNumSamples(); ++i)
            {
                // Enhanced saturation curve
                double input = d[i] * driveLin;
                d[i] = softClip (input);
                
                // Add harmonics for more character
                if (driveLin > 2.0) {
                    double harmonic = std::sin(input * 2.0) * 0.1 * (driveLin - 2.0);
                    d[i] += harmonic;
                }
            }
        }
        oversampling->processSamplesDown (blk);
    }
    else
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* d = buffer.getWritePointer (ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                // Enhanced saturation curve
                double input = d[i] * driveLin;
                d[i] = softClip (input);
                
                // Add harmonics for more character
                if (driveLin > 2.0) {
                    double harmonic = std::sin(input * 2.0) * 0.1 * (driveLin - 2.0);
                    d[i] += harmonic;
                }
            }
        }
    }

    const double wet = juce::jlimit (0.0, 1.0, (double)mix01);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.addFrom (ch, 0, dry, ch, 0, buffer.getNumSamples(), 1.0 - wet);
}

void MyPluginAudioProcessor::applySpaceAlgorithm (juce::AudioBuffer<double>& buffer, double depth, int algorithm)
{
    depth = juce::jlimit (0.0, 1.0, depth);
    
    switch (algorithm) {
        case 0: // Inner - Clean, scooped mids, bright and focused
        {
            // Professional "Inner" algorithm: scooped mids with enhanced clarity
            const double midCutDb = juce::jmap (depth, 0.0, 1.0, 0.0, -8.0);
            const double highShelfDb = juce::jmap (depth, 0.0, 1.0, 0.0, 4.0);
            const double lowShelfDb = juce::jmap (depth, 0.0, 1.0, 0.0, 2.0);
            
            // Create multi-band processing for "Inner" character
            auto midCutCoef = juce::dsp::IIR::Coefficients<double>::makePeakFilter (currentSR, 350.0, 0.8, juce::Decibels::decibelsToGain (midCutDb));
            auto highShelfCoef = juce::dsp::IIR::Coefficients<double>::makeHighShelf (currentSR, 6000.0, 0.7, juce::Decibels::decibelsToGain (highShelfDb));
            auto lowShelfCoef = juce::dsp::IIR::Coefficients<double>::makeLowShelf (currentSR, 120.0, 0.8, juce::Decibels::decibelsToGain (lowShelfDb));
            
            // Apply filters with enhanced processing
            juce::dsp::AudioBlock<double> blk (buffer);
            juce::dsp::ProcessContextReplacing<double> ctx (blk);
            
            // Create temporary filters for this processing
            juce::dsp::IIR::Filter<double> tempMidCut, tempHighShelf, tempLowShelf;
            tempMidCut.prepare ({ currentSR, (juce::uint32) buffer.getNumSamples(), (juce::uint32) buffer.getNumChannels() });
            tempHighShelf.prepare ({ currentSR, (juce::uint32) buffer.getNumSamples(), (juce::uint32) buffer.getNumChannels() });
            tempLowShelf.prepare ({ currentSR, (juce::uint32) buffer.getNumSamples(), (juce::uint32) buffer.getNumChannels() });
            
            tempMidCut.coefficients = midCutCoef;
            tempHighShelf.coefficients = highShelfCoef;
            tempLowShelf.coefficients = lowShelfCoef;
            
            // Apply filters in sequence for "Inner" character
            tempLowShelf.process (ctx);
            tempMidCut.process (ctx);
            tempHighShelf.process (ctx);
            
            // Add subtle compression for "focused" character
            const double compressionAmount = juce::jmap (depth, 0.0, 1.0, 1.0, 0.85);
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                auto* channelData = buffer.getWritePointer (ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i) {
                    channelData[i] = softClip (channelData[i] * compressionAmount) / compressionAmount;
                }
            }
            break;
        }
        
        case 1: // Outer - Balanced, natural, spacious
        {
            // Professional "Outer" algorithm: natural reverb with enhanced stereo imaging
            const double wet = juce::jmap (depth, 0.00, 1.00, 0.00, 0.35);
            const double damp = juce::jmap (depth, 0.00, 1.00, 0.15, 0.45);
            const double roomSize = juce::jmap (depth, 0.00, 1.00, 0.25, 0.65);
            const double stereoWidth = juce::jmap (depth, 0.00, 1.00, 1.0, 1.3);
            
            // Enhanced reverb parameters for "Outer" character
            rvParams.wetLevel = wet;
            rvParams.damping = damp;
            rvParams.roomSize = roomSize;
            rvParams.width = 0.8; // Slightly narrower for natural feel
            reverb.setParameters (rvParams);
            
            // Create reverb buffer - convert to float for reverb processing
            juce::AudioBuffer<float> revBuf; revBuf.makeCopyOf (buffer);
            juce::dsp::AudioBlock<float> blk (revBuf);
            juce::dsp::ProcessContextReplacing<float> ctx (blk);
            reverb.process (ctx);
            
            // Apply subtle stereo enhancement for "Outer" character
            if (revBuf.getNumChannels() >= 2) {
                auto* L = revBuf.getWritePointer (0);
                auto* R = revBuf.getWritePointer (1);
                const int N = revBuf.getNumSamples();
                
                for (int i = 0; i < N; ++i) {
                    const float l = L[i], r = R[i];
                    float M = 0.70710678f * (l + r);  // Mid
                    float S = 0.70710678f * (l - r);  // Side
                    
                    // Enhance stereo width for "Outer" character
                    S *= static_cast<float>(stereoWidth);
                    
                    L[i] = 0.70710678f * (M + S);
                    R[i] = 0.70710678f * (M - S);
                }
            }
            
            // Mix reverb with dry signal - convert float reverb buffer to double
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* dest = buffer.getWritePointer(ch);
                auto* src = revBuf.getReadPointer(ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    dest[i] += static_cast<double>(src[i]) * rvParams.wetLevel;
                }
            }
            break;
        }
        
        case 2: // Deep - Rich, warm, immersive
        {
            // Professional "Deep" algorithm: warm, immersive reverb with harmonic enhancement
            const double wet = juce::jmap (depth, 0.00, 1.00, 0.00, 0.50);
            const double damp = juce::jmap (depth, 0.00, 1.00, 0.25, 0.85);
            const double roomSize = juce::jmap (depth, 0.00, 1.00, 0.40, 0.85);
            const double cutoff = juce::jmap (depth, 0.00, 1.00, 20000.0, 3500.0);
            const double saturation = juce::jmap (depth, 0.00, 1.00, 1.0, 1.4);
            
            // Enhanced reverb parameters for "Deep" character
            rvParams.wetLevel = wet;
            rvParams.damping = damp;
            rvParams.roomSize = roomSize;
            rvParams.width = 1.0; // Full stereo for immersive feel
            reverb.setParameters (rvParams);
            
            // Apply low-pass filter for "Deep" character
            depthLPF.setCutoffFrequency (cutoff);
            
            // Create reverb buffer - convert to float for reverb processing
            juce::AudioBuffer<float> revBuf; revBuf.makeCopyOf (buffer);
            juce::dsp::AudioBlock<float> blk (revBuf);
            juce::dsp::ProcessContextReplacing<float> ctx (blk);
            reverb.process (ctx);
            
            // Apply low-pass to reverb for warmth
            juce::dsp::AudioBlock<float> lpfBlk (revBuf);
            juce::dsp::ProcessContextReplacing<float> lpfCtx (lpfBlk);
            depthLPF.process (lpfCtx);
            
            // Add harmonic saturation for "Deep" character
            for (int ch = 0; ch < revBuf.getNumChannels(); ++ch) {
                auto* channelData = revBuf.getWritePointer (ch);
                for (int i = 0; i < revBuf.getNumSamples(); ++i) {
                    // Soft saturation for warm harmonics
                    channelData[i] = static_cast<float>(softClip (static_cast<double>(channelData[i]) * saturation) / saturation);
                }
            }
            
            // Mix reverb with dry signal - convert float reverb buffer to double
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* dest = buffer.getWritePointer(ch);
                auto* src = revBuf.getReadPointer(ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    dest[i] += static_cast<double>(src[i]) * rvParams.wetLevel;
                }
            }
            break;
        }
    }
}

void MyPluginAudioProcessor::applyDucking (juce::AudioBuffer<double>& buffer, double ducking)
{
    if (ducking < 0.01) return; // Skip if no ducking
    
    // Proper ducking algorithm: reduce signal level based on input amplitude
    // This creates a sidechain-like effect that ducks the space signal
    auto* L = buffer.getWritePointer (0);
    auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : L;
    const int N = buffer.getNumSamples();
    
    for (int i = 0; i < N; ++i)
    {
        // Calculate input amplitude (RMS-like)
        double inputLevel = std::sqrt(L[i] * L[i] + R[i] * R[i]);
        
        // Create ducking curve: more input = more ducking
        double duckAmount = juce::jmap(inputLevel, 0.0, 1.0, 0.0, ducking);
        duckAmount = std::pow(duckAmount, 0.5); // Smooth curve
        
        // Apply ducking (reduce signal level)
        double duckGain = 1.0 - duckAmount;
        L[i] *= duckGain;
        if (buffer.getNumChannels() > 1) {
            R[i] *= duckGain;
        }
    }
}

void MyPluginAudioProcessor::applyAirBand (juce::AudioBuffer<double>& buffer, double airDb, double airFreq)
{
    if (std::abs(airDb) < 0.1) return; // Skip if no air processing
    
    // Maag-style air band: gentle high shelf at user-defined frequency
    const double airQ = 0.3; // Much gentler Q for subtle air
    const double airGain = juce::Decibels::decibelsToGain (juce::jlimit (0.0, 6.0, airDb)); // Safe range for air band
    
    auto airCoef = juce::dsp::IIR::Coefficients<double>::makeHighShelf (currentSR, airFreq, airQ, airGain);
    airFilter.coefficients = airCoef;
    
    juce::dsp::AudioBlock<double> blk (buffer);
    juce::dsp::ProcessContextReplacing<double> ctx (blk);
    airFilter.process (ctx);
}

void MyPluginAudioProcessor::applyDepthShaping (juce::AudioBuffer<double>& buffer, double depth)
{
    depth = juce::jlimit (0.0, 1.0, depth);

    const double wet    = juce::jmap (depth, 0.00, 1.00, 0.00, 0.35);
    const double damp   = juce::jmap (depth, 0.00, 1.00, 0.20, 0.80);
    const double cutoff = juce::jmap (depth, 0.00, 1.00, 20000.0, 6000.0);
    const double attnDb = juce::jmap (depth, 0.00, 1.00, 0.0, -6.0);

    rvParams.wetLevel = wet;
    rvParams.damping  = damp;
    reverb.setParameters (rvParams);
    depthLPF.setCutoffFrequency (cutoff);

    juce::AudioBuffer<double> revBuf; revBuf.makeCopyOf (buffer);

    // Process reverb (without pre-delay for now to avoid crashes)
    if (rvParams.wetLevel > 0.0001)
    {
        juce::dsp::AudioBlock<float> blk (revBuf);
        juce::dsp::ProcessContextReplacing<float> ctx (blk);
        reverb.process (ctx);
    }

    {
        juce::dsp::AudioBlock<double> blk (buffer);
        juce::dsp::ProcessContextReplacing<double> ctx (blk);
        depthLPF.process (ctx);
    }

    if (rvParams.wetLevel > 0.0001)
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* dest = buffer.getWritePointer(ch);
            auto* src = revBuf.getReadPointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                dest[i] += static_cast<double>(src[i]) * rvParams.wetLevel;
            }
        }

    buffer.applyGain (juce::Decibels::decibelsToGain (attnDb));
}

void MyPluginAudioProcessor::applyPan (juce::AudioBuffer<double>& buffer, double pan)
{
    if (buffer.getNumChannels() < 2) return;
    
    // Equal-power stereo pan treating L and R as two virtual sources
    // Original virtual positions: Lorig=-1, Rorig=+1
    // Shift both by pan; keep width handling separate (handled later by applyWidthMS)
    pan = juce::jlimit (-1.0, 1.0, pan);
    auto positionToGains = [] (double x, double& gLeft, double& gRight)
    {
        const double theta = (juce::jlimit(-1.0, 1.0, x) + 1.0) * (juce::MathConstants<double>::pi * 0.25);
        gLeft  = std::cos(theta);
        gRight = std::sin(theta);
    };

    // Compute new virtual positions for each channel
    const double pL = juce::jlimit(-1.0, 1.0, -1.0 + pan); // shift left source
    const double pR = juce::jlimit(-1.0, 1.0, +1.0 + pan); // shift right source

    double gLL = 1.0, gLR = 0.0; // gains for left source to L/R outs
    double gRL = 0.0, gRR = 1.0; // gains for right source to L/R outs
    positionToGains(pL, gLL, gRL);
    positionToGains(pR, gLR, gRR);

    auto* L = buffer.getWritePointer (0);
    auto* R = buffer.getWritePointer (1);
    const int N = buffer.getNumSamples();

    for (int i = 0; i < N; ++i)
    {
        const double l = L[i];
        const double r = R[i];
        L[i] = (gLL * l) + (gLR * r);
        R[i] = (gRL * l) + (gRR * r);
    }
}

void MyPluginAudioProcessor::applySplitPan (juce::AudioBuffer<double>& buffer, double panL, double panR)
{
    if (buffer.getNumChannels() < 2) return;
    
    // Split stereo equal-power panning for independent L/R positions
    panL = juce::jlimit (-1.0, 1.0, panL);
    panR = juce::jlimit (-1.0, 1.0, panR);

    auto positionToGains = [] (double x, double& gLeft, double& gRight)
    {
        const double theta = (juce::jlimit(-1.0, 1.0, x) + 1.0) * (juce::MathConstants<double>::pi * 0.25);
        gLeft  = std::cos(theta);
        gRight = std::sin(theta);
    };

    double gLL = 1.0, gLR = 0.0; // gains from Left input to L/R outs
    double gRL = 0.0, gRR = 1.0; // gains from Right input to L/R outs
    positionToGains(panL, gLL, gRL);
    positionToGains(panR, gLR, gRR);

    auto* L = buffer.getWritePointer (0);
    auto* R = buffer.getWritePointer (1);
    const int N = buffer.getNumSamples();

    for (int i = 0; i < N; ++i)
    {
        const double l = L[i];
        const double r = R[i];
        L[i] = (gLL * l) + (gLR * r);
        R[i] = (gRL * l) + (gRR * r);
    }
}

void MyPluginAudioProcessor::updateTiltEQ (double tiltDb)
{
    const double fs = currentSR;
    const double lowFc  = 150.0;  // Lower frequency for more bass control
    const double highFc = 6000.0; // Higher frequency for more treble control
    const double  lowGain  = juce::Decibels::decibelsToGain ( juce::jlimit (-12.0, 12.0, tiltDb) );
    const double  highGain = juce::Decibels::decibelsToGain (-juce::jlimit (-12.0, 12.0, tiltDb));
    
    // More aggressive Q values for more pronounced effect
    auto lowCoef  = juce::dsp::IIR::Coefficients<double>::makeLowShelf  (fs, lowFc,  1.0, lowGain);
    auto highCoef = juce::dsp::IIR::Coefficients<double>::makeHighShelf (fs, highFc, 1.0, highGain);
    lowShelf.coefficients = lowCoef;
    highShelf.coefficients = highCoef;
}

void MyPluginAudioProcessor::syncWithHostParameters()
{
    // This function is called when the plugin is loaded
    // In a real implementation, you would query the host for current track parameters
    // and sync them with your plugin parameters
    
    // For now, we'll implement a basic approach that works with parameter automation
    // The actual host parameter reading would require host-specific APIs
    
    // This is a placeholder for future host parameter synchronization
    // The timer callback in the editor will handle real-time parameter syncing
}

void MyPluginAudioProcessor::updateHostParameters()
{
    // This function sends our parameter changes back to the host
    // In a real implementation, you would update host automation parameters
    
    // For now, this is a placeholder for future host parameter updates
    // The parameter automation system will handle this automatically
}

void MyPluginAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // This callback is called when parameters change
    // You can use this to trigger host parameter updates or other actions
    
    juce::ignoreUnused(parameterID, newValue);
    
    // For future implementation: update host parameters when our parameters change
    // if (parameterID == IDs::pan || parameterID == IDs::gain)
    // {
    //     updateHostParameters();
    // }
}

juce::AudioProcessorEditor* MyPluginAudioProcessor::createEditor() { return new MyPluginAudioProcessorEditor (*this); }

void MyPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
}

void MyPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorValueTreeState::ParameterLayout MyPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::gain, 1 }, "Gain +", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::pan, 1 }, "Pan", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::panL, 1 }, "Pan L", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::panR, 1 }, "Pan R", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::depth, 1 }, "Depth", juce::NormalisableRange<float> (0.0f, 1.0f, 0.0001f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::width, 1 }, "Width", juce::NormalisableRange<float> (0.5f, 2.0f, 0.00001f), 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::tilt, 1 }, "Tone (Tilt)", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::scoop, 1 }, "Scoop", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::monoHz, 1 }, "Mono Maker (Hz)", juce::NormalisableRange<float> (0.0f, 300.0f, 0.01f, 0.5f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::hpHz, 1 }, "HP (Hz)", juce::NormalisableRange<float> (20.0f, 1000.0f, 0.01f, 0.5f), 20.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::lpHz, 1 }, "LP (Hz)", juce::NormalisableRange<float> (2000.0f, 20000.0f, 0.01f, 0.5f), 20000.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::satDriveDb, 1 }, "Saturation Drive (dB)", juce::NormalisableRange<float> (0.0f, 36.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::satMix, 1 }, "Saturation Mix", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID{ IDs::bypass, 1 }, "Bypass", juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ IDs::spaceAlgo, 1 }, "Space Algorithm", juce::StringArray { "Inner", "Outer", "Deep" }, 0));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::airDb, 1 }, "Air", juce::NormalisableRange<float> (0.0f, 6.0f, 0.1f), 0.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::bassDb, 1 }, "Bass", juce::NormalisableRange<float> (-6.0f, 6.0f, 0.1f), 0.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::ducking, 1 }, "Ducking", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ IDs::osMode, 1 }, "Oversampling", juce::StringArray { "Off", "2x", "4x" }, 1));

    params.push_back (std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ IDs::splitMode, 1 }, "Split Mode", false));

    // NEW: Frequency control parameters for EQ starting points
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::tiltFreq, 1 }, "Tilt Frequency", juce::NormalisableRange<float> (100.0f, 1000.0f, 1.0f, 0.5f), 500.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::scoopFreq, 1 }, "Scoop Frequency", juce::NormalisableRange<float> (200.0f, 2000.0f, 1.0f, 0.5f), 800.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::bassFreq, 1 }, "Bass Frequency", juce::NormalisableRange<float> (50.0f, 500.0f, 1.0f, 0.5f), 150.0f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ IDs::airFreq, 1 }, "Air Frequency", juce::NormalisableRange<float> (2000.0f, 20000.0f, 10.0f, 0.5f), 8000.0f));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MyPluginAudioProcessor(); } 