#pragma once
#include <JuceHeader.h>

/**
 * Phase Alignment DSP Engine
 * 
 * Provides comprehensive time/phase alignment capabilities including:
 * - Farrow fractional delay (4-tap cubic interpolation)
 * - All-Pass filters (AP1/AP2) with coefficient mapping
 * - FIR phase matching for Studio mode
 * - GCC-PHAT coarse alignment algorithm
 * - Sub-sample refinement via parabolic peak
 * - Dynamic Phase (transient-aware reduction)
 * - Audition Blend (50/50 parallel processing)
 */
class PhaseAlignmentEngine
{
public:
    PhaseAlignmentEngine();
    ~PhaseAlignmentEngine();

    // Core processing
    void prepare(double sampleRate, int maximumBlockSize, int numChannels);
    void reset();
    void processBlock(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& dryBuffer);
    
    // Parameter updates
    void updateParameters(const juce::AudioProcessorValueTreeState& apvts);
    
    // Engine modes
    enum class EngineMode { Live, Studio };
    void setEngineMode(EngineMode mode);
    
    // Alignment modes
    enum class AlignMode { Manual, Semi, Auto };
    void setAlignMode(AlignMode mode);
    
    // Alignment goals
    enum class AlignGoal { MonoPunch, BassTight, StereoFocus };
    void setAlignGoal(AlignGoal goal);
    
    // Delay parameters
    void setDelayCoarse(float ms);
    void setDelayFine(float ms);
    void setDelayUnits(bool useSamples);
    
    // Helper methods
    void updateDelay();
    void updateAllPassFrequencies();
    
    // All-Pass parameters
    void setLowAP(float degrees, float Q);
    void setMidAP(float degrees, float Q);
    void setHighAP(float degrees, float Q);
    
    // Crossover parameters
    void setCrossoverLow(float hz);
    void setCrossoverHigh(float hz);
    void setFollowCrossovers(bool follow);
    
    // Dynamic Phase
    enum class DynamicMode { Off, Light, Med, Hard };
    void setDynamicMode(DynamicMode mode);
    
    // Audition Blend
    enum class AuditionBlend { Apply100, Blend50 };
    void setAuditionBlend(AuditionBlend blend);
    
    // Monitoring
    enum class MonitorMode { Stereo, MonoMinus6, Mid, Side, A, B };
    void setMonitorMode(MonitorMode mode);
    
    // Metrics
    enum class MetricMode { Corr, Coherence, DeltaPhiRMS, MonoLFRMS };
    void setMetricMode(MetricMode mode);
    
    // Commands
    void resetTime();
    void resetPhase();
    void resetAll();
    void commitToBands();
    
    // State queries
    float getLatencyMs() const;
    bool isEngineLive() const { return engineMode == EngineMode::Live; }
    
private:
    // DSP Components
    class FarrowDelay;
    class AllPassFilter;
    class FIRPhaseMatch;
    class GCCPHAT;
    class DynamicPhase;
    class AuditionBlendProcessor;
    
    std::unique_ptr<FarrowDelay> farrowDelay;
    std::unique_ptr<AllPassFilter> lowAP, midAP, highAP;
    std::unique_ptr<FIRPhaseMatch> firPhaseMatch;
    std::unique_ptr<GCCPHAT> gccPHAT;
    std::unique_ptr<DynamicPhase> dynamicPhase;
    std::unique_ptr<AuditionBlendProcessor> auditionBlend;
    
    // State
    double sampleRate = 48000.0;
    int blockSize = 512;
    int numChannels = 2;
    
    EngineMode engineMode = EngineMode::Live;
    AlignMode alignMode = AlignMode::Manual;
    AlignGoal alignGoal = AlignGoal::MonoPunch;
    DynamicMode dynamicMode = DynamicMode::Light;
    AuditionBlend auditionBlendMode = AuditionBlend::Apply100;
    MonitorMode monitorMode = MonitorMode::Stereo;
    MetricMode metricMode = MetricMode::Corr;
    
    // Delay state
    float delayCoarseMs = 0.0f;
    float delayFineMs = 0.0f;
    bool useSamples = false;
    
    // Crossover state
    float xoLowHz = 120.0f;
    float xoHighHz = 2200.0f;
    bool followCrossovers = true;
    
    // All-Pass state
    struct APState {
        float degrees = 0.0f;
        float Q = 1.0f;
    } lowAPState, midAPState, highAPState;
    
    // Latency calculation
    float calculateLatency() const;
    
    // Internal processing
    void processLiveMode(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& dryBuffer);
    void processStudioMode(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& dryBuffer);
    void applyAllPassFilters(juce::AudioBuffer<float>& buffer);
    void applyDynamicPhase(juce::AudioBuffer<float>& buffer);
    void applyAuditionBlend(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& dryBuffer);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseAlignmentEngine)
};

/**
 * Farrow Fractional Delay
 * 4-tap cubic interpolation for sub-sample delay
 */
class PhaseAlignmentEngine::FarrowDelay
{
public:
    FarrowDelay();
    ~FarrowDelay();
    
    void prepare(double sampleRate, int maximumBlockSize, int numChannels);
    void reset();
    void setDelay(float delayMs);
    void processBlock(juce::AudioBuffer<float>& buffer);
    
private:
    struct DelayLine {
        std::vector<float> buffer;
        int writePos = 0;
        int bufferSize = 0;
    };
    
    std::vector<DelayLine> delayLines;
    double sampleRate = 48000.0;
    int numChannels = 2;
    float currentDelayMs = 0.0f;
    
    // Farrow coefficients for 4-tap cubic interpolation
    float getFarrowCoeff(float frac, int tap) const;
    void processChannel(float* channelData, int numSamples, int channel);
};

/**
 * All-Pass Filter
 * Implements both 1st-order and biquad all-pass filters
 */
class PhaseAlignmentEngine::AllPassFilter
{
public:
    AllPassFilter();
    ~AllPassFilter();
    
    void prepare(double sampleRate, int maximumBlockSize, int numChannels);
    void reset();
    void setParameters(float degrees, float Q, float centerFreq);
    void processBlock(juce::AudioBuffer<float>& buffer);
    
private:
    struct BiquadCoeffs {
        float b0, b1, b2, a1, a2;
    };
    
    std::vector<BiquadCoeffs> coeffs;
    std::vector<std::array<float, 4>> state; // x1, x2, y1, y2 for each channel
    
    double sampleRate = 48000.0;
    int numChannels = 2;
    float currentDegrees = 0.0f;
    float currentQ = 1.0f;
    float currentFreq = 1000.0f;
    
    void updateCoefficients();
    void processChannel(float* channelData, int numSamples, int channel, const BiquadCoeffs& coeff);
};

/**
 * FIR Phase Match
 * Linear-phase FIR filter for Studio mode phase matching
 */
class PhaseAlignmentEngine::FIRPhaseMatch
{
public:
    FIRPhaseMatch();
    ~FIRPhaseMatch();
    
    void prepare(double sampleRate, int maximumBlockSize, int numChannels);
    void reset();
    void setFIRLength(int length);
    void setPhaseResponse(const std::vector<float>& phaseResponse);
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    int getLatencySamples() const;
    
private:
    std::vector<std::vector<float>> firKernels;
    std::vector<std::vector<float>> delayLines;
    std::vector<int> writePositions;
    
    double sampleRate = 48000.0;
    int numChannels = 2;
    int firLength = 256;
    int latencySamples = 0;
    
    void updateFIRKernel();
    void processChannel(float* channelData, int numSamples, int channel);
};

/**
 * GCC-PHAT Algorithm
 * Generalized Cross-Correlation with Phase Transform for coarse alignment
 */
class PhaseAlignmentEngine::GCCPHAT
{
public:
    GCCPHAT();
    ~GCCPHAT();
    
    void prepare(double sampleRate, int maximumBlockSize, int numChannels);
    void reset();
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    float getCoarseDelay() const { return coarseDelayMs; }
    float getFineDelay() const { return fineDelayMs; }
    
private:
    std::vector<std::unique_ptr<juce::dsp::FFT>> ffts;
    std::vector<std::vector<std::complex<float>>> fftBuffers;
    std::vector<std::vector<float>> correlationBuffer;
    
    double sampleRate = 48000.0;
    int numChannels = 2;
    int fftSize = 1024;
    float coarseDelayMs = 0.0f;
    float fineDelayMs = 0.0f;
    
    void computeGCCPHAT(const float* ref, const float* target, int numSamples, int channel);
    float parabolicPeakRefinement(const std::vector<float>& correlation, int peakIndex);
};

/**
 * Dynamic Phase Processor
 * Transient-aware phase reduction
 */
class PhaseAlignmentEngine::DynamicPhase
{
public:
    DynamicPhase();
    ~DynamicPhase();
    
    void prepare(double sampleRate, int maximumBlockSize, int numChannels);
    void reset();
    void setMode(PhaseAlignmentEngine::DynamicMode mode);
    void processBlock(juce::AudioBuffer<float>& buffer);
    
private:
    enum class DynamicMode { Off, Light, Med, Hard };
    
    std::vector<juce::dsp::Gain<float>> envelopeFollowers;
    std::vector<float> reductionFactors;
    
    double sampleRate = 48000.0;
    int numChannels = 2;
    DynamicMode currentMode = DynamicMode::Off;
    
    float getReductionFactor(DynamicMode mode) const;
    void processChannel(float* channelData, int numSamples, int channel);
};

/**
 * Audition Blend Processor
 * 50/50 parallel processing for auditioning
 */
class PhaseAlignmentEngine::AuditionBlendProcessor
{
public:
    AuditionBlendProcessor();
    ~AuditionBlendProcessor();
    
    void prepare(double sampleRate, int maximumBlockSize, int numChannels);
    void reset();
    void setBlendMode(PhaseAlignmentEngine::AuditionBlend mode);
    void processBlock(juce::AudioBuffer<float>& processed, juce::AudioBuffer<float>& dry);
    
private:
    enum class AuditionBlend { Apply100, Blend50 };
    
    std::vector<juce::dsp::Gain<float>> gainProcessors;
    
    int numChannels = 2;
    AuditionBlend currentMode = AuditionBlend::Apply100;
    
    void updateGains();
};
