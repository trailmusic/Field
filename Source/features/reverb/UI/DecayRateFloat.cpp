#include "DecayRateFloat.h"
#include "../DSP/ReverbParamIDs.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/ui/Utilities/ComponentGreyout.h"

/*
====================================================================================================
 DecayRateFloat.cpp — implementation notes
 ---------------------------------------------------------------------------------------------------
 • This version wires *every* UI control to APVTS using IDs from ReverbParamIDs.h.
 • Sliders are bound to:
       decayLoMult, decayHiMult, decayMidDb, decayMidFreqHz, decayMidQ, decayTiltDb, decaySmoothing, decayMode
   Ranges set here are purely for local interaction; truth comes from APVTS params. The attachment
   will push/pull values, and your pretty formatters (added earlier) are host-visible.
 • Matches DuckingFloat styling but without GR meter or selectors.
 • Uses same compact padding system as DuckingFloat.
====================================================================================================
*/

namespace
{
    // Local UI ranges (visual), independent of APVTS internal ranges.
    struct LocalRanges
    {
        static inline juce::Range<double> loMult()     { return { 0.25, 4.0 }; }
        static inline juce::Range<double> hiMult()      { return { 0.25, 4.0 }; }
        static inline juce::Range<double> midDb()      { return { -12.0, 12.0 }; }
        static inline juce::Range<double> midFreq()    { return { 20.0, 20000.0 }; }
        static inline juce::Range<double> midQ()       { return { 0.3, 6.0 }; }
        static inline juce::Range<double> tiltDb()      { return { -12.0, 12.0 }; }
        static inline juce::Range<double> smoothing()  { return { 0.0, 2.0 }; }
        static inline juce::Range<double> mode()        { return { 0.0, 1.0 }; }
    };
}

DecayRateFloat::DecayRateFloat(juce::AudioProcessorValueTreeState& apvts)
    : loMultSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , hiMultSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , midDbSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , midFreqSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , midQSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , tiltDbSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , smoothingSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , modeSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , loMultLabel("loMultLabel", "Lo Mult")
    , hiMultLabel("hiMultLabel", "Hi Mult")
    , midDbLabel("midDbLabel", "Mid Db")
    , midFreqLabel("midFreqLabel", "Mid Freq")
    , midQLabel("midQLabel", "Mid Q")
    , tiltDbLabel("tiltDbLabel", "Tilt Db")
    , smoothingLabel("smoothingLabel", "Smoothing")
    , modeLabel("modeLabel", "Mode")
    , loMultValue("loMultValue", "1.0")
    , hiMultValue("hiMultValue", "1.0")
    , midDbValue("midDbValue", "0.0")
    , midFreqValue("midFreqValue", "1000.0")
    , midQValue("midQValue", "1.0")
    , tiltDbValue("tiltDbValue", "0.0")
    , smoothingValue("smoothingValue", "0.5")
    , modeValue("modeValue", "0.0")
    , apvtsRef(apvts)
{
    setupComponents();
    setSize(360, EXPANDED_HEIGHT);
    setExpanded(true);
}

DecayRateFloat::~DecayRateFloat()
{
    // APVTS attachments auto-clean
}

void DecayRateFloat::setupComponents()
{
    using namespace ReverbParamIDs;

    // ===== Sliders =====
    addAndMakeVisible(loMultSlider);
    loMultSlider.setRange(LocalRanges::loMult(), 0.01);
    loMultSlider.setValue(1.0);
    loMultSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    loMultSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(hiMultSlider);
    hiMultSlider.setRange(LocalRanges::hiMult(), 0.01);
    hiMultSlider.setValue(1.0);
    hiMultSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    hiMultSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(midDbSlider);
    midDbSlider.setRange(LocalRanges::midDb(), 0.1);
    midDbSlider.setValue(0.0);
    midDbSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    midDbSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(midFreqSlider);
    midFreqSlider.setRange(LocalRanges::midFreq(), 1.0);
    midFreqSlider.setValue(1000.0);
    midFreqSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    midFreqSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(midQSlider);
    midQSlider.setRange(LocalRanges::midQ(), 0.01);
    midQSlider.setValue(1.0);
    midQSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    midQSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(tiltDbSlider);
    tiltDbSlider.setRange(LocalRanges::tiltDb(), 0.1);
    tiltDbSlider.setValue(0.0);
    tiltDbSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tiltDbSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(smoothingSlider);
    smoothingSlider.setRange(LocalRanges::smoothing(), 0.01);
    smoothingSlider.setValue(0.5);
    smoothingSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    smoothingSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(modeSlider);
    modeSlider.setRange(LocalRanges::mode(), 0.01);
    modeSlider.setValue(0.0);
    modeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    modeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    // ===== Labels (hidden in this design) =====
    addChildComponent(loMultLabel);
    addChildComponent(hiMultLabel);
    addChildComponent(midDbLabel);
    addChildComponent(midFreqLabel);
    addChildComponent(midQLabel);
    addChildComponent(tiltDbLabel);
    addChildComponent(smoothingLabel);
    addChildComponent(modeLabel);

    // ===== Value Labels =====
    addChildComponent(loMultValue);
    addChildComponent(hiMultValue);
    addChildComponent(midDbValue);
    addChildComponent(midFreqValue);
    addChildComponent(midQValue);
    addChildComponent(tiltDbValue);
    addChildComponent(smoothingValue);
    addChildComponent(modeValue);

    // ===== KnobCells =====
    loMultKnobCell = std::make_unique<KnobCell>(loMultSlider, loMultValue, "Lo Mult");
    hiMultKnobCell = std::make_unique<KnobCell>(hiMultSlider, hiMultValue, "Hi Mult");
    midDbKnobCell = std::make_unique<KnobCell>(midDbSlider, midDbValue, "Mid Db");
    midFreqKnobCell = std::make_unique<KnobCell>(midFreqSlider, midFreqValue, "Mid Freq");
    midQKnobCell = std::make_unique<KnobCell>(midQSlider, midQValue, "Mid Q");
    tiltDbKnobCell = std::make_unique<KnobCell>(tiltDbSlider, tiltDbValue, "Tilt Db");
    smoothingKnobCell = std::make_unique<KnobCell>(smoothingSlider, smoothingValue, "Smoothing");
    modeKnobCell = std::make_unique<KnobCell>(modeSlider, modeValue, "Mode");

    addAndMakeVisible(*loMultKnobCell);
    addAndMakeVisible(*hiMultKnobCell);
    addAndMakeVisible(*midDbKnobCell);
    addAndMakeVisible(*midFreqKnobCell);
    addAndMakeVisible(*midQKnobCell);
    addAndMakeVisible(*tiltDbKnobCell);
    addAndMakeVisible(*smoothingKnobCell);
    addAndMakeVisible(*modeKnobCell);

    // ===== APVTS Attachments =====
    loMultAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, decayLoMult, loMultSlider);
    hiMultAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, decayHiMult, hiMultSlider);
    midDbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, decayMidDb, midDbSlider);
    midFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, decayMidFreqHz, midFreqSlider);
    midQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, decayMidQ, midQSlider);
    tiltDbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, decayTiltDb, tiltDbSlider);
    smoothingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, decaySmoothing, smoothingSlider);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, decayMode, modeSlider);
}

void DecayRateFloat::resized()
{
    updateLayout();
}

void DecayRateFloat::updateLayout()
{
    auto bounds = getLocalBounds().toFloat();

    if (expanded)
    {
        // Header area with labels (36px - matching DuckingFloat's meter strip)
        auto headerArea = bounds.removeFromTop(36.0f);
        
        // Control row (28px - matching DuckingFloat's selectors row)
        auto controlRow = bounds.removeFromTop(28.0f).reduced(2.0f, 2.0f);
        
        // Two columns of 4 knobs each
        auto knobsArea = bounds.reduced(1.0f);
        auto leftCol   = knobsArea.removeFromLeft(knobsArea.getWidth() * 0.5f).reduced(1.0f);
        auto rightCol  = knobsArea.reduced(1.0f);

        auto layColumn = [](juce::Component& a, juce::Component& b, juce::Component& c, juce::Component& d, juce::Rectangle<float> area)
        {
            const float cellH = area.getHeight() / 4.0f;
            a.setBounds(area.removeFromTop(cellH).toNearestInt());
            b.setBounds(area.removeFromTop(cellH).toNearestInt());
            c.setBounds(area.removeFromTop(cellH).toNearestInt());
            d.setBounds(area.removeFromTop(cellH).toNearestInt());
        };

        layColumn(*loMultKnobCell, *hiMultKnobCell, *midDbKnobCell, *midFreqKnobCell, leftCol);
        layColumn(*midQKnobCell, *tiltDbKnobCell, *smoothingKnobCell, *modeKnobCell, rightCol);
    }
    else
    {
        // Collapsed layout (unused by design)
    }
}

void DecayRateFloat::paint(juce::Graphics& g)
{
    if (expanded)
        paintExpanded(g);
    else
        paintCollapsed(g);

    if (!active)
    {
        auto b = getLocalBounds().toFloat();
        ComponentGreyout::paintGreyoutOverlay(g, b, 0.4f, 8.0f);
    }
}

void DecayRateFloat::paintExpanded(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    const float cr = 8.0f;

    g.setColour(th.meters.panelDark); g.fillRect(bounds);
    g.fillRoundedRectangle(bounds, cr);

    g.setColour(th.sh.withAlpha(0.6f));  g.drawRoundedRectangle(bounds.reduced(0.5f), cr - 0.5f, 2.0f);
    g.setColour(th.accent.withAlpha(0.9f)); g.drawRoundedRectangle(bounds.reduced(1.0f), cr - 1.0f, 1.5f);
    g.setColour(th.text.withAlpha(0.3f)); g.drawRoundedRectangle(bounds.reduced(0.5f), cr - 0.5f, 0.5f);

    // Header area with labels (36px - matching DuckingFloat's meter strip)
    auto headerArea = bounds.removeFromTop(36.0f);
    paintHeaderArea(g, headerArea);
    
    // Control row (28px - matching DuckingFloat's selectors row)
    auto controlRow = bounds.removeFromTop(28.0f);
    paintControlRow(g, controlRow);
}

void DecayRateFloat::paintHeaderArea(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    const float cr = 4.0f;
    auto headerArea = bounds.reduced(6.0f, 2.0f);

    // Border
    g.setColour(th.accent.withAlpha(0.30f));
    g.drawRoundedRectangle(headerArea, cr, 1.0f);

    // Header text
    g.setColour(th.text.withAlpha(0.7f));
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("DECAY RATE", headerArea, juce::Justification::centred);

    // Subtle accent line
    auto accentLine = juce::Rectangle<float>(headerArea.getX() + 8, headerArea.getBottom() - 3, headerArea.getWidth() - 16, 1.0f);
    g.setColour(th.accent.withAlpha(0.4f));
    g.fillRoundedRectangle(accentLine, 0.5f);
}

void DecayRateFloat::paintControlRow(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    const float cr = 3.0f;
    auto controlArea = bounds.reduced(6.0f, 2.0f);

    // Border (similar to DuckingFloat selectors)
    g.setColour(th.accent.withAlpha(0.25f));
    g.drawRoundedRectangle(controlArea, cr, 1.0f);

    // Control labels (Frequency vs Time controls)
    g.setColour(th.text.withAlpha(0.6f));
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    
    // Left side: Frequency controls
    auto leftArea = juce::Rectangle<float>(controlArea.getX(), controlArea.getY(), controlArea.getWidth() * 0.5f, controlArea.getHeight());
    g.drawText("FREQ", leftArea, juce::Justification::centred);
    
    // Right side: Time controls  
    auto rightArea = juce::Rectangle<float>(controlArea.getX() + controlArea.getWidth() * 0.5f, controlArea.getY(), controlArea.getWidth() * 0.5f, controlArea.getHeight());
    g.drawText("TIME", rightArea, juce::Justification::centred);

    // Subtle divider line
    auto dividerLine = juce::Rectangle<float>(controlArea.getX() + controlArea.getWidth() * 0.5f - 0.5f, controlArea.getY() + 4, 1.0f, controlArea.getHeight() - 8);
    g.setColour(th.accent.withAlpha(0.3f));
    g.fillRoundedRectangle(dividerLine, 0.5f);
}

void DecayRateFloat::paintCollapsed(juce::Graphics& g)
{
    // Collapsed layout (unused by design)
}

void DecayRateFloat::lookAndFeelChanged()
{
    // Ensure LNF propagation to children
    for (auto* child : getChildren())
        child->lookAndFeelChanged();
}

void DecayRateFloat::setExpanded(bool shouldBeExpanded)
{
    expanded = shouldBeExpanded;
    updateLayout();
    repaint();
}

void DecayRateFloat::setActive(bool shouldBeActive)
{
    active = shouldBeActive;
    repaint();
}

void DecayRateFloat::setVisible(bool shouldBeVisible)
{
    juce::Component::setVisible(shouldBeVisible);
}

void DecayRateFloat::setLookAndFeel(juce::LookAndFeel* newLookAndFeel)
{
    juce::Component::setLookAndFeel(newLookAndFeel);
    lookAndFeelChanged();
}
