#pragma once
#include <JuceHeader.h>

struct ReverbParams;

// ER Filter struct for reverb processing
struct ERFilter
{
    double b0{}, b1{}, b2{}, a0{1.0}, a1{}, a2{};
    std::vector<float> z1, z2; // per-channel states
    void resize (int channels) { z1.assign (channels, 0.f); z2.assign (channels, 0.f); }
    static ERFilter makePeaking (double fs, double f0, double Q, double gainDb)
    {
        ERFilter q;
        const double A = std::pow (10.0, gainDb / 40.0);
        const double w0 = 2.0 * juce::MathConstants<double>::pi * f0 / fs;
        const double cw = std::cos (w0), sw = std::sin (w0);
        const double alpha = sw / (2.0 * Q);
        q.b0 = 1 + alpha * A;
        q.b1 = -2 * cw;
        q.b2 = 1 - alpha * A;
        q.a0 = 1 + alpha / A;
        q.a1 = -2 * cw;
        q.a2 = 1 - alpha / A;
        return q;
    }
    void processInPlace (juce::AudioBuffer<float>& buf)
    {
        const int C = buf.getNumChannels();
        const int N = buf.getNumSamples();
        if ((int) z1.size() != C) resize (C);
        const float fb0 = (float) (b0 / a0);
        const float fb1 = (float) (b1 / a0);
        const float fb2 = (float) (b2 / a0);
        const float fa1 = (float) (a1 / a0);
        const float fa2 = (float) (a2 / a0);
        for (int c = 0; c < C; ++c)
        {
            float* d = buf.getWritePointer (c);
            float z1c = z1[c], z2c = z2[c];
            for (int i = 0; i < N; ++i)
            {
                const float x = d[i];
                const float y = fb0 * x + z1c;
                z1c = fb1 * x - fa1 * y + z2c;
                z2c = fb2 * x - fa2 * y;
                d[i] = y;
            }
            z1[c] = z1c; z2[c] = z2c;
        }
    }
    
    float processInPlace(float input, int channel)
    {
        if (channel >= static_cast<int>(z1.size())) return input;
        
        const float fb0 = (float) (b0 / a0);
        const float fb1 = (float) (b1 / a0);
        const float fb2 = (float) (b2 / a0);
        const float fa1 = (float) (a1 / a0);
        const float fa2 = (float) (a2 / a0);
        
        const float x = input;
        const float y = fb0 * x + z1[channel];
        z1[channel] = fb1 * x - fa1 * y + z2[channel];
        z2[channel] = fb2 * x - fa2 * y;
        
        return y;
    }
};

class ReverbEngine
{
public:
    void prepare (double sr, int maxBlock, int channels);
    void reset ();

    void setParams (const ReverbParams& p);

    // Render 100% wet into 'wet' (size/channels already set). Sidechain is post-FX dry.
    void processWet (juce::AudioBuffer<float>& wet,
                     const juce::AudioBuffer<float>& sidechain);

    float  getCurrentDuckGrDb() const noexcept { return duckGrDb.load(); }
    float  getErRms()           const noexcept { return erRms.load(); }
    float  getTailRms()         const noexcept { return tailRms.load(); }
    double getTailSeconds()     const noexcept { return 4.0; }

private:
    double sampleRate { 48000.0 };
    int    maxSamples { 0 };
    int    chans { 2 };

    juce::AudioBuffer<float> erBuf, tailBuf, tmpBuf;
    
    // Early Reflections (ER) system
    struct EarlyReflections
    {
        static constexpr int MAX_ER_TAPS = 32;
        struct Tap
        {
            float delayMs = 0.0f;
            float gain = 0.0f;
            float pan = 0.0f; // -1.0 to 1.0
            float filterFreq = 1000.0f;
            float filterQ = 0.707f;
        };
        
        std::array<Tap, MAX_ER_TAPS> taps;
        int numTaps = 0;
        double sampleRate = 48000.0; // Store sample rate locally
        
        // Delay lines for ER taps
        std::vector<std::vector<float>> delayLines;
        std::vector<int> delayIndices;
        
        // Filters for each tap
        std::vector<ERFilter> tapFilters;
        
        void prepare(int channels, double sampleRate);
        void reset();
        void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output);
        void setParams(float erTimeMs, float erDensity, float erWidthPct, float erLevelDb);
        void generateERTaps();
    } earlyReflections;

    std::atomic<float> duckGrDb { 0.f }, erRms { 0.f }, tailRms { 0.f };
};

// Lightweight parameter bundle for the engine (filled from APVTS in processor)
struct ReverbParams
{
    float preDelayMs{}, decaySec{}, density{}, diffusion{}, modDepthCents{}, modRateHz{};
    float erLevelDb{}, erTimeMs{}, erDensity{}, erWidthPct{}, erToTailPct{};
    // Old knob-based EQ controls removed - now handled by DynEQ pane
    float dreqLowX{}, dreqMidX{}, dreqHighX{};
    float widthPct{}, widthStartPct{}, widthEndPct{}, widthCurve{};
    float rotStartDeg{}, rotEndDeg{}, rotCurve{};
    int   duckMode{}; float duckDepthDb{}, duckThrDb{}, duckKneeDb{}, duckRatio{};
    float duckAtkMs{}, duckRelMs{}, duckLaMs{}, duckRmsMs{}, duckBandHz{}, duckBandQ{};
    bool  freeze{}; float gateAmtPct{}, shimmerAmtPct{}; int shimmerIntervalMode{};
    // Old Post-EQ removed - now handled by DynEQ pane
};


