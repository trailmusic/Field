#pragma once
#include <JuceHeader.h>
#include "shared/ui/Engines/SpectrumAnalyzer.h"
#include "core/params/ParamIDs.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/ui/Controls/ZoomState.h"

class MyPluginAudioProcessor; // fwd

// ─────────────────────────────────────────────────────────────────────────────
// DEV NOTES
// - This component is a lightweight, Pro-Q–style editor for the reverb's tone
//   EQ (4 bands). It draws combined/per-band curves, band handles, grid/units,
//   a compact badge, and a floating overlay editor for the selected band.
// - Host/state coupling: band params live in APVTS (see ReverbEQParamIDs.h).
// - Visual-only preview math is intentionally approximate; DSP lives elsewhere.
// - Keep interaction snappy: mouse operations update APVTS immediately.
// ─────────────────────────────────────────────────────────────────────────────

// Band point structure for EQ visualization / interaction
struct BandPoint
{
    float hz      = 1000.0f;
    float db      = 0.0f;
    float q       = 0.707f;
    int   type    = 0;    // 0=Bell, 1=LowShelf, 2=HighShelf (matches overlay)
    int   phase   = 1;    // 0=Zero,1=Natural,2=Linear
    int   bandIdx = -1;   // APVTS band slot index
    float dynAmt  = 0.0f; // Reserved for future (0–100%)
};

// Stripped Pro-Q style EQ for reverb tone shaping (4 bands)
class ReverbToneEQ final : public juce::Component,
                           private juce::Timer,
                           private juce::ChangeListener
{
public:
    explicit ReverbToneEQ (MyPluginAudioProcessor& p);
    ~ReverbToneEQ() override;

    // juce::Component
    void paint (juce::Graphics& g) override;
    void resized() override;
    void lookAndFeelChanged() override;
    void parentHierarchyChanged() override;
    void visibilityChanged() override;

    // Analyzer control
    void setSampleRate (double sr)                    { analyzer.setSampleRate (sr); }
    void pause()                                      { analyzer.pauseAudio(); }
    void resume()                                     { analyzer.resumeAudio(); }
    void pushBlock   (const float* L, const float* R, int n) { analyzer.pushBlock   (L, R, n); }
    void pushBlockPre(const float* L, const float* R, int n) { analyzer.pushBlockPre(L, R, n); }

private:
    // Timer
    void timerCallback() override;

    // ChangeListener
    void changeListenerCallback (juce::ChangeBroadcaster* src) override;

    // Mouse interaction
    void mouseDown       (const juce::MouseEvent& e) override;
    void mouseDrag       (const juce::MouseEvent& e) override;
    void mouseUp         (const juce::MouseEvent& e) override;
    void mouseWheelMove  (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseMove       (const juce::MouseEvent& e) override;
    void mouseExit       (const juce::MouseEvent& e) override;

    // Drawing & geometry
    void rebuildEqPath();
    void drawUnits (juce::Graphics& g);

    // Mapping helpers
    float mapHzToX (float hz)  const;
    float mapDbToY (float dB)  const;
    float mapXToHz (int   px)  const;
    float mapYToDb (int   py)  const;

    // Hit testing
    int   hitTestPoint (juce::Point<int> p) const;

    // Band management (APVTS coupling)
    int   allocateBandSlot();
    void  setBandParam      (int bandIdx, const char* baseId, float value);
    float getBandParamFloat (int bandIdx, const char* baseId, float fallback) const;

    // Visual helpers
    juce::Colour bandColourFor (int bandIdx) const;
    float        bandDbAtForPaint (const BandPoint& b, float hz) const;

    // Per-band modules (overlay editor + compact badge)
    void positionOverlay();
    void positionBadgeFor (int idx);

    // ─────────────────────────────────────────────────────────────────────────
    // Floating band editor overlay
    class BandOverlay : public juce::Component
    {
    public:
        std::function<void(float)> onGainChanged;
        std::function<void(float)> onQChanged;
        std::function<void(float)> onFreqChanged;
        std::function<void(int)>   onTypeChanged;
        std::function<void(bool)>  onDragAny;

        BandOverlay();
        ~BandOverlay() override = default;

        void paint   (juce::Graphics& g) override;
        void resized() override;

        void setValues (float gain, float q, float freq, int type);

    private:
        juce::Slider  gain, q, freq;
        juce::Label   gainLabel, qLabel, freqLabel;
        juce::ComboBox typeCb;
        juce::Label    typeLabel;

        bool updating = false;
    };

    // Compact per-band badge
    class BandBadge : public juce::Component
    {
    public:
        std::function<void()>        onDelete;
        std::function<void(bool)>    onBypass;
        std::function<void(int)>     onSetType;
        std::function<void(float)>   onSetFreq;
        std::function<void(float)>   onSetQ;
        std::function<void(float)>   onSetGain;

        BandBadge();
        ~BandBadge() override = default;

        void paint   (juce::Graphics& g) override;
        void resized() override;

        void setValues  (float gr, float freq, int type, bool bypass);
        void setDetails (float q, float gain, bool dynOn, bool dynUp, float dynRange,
                         bool specOn, const juce::String& channel, int slopeDb, const juce::String& tap);

    private:
        juce::TextButton deleteBtn, bypassBtn, typeBtn;
        juce::Label      freqLabel,  gainLabel,   qLabel;
        juce::Label      grLabel,    dynLabel,    specLabel;

        float currentGr    = 0.0f;
        float currentFreq  = 1000.0f;
        float currentGain  = 0.0f;
        float currentQ     = 0.707f;
        int   currentType  = 0;
        bool  currentBypass= false;
    };

    // ─────────────────────────────────────────────────────────────────────────
    // Members

    MyPluginAudioProcessor& proc;
    SpectrumAnalyzer        analyzer;
    ZoomState               zoomState;

    std::vector<BandPoint>  points;
    int                     selected         = -1;
    int                     hover            = -1;
    bool                    hoverInPane      = false;
    juce::Point<int>        hoverPos         { 0, 0 };
    float                   hoverHz          = 0.0f;
    juce::int64             lastMouseMoveMs  = 0;
    int                     ghostDelayMs     = 220;
    int                     badgeFor         = -1;

    juce::Path              eqPath;
    std::vector<juce::Path> bandPaths;

    // Theme change listening
    FieldLNF*               listeningTo      = nullptr;

    // UI helpers
    BandOverlay             overlay;
    BandBadge               badge;

    // Constants
    static constexpr int   kMaxBands    = 4;
    static constexpr float kMinHz       = 20.0f;
    static constexpr float kMaxHz       = 20000.0f;
    static constexpr float kMinGainDb   = -24.0f;
    static constexpr float kMaxGainDb   = +24.0f;
    static constexpr float kMinQ        = 0.1f;
    static constexpr float kMaxQ        = 36.0f;
    static constexpr float kHandleRadius= 12.0f;

    static juce::String bandId (const char* base, int idx) { return juce::String (base) + "_" + juce::String (idx); }
};