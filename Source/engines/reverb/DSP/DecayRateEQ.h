/*
====================================================================================================
 DecayRateEQ — UI for Reverb Decay-Rate Shaping (3 bands)
 ---------------------------------------------------------------------------------------------------
 Purpose
    Visual, interactive editor that shapes the *decay time multiplier* versus frequency for the
    reverb tail. Each band scales decay locally (Bell) or with a tilt bias (TiltLo / TiltHi).
    The y-axis is multiplier (0.5× … 2.0×), with 1.0× as the neutral line.

 Public API (hosted component)
    - setSampleRate(double) / pushBlock(L,R,n) / pushBlockPre(L,R,n) : drive the spectrum analyzer.
    - pause() / resume()                                           : analyzer control.
    (Note: The EQ curves shown here are *visual only*; the DSP must read the same APVTS values.)

 Parameters & Wiring (APVTS)
    Decay Band (per-band, X = 0..2):
      • db_active_X     : bool (0/1) — UI treats "inactive" as an empty slot for allocation.
      • db_freqHz_X     : float 20–20k
      • db_q_X          : float 0.1–36
      • db_decayMult_X  : float 0.5–2.0 (1.0 = neutral)
      • db_dynAmt_X     : float 0–100 (free field; NOT used by this UI unless you choose to)
    NOTE: There is no persisted "type" (Bell/TiltLo/TiltHi) parameter in DecayBand. The UI maintains
          `DecayBandPoint::type` in memory. If you want persistence, either:
          (a) add DecayBand::type to ReverbEQParamIDs, or
          (b) agree on using db_dynAmt_X as a storage proxy (documented contract).

 Data Model (UI)
    struct DecayBandPoint {
        float hz, mult, q;
        int   type;    // 0=Bell, 1=TiltLo, 2=TiltHi (UI-only unless persisted)
        int   bandIdx; // APVTS slot index (0..2), -1 if not bound yet
    }
    • `bandIdx` maps the on-screen point to a concrete APVTS band (db_*_bandIdx).

 Coordinate Mapping
    • X (freq): logarithmic 20 Hz → 20 kHz.
    • Y (mult): top=2.0×, center=~1.0×, bottom=0.5×.
    • zoomState is prepared around a 0.5–2.0× visual range; 1.0× is drawn as the "zero line".

 Interactions
    • Click empty space        → add a band (Bell in mid, TiltLo <~50 Hz, TiltHi >~10 kHz), allocates a
                                 free APVTS slot and writes initial values.
    • Click point              → select; shows floating BandOverlay (MULT / Q / FREQ / TYPE).
    • Drag point               → updates freq & multiplier in APVTS.
    • Mouse wheel (on point)   → adjusts Q multiplicatively (with Shift for fine).
    • Double-click point       → removes it and clears that APVTS slot (db_active = 0).
    • Hover                    → vertical guide + a faint "ghost" preview of what adding a band *here*
                                 would look like (Bell/Tilt decided by region).

 Rendering
    • Uses SpectrumAnalyzer background; draws grid/ticks (Hz & multipliers), 1.0× line emphasized.
    • Renders combined response path (thicker) and per-band paths (thin, colored).
    • Anti-aliasing white-corner fix: fill full rect first, then rounded rect.

 Theme & LNF
    • Colors are read from FieldLNF (eq* ColourIds). The component observes FieldLNF via a
      ChangeListener and repaints on theme changes.

 Limits & Constants
    • kMaxBands = 3 (UI and allocator enforce 3 active bands max).
    • Ghost preview fade timing ~220 ms of mouse idle.

 Integration Tips
    • On plugin/editor open, if you want points to auto-appear from existing APVTS values, provide a
      small "syncFromParameters()" that scans 0..2 slots and pushes DecayBandPoint structs into
      `points`. Otherwise the user will add bands explicitly.
    • Ensure DSP reads the same APVTS ids (db_*_X) so visual curves match the sound.

 Known Considerations
    • "Type" persistence: add a DecayBand::type parameter if you want the TYPE combo to survive
      session reloads. Using db_dynAmt as a proxy is possible but should be formalized.
    • This is a UI-only envelope; performance-oriented enough (path sampled at ~pixel density).
====================================================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "shared/ui/Engines/SpectrumAnalyzer.h"
#include "core/params/ParamIDs.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/ui/Controls/ZoomState.h"

class MyPluginAudioProcessor; // fwd

// Band point structure for Decay-Rate EQ visualization
struct DecayBandPoint
{
    float hz   = 1000.f;
    float mult = 1.0f;   // Decay multiplier (0.5x - 2.0x)
    float q    = 0.707f;
    int   type = 0;      // 0=Bell, 1=TiltLo, 2=TiltHi
    int   bandIdx = -1;
};

// Decay-Rate EQ for reverb decay shaping (3 bands)
class DecayRateEQ
    : public juce::Component
    , private juce::Timer
    , private juce::ChangeListener
{
public:
    JUCE_LEAK_DETECTOR (DecayRateEQ)

    explicit DecayRateEQ (MyPluginAudioProcessor& p);
    ~DecayRateEQ() override;

    // juce::Component
    void paint (juce::Graphics& g) override;
    void resized() override;
    void lookAndFeelChanged() override;
    void parentHierarchyChanged() override;
    void visibilityChanged() override;

    // Analyzer control (pass-through)
    void setSampleRate (double sr)           { analyzer.setSampleRate (sr); }
    void pause()                             { analyzer.pauseAudio(); }
    void resume()                            { analyzer.resumeAudio(); }
    void pushBlock (const float* L, const float* R, int n)    { analyzer.pushBlock (L, R, n); }
    void pushBlockPre (const float* L, const float* R, int n) { analyzer.pushBlockPre (L, R, n); }

private:
    // Timer
    void timerCallback() override;

    // Theme change listening
    void changeListenerCallback (juce::ChangeBroadcaster* src) override;

    // Mouse / keyboard interaction
    void mouseDown      (const juce::MouseEvent& e) override;
    void mouseDrag      (const juce::MouseEvent& e) override;
    void mouseUp        (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;
    void mouseMove      (const juce::MouseEvent& e) override;
    void mouseExit      (const juce::MouseEvent& e) override;
    bool keyPressed     (const juce::KeyPress& key) override;

    // Mapping helpers
    float mapHzToX     (float hz) const;
    float mapMultToY   (float mult) const;
    float mapXToHz     (int px) const;
    float mapYToMult   (int py) const;

    // Hit testing
    int   hitTestPoint (juce::Point<int> p) const;

    // Band management
    int   allocateBandSlot();
    void  setBandParam (int bandIdx, const char* baseId, float value);
    float getBandParamFloat (int bandIdx, const char* baseId, float fallback) const;

    // Visual helpers
    juce::Colour bandColourFor (int bandIdx) const;
    float bandMultAtForPaint (const DecayBandPoint& b, float hz) const;

    // Curves + units
    void rebuildEqPath();
    void drawUnits (juce::Graphics& g);

    // Floating editors
    void positionOverlay();
    void positionBadgeFor (int idx);

    // Floating band editor overlay
    class BandOverlay : public juce::Component
    {
    public:
        std::function<void (float)> onMultChanged;
        std::function<void (float)> onQChanged;
        std::function<void (float)> onFreqChanged;
        std::function<void (int)>   onTypeChanged;
        std::function<void (bool)>  onDragAny;

        BandOverlay();
        ~BandOverlay() override;

        void paint    (juce::Graphics& g) override;
        void resized  () override;

        void setValues (float mult, float q, float freq, int type);

    private:
        juce::Slider mult, q, freq;
        juce::Label  multLabel, qLabel, freqLabel;
        juce::ComboBox typeCb;
        juce::Label  typeLabel;
        bool updating = false;
    };

    // Compact per-band badge
    class BandBadge : public juce::Component
    {
    public:
        std::function<void()>        onDelete;
        std::function<void (bool)>   onBypass;
        std::function<void (int)>    onSetType;
        std::function<void (float)>  onSetFreq;
        std::function<void (float)>  onSetQ;
        std::function<void (float)>  onSetMult;

        BandBadge();
        ~BandBadge() override;

        void paint    (juce::Graphics& g) override;
        void resized  () override;

        void setValues (float mult, float freq, int type, bool bypass);
        void setDetails (float q, float mult, bool dynOn, bool dynUp,
                         float dynRange, bool specOn, const juce::String& channel,
                         int slopeDb, const juce::String& tap);

    private:
        juce::TextButton deleteBtn, bypassBtn, typeBtn;
        juce::Label freqLabel, multLabel, qLabel;
        juce::Label grLabel, dynLabel, specLabel;

        float currentMult = 1.0f;
        float currentFreq = 1000.0f;
        float currentQ    = 0.707f;
        int   currentType = 0;
        bool  currentBypass = false;
    };

    // Per-band module instances
    BandOverlay overlay;
    BandBadge   badge;

    // State
    MyPluginAudioProcessor& proc;
    SpectrumAnalyzer analyzer;
    ZoomState zoomState;

    std::vector<DecayBandPoint> points;
    int   selected     = -1;
    int   hover        = -1;
    bool  hoverInPane  = false;
    juce::Point<int> hoverPos { 0, 0 };
    float hoverHz      = 0.0f;
    juce::int64 lastMouseMoveMs = 0;
    int   ghostDelayMs = 220;
    int   badgeFor     = -1;

    juce::Path eqPath;
    std::vector<juce::Path> bandPaths;

    bool dragging = false;

    // Theme change listening
    FieldLNF* listeningTo = nullptr;

    // Consts
    static constexpr int kMaxBands = 3;
    static juce::String bandId (const char* base, int idx) { return juce::String (base) + "_" + juce::String (idx); }
};