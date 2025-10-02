#pragma once

#include <JuceHeader.h>
#include <vector>
#include <complex>

// Forward declarations
template<typename T> class CustomOversampler;
class TPSafe;

enum class NormMode { Peak, Energy, DCUnity };

struct MagPlot
{
    std::vector<float> freqHz;
    std::vector<float> magDb;
};

struct AuditionHarness
{
    double sampleRate = 48000.0;
    int blockSize = 512;
    
    // External dependencies (injected)
    CustomOversampler<float>* os = nullptr;
    TPSafe* tpSafe = nullptr;
    
    // Generated buffers
    juce::AudioBuffer<float> impulse;
    juce::AudioBuffer<float> step;
    juce::AudioBuffer<float> irOut;
    juce::AudioBuffer<float> stepOut;
    
    // Results
    MagPlot baselineMag;
    MagPlot candidateMag;
    MagPlot deltaMag;
    
    void prepare(double sr, int bs);
    void runThroughPath(const juce::AudioBuffer<float>& in, juce::AudioBuffer<float>& out);
    void computeTimeDomain();
    MagPlot computeMagnitude(const float* ir, int n, double sr, bool smooth1_24 = false);
    void normalizeIR(juce::AudioBuffer<float>& ir, NormMode mode);
    void fractionalOctaveSmooth(std::vector<float>& freqHz, std::vector<float>& magDb, float bandsPerOctave);
    
    // A/B comparison
    void computeBaseline();
    void computeCandidate();
    void computeDelta();
    
    // TP-Safe verification
    bool verifyTPSafe();
    void runTPSafeTest();
    
    // Export functions
    juce::String exportCSV(const juce::String& filename);
    juce::String copyTapArray();
};

// FFT helper functions
namespace FFTUtils
{
    void fft_real_forward(float* data, int n, std::complex<float>* output);
    void fractionalOctaveSmooth(std::vector<float>& freqHz, std::vector<float>& magDb, float bandsPerOctave);
}
