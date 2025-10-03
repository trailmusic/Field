#include "DuckingFloat.h"
#include "../DSP/ReverbParamIDs.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/ui/Utilities/ComponentGreyout.h"

/*
====================================================================================================
 DuckingFloat.cpp — implementation notes
 ---------------------------------------------------------------------------------------------------
 • This version wires *every* UI control to APVTS using IDs from ReverbParamIDs.h.
 • ComboBox items are set for human readability; ComboBoxAttachment maps to APVTS choice params:
       duckMode, duckDetectorSrc
 • Sliders are bound to:
       duckDepthDb, duckThrDb, duckRatio, duckKneeDb, duckAtkMs, duckRelMs, duckBandHz, duckBandQ
   Ranges set here are purely for local interaction; truth comes from APVTS params. The attachment
   will push/pull values, and your pretty formatters (added earlier) are host-visible.
 • The GR meter is painted manually (paintGrMeter) and driven by updateGrMeter(dB).
 • Greyout uses ComponentGreyout helper so the entire sub-tree dims and disables mouse events.
====================================================================================================
*/

namespace
{
    // Local UI ranges (visual), independent of APVTS internal ranges.
    struct LocalRanges
    {
        static inline juce::Range<double> depthDb()    { return { 0.0, 24.0 }; }
        static inline juce::Range<double> thrDb()      { return { -60.0, -6.0 }; }
        static inline juce::Range<double> ratio()      { return { 1.0,  8.0 }; }
        static inline juce::Range<double> kneeDb()     { return { 0.0, 24.0 }; }
        static inline juce::Range<double> atkMs()      { return { 1.0, 100.0 }; }
        static inline juce::Range<double> relMs()      { return { 50.0, 2000.0 }; }
        static inline juce::Range<double> bandHz()     { return { 50.0, 8000.0 }; }
        static inline juce::Range<double> bandQ()      { return { 0.3, 4.0 }; }
    };
}

DuckingFloat::DuckingFloat(juce::AudioProcessorValueTreeState& apvts)
    : grLabel("grLabel", "GR")
    , grMeter(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox)
    , modeSelector("modeSelector")
    , detectorSelector("detectorSelector")
    , depthSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , thresholdSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , ratioSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , kneeSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , attackSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , releaseSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , bandFreqSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , bandQSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    , depthLabel("depthLabel", "Depth")
    , thresholdLabel("thresholdLabel", "Threshold")
    , ratioLabel("ratioLabel", "Ratio")
    , kneeLabel("kneeLabel", "Knee")
    , attackLabel("attackLabel", "Attack")
    , releaseLabel("releaseLabel", "Release")
    , bandFreqLabel("bandFreqLabel", "Band Freq")
    , bandQLabel("bandQLabel", "Band Q")
    , modeLabel("modeLabel", "Mode")
    , detectorLabel("detectorLabel", "Detector")
    , depthValue("depthValue", "6.0")
    , thresholdValue("thresholdValue", "-24.0")
    , ratioValue("ratioValue", "3.0")
    , kneeValue("kneeValue", "6.0")
    , attackValue("attackValue", "10.0")
    , releaseValue("releaseValue", "300.0")
    , bandFreqValue("bandFreqValue", "2000.0")
    , bandQValue("bandQValue", "1.0")
    , apvtsRef(apvts)
{
    setupComponents();
    setSize(360, EXPANDED_HEIGHT);
    setExpanded(true);
}

DuckingFloat::~DuckingFloat()
{
    // APVTS attachments auto-clean
}

void DuckingFloat::setupComponents()
{
    using namespace ReverbParamIDs;

    // ===== Selectors (UI items) =====
    addAndMakeVisible(modeSelector);
    modeSelector.addItem("General", 1);  // index aligns with your duckModeChoices()
    modeSelector.addItem("Vocal",   2);
    modeSelector.addItem("DrumBus", 3);
    modeSelector.addItem("Guitar",  4);
    modeSelector.addItem("Keys",    5);
    modeSelector.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(detectorSelector);
    detectorSelector.addItem("Dry",     1); // aligns with duckDetectorChoices(): 0=Dry,1=ER,2=Tail,3=Wet
    detectorSelector.addItem("ER",      2);
    detectorSelector.addItem("Tail",    3);
    detectorSelector.addItem("Wet Sum", 4);
    detectorSelector.setJustificationType(juce::Justification::centred);

    // ===== Sliders (visual configuration only – attachments do the real binding) =====
    auto configureKnob = [this](juce::Slider& s, double min, double max, double step)
    {
        s.setRange(min, max, step);
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setRotaryParameters(juce::MathConstants<float>::pi,
                              juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi,
                              true);
        addAndMakeVisible(s);
    };

    configureKnob(depthSlider,     LocalRanges::depthDb().getStart(), LocalRanges::depthDb().getEnd(), 0.1);
    configureKnob(thresholdSlider, LocalRanges::thrDb().getStart(),   LocalRanges::thrDb().getEnd(),   0.1);
    configureKnob(ratioSlider,     LocalRanges::ratio().getStart(),   LocalRanges::ratio().getEnd(),   0.01);
    configureKnob(kneeSlider,      LocalRanges::kneeDb().getStart(),  LocalRanges::kneeDb().getEnd(),  0.1);
    configureKnob(attackSlider,    LocalRanges::atkMs().getStart(),   LocalRanges::atkMs().getEnd(),   0.1);
    configureKnob(releaseSlider,   LocalRanges::relMs().getStart(),   LocalRanges::relMs().getEnd(),   0.1);
    configureKnob(bandFreqSlider,  LocalRanges::bandHz().getStart(),  LocalRanges::bandHz().getEnd(),  0.01);
    bandFreqSlider.setSkewFactorFromMidPoint(1000.0);
    configureKnob(bandQSlider,     LocalRanges::bandQ().getStart(),   LocalRanges::bandQ().getEnd(),   0.001);

    // ===== Create KnobCells (they own their value labels / abbreviations) =====
    depthKnobCell    = std::make_unique<KnobCell>(depthSlider,     depthValue,     "DEP");
    thresholdKnobCell= std::make_unique<KnobCell>(thresholdSlider, thresholdValue, "THR");
    ratioKnobCell    = std::make_unique<KnobCell>(ratioSlider,     ratioValue,     "RAT");
    kneeKnobCell     = std::make_unique<KnobCell>(kneeSlider,      kneeValue,      "KNE");
    attackKnobCell   = std::make_unique<KnobCell>(attackSlider,    attackValue,    "ATK");
    releaseKnobCell  = std::make_unique<KnobCell>(releaseSlider,   releaseValue,   "REL");
    bandFreqKnobCell = std::make_unique<KnobCell>(bandFreqSlider,  bandFreqValue,  "FREQ");
    bandQKnobCell    = std::make_unique<KnobCell>(bandQSlider,     bandQValue,     "Q");

    for (auto* kc : { depthKnobCell.get(), thresholdKnobCell.get(), ratioKnobCell.get(), kneeKnobCell.get(),
                      attackKnobCell.get(), releaseKnobCell.get(), bandFreqKnobCell.get(), bandQKnobCell.get() })
    {
        kc->setValueLabelMode(KnobCell::ValueLabelMode::Managed);
        kc->getProperties().set("reverbMetallic", true);
        addAndMakeVisible(*kc);
    }

    // Hide static labels (KnobCell provides titles)
    for (auto* l : { &depthLabel, &thresholdLabel, &ratioLabel, &kneeLabel,
                     &attackLabel, &releaseLabel, &bandFreqLabel, &bandQLabel,
                     &modeLabel, &detectorLabel })
        l->setVisible(false);

    // ===== APVTS Attachments =====
    modeAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvtsRef, ReverbParamIDs::duckMode,        modeSelector);
    detectorAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvtsRef, ReverbParamIDs::duckDetectorSrc, detectorSelector);

    depthAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvtsRef, ReverbParamIDs::duckDepthDb,  depthSlider);
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvtsRef, ReverbParamIDs::duckThrDb,    thresholdSlider);
    ratioAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvtsRef, ReverbParamIDs::duckRatio,    ratioSlider);
    kneeAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvtsRef, ReverbParamIDs::duckKneeDb,   kneeSlider);
    attackAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvtsRef, ReverbParamIDs::duckAtkMs,    attackSlider);
    releaseAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvtsRef, ReverbParamIDs::duckRelMs,    releaseSlider);
    bandFreqAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvtsRef, ReverbParamIDs::duckBandHz,   bandFreqSlider);
    bandQAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvtsRef, ReverbParamIDs::duckBandQ,    bandQSlider);

    // Ensure module is enabled initially (parent can later call setActive/greyout)
    setActive(true);
    setGreyedOut(false);
}

void DuckingFloat::setExpanded(bool shouldExpand)
{
    expanded = shouldExpand;
    updateLayout();
    repaint();
}

void DuckingFloat::updateGrMeter(float grDb)
{
    // Clamp and repaint; negative values mean reduction
    currentGrDb = juce::jlimit(-20.0f, 0.0f, grDb);
    repaint();
}

void DuckingFloat::setActive(bool activeState)
{
    active = activeState;
    updateLayout();
    repaint();
}

void DuckingFloat::setGreyedOut(bool greyedOutState)
{
    greyedOut = greyedOutState;
    updateLayout();
    repaint();
}

void DuckingFloat::setVisible(bool shouldBeVisible)
{
    juce::Component::setVisible(shouldBeVisible);
    if (shouldBeVisible)
        updateLayout();
}

void DuckingFloat::lookAndFeelChanged()
{
    repaint();
}

void DuckingFloat::setLookAndFeel(juce::LookAndFeel* newLookAndFeel)
{
    juce::Component::setLookAndFeel(newLookAndFeel);

    // Propagate to children that render text/graphics
    for (auto* s : { &depthSlider, &thresholdSlider, &ratioSlider, &kneeSlider,
                     &attackSlider, &releaseSlider, &bandFreqSlider, &bandQSlider })
        s->setLookAndFeel(newLookAndFeel);

    for (auto* c : { &modeSelector, &detectorSelector })
        c->setLookAndFeel(newLookAndFeel);

    for (auto* l : { &depthLabel, &thresholdLabel, &ratioLabel, &kneeLabel,
                     &attackLabel, &releaseLabel, &bandFreqLabel, &bandQLabel,
                     &modeLabel, &detectorLabel })
        l->setLookAndFeel(newLookAndFeel);

    for (auto* kc : { depthKnobCell.get(), thresholdKnobCell.get(), ratioKnobCell.get(), kneeKnobCell.get(),
                      attackKnobCell.get(), releaseKnobCell.get(), bandFreqKnobCell.get(), bandQKnobCell.get() })
        if (kc) kc->setLookAndFeel(newLookAndFeel);
}

void DuckingFloat::resized()
{
    updateLayout();
}

void DuckingFloat::updateLayout()
{
    auto bounds = getLocalBounds().toFloat();

    // Disable mouse & grey visuals via helper
    const bool componentsEnabled = active && !greyedOut;
    ComponentGreyout::setGreyedOut(*this, !componentsEnabled, 0.4f);

    if (expanded)
    {
        // Top meter strip
        auto top = bounds.removeFromTop(36.0f);
        grMeterBounds = top.reduced(2.0f, 2.0f);

        // Selectors row
        auto selRow = bounds.removeFromTop(28.0f).reduced(2.0f, 2.0f);
        auto leftSel  = selRow.removeFromLeft(selRow.getWidth() * 0.5f).reduced(1.0f);
        auto rightSel = selRow.reduced(1.0f);
        modeSelector.setBounds(leftSel.toNearestInt());
        detectorSelector.setBounds(rightSel.toNearestInt());

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

        layColumn(*depthKnobCell, *thresholdKnobCell, *ratioKnobCell, *kneeKnobCell, leftCol);
        layColumn(*attackKnobCell, *releaseKnobCell, *bandFreqKnobCell, *bandQKnobCell, rightCol);
    }
    else
    {
        // Collapsed layout (unused by design); leave minimal area for a single-line meter if desired
        grMeterBounds = bounds.reduced(6.0f);
    }
}

void DuckingFloat::paint(juce::Graphics& g)
{
    if (expanded) paintExpanded(g);
    else          paintCollapsed(g);

    if (greyedOut || !active)
    {
        auto b = getLocalBounds().toFloat();
        ComponentGreyout::paintGreyoutOverlay(g, b, 0.4f, 8.0f);
    }
}

void DuckingFloat::paintCollapsed(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    const float cr = PILL_CORNER_RADIUS;

    g.setColour(th.meters.panelDark); g.fillRect(bounds);
    g.fillRoundedRectangle(bounds, cr);
    g.setColour(th.sh.withAlpha(0.6f)); g.drawRoundedRectangle(bounds.reduced(0.5f), cr - 0.5f, 2.0f);
    g.setColour(th.accent.withAlpha(0.9f)); g.drawRoundedRectangle(bounds.reduced(1.0f), cr - 1.0f, 1.5f);
}

void DuckingFloat::paintExpanded(juce::Graphics& g)
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

    if (grMeterBounds.getWidth() > 0 && grMeterBounds.getHeight() > 0)
        paintGrMeter(g, grMeterBounds);
}

void DuckingFloat::paintGrMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    const float cr = 4.0f;
    auto meterArea = bounds.reduced(6.0f, 2.0f);

    // Border
    g.setColour(th.accent.withAlpha(0.30f));
    g.drawRoundedRectangle(meterArea, cr, 1.0f);

    // Idle / ready hints
    if (!active || greyedOut)
    {
        auto inactive = juce::Rectangle<float>(meterArea.getX(), meterArea.getBottom() - 2.0f, meterArea.getWidth(), 2.0f);
        g.setColour(th.accent.withAlpha(0.30f));
        g.fillRoundedRectangle(inactive, 1.0f);
    }
    else if (currentGrDb >= -0.1f)
    {
        auto ready = juce::Rectangle<float>(meterArea.getX(), meterArea.getBottom() - 2.0f, meterArea.getWidth(), 2.0f);
        g.setColour(th.accent.withAlpha(0.40f));
        g.fillRoundedRectangle(ready, 1.0f);
    }

    // Actual GR bar
    if (currentGrDb < -0.1f)
    {
        const float t = juce::jlimit(0.0f, 1.0f, juce::jmap(currentGrDb, -20.0f, 0.0f, 1.0f, 0.0f));
        auto grBar = juce::Rectangle<float>(meterArea.getX(), meterArea.getY(), meterArea.getWidth() * t, meterArea.getHeight());

        juce::Colour startColor, endColor;
        if (currentGrDb > -3.0f)        { startColor = th.meters.error.withAlpha(0.60f);   endColor = th.meters.error.withAlpha(0.75f); }
        else if (currentGrDb > -6.0f)   { startColor = th.meters.warning.withAlpha(0.70f); endColor = th.meters.warning.withAlpha(0.90f); }
        else if (currentGrDb > -12.0f)  { startColor = th.meters.safe.withAlpha(0.55f);    endColor = th.meters.safe.withAlpha(0.85f); }
        else                            { startColor = th.meters.trackBase.withAlpha(0.30f); endColor = th.meters.trackBase.withAlpha(0.50f); }

        juce::ColourGradient grad(startColor, grBar.getX(), grBar.getY(), endColor, grBar.getX(), grBar.getBottom(), false);
        g.setFillType(juce::FillType(grad));
        g.fillRoundedRectangle(grBar, cr - 1.0f);
        g.setFillType({});
    }

    // Scale ticks: 0, -5, -10, -15, -20
    g.setColour(th.text.withAlpha(0.45f));
    g.setFont(juce::Font(9.0f));

    auto tickArea = meterArea.reduced(6.0f, 0.0f);
    for (int i = 0; i <= 4; ++i)
    {
        const float db = -5.0f * i;
        const float x  = tickArea.getX() + (tickArea.getWidth() * (float) i / 4.0f);
        g.drawText(juce::String((int)db), juce::Rectangle<float>(x - 10.0f, meterArea.getY() + 2.0f, 20.0f, meterArea.getHeight() - 4.0f),
                   juce::Justification::centred);
    }

    // Units
    g.setColour(th.text.withAlpha(0.80f));
    g.setFont(juce::Font(9.0f));
    g.drawText("GR dB", meterArea.removeFromTop(0).withHeight(12.0f).translated(0, -14.0f),
               juce::Justification::centred);
}