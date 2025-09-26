#include "PhaseAlignmentEngine.h"
#include "../Core/PluginProcessor.h"

// =============================================================================
// PhaseAlignmentEngine Implementation
// =============================================================================

PhaseAlignmentEngine::PhaseAlignmentEngine()
{
    farrowDelay = std::make_unique<FarrowDelay>();
    lowAP = std::make_unique<AllPassFilter>();
    midAP = std::make_unique<AllPassFilter>();
    highAP = std::make_unique<AllPassFilter>();
    firPhaseMatch = std::make_unique<FIRPhaseMatch>();
    gccPHAT = std::make_unique<GCCPHAT>();
    dynamicPhase = std::make_unique<DynamicPhase>();
    auditionBlend = std::make_unique<AuditionBlendProcessor>();
}

PhaseAlignmentEngine::~PhaseAlignmentEngine() = default;

void PhaseAlignmentEngine::prepare(double sampleRate, int maximumBlockSize, int numChannels)
{
    this->sampleRate = sampleRate;
    this->blockSize = maximumBlockSize;
    this->numChannels = numChannels;
    
    farrowDelay->prepare(sampleRate, maximumBlockSize, numChannels);
    lowAP->prepare(sampleRate, maximumBlockSize, numChannels);
    midAP->prepare(sampleRate, maximumBlockSize, numChannels);
    highAP->prepare(sampleRate, maximumBlockSize, numChannels);
    firPhaseMatch->prepare(sampleRate, maximumBlockSize, numChannels);
    gccPHAT->prepare(sampleRate, maximumBlockSize, numChannels);
    dynamicPhase->prepare(sampleRate, maximumBlockSize, numChannels);
    auditionBlend->prepare(sampleRate, maximumBlockSize, numChannels);
}

void PhaseAlignmentEngine::reset()
{
    farrowDelay->reset();
    lowAP->reset();
    midAP->reset();
    highAP->reset();
    firPhaseMatch->reset();
    gccPHAT->reset();
    dynamicPhase->reset();
    auditionBlend->reset();
}

void PhaseAlignmentEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& dryBuffer)
{
    if (engineMode == EngineMode::Live)
        processLiveMode(buffer, dryBuffer);
    else
        processStudioMode(buffer, dryBuffer);
}

void PhaseAlignmentEngine::updateParameters(const juce::AudioProcessorValueTreeState& apvts)
{
    // Engine mode
    auto engineChoice = apvts.getRawParameterValue(IDs::phase_engine)->load();
    setEngineMode(engineChoice > 0.5f ? EngineMode::Studio : EngineMode::Live);
    
    // Alignment mode
    auto alignModeChoice = apvts.getRawParameterValue(IDs::phase_align_mode)->load();
    setAlignMode(static_cast<AlignMode>(static_cast<int>(alignModeChoice)));
    
    // Alignment goal
    auto alignGoalChoice = apvts.getRawParameterValue(IDs::phase_align_goal)->load();
    setAlignGoal(static_cast<AlignGoal>(static_cast<int>(alignGoalChoice)));
    
    // Delay parameters
    setDelayCoarse(apvts.getRawParameterValue(IDs::phase_delay_ms_coarse)->load());
    setDelayFine(apvts.getRawParameterValue(IDs::phase_delay_ms_fine)->load());
    setDelayUnits(apvts.getRawParameterValue(IDs::phase_delay_units)->load() > 0.5f);
    
    // All-Pass parameters
    setLowAP(apvts.getRawParameterValue(IDs::phase_lo_ap_deg)->load(),
             apvts.getRawParameterValue(IDs::phase_lo_q)->load());
    setMidAP(apvts.getRawParameterValue(IDs::phase_mid_ap_deg)->load(),
             apvts.getRawParameterValue(IDs::phase_mid_q)->load());
    setHighAP(apvts.getRawParameterValue(IDs::phase_hi_ap_deg)->load(),
              apvts.getRawParameterValue(IDs::phase_hi_q)->load());
    
    // Crossover parameters
    setCrossoverLow(apvts.getRawParameterValue(IDs::phase_xo_lo_hz)->load());
    setCrossoverHigh(apvts.getRawParameterValue(IDs::phase_xo_hi_hz)->load());
    setFollowCrossovers(apvts.getRawParameterValue(IDs::phase_follow_xo)->load() > 0.5f);
    
    // Dynamic Phase
    auto dynamicChoice = apvts.getRawParameterValue(IDs::phase_dynamic_mode)->load();
    setDynamicMode(static_cast<DynamicMode>(static_cast<int>(dynamicChoice)));
    
    // Audition Blend
    auto auditionChoice = apvts.getRawParameterValue(IDs::phase_audition_blend)->load();
    setAuditionBlend(static_cast<AuditionBlend>(static_cast<int>(auditionChoice)));
    
    // Monitor mode
    auto monitorChoice = apvts.getRawParameterValue(IDs::phase_monitor_mode)->load();
    setMonitorMode(static_cast<MonitorMode>(static_cast<int>(monitorChoice)));
    
    // Metric mode
    auto metricChoice = apvts.getRawParameterValue(IDs::phase_metric_mode)->load();
    setMetricMode(static_cast<MetricMode>(static_cast<int>(metricChoice)));
}

void PhaseAlignmentEngine::setEngineMode(EngineMode mode)
{
    engineMode = mode;
}

void PhaseAlignmentEngine::setAlignMode(AlignMode mode)
{
    alignMode = mode;
}

void PhaseAlignmentEngine::setAlignGoal(AlignGoal goal)
{
    alignGoal = goal;
}

void PhaseAlignmentEngine::setDelayCoarse(float ms)
{
    delayCoarseMs = ms;
    updateDelay();
}

void PhaseAlignmentEngine::setDelayFine(float ms)
{
    delayFineMs = ms;
    updateDelay();
}

void PhaseAlignmentEngine::setDelayUnits(bool useSamples)
{
    this->useSamples = useSamples;
    updateDelay();
}

void PhaseAlignmentEngine::setLowAP(float degrees, float Q)
{
    lowAPState.degrees = degrees;
    lowAPState.Q = Q;
    lowAP->setParameters(degrees, Q, xoLowHz);
}

void PhaseAlignmentEngine::setMidAP(float degrees, float Q)
{
    midAPState.degrees = degrees;
    midAPState.Q = Q;
    midAP->setParameters(degrees, Q, (xoLowHz + xoHighHz) * 0.5f);
}

void PhaseAlignmentEngine::setHighAP(float degrees, float Q)
{
    highAPState.degrees = degrees;
    highAPState.Q = Q;
    highAP->setParameters(degrees, Q, xoHighHz);
}

void PhaseAlignmentEngine::setCrossoverLow(float hz)
{
    xoLowHz = hz;
    updateAllPassFrequencies();
}

void PhaseAlignmentEngine::setCrossoverHigh(float hz)
{
    xoHighHz = hz;
    updateAllPassFrequencies();
}

void PhaseAlignmentEngine::setFollowCrossovers(bool follow)
{
    followCrossovers = follow;
}

void PhaseAlignmentEngine::setDynamicMode(DynamicMode mode)
{
    dynamicMode = mode;
    dynamicPhase->setMode(mode);
}

void PhaseAlignmentEngine::setAuditionBlend(AuditionBlend blend)
{
    auditionBlendMode = blend;
    auditionBlend->setBlendMode(blend);
}

void PhaseAlignmentEngine::setMonitorMode(MonitorMode mode)
{
    monitorMode = mode;
}

void PhaseAlignmentEngine::setMetricMode(MetricMode mode)
{
    metricMode = mode;
}

void PhaseAlignmentEngine::resetTime()
{
    setDelayCoarse(0.0f);
    setDelayFine(0.0f);
}

void PhaseAlignmentEngine::resetPhase()
{
    setLowAP(0.0f, lowAPState.Q);
    setMidAP(0.0f, midAPState.Q);
    setHighAP(0.0f, highAPState.Q);
}

void PhaseAlignmentEngine::resetAll()
{
    resetTime();
    resetPhase();
}

void PhaseAlignmentEngine::commitToBands()
{
    // Implementation for committing current settings to band slots
    // This would typically save the current alignment to preset slots
}

float PhaseAlignmentEngine::getLatencyMs() const
{
    return calculateLatency();
}

void PhaseAlignmentEngine::processLiveMode(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& dryBuffer)
{
    // Live mode: AP only, zero latency
    applyAllPassFilters(buffer);
    applyDynamicPhase(buffer);
    applyAuditionBlend(buffer, dryBuffer);
}

void PhaseAlignmentEngine::processStudioMode(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& dryBuffer)
{
    // Studio mode: AP + FIR, adds latency
    applyAllPassFilters(buffer);
    firPhaseMatch->processBlock(buffer);
    applyDynamicPhase(buffer);
    applyAuditionBlend(buffer, dryBuffer);
}

void PhaseAlignmentEngine::applyAllPassFilters(juce::AudioBuffer<float>& buffer)
{
    lowAP->processBlock(buffer);
    midAP->processBlock(buffer);
    highAP->processBlock(buffer);
}

void PhaseAlignmentEngine::applyDynamicPhase(juce::AudioBuffer<float>& buffer)
{
    dynamicPhase->processBlock(buffer);
}

void PhaseAlignmentEngine::applyAuditionBlend(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& dryBuffer)
{
    auditionBlend->processBlock(buffer, dryBuffer);
}

float PhaseAlignmentEngine::calculateLatency() const
{
    if (engineMode == EngineMode::Live)
        return 0.0f;
    else
        return firPhaseMatch->getLatencySamples() * 1000.0f / sampleRate;
}

void PhaseAlignmentEngine::updateDelay()
{
    float totalDelayMs = delayCoarseMs + delayFineMs;
    if (useSamples)
        totalDelayMs = totalDelayMs * sampleRate / 1000.0f;
    
    farrowDelay->setDelay(totalDelayMs);
}

void PhaseAlignmentEngine::updateAllPassFrequencies()
{
    setLowAP(lowAPState.degrees, lowAPState.Q);
    setMidAP(midAPState.degrees, midAPState.Q);
    setHighAP(highAPState.degrees, highAPState.Q);
}

// =============================================================================
// FarrowDelay Implementation
// =============================================================================

PhaseAlignmentEngine::FarrowDelay::FarrowDelay() = default;
PhaseAlignmentEngine::FarrowDelay::~FarrowDelay() = default;

void PhaseAlignmentEngine::FarrowDelay::prepare(double sampleRate, int maximumBlockSize, int numChannels)
{
    this->sampleRate = sampleRate;
    this->numChannels = numChannels;
    
    delayLines.resize(numChannels);
    for (auto& line : delayLines)
    {
        line.bufferSize = static_cast<int>(sampleRate * 0.1); // 100ms max delay
        line.buffer.resize(line.bufferSize, 0.0f);
        line.writePos = 0;
    }
}

void PhaseAlignmentEngine::FarrowDelay::reset()
{
    for (auto& line : delayLines)
    {
        std::fill(line.buffer.begin(), line.buffer.end(), 0.0f);
        line.writePos = 0;
    }
}

void PhaseAlignmentEngine::FarrowDelay::setDelay(float delayMs)
{
    currentDelayMs = delayMs;
}

void PhaseAlignmentEngine::FarrowDelay::processBlock(juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < numChannels; ++channel)
    {
        processChannel(buffer.getWritePointer(channel), buffer.getNumSamples(), channel);
    }
}

float PhaseAlignmentEngine::FarrowDelay::getFarrowCoeff(float frac, int tap) const
{
    // 4-tap cubic interpolation coefficients
    const float t = frac;
    const float t2 = t * t;
    const float t3 = t2 * t;
    
    switch (tap)
    {
        case 0: return -t3/6.0f + t2/2.0f - t/2.0f + 1.0f/6.0f;
        case 1: return t3/2.0f - t2 + 2.0f/3.0f;
        case 2: return -t3/2.0f + t2/2.0f + t/2.0f + 1.0f/6.0f;
        case 3: return t3/6.0f;
        default: return 0.0f;
    }
}

void PhaseAlignmentEngine::FarrowDelay::processChannel(float* channelData, int numSamples, int channel)
{
    auto& line = delayLines[channel];
    const float delaySamples = currentDelayMs * sampleRate / 1000.0f;
    const int delayInt = static_cast<int>(delaySamples);
    const float delayFrac = delaySamples - delayInt;
    
    for (int i = 0; i < numSamples; ++i)
    {
        // Write to delay line
        line.buffer[line.writePos] = channelData[i];
        
        // Read with fractional delay using Farrow interpolation
        float output = 0.0f;
        for (int tap = 0; tap < 4; ++tap)
        {
            int readPos = (line.writePos - delayInt - tap + line.bufferSize) % line.bufferSize;
            output += line.buffer[readPos] * getFarrowCoeff(delayFrac, tap);
        }
        
        channelData[i] = output;
        line.writePos = (line.writePos + 1) % line.bufferSize;
    }
}

// =============================================================================
// AllPassFilter Implementation
// =============================================================================

PhaseAlignmentEngine::AllPassFilter::AllPassFilter() = default;
PhaseAlignmentEngine::AllPassFilter::~AllPassFilter() = default;

void PhaseAlignmentEngine::AllPassFilter::prepare(double sampleRate, int maximumBlockSize, int numChannels)
{
    this->sampleRate = sampleRate;
    this->numChannels = numChannels;
    
    coeffs.resize(numChannels);
    state.resize(numChannels);
    
    for (auto& s : state)
        std::fill(s.begin(), s.end(), 0.0f);
}

void PhaseAlignmentEngine::AllPassFilter::reset()
{
    for (auto& s : state)
        std::fill(s.begin(), s.end(), 0.0f);
}

void PhaseAlignmentEngine::AllPassFilter::setParameters(float degrees, float Q, float centerFreq)
{
    currentDegrees = degrees;
    currentQ = Q;
    currentFreq = centerFreq;
    updateCoefficients();
}

void PhaseAlignmentEngine::AllPassFilter::processBlock(juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < numChannels; ++channel)
    {
        processChannel(buffer.getWritePointer(channel), buffer.getNumSamples(), channel, coeffs[channel]);
    }
}

void PhaseAlignmentEngine::AllPassFilter::updateCoefficients()
{
    const float omega = 2.0f * juce::MathConstants<float>::pi * currentFreq / sampleRate;
    const float cosOmega = std::cos(omega);
    const float sinOmega = std::sin(omega);
    const float alpha = sinOmega / (2.0f * currentQ);
    
    // Convert phase degrees to radians
    const float phaseRad = currentDegrees * juce::MathConstants<float>::pi / 180.0f;
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto& coeff = coeffs[channel];
        
        // Biquad all-pass filter coefficients
        coeff.b0 = 1.0f - alpha;
        coeff.b1 = -2.0f * cosOmega;
        coeff.b2 = 1.0f + alpha;
        coeff.a1 = -2.0f * cosOmega;
        coeff.a2 = 1.0f - alpha;
        
        // Normalize for unity gain
        const float norm = 1.0f / (1.0f + alpha);
        coeff.b0 *= norm;
        coeff.b1 *= norm;
        coeff.b2 *= norm;
        coeff.a1 *= norm;
        coeff.a2 *= norm;
    }
}

void PhaseAlignmentEngine::AllPassFilter::processChannel(float* channelData, int numSamples, int channel, const BiquadCoeffs& coeff)
{
    auto& s = state[channel];
    
    for (int i = 0; i < numSamples; ++i)
    {
        const float x = channelData[i];
        const float y = coeff.b0 * x + coeff.b1 * s[0] + coeff.b2 * s[1] - coeff.a1 * s[2] - coeff.a2 * s[3];
        
        // Update state
        s[1] = s[0];
        s[0] = x;
        s[3] = s[2];
        s[2] = y;
        
        channelData[i] = y;
    }
}

// =============================================================================
// FIRPhaseMatch Implementation
// =============================================================================

PhaseAlignmentEngine::FIRPhaseMatch::FIRPhaseMatch() = default;
PhaseAlignmentEngine::FIRPhaseMatch::~FIRPhaseMatch() = default;

void PhaseAlignmentEngine::FIRPhaseMatch::prepare(double sampleRate, int maximumBlockSize, int numChannels)
{
    this->sampleRate = sampleRate;
    this->numChannels = numChannels;
    
    firKernels.resize(numChannels);
    delayLines.resize(numChannels);
    writePositions.resize(numChannels);
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        firKernels[channel].resize(firLength, 0.0f);
        delayLines[channel].resize(firLength, 0.0f);
        writePositions[channel] = 0;
    }
    
    updateFIRKernel();
}

void PhaseAlignmentEngine::FIRPhaseMatch::reset()
{
    for (int channel = 0; channel < numChannels; ++channel)
    {
        std::fill(firKernels[channel].begin(), firKernels[channel].end(), 0.0f);
        std::fill(delayLines[channel].begin(), delayLines[channel].end(), 0.0f);
        writePositions[channel] = 0;
    }
}

void PhaseAlignmentEngine::FIRPhaseMatch::setFIRLength(int length)
{
    firLength = length;
    latencySamples = length / 2;
    updateFIRKernel();
}

void PhaseAlignmentEngine::FIRPhaseMatch::setPhaseResponse(const std::vector<float>& phaseResponse)
{
    // Update FIR kernel based on desired phase response
    updateFIRKernel();
}

void PhaseAlignmentEngine::FIRPhaseMatch::processBlock(juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < numChannels; ++channel)
    {
        processChannel(buffer.getWritePointer(channel), buffer.getNumSamples(), channel);
    }
}

int PhaseAlignmentEngine::FIRPhaseMatch::getLatencySamples() const
{
    return latencySamples;
}

void PhaseAlignmentEngine::FIRPhaseMatch::updateFIRKernel()
{
    // Generate linear-phase FIR kernel
    // This is a simplified implementation - in practice, you'd design the kernel
    // based on the desired phase response
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto& kernel = firKernels[channel];
        kernel.resize(firLength);
        
        // Simple windowed sinc for demonstration
        const float fc = 0.5f; // Cutoff frequency
        for (int i = 0; i < firLength; ++i)
        {
            const float n = i - firLength / 2.0f;
            if (n == 0.0f)
                kernel[i] = 2.0f * fc;
            else
                kernel[i] = 2.0f * fc * std::sin(2.0f * juce::MathConstants<float>::pi * fc * n) / (juce::MathConstants<float>::pi * n);
            
            // Apply window
            const float window = 0.5f - 0.5f * std::cos(2.0f * juce::MathConstants<float>::pi * i / (firLength - 1));
            kernel[i] *= window;
        }
    }
}

void PhaseAlignmentEngine::FIRPhaseMatch::processChannel(float* channelData, int numSamples, int channel)
{
    auto& kernel = firKernels[channel];
    auto& delayLine = delayLines[channel];
    int& writePos = writePositions[channel];
    
    for (int i = 0; i < numSamples; ++i)
    {
        // Write to delay line
        delayLine[writePos] = channelData[i];
        
        // Convolve with FIR kernel
        float output = 0.0f;
        for (int j = 0; j < firLength; ++j)
        {
            int readPos = (writePos - j + firLength) % firLength;
            output += delayLine[readPos] * kernel[j];
        }
        
        channelData[i] = output;
        writePos = (writePos + 1) % firLength;
    }
}

// =============================================================================
// GCCPHAT Implementation
// =============================================================================

PhaseAlignmentEngine::GCCPHAT::GCCPHAT()
{
    ffts.resize(2); // For stereo processing
    for (auto& fft : ffts) {
        fft = std::make_unique<juce::dsp::FFT>(10); // 1024 point FFT
    }
    fftBuffers.resize(2);
    correlationBuffer.resize(2);
}

PhaseAlignmentEngine::GCCPHAT::~GCCPHAT() = default;

void PhaseAlignmentEngine::GCCPHAT::prepare(double sampleRate, int maximumBlockSize, int numChannels)
{
    this->sampleRate = sampleRate;
    this->numChannels = numChannels;
    
    for (int i = 0; i < 2; ++i)
    {
        ffts[i] = std::make_unique<juce::dsp::FFT>(10); // 1024 point FFT
        fftBuffers[i].resize(fftSize);
        correlationBuffer[i].resize(fftSize);
    }
}

void PhaseAlignmentEngine::GCCPHAT::reset()
{
    for (auto& buffer : fftBuffers)
        std::fill(buffer.begin(), buffer.end(), 0.0f);
    
    for (auto& buffer : correlationBuffer)
        std::fill(buffer.begin(), buffer.end(), 0.0f);
    
    coarseDelayMs = 0.0f;
    fineDelayMs = 0.0f;
}

void PhaseAlignmentEngine::GCCPHAT::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (numChannels >= 2)
    {
        computeGCCPHAT(buffer.getReadPointer(0), buffer.getReadPointer(1), buffer.getNumSamples(), 0);
    }
}

void PhaseAlignmentEngine::GCCPHAT::computeGCCPHAT(const float* ref, const float* target, int numSamples, int channel)
{
    // Simplified GCC-PHAT implementation
    // In practice, this would perform full cross-correlation analysis
    
    // For now, just set some example values
    coarseDelayMs = 0.0f;
    fineDelayMs = 0.0f;
}

float PhaseAlignmentEngine::GCCPHAT::parabolicPeakRefinement(const std::vector<float>& correlation, int peakIndex)
{
    // Parabolic peak refinement for sub-sample accuracy
    if (peakIndex <= 0 || peakIndex >= static_cast<int>(correlation.size()) - 1)
        return static_cast<float>(peakIndex);
    
    const float y1 = correlation[peakIndex - 1];
    const float y2 = correlation[peakIndex];
    const float y3 = correlation[peakIndex + 1];
    
    const float a = (y1 - 2.0f * y2 + y3) / 2.0f;
    const float b = (y3 - y1) / 2.0f;
    
    if (std::abs(a) < 1e-6f)
        return static_cast<float>(peakIndex);
    
    return static_cast<float>(peakIndex) - b / (2.0f * a);
}

// =============================================================================
// DynamicPhase Implementation
// =============================================================================

PhaseAlignmentEngine::DynamicPhase::DynamicPhase() = default;
PhaseAlignmentEngine::DynamicPhase::~DynamicPhase() = default;

void PhaseAlignmentEngine::DynamicPhase::prepare(double sampleRate, int maximumBlockSize, int numChannels)
{
    this->sampleRate = sampleRate;
    this->numChannels = numChannels;
    
    envelopeFollowers.resize(numChannels);
    reductionFactors.resize(numChannels);
    
    for (int i = 0; i < numChannels; ++i)
    {
        // Simple envelope follower implementation
        // envelopeFollowers[i] is a Gain<float>, so we'll use a different approach
        reductionFactors[i] = 1.0f;
    }
}

void PhaseAlignmentEngine::DynamicPhase::reset()
{
    for (auto& follower : envelopeFollowers)
        follower.reset();
    
    std::fill(reductionFactors.begin(), reductionFactors.end(), 1.0f);
}

void PhaseAlignmentEngine::DynamicPhase::setMode(PhaseAlignmentEngine::DynamicMode mode)
{
    currentMode = static_cast<DynamicMode>(mode);
}

void PhaseAlignmentEngine::DynamicPhase::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (currentMode == DynamicMode::Off)
        return;
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        processChannel(buffer.getWritePointer(channel), buffer.getNumSamples(), channel);
    }
}

float PhaseAlignmentEngine::DynamicPhase::getReductionFactor(DynamicMode mode) const
{
    switch (mode)
    {
        case DynamicMode::Off: return 1.0f;
        case DynamicMode::Light: return 0.8f;
        case DynamicMode::Med: return 0.6f;
        case DynamicMode::Hard: return 0.4f;
        default: return 1.0f;
    }
}

void PhaseAlignmentEngine::DynamicPhase::processChannel(float* channelData, int numSamples, int channel)
{
    auto& follower = envelopeFollowers[channel];
    const float reductionFactor = getReductionFactor(currentMode);
    
    for (int i = 0; i < numSamples; ++i)
    {
        const float envelope = follower.processSample(std::abs(channelData[i]));
        const float dynamicReduction = 1.0f - (1.0f - reductionFactor) * envelope;
        
        channelData[i] *= dynamicReduction;
    }
}

// =============================================================================
// AuditionBlendProcessor Implementation
// =============================================================================

PhaseAlignmentEngine::AuditionBlendProcessor::AuditionBlendProcessor() = default;
PhaseAlignmentEngine::AuditionBlendProcessor::~AuditionBlendProcessor() = default;

void PhaseAlignmentEngine::AuditionBlendProcessor::prepare(double sampleRate, int maximumBlockSize, int numChannels)
{
    this->numChannels = numChannels;
    
    gainProcessors.resize(numChannels);
    for (auto& gain : gainProcessors)
    {
        gain.setGainLinear(1.0f);
    }
}

void PhaseAlignmentEngine::AuditionBlendProcessor::reset()
{
    for (auto& gain : gainProcessors)
        gain.reset();
}

void PhaseAlignmentEngine::AuditionBlendProcessor::setBlendMode(PhaseAlignmentEngine::AuditionBlend mode)
{
    currentMode = static_cast<AuditionBlend>(mode);
    updateGains();
}

void PhaseAlignmentEngine::AuditionBlendProcessor::processBlock(juce::AudioBuffer<float>& processed, juce::AudioBuffer<float>& dry)
{
    if (currentMode == AuditionBlend::Apply100)
        return; // No blending needed
    
    // 50/50 blend of processed and dry signals
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* procData = processed.getWritePointer(channel);
        const auto* dryData = dry.getReadPointer(channel);
        
        for (int i = 0; i < processed.getNumSamples(); ++i)
        {
            procData[i] = 0.5f * procData[i] + 0.5f * dryData[i];
        }
    }
}

void PhaseAlignmentEngine::AuditionBlendProcessor::updateGains()
{
    const float gain = (currentMode == AuditionBlend::Apply100) ? 1.0f : 0.5f;
    
    for (auto& gainProc : gainProcessors)
    {
        gainProc.setGainLinear(gain);
    }
}
