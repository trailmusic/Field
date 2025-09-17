#pragma once
#include <JuceHeader.h>

struct ReverbParams;

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

    std::atomic<float> duckGrDb { 0.f }, erRms { 0.f }, tailRms { 0.f };
};

// Lightweight parameter bundle for the engine (filled from APVTS in processor)
struct ReverbParams
{
    float preDelayMs{}, decaySec{}, density{}, diffusion{}, modDepthCents{}, modRateHz{};
    float erLevelDb{}, erTimeMs{}, erDensity{}, erWidthPct{}, erToTailPct{};
    float hpfHz{}, lpfHz{}, tiltDb{};
    float dreqLowX{}, dreqMidX{}, dreqHighX{};
    float widthPct{}, widthStartPct{}, widthEndPct{}, widthCurve{};
    float rotStartDeg{}, rotEndDeg{}, rotCurve{};
    int   duckMode{}; float duckDepthDb{}, duckThrDb{}, duckKneeDb{}, duckRatio{};
    float duckAtkMs{}, duckRelMs{}, duckLaMs{}, duckRmsMs{}, duckBandHz{}, duckBandQ{};
    bool  freeze{}; float gateAmtPct{}, shimmerAmtPct{}; int shimmerIntervalMode{};
    bool  eqOn{}; float eqMixPct{};
    float eqLowHz{}, eqLowGainDb{}, eqLowQ{};
    float eqMidHz{}, eqMidGainDb{}, eqMidQ{};
    float eqHighHz{}, eqHighGainDb{}, eqHighQ{};
};


