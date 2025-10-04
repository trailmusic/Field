#include "DecayRateFloat.h"
#include "core/params/ParamIDs.h"
#include "engines/reverb/Presets/ReverbParameters.h"
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
    , strengthLabel("strengthLabel", "Strength")
    , windowLabel("windowLabel", "Window")
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
    using namespace field::params;

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

    // ===== Decay Profile Selectors =====
    addAndMakeVisible(profileModeCombo);
    profileModeCombo.addItemList(ReverbParameters::decayProfileModeChoices(), 1);
    profileModeCombo.setSelectedId(1); // Manual 3-Band
    profileModeCombo.setTooltip("Chooses the spectral shape of decay time (T60) across frequency.");

    addAndMakeVisible(profileCouplingCombo);
    profileCouplingCombo.addItemList(ReverbParameters::decayProfileCouplingChoices(), 1);
    profileCouplingCombo.setSelectedId(1); // Independent
    profileCouplingCombo.setTooltip("Optionally bias the profile using other controls (tone EQ, filters, width).");

    // ===== Sidechain Learn Controls =====
    addAndMakeVisible(learnButton);
    learnButton.setButtonText("Learn");
    learnButton.setTooltip("Start learning decay profile from external signal");
    
    addAndMakeVisible(resetButton);
    resetButton.setButtonText("Reset");
    resetButton.setTooltip("Reset learned profile");
    
    addAndMakeVisible(strengthSlider);
    strengthSlider.setRange(0.0, 1.0, 0.01);
    strengthSlider.setValue(0.5);
    strengthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    strengthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    strengthSlider.setTooltip("Blend strength of learned profile (0-100%)");
    
    addAndMakeVisible(windowSlider);
    windowSlider.setRange(2.0, 8.0, 0.1);
    windowSlider.setValue(4.0);
    windowSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    windowSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    windowSlider.setTooltip("Capture window duration (2-8 seconds)");
    
    addAndMakeVisible(strengthLabel);
    strengthLabel.setText("Strength", juce::dontSendNotification);
    strengthLabel.setJustificationType(juce::Justification::centredLeft);
    
    addAndMakeVisible(windowLabel);
    windowLabel.setText("Window", juce::dontSendNotification);
    windowLabel.setJustificationType(juce::Justification::centredLeft);
    

    // ===== APVTS Attachments =====
    loMultAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, "decayLoMult", loMultSlider);
    hiMultAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, "decayHiMult", hiMultSlider);
    midDbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, "decayMidDb", midDbSlider);
    midFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, "decayMidFreqHz", midFreqSlider);
    midQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, "decayMidQ", midQSlider);
    tiltDbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, "decayTiltDb", tiltDbSlider);
    smoothingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, "decaySmoothing", smoothingSlider);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, "decayMode", modeSlider);
    
    // Decay profile selector attachments
    profileModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsRef, "decay_profile_mode", profileModeCombo);
    profileCouplingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsRef, "decay_profile_coupling", profileCouplingCombo);
    
    // Sidechain Learn attachments
    learnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsRef, "decay_learn", learnButton);
    resetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsRef, "decay_learn_reset", resetButton);
    strengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, "decay_learn_strength", strengthSlider);
    windowAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsRef, "decay_learn_window_s", windowSlider);
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
        
        // Layout selectors in control row (like DuckingFloat's Mode/Detector)
        auto leftSelector = controlRow.removeFromLeft(controlRow.getWidth() * 0.5f).reduced(1.0f);
        auto rightSelector = controlRow.reduced(1.0f);
        profileModeCombo.setBounds(leftSelector.toNearestInt());
        profileCouplingCombo.setBounds(rightSelector.toNearestInt());
        
        // Learn controls area (28px - matching control row height)
        auto learnArea = bounds.removeFromTop(28.0f).reduced(2.0f, 2.0f);
        
        // Learn buttons (left side)
        auto buttonArea = learnArea.removeFromLeft(learnArea.getWidth() * 0.4f).reduced(1.0f);
        auto buttonRow = buttonArea.removeFromTop(buttonArea.getHeight() * 0.6f);
        learnButton.setBounds(buttonRow.removeFromLeft(buttonRow.getWidth() * 0.5f).reduced(1.0f).toNearestInt());
        resetButton.setBounds(buttonRow.reduced(1.0f).toNearestInt());
        
        
        // Learn sliders (right side)
        auto sliderArea = learnArea.reduced(1.0f);
        auto sliderRow = sliderArea.removeFromTop(sliderArea.getHeight() * 0.5f);
        
        // Strength slider
        auto strengthArea = sliderRow.removeFromLeft(sliderRow.getWidth() * 0.5f).reduced(1.0f);
        strengthLabel.setBounds(strengthArea.removeFromTop(12).toNearestInt());
        strengthSlider.setBounds(strengthArea.toNearestInt());
        
        // Window slider
        auto windowArea = sliderRow.reduced(1.0f);
        windowLabel.setBounds(windowArea.removeFromTop(12).toNearestInt());
        windowSlider.setBounds(windowArea.toNearestInt());
        
        // Two rows of 4 knobs each (matching actual KnobCell heights)
        auto knobsArea = bounds.reduced(1.0f);
        auto topRow    = knobsArea.removeFromTop(knobsArea.getHeight() * 0.5f).reduced(1.0f);
        auto bottomRow = knobsArea.reduced(1.0f);

        auto layRow = [](juce::Component& a, juce::Component& b, juce::Component& c, juce::Component& d, juce::Rectangle<float> area)
        {
            const float cellW = area.getWidth() / 4.0f;
            a.setBounds(area.removeFromLeft(cellW).toNearestInt());
            b.setBounds(area.removeFromLeft(cellW).toNearestInt());
            c.setBounds(area.removeFromLeft(cellW).toNearestInt());
            d.setBounds(area.removeFromLeft(cellW).toNearestInt());
        };

        layRow(*loMultKnobCell, *hiMultKnobCell, *midDbKnobCell, *midFreqKnobCell, topRow);
        layRow(*midQKnobCell, *tiltDbKnobCell, *smoothingKnobCell, *modeKnobCell, bottomRow);
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

    // Decay Rate Spectral Dots Meter
    paintDecayRateSpectralMeter(g, headerArea);

    // Subtle accent line
    auto accentLine = juce::Rectangle<float>(headerArea.getX() + 8, headerArea.getBottom() - 3, headerArea.getWidth() - 16, 1.0f);
    g.setColour(th.accent.withAlpha(0.4f));
    g.fillRoundedRectangle(accentLine, 0.5f);
}

void DecayRateFloat::paintDecayRateSpectralMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    // Calculate decay rate activity across frequency spectrum
    const int numBands = 24; // Number of spectral dots
    const float dotSize = 2.0f;
    const float dotSpacing = bounds.getWidth() / (numBands + 1);
    
    // Get current parameter values (simplified calculation)
    float loMult = loMultSlider.getValue();
    float hiMult = hiMultSlider.getValue();
    float midDb = midDbSlider.getValue();
    float tiltDb = tiltDbSlider.getValue();
    
    // Calculate decay rate profile across frequency bands
    for (int i = 0; i < numBands; ++i)
    {
        float freq = (float)i / (numBands - 1); // 0.0 to 1.0 across spectrum
        
        // Calculate decay rate activity for this frequency band
        float activity = 0.0f;
        
        // Low frequency contribution (left side)
        if (freq < 0.3f)
        {
            float lowWeight = 1.0f - (freq / 0.3f);
            activity += std::abs(loMult - 1.0f) * lowWeight;
        }
        
        // High frequency contribution (right side)
        if (freq > 0.7f)
        {
            float highWeight = (freq - 0.7f) / 0.3f;
            activity += std::abs(hiMult - 1.0f) * highWeight;
        }
        
        // Mid frequency contribution (center)
        if (freq >= 0.3f && freq <= 0.7f)
        {
            float midWeight = 1.0f - std::abs(freq - 0.5f) / 0.2f;
            activity += std::abs(midDb / 12.0f) * midWeight;
        }
        
        // Tilt contribution (affects entire spectrum)
        float tiltContribution = std::abs(tiltDb / 12.0f) * (1.0f - std::abs(freq - 0.5f) * 2.0f);
        activity += tiltContribution;
        
        // Normalize activity (0.0 to 1.0)
        activity = juce::jlimit(0.0f, 1.0f, activity);
        
        // Calculate dot position
        float x = bounds.getX() + (i + 1) * dotSpacing - dotSize * 0.5f;
        float y = bounds.getY() + bounds.getHeight() * 0.5f - dotSize * 0.5f;
        
        // Color coding based on activity level
        juce::Colour dotColor;
        if (activity < 0.3f)
        {
            // Green (subtle)
            dotColor = juce::Colour(0xff4CAF50).withAlpha(0.3f + activity * 0.7f);
        }
        else if (activity < 0.7f)
        {
            // Yellow (moderate)
            dotColor = juce::Colour(0xffFFC107).withAlpha(0.3f + activity * 0.7f);
        }
        else
        {
            // Red (strong)
            dotColor = juce::Colour(0xffF44336).withAlpha(0.3f + activity * 0.7f);
        }
        
        // Draw the spectral dot
        g.setColour(dotColor);
        g.fillEllipse(x, y, dotSize, dotSize);
    }
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

    // Control labels (Profile vs Coupling selectors)
    g.setColour(th.text.withAlpha(0.6f));
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    
    // Left side: Profile Mode
    auto leftArea = juce::Rectangle<float>(controlArea.getX(), controlArea.getY(), controlArea.getWidth() * 0.5f, controlArea.getHeight());
    g.drawText("PROFILE", leftArea, juce::Justification::centred);
    
    // Right side: Coupling
    auto rightArea = juce::Rectangle<float>(controlArea.getX() + controlArea.getWidth() * 0.5f, controlArea.getY(), controlArea.getWidth() * 0.5f, controlArea.getHeight());
    g.drawText("COUPLING", rightArea, juce::Justification::centred);

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
