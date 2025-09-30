#pragma once
#include <JuceHeader.h>
#include "shared/ui/Engines/SpectrumAnalyzer.h"
#include "ReverbEQParamIDs.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/ui/Controls/ZoomState.h"

class MyPluginAudioProcessor; // fwd

// Decay Rate EQ for reverb (3 bands with decay multipliers)
class DecayRateEQ : public juce::Component, private juce::Timer
{
public:
    DecayRateEQ(MyPluginAudioProcessor& p, juce::LookAndFeel* lnf);
    ~DecayRateEQ() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Analyzer control
    void setSampleRate(double sr) { analyzer.setSampleRate(sr); }
    void pause() { analyzer.pauseAudio(); }
    void resume() { analyzer.resumeAudio(); }
    void pushBlock(const float* L, const float* R, int n) { analyzer.pushBlock(L, R, n); }
    void pushBlockPre(const float* L, const float* R, int n) { analyzer.pushBlockPre(L, R, n); }

private:
    void timerCallback() override;
    void rebuildDecayPath();
    void drawUnits(juce::Graphics& g);
    
    // Mouse interaction
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    
    // Mapping helpers
    float mapHzToX(float hz) const;
    float mapMultToY(float mult) const;
    float mapXToHz(int px) const;
    float mapYToMult(int py) const;
    
    // Hit testing
    int hitTestPoint(juce::Point<int> p) const;
    
    // Band management
    int allocateBandSlot();
    void setBandParam(int bandIdx, const char* baseId, float value);
    float getBandParamFloat(int bandIdx, const char* baseId, float fallback) const;
    
    // Visual helpers
    juce::Colour bandColourFor(int bandIdx) const;
    float decayMultAtForPaint(const DecayPoint& b, float hz) const;
    
    struct DecayPoint 
    { 
        float hz = 1000.f; 
        float mult = 1.0f; 
        float q = 0.707f; 
        float dynAmt = 0.0f;
        int bandIdx = -1; 
    };
    
    MyPluginAudioProcessor& proc;
    SpectrumAnalyzer analyzer;
    ZoomState zoomState;
    
    std::vector<DecayPoint> points;
    int selected = -1;
    int hover = -1;
    bool hoverInPane = false;
    juce::Point<int> hoverPos{0, 0};
    float hoverHz = 0.0f;
    
    juce::Path decayPath;
    std::vector<juce::Path> bandPaths;
    
    // Drag state
    bool dragging = false;
    juce::Point<int> dragStart;
    
    static constexpr int kMaxBands = 3;
    static juce::String bandId(const char* base, int idx) { return juce::String(base) + "_" + juce::String(idx); }
};
