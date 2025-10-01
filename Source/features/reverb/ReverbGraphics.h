#pragma once
#include <JuceHeader.h>
#include "DuckingFloat.h"
#include "ReverbEQ.h"
#include "DecayRateEQ.h"
#include "BandIdFinder.h"
#include "BandCounter.h"
#include "ReverbVisuals.h"
#include "shared/ui/Utilities/SafetySentinels.h"

class MyPluginAudioProcessor; // fwd
class ReverbToneEQ; // fwd
class DecayRateEQ; // fwd

class ReverbGraphics : public juce::Component, public juce::Timer
{
    JUCE_LEAK_DETECTOR(ReverbGraphics)
    
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

    ~ReverbGraphics() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void visibilityChanged() override;
    
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
    
    // Band indicators now update automatically via BandCounter listeners
    void updateBandIndicatorsManually();
    
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
    
    // Visualization paint methods with bounds
    void paintRaysInBounds(juce::Graphics& g, juce::Rectangle<float> bounds);
    void paintWaterfallInBounds(juce::Graphics& g, juce::Rectangle<float> bounds);
    void paintSpectralInBounds(juce::Graphics& g, juce::Rectangle<float> bounds);
    
    MyPluginAudioProcessor& proc;
    juce::AudioProcessorValueTreeState& state;
    
    // View mode controls
    juce::TextButton raysButton, waterfallButton, spectralButton;
    ViewMode currentViewMode = ViewMode::Rays;
    
    
    // Visualization component
    std::unique_ptr<ReverbVisuals> reverbVisuals;
    
    // Visualization control container
    class VisualizationControlPanel : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override;
    };
    
    VisualizationControlPanel visualizationControlPanel;
    
    // Band indicator component for showing EQ band usage
    class BandIndicator : public juce::Component
    {
    public:
        BandIndicator(int maxBands);
        void paint(juce::Graphics& g) override;
        void setActiveBands(int count);
        void setMaxBands(int max);
        
    private:
        int maxBands = 0;
        int activeBands = 0;
        static constexpr float circleSize = 8.0f;
        static constexpr float circleSpacing = 12.0f;
    };
    
    // EQ labels for visual separation
    juce::Label toneEqLabel, decayRateEqLabel, duckingLabel, visualizationLabel;
    
    // Band indicators for showing EQ usage
    BandIndicator toneEqIndicator, decayRateEqIndicator;
    
    // Band counters for reliable parameter detection
    std::unique_ptr<BandCounter> toneCounter, decayCounter;
    juce::StringArray toneEnabledIds, decayEnabledIds;
    
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
    
    // Debug safety sentinel (temporarily disabled for compilation)
    // TimerSentinel timerSentinel;
};


