#pragma once
#include <JuceHeader.h>

// Top-pane visualization for the Reverb engine.
// Non-interactive; reads APVTS + a few live meters.
class ReverbCanvasComponent : public juce::Component, private juce::Timer
{
public:
    ReverbCanvasComponent(juce::AudioProcessorValueTreeState& s,
                          std::function<float()> erLevelNow,
                          std::function<float()> tailLevelNow,
                          std::function<float()> duckGrNow,
                          std::function<float()> widthNow,
                          std::function<double()> sampleRateNow = []{ return 48000.0; });

    void paint(juce::Graphics&) override;
    // Optional: provider for per-band DynEQ GR in dB (size<=4)
    void setDynEqGrProvider (std::function<std::array<float,4>()> provider) { dyneqGrNow = std::move(provider); }

private:
    juce::AudioProcessorValueTreeState& state;
    std::function<float()> erNow, tailNow, grNow, widthNowFn;
    std::function<double()> srNow;

    // Offscreen heatmap for tail (frequency x time)
    juce::Image heatmap { juce::Image::ARGB, 128, 72, true };
    int writeRow = 0;
    float tSeconds = 0.f;
    float horizonSec = 3.f;
    float phase = 0.f; // for subtle modulation shimmer

    // === paint pipeline ===
    void timerCallback() override;
    void visibilityChanged() override { if (isVisible()) startTimerHz(30); else stopTimer(); }
    void drawBackground(juce::Graphics&, juce::Rectangle<float> r);
    void drawER(juce::Graphics&, juce::Rectangle<float> r);
    void drawTailHeatmapImage(juce::Graphics&, juce::Rectangle<float> r);
    void drawWidthRotation(juce::Graphics&, juce::Rectangle<float> r);
    void drawToneCurtainAndEQ(juce::Graphics&, juce::Rectangle<float> r);
    void drawDucking(juce::Graphics&, juce::Rectangle<float> r);
    void drawSpecials(juce::Graphics&, juce::Rectangle<float> r);
    void drawDynEqOverlays(juce::Graphics&, juce::Rectangle<float> r);

    // === heatmap writer ===
    void advanceHeatmapRow();

    // === small utils ===
    float getF(const juce::String& id, float fallback = 0.f) const;
    float getF2(const juce::String& idA, const juce::String& idB, float fb = 0.f) const;
    int   getI(const juce::String& id, int fallback = 0) const;
    bool  getB(const juce::String& id, bool fallback = false) const;

    static float clamp01(float v) { return juce::jlimit(0.f, 1.f, v); }
    static float lerp(float a, float b, float t) { return a + (b - a) * t; }
    static float log01FromHz(float hz)
    {
        const float a = std::log10(20.f), b = std::log10(20000.f);
        return juce::jlimit(0.f, 1.f, (std::log10(juce::jlimit(20.f, 20000.f, hz)) - a) / (b - a));
    }
    static float hzFromLog01(float n01)
    {
        const float a = std::log10(20.f), b = std::log10(20000.f);
        return std::pow(10.f, juce::jlimit(a, b, juce::jmap(n01, 0.f, 1.f, a, b)));
    }

    // RBJ utilities for wet EQ magnitude
    struct RBJ
    {
        static void lowShelf(double fs, double f0, double Q, double gainDb, double& b0,double& b1,double& b2,double& a0,double& a1,double& a2);
        static void peaking (double fs, double f0, double Q, double gainDb, double& b0,double& b1,double& b2,double& a0,double& a1,double& a2);
        static void highShelf(double fs, double f0, double Q, double gainDb, double& b0,double& b1,double& b2,double& a0,double& a1,double& a2);
        static double magAt (double b0,double b1,double b2,double a0,double a1,double a2, double omega);
    };

    // Optional DynEQ GR callback
    std::function<std::array<float,4>()> dyneqGrNow;
};


