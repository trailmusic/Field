#pragma once
#include <JuceHeader.h>

class MyPluginAudioProcessor; // fwd

class ReverbVisuals : public juce::Component
{
    JUCE_LEAK_DETECTOR(ReverbVisuals)
    
public:
    enum class ViewMode
    {
        Rays,
        Waterfall,
        Spectral
    };

    ReverbVisuals(MyPluginAudioProcessor& p,
                  juce::AudioProcessorValueTreeState& s,
                  std::function<float()> getEr,
                  std::function<float()> getTail,
                  std::function<float()> getDuckDb,
                  std::function<float()> getWidthNow);

    ~ReverbVisuals() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    
    // View mode control
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const { return currentViewMode; }
    
    // Theme color updates
    void lookAndFeelChanged() override;

private:
    // Processor and state references
    MyPluginAudioProcessor& proc;
    juce::AudioProcessorValueTreeState& state;
    
    // Level getters
    std::function<float()> getErRms;
    std::function<float()> getTailRms;
    std::function<float()> getDuckGrDb;
    std::function<float()> getWidthNow;
    
    // Current view mode
    ViewMode currentViewMode = ViewMode::Rays;
    
    // Visualization state machine
    enum class VizState { Disabled, ActiveSignal, IdlePreview, Frozen };
    
    struct VizResolve {
        VizState state;
        float er;   // levels you'll feed painters with
        float tail; // …
        float alpha; // overall viz intensity (for dimming disabled/idle)
        const char* banner; // optional UI tag ("Bypassed", "Frozen", etc.)
    };
    
    // State machine resolver
    VizResolve resolveViz(float erLevel, float tailLevel,
                          bool enabledParam,
                          bool hostBypassed,
                          bool freezeParam,
                          bool allowPreview,
                          double nowMs);
    
    // State tracking
    double lastLoudMs = 0.0;
    bool allowIdlePreview = true;
    
    // Visualization painting methods
    void paintRaysInBounds(juce::Graphics& g, juce::Rectangle<float> bounds, float er, float tail);
    void paintWaterfallInBounds(juce::Graphics& g, juce::Rectangle<float> bounds, float er, float tail);
    void paintSpectralInBounds(juce::Graphics& g, juce::Rectangle<float> bounds, float er, float tail);
};
