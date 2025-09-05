#pragma once
#include <JuceHeader.h>

class SpectrumAnalyzer : public juce::Component, private juce::Timer
{
public:
    struct Params
    {
        int    fftOrder         = 11;     // 2^11 = 2048
        float  avgTimeMs        = 120.0f; // exp smoothing time-constant
        float  peakHoldSec      = 1.2f;   // 0 = off
        float  peakFallDbPerSec = 12.0f;  // peak decay if held
        float  minDb            = -90.0f;
        float  maxDb            =   6.0f;
        float  refDb            =   0.0f; // 0 dB ref line (optional)
        float  slopeDbPerOct    = 0.0f;   // e.g. +3 to "flatten" pink-ish spectra
        bool   monoSum          = true;   // sum L/R before FFT
        bool   drawPeaks        = true;
        int    fps              = 30;     // UI refresh rate
    };

    explicit SpectrumAnalyzer();
    ~SpectrumAnalyzer() override = default;

    void setSampleRate (double sr);
    void setParams     (const Params& p);

    // Real-time safe audio feeds (call from audio thread)
    // Post-processed (current behavior)
    void pushBlock (const float* left, const float* right, int numSamples);
    // Pre-processed overlay
    void pushBlockPre (const float* left, const float* right, int numSamples);

    void setEqOverlayFn (std::function<float(double /*Hz*/)> fn) { eqOverlayFn = std::move (fn); }

    std::pair<float,float> getDbRange() const { return {params.minDb, params.maxDb}; }

    // Colour hooks (set these from your LookAndFeel/theme)
    juce::Colour fillColour      { juce::Colours::aqua.withAlpha (0.30f) };
    juce::Colour strokeColour    { juce::Colours::white.withAlpha (0.80f) };
    juce::Colour peakColour      { juce::Colours::orange.withAlpha (0.85f) };
    juce::Colour gridColour      { juce::Colours::white.withAlpha (0.12f) };
    // Pre overlay colours
    juce::Colour preFillColour   { juce::Colours::white.withAlpha (0.15f) };
    juce::Colour preStrokeColour { juce::Colours::white.withAlpha (0.65f) };
    juce::Colour prePeakColour   { juce::Colours::white.withAlpha (0.70f) };
    juce::Colour eqOverlayColour { juce::Colours::yellow.withAlpha (0.90f) };

    void setFreqRange (double fMin, double fMax);
    void setPreDelaySamples (int n);

    void paint (juce::Graphics&) override;
    void resized() override {}

    // Gate audio thread writes during reconfig/tab switches
    void pauseAudio()  noexcept { acceptingAudio.store (false, std::memory_order_release); }
    void resumeAudio() noexcept { acceptingAudio.store (true,  std::memory_order_release); }

private:
    // Frame/state guards
    std::atomic<bool> postFrameReady { false };
    std::atomic<bool> preFrameReady  { false };
    std::atomic<bool> acceptingAudio { false };
    juce::SpinLock    dataLock; // guard UI reads vs audio writes
    juce::CriticalSection cfgLock; // guard (re)configuration of FFT/buffers
    std::atomic<bool> postConfigured { false };

    // Pixel-column caches to avoid realloc during paint
    std::vector<float> columnMain, columnPeak, columnEq;

    bool isFrameDrawable (juce::Rectangle<float> r) const;
    void drawGridOnly (juce::Graphics& g, juce::Rectangle<float> r);
    void configurePost (int fftOrder);
    void resetPost();

    void timerCallback() override;
    void performFFTIfReadyPost();
    void performFFTIfReadyPre();
    void renderPaths (juce::Graphics& g, juce::Rectangle<float> bounds);

    float freqToX (double hz, float left, float right) const;
    float dbToY   (float dB, float top, float bottom) const;

    Params params;
    double sampleRate = 48000.0;
    double fMin = 20.0, fMax = 20000.0;

    int fftOrder = 11;
    int fftSize  = 1 << 11;
    int hopSize  = (1 << 11) / 2; // 50% overlap

    juce::dsp::FFT fft { 11 };
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    float windowScale = 1.0f; // calibration: single-sided + window RMS normalization

    // ring buffers (mono streams)
    juce::AudioBuffer<float> fifoPost;     // 1 x fftSize
    std::atomic<int> fifoWritePost { 0 };  // write index [0..fftSize-1]
    std::atomic<int> samplesAccumPost { 0 };
    juce::AudioBuffer<float> fifoPre;      // 1 x fftSize
    std::atomic<int> fifoWritePre { 0 };
    std::atomic<int> samplesAccumPre { 0 };
    // pre-delay line for latency alignment
    juce::AudioBuffer<float> preDelay;
    int preDelayWrite = 0;
    int preDelaySamples = 0;

    // work buffers
    juce::HeapBlock<float> timeDomain; // fftSize
    juce::HeapBlock<float> freqDomain; // 2*fftSize (real-only)
    std::vector<float> magDbPost;      // fftSize/2
    std::vector<float> smoothedDbPost; // fftSize/2
    std::vector<float> peakDbPost;     // fftSize/2
    std::vector<float> magDbPre;       // fftSize/2
    std::vector<float> smoothedDbPre;  // fftSize/2
    std::vector<float> peakDbPre;      // fftSize/2
    struct Smoother { float yFast=-120.f, ySlow=-120.f; };
    std::vector<Smoother> smoothersPost, smoothersPre;

    // UI-ready curves (downsampled to pixels)
    juce::Path areaPathPost, linePathPost, peakPathPost;
    juce::Path areaPathPre,  linePathPre,  peakPathPre, eqPath;
    std::function<float(double)> eqOverlayFn;

    // smoothing coefficients
    float alphaAvg = 0.2f;  // legacy
    float peakFallPerFrameDb = 0.4f;
    std::atomic<bool> hasPre { false }, hasPost { false };
    float alphaFast = 0.0f, alphaSlow = 0.0f; // asymmetric smoothing
};


