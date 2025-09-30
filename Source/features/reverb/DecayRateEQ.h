#pragma once
#include <JuceHeader.h>
#include "shared/ui/Engines/SpectrumAnalyzer.h"
#include "ReverbEQParamIDs.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/ui/Controls/ZoomState.h"

class MyPluginAudioProcessor; // fwd

// Band point structure for Decay-Rate EQ visualization
struct DecayBandPoint 
{ 
    float hz = 1000.f; 
    float mult = 1.0f; // Decay multiplier (0.5x - 2.0x)
    float q = 0.707f; 
    int type = 0; // 0=Bell, 1=TiltLo, 2=TiltHi
    int bandIdx = -1; 
};

// Decay-Rate EQ for reverb decay shaping (3 bands)
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
    void rebuildEqPath();
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
    float bandMultAtForPaint(const DecayBandPoint& b, float hz) const;
    
    // Per-band modules (based on Dynamic EQ pattern)
    void positionOverlay();
    void positionBadgeFor(int idx);
    
    // Floating band editor overlay
    class BandOverlay : public juce::Component
    {
    public:
        std::function<void(float)> onMultChanged;
        std::function<void(float)> onQChanged;
        std::function<void(float)> onFreqChanged;
        std::function<void(int)> onTypeChanged;
        std::function<void(bool)> onDragAny;
        
        BandOverlay();
        ~BandOverlay() override;
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        
        void setValues(float mult, float q, float freq, int type);
        void setAccentColour(juce::Colour c);
        
    private:
        juce::Slider mult, q, freq;
        juce::Label multLabel, qLabel, freqLabel;
        juce::ComboBox typeCb;
        juce::Label typeLabel;
        
        bool updating = false;
        juce::Colour accentColour = juce::Colours::orange;
    };
    
    // Compact per-band badge
    class BandBadge : public juce::Component
    {
    public:
        std::function<void()> onDelete;
        std::function<void(bool)> onBypass;
        std::function<void(int)> onSetType;
        std::function<void(float)> onSetFreq;
        std::function<void(float)> onSetQ;
        std::function<void(float)> onSetMult;
        
        BandBadge();
        ~BandBadge() override;
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        
        void setValues(float mult, float freq, int type, bool bypass);
        void setDetails(float q, float mult, bool dynOn, bool dynUp, float dynRange, bool specOn, const juce::String& channel, int slopeDb, const juce::String& tap);
        void setAccentColour(juce::Colour c);
        
    private:
        juce::TextButton deleteBtn, bypassBtn, typeBtn;
        juce::Label freqLabel, multLabel, qLabel;
        juce::Label grLabel, dynLabel, specLabel;
        
        juce::Colour accentColour = juce::Colours::orange;
        float currentMult = 1.0f;
        float currentFreq = 1000.0f;
        float currentQ = 0.707f;
        int currentType = 0;
        bool currentBypass = false;
    };
    
    // Per-band module instances
    BandOverlay overlay;
    BandBadge badge;
    
    
    MyPluginAudioProcessor& proc;
    SpectrumAnalyzer analyzer;
    ZoomState zoomState;
    
    std::vector<DecayBandPoint> points;
    int selected = -1;
    int hover = -1;
    bool hoverInPane = false;
    juce::Point<int> hoverPos{0, 0};
    float hoverHz = 0.0f;
    int badgeFor = -1;
    
    juce::Path eqPath;
    std::vector<juce::Path> bandPaths;
    
    // Drag state
    bool dragging = false;
    juce::Point<int> dragStart;
    
    static constexpr int kMaxBands = 3;
    static juce::String bandId(const char* base, int idx) { return juce::String(base) + "_" + juce::String(idx); }
};