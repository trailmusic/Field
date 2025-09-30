#pragma once
#include <JuceHeader.h>
#include "DuckingFloat.h"

class MyPluginAudioProcessor; // fwd

class ReverbGraphics : public juce::Component, public juce::Timer
{
public:
    enum class ViewMode
    {
        Rays,
        Waterfall,
        Spectral
    };

    ReverbGraphics (MyPluginAudioProcessor& p,
                    juce::AudioProcessorValueTreeState& s,
                    std::function<float()> getEr,
                    std::function<float()> getTail,
                    std::function<float()> getDuckDb,
                    std::function<float()> getWidthNow);

    void resized() override;
    void paint(juce::Graphics& g) override;
    
    // View mode control
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const { return currentViewMode; }
    
    // Ducking float access
    DuckingFloat* getDuckingFloat() { return duckingFloat.get(); }
    
    // EQ access (temporarily disabled)
    // ReverbEQ* getReverbEQ() { return reverbEQ.get(); }
    // DecayRateEQ* getDecayRateEQ() { return decayRateEQ.get(); }
    
    // Analyzer control
    void setSampleRate(double sr);
    void pause();
    void resume();
    void pushBlock(const float* L, const float* R, int n);
    void pushBlockPre(const float* L, const float* R, int n);
    
    // Timer callback for animation
    void timerCallback() override;

private:
    void setupViewModeButtons();
    void paintRays(juce::Graphics& g);
    void paintWaterfall(juce::Graphics& g);
    void paintSpectral(juce::Graphics& g);
    void paintGrOverlay(juce::Graphics& g);
    
    MyPluginAudioProcessor& proc;
    juce::AudioProcessorValueTreeState& state;
    
    // View mode controls
    juce::TextButton raysButton, waterfallButton, spectralButton;
    ViewMode currentViewMode = ViewMode::Rays;
    
    // Ducking float
    std::unique_ptr<DuckingFloat> duckingFloat;
    
    // EQ panels (temporarily disabled)
    // std::unique_ptr<ReverbEQ> reverbEQ;
    // std::unique_ptr<DecayRateEQ> decayRateEQ;
    
    // Callback functions
    std::function<float()> getErRms, getTailRms, getDuckGrDb, getWidthNow;
    
    // Animation
    float animationTime = 0.0f;
    static constexpr float ANIMATION_SPEED = 0.02f;
};


