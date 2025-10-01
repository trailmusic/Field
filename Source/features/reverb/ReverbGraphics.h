#pragma once
#include <JuceHeader.h>
#include "DuckingFloat.h"
#include "ReverbEQ.h"
#include "DecayRateEQ.h"

class MyPluginAudioProcessor; // fwd
class ReverbToneEQ; // fwd
class DecayRateEQ; // fwd

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
    
    // Theme color updates
    void lookAndFeelChanged() override;
    void updateLabelColors();
    
    // Ducking float access
    DuckingFloat* getDuckingFloat() { return duckingFloat.get(); }
    
        // EQ access
        ReverbToneEQ* getReverbEQ() { return reverbEQ.get(); }
        DecayRateEQ* getDecayRateEQ() { return decayRateEQ.get(); }
    
    // Analyzer control
    void setSampleRate(double sr);
    void pause();
    void resume();
    void pushBlock(const float* L, const float* R, int n);
    void pushBlockPre(const float* L, const float* R, int n);
    
    // Timer callback for animation
    void timerCallback() override;
    
    // Ducking module control
    void updateDuckingModuleVisibility();
    
    // Visualization control panel setup
    void setupVisualizationControlPanel();
    
    // EQ labels setup
    void setupEQLabels();

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
    
    // Visualization control container
    class VisualizationControlPanel : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override;
    };
    
    VisualizationControlPanel visualizationControlPanel;
    
    // EQ labels for visual separation
    juce::Label toneEqLabel, decayRateEqLabel, duckingLabel, visualizationLabel;
    
    // Ducking float
    std::unique_ptr<DuckingFloat> duckingFloat;
    
        // EQ panels
        std::unique_ptr<ReverbToneEQ> reverbEQ;
        std::unique_ptr<DecayRateEQ> decayRateEQ;
    
    // Callback functions
    std::function<float()> getErRms, getTailRms, getDuckGrDb, getWidthNow;
    
    // Animation
    float animationTime = 0.0f;
    static constexpr float ANIMATION_SPEED = 0.02f;
};


