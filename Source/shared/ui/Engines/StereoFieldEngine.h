#pragma once
#include <JuceHeader.h>

// Heatmap / analysis engine for Imager (UI-thread driven)
class StereoFieldEngine
{
public:
    struct Settings
    {
        int    fftOrder       = 11;   // 2048
        int    hopDiv         = 2;    // 50% overlap
        double fMin           = 40.0;
        double fMax           = 16000.0;
        int    bandsPerDecade = 12;   // 1/3 octave feel
        int    historyWidthPx = 600;  // columns
        float  energyGateDb   = -60.0f;
        float  gateHystDb     = 3.0f;
        float  aAtk           = 0.35f; // smoothing toward new (fast)
        float  aRel           = 0.12f; // smoothing away from new (slow)
        bool   enablePre      = false;
    };

    void prepare (double sampleRate, const Settings& s);
    void reset();
    void setSettings (const Settings& s);
    void setFreeze (bool on) { frozen = on; }
    void setEnablePre (bool on) { settings.enablePre = on; }
    void setHistoryWidth (int px);

    // feed from PaneManager/UI thread
    void pushBlock (const float* L, const float* R, int n, bool isPre);
    // generate next column(s) if enough data available
    void process();

    // painting helpers
    const juce::Image& getImagePost() const { return imgPost; }
    const juce::Image& getImagePre()  const { return imgPre; }
    int  getWriteX() const { return writeX; }
    int  getHistoryWidth() const { return imgPost.getWidth(); }
    int  getBandCount() const { return (int) bands.size(); }
    double getBandCenterHz (int b) const { return bands.empty() ? 0.0 : bands[(size_t) juce::jlimit (0, (int) bands.size()-1, b)].fC; }
    const std::vector<float>& getWidthPerBandPost() const { return widthPost; }

private:
    struct Band { double fLo{}, fHi{}, fC{}; int k0{}, k1{}; double w0{}, w1{}; float rSmooth{0.f}; bool gateOpen{false}; };
    Settings settings;
    double sr = 48000.0;
    int fftOrder = 11, fftSize = 2048, hopSize = 1024;

    // bands & window/fft
    std::vector<Band> bands;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    juce::dsp::FFT fft { 11 };
    juce::HeapBlock<float> frameL, frameR;      // rolling frame (fftSize)
    juce::HeapBlock<float> tdBuf;               // temp time buffer
    juce::HeapBlock<float> fdL, fdR;            // complex freq (2*fftSize)

    // ring buffers (UI thread)
    juce::AudioBuffer<float> fifoPost { 2, 1 << 16 };
    juce::AbstractFifo       afifoPost { 1 << 16 };
    juce::AudioBuffer<float> fifoPre  { 2, 1 << 16 };
    juce::AbstractFifo       afifoPre  { 1 << 16 };

    // images (circular writer)
    juce::Image imgPost, imgPre;
    int writeX = 0;
    bool frozen = false;
    juce::Image lutImg; // 256x1 colour ramp
    std::vector<float> widthPost; // |S|/(|M|+eps) per band (latest)

    // helpers
    static inline double hzToK (double hz, double sr, int N) { return juce::jlimit (0.0, (double) N/2.0, hz * (double) N / sr); }
    void rebuildFFT();
    void rebuildBands();
    void rebuildImages();
    void ensureFrames();
    void pushSamplesImpl (juce::AudioBuffer<float>& dstBuf, juce::AbstractFifo& a, const float* L, const float* R, int n);
    bool popHop (juce::AudioBuffer<float>& src, juce::AbstractFifo& a);
    void stft (const float* in, float* td, float* fd);
    bool computeRColumn (juce::AudioBuffer<float>& src, juce::AbstractFifo& a, juce::Image& dest);
    static juce::Colour mapColour (float r, const juce::Image& lut);
};


