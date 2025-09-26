#include "PluginProcessor.h"
#include "PluginEditor.h"
// Implement XYPaneAdapter methods now that XYPad is fully defined here
XYPaneAdapter::XYPaneAdapter (XYPad& padRef) : pad (padRef) { addAndMakeVisible ((juce::Component&) pad); }
void XYPaneAdapter::resized() { pad.setBounds (getLocalBounds()); }
void XYPaneAdapter::pushWaveformSample (double L, double R) { pad.pushWaveformSample (L, R); }
#include "ui/PaneManager.h"
#include "reverb/ReverbParamIDs.h"
#include "ui/Layout.h"
#include "dsp/DelayPresetLibrary.h"
#include "reverb/ui/ReverbControlsPanel.h"
#include "reverb/ui/ReverbPanel.h"
#include "reverb/ui/ReverbDynEQPane.h"
#include "ui/ControlGridMetrics.h"

//==============================================================

// Reusable tinted PopupMenu LookAndFeel + helper
struct TintMenuLNFEx : public juce::LookAndFeel_V4
{
    juce::Colour defaultTint { juce::Colours::skyblue };
    juce::Array<juce::Colour> itemTints;
    bool hideChecks = true;
    mutable int paintIndex = 0; // reset on background draw

    void drawPopupMenuBackground (juce::Graphics& g, int w, int h) override
    {
        paintIndex = 0;
        auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h);
        // Use configured colours when available; avoid hardcoded hex
        auto bg = findColour (juce::PopupMenu::backgroundColourId);
        auto text = findColour (juce::PopupMenu::textColourId);
        g.setColour (bg);
        g.fillRect (r);
        g.setColour (text.withAlpha (0.06f));
        g.drawRoundedRectangle (r.reduced (1.0f), 5.0f, 1.0f);
    }

    void drawPopupMenuSeparator (juce::Graphics& g, const juce::Rectangle<int>& area)
    {
        auto r = area.toFloat().reduced (10.0f, 0.0f);
        auto text = findColour (juce::PopupMenu::textColourId);
        g.setColour (text.withAlpha (0.10f));
        g.fillRect (juce::Rectangle<float> (r.getX(), r.getCentreY() - 0.5f, r.getWidth(), 1.0f));
    }

    void drawPopupMenuSectionHeader (juce::Graphics& g, const juce::String& title,
                                     const juce::Rectangle<int>& area)
    {
        auto r = area.toFloat().reduced (8.0f, 4.0f);
        auto text = findColour (juce::PopupMenu::textColourId);
        g.setColour (text.withAlpha (0.60f));
        g.setFont (juce::Font (juce::FontOptions (12.5f)).withExtraKerningFactor (0.02f).boldened());
        g.drawFittedText (title.toUpperCase(), r.toNearestInt(), juce::Justification::centredLeft, 1);
    }

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool /*isActive*/, bool isHighlighted, bool isTicked,
                            bool /*hasSubMenu*/, const juce::String& text, const juce::String& shortcutKeyText,
                            const juce::Drawable* /*icon*/, const juce::Colour* textColour) override
    {
        if (isSeparator) { drawPopupMenuSeparator (g, area); return; }

        auto r = area.toFloat().reduced (4.0f, 2.0f);
        const juce::Colour tint = (paintIndex >= 0 && paintIndex < itemTints.size())
                                  ? itemTints.getReference (paintIndex++)
                                  : defaultTint;

        if (isHighlighted || isTicked)
        {
            g.setColour (tint.withAlpha (isHighlighted ? 0.90f : 0.65f));
            g.fillRoundedRectangle (r, 4.0f);
        auto text = findColour (juce::PopupMenu::textColourId);
        g.setColour (text.withAlpha (0.10f));
            g.drawRoundedRectangle (r, 4.0f, 1.0f);
        }

        auto ta = r.reduced (hideChecks ? 8.0f : 22.0f, 0.0f);
        g.setColour (textColour ? *textColour : findColour (juce::PopupMenu::textColourId).withAlpha (0.95f));
        g.setFont (juce::Font (juce::FontOptions (14.0f)));
        g.drawFittedText (text, ta.toNearestInt(), juce::Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty())
        {
            g.setColour (findColour (juce::PopupMenu::textColourId).withAlpha (0.55f));
            g.setFont (juce::Font (juce::FontOptions (13.0f)));
            auto rt = ta.removeFromRight (60).toNearestInt();
            g.drawFittedText (shortcutKeyText, rt, juce::Justification::centredRight, 1);
        }
    }
};

// Small per-button LNF to draw an up/down chevron on the Group 2 toggle
struct BottomChevronLNF : public FieldLNF
{
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                               bool isOver, bool isDown) override
    {
        auto r = button.getLocalBounds().toFloat().reduced (2.0f);
        // Background consistent with our buttons: gradient panel + outline
        auto top = theme.panel.brighter (0.10f);
        auto bot = theme.panel.darker   (0.10f);
        g.setGradientFill (juce::ColourGradient (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (theme.sh);
        g.drawRoundedRectangle (r, 6.0f, 1.0f);

        // Subtle elevation on hover/down
        if (isOver || isDown)
        {
            juce::DropShadow ds1 (theme.shadowDark.withAlpha (0.20f), 8, { -1, -1 });
            juce::DropShadow ds2 (theme.shadowLight.withAlpha (0.18f), 5, { -1, -1 });
            ds1.drawForRectangle (g, r.getSmallestIntegerContainer());
            ds2.drawForRectangle (g, r.getSmallestIntegerContainer());
        }
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool isOver, bool /*isDown*/) override
    {
        // Chevron points up when not engaged, down when engaged
        const bool engaged = button.getToggleState();
        auto col = isOver ? theme.accent : theme.textMuted;
        g.setColour (col);

        auto b = button.getLocalBounds().toFloat().reduced (button.getHeight() * 0.22f, button.getHeight() * 0.30f);
        const float cx = b.getCentreX();
        const float cy = b.getCentreY();
        const float half = b.getWidth() * 0.40f;   // wider span for a flatter chevron
        const float vAmp = b.getHeight() * 0.18f;  // smaller vertical excursion for flat look

        juce::Path p;
        if (engaged)
        {
            // Down chevron (V)
            p.startNewSubPath (cx - half, cy - vAmp);
            p.lineTo          (cx,        cy + vAmp);
            p.lineTo          (cx + half, cy - vAmp);
        }
        else
        {
            // Up chevron (^)
            p.startNewSubPath (cx - half, cy + vAmp);
            p.lineTo          (cx,        cy - vAmp);
            p.lineTo          (cx + half, cy + vAmp);
        }
        g.strokePath (p, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
};

template <typename BuildFn, typename ResultFn>
static void showTintedMenu (juce::Component& anchor, const TintMenuLNFEx& configuredLnf,
                            BuildFn&& build, ResultFn&& onResult)
{
    auto lnfHold = std::make_shared<TintMenuLNFEx>();
    // Copy relevant configuration
    lnfHold->defaultTint = configuredLnf.defaultTint;
    lnfHold->hideChecks  = configuredLnf.hideChecks;
    lnfHold->itemTints   = configuredLnf.itemTints;
    // Copy colours that might have been set on configuredLnf
    lnfHold->setColour (juce::PopupMenu::textColourId,
                        configuredLnf.findColour (juce::PopupMenu::textColourId));
    lnfHold->setColour (juce::PopupMenu::highlightedBackgroundColourId,
                        configuredLnf.findColour (juce::PopupMenu::highlightedBackgroundColourId));
    lnfHold->setColour (juce::PopupMenu::highlightedTextColourId,
                        configuredLnf.findColour (juce::PopupMenu::highlightedTextColourId));

    juce::PopupMenu m; m.setLookAndFeel (lnfHold.get());

    build (m, *lnfHold);

    auto* parent = anchor.getTopLevelComponent();
    juce::PopupMenu::Options opt;
    opt = opt.withTargetComponent (&anchor)
             .withParentComponent (parent)
             .withMinimumWidth (juce::jmax (160, anchor.getWidth()));

    m.showMenuAsync (opt, [lnfHold, onResult] (int r) { onResult (r); });
}

//==============================================================

//==============================================================
// ToggleSwitch (compact, slow animation, keeps original visual)
ToggleSwitch::ToggleSwitch()
{
    setLabels ("STEREO", "SPLIT");
    sliderValue.reset (0.0, 0.02);
    sliderValue.setCurrentAndTargetValue (0.0f);
}

void ToggleSwitch::setLabels (const juce::String& offLabel, const juce::String& onLabel)
{
    offText = offLabel;
    onText  = onLabel;
    repaint();
}

void ToggleSwitch::setToggleState (bool shouldBeOn, juce::NotificationType nt)
{
    if (isOn == shouldBeOn) return;
    isOn = shouldBeOn;
    sliderValue.setTargetValue (isOn ? 1.0f : 0.0f);
    if (nt == juce::sendNotification && onToggleChange) onToggleChange (isOn);
    repaint();
}

void ToggleSwitch::mouseDown (const juce::MouseEvent&) { isMouseDown = true; repaint(); }
void ToggleSwitch::mouseUp   (const juce::MouseEvent&)
{
    if (!isMouseDown) return;
    isMouseDown = false;
    setToggleState (!isOn, juce::sendNotification);
}

void ToggleSwitch::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const float rad = b.getHeight() * 0.5f;
    const float knobR = b.getHeight() * 0.45f;

    // match editor theme
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    const auto accent = th.accent;

    // track
    g.setColour (th.sh);
    g.fillRoundedRectangle (b, rad);
    g.setColour (th.hl);
    g.drawRoundedRectangle (b, rad, 2.0f);

    // hover glow
    const bool over = isMouseOverOrDragging();
    if (over || hoverActive)
    {
        g.setColour (accent);
        g.drawRoundedRectangle (b, rad, 1.0f);
    }

    // knob travel—slightly inside edges for compact feel
    const float leftCx  = b.getX() + b.getWidth() * 0.30f;
    const float rightCx = b.getX() + b.getWidth() * 0.70f;
    const float t = sliderValue.getCurrentValue();
    const float kx = juce::jmap (t, leftCx - knobR, rightCx - knobR);
    const float ky = b.getCentreY() - knobR;
    juce::Rectangle<float> k (kx, ky, knobR * 2.0f, knobR * 2.0f);

    // shadow
    g.setColour ((lf ? lf->theme.shadowDark : juce::Colours::black).withAlpha (0.4f));
    g.fillEllipse (k.translated (2.0f, 2.0f));

    // fill: stereo = accent blue/green, split = grey
    g.setColour (isOn ? (lf ? lf->theme.textMuted : juce::Colour (0xFF7A7D85)) : accent);
    g.fillEllipse (k);

    // rim + split marker
    g.setColour (lf ? lf->theme.textMuted : juce::Colour (0xFF9A9DA5));
    g.drawEllipse (k, 2.0f);
    if (isOn)
    {
        g.setColour (lf ? lf->theme.text : juce::Colour (0xFFB0B3B8));
        const float cx = k.getCentreX();
        g.drawLine (cx, k.getY() + 4.0f, cx, k.getBottom() - 4.0f, 1.5f);
    }

    if (sliderValue.isSmoothing()) repaint();
}

//==============================================================
// VerticalSlider3D implementation
//==============================================================
VerticalSlider3D::VerticalSlider3D()
{
    setSliderStyle (juce::Slider::LinearVertical);
    setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 20);
    setRange (-60.0, 12.0, 0.1);
    setValue (0.0);
    setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00000000));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0x00000000));
}

void VerticalSlider3D::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const float trackWidth = 8.0f;
    const float handleSize = 20.0f; // Slightly larger handle
    
    // Get accent color from look and feel
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto accentColor = lf ? lf->theme.accent : juce::Colours::cyan;
    
    // Add accent color back glow
    g.setColour (accentColor.withAlpha (0.3f));
    g.fillRoundedRectangle (bounds.reduced (2.0f), 6.0f);
    
    // Add gradient border (full color at top, transparent at bottom) - more prominent
    juce::ColourGradient borderGradient (accentColor, bounds.getX(), bounds.getY(),
                                        accentColor.withAlpha (0.0f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (borderGradient);
    g.drawRoundedRectangle (bounds.reduced (1.0f), 6.0f, 3.0f); // Increased thickness from 2.0f to 3.0f
    
    // Draw metallic background extending to container top and bottom (meeting meters)
    auto extendedBounds = bounds;
    extendedBounds.setY (0); // Extend to top of container
    // Extend to full container height to meet meters at bottom
    // Account for the padding that was applied to the container
    if (getParentComponent()) {
        extendedBounds.setHeight (getParentComponent()->getHeight());
        // Extend beyond the container bounds to account for padding
        extendedBounds.setY (-10); // Extend above container
        extendedBounds.setHeight (extendedBounds.getHeight() + 20); // Extend below container
    } else {
        extendedBounds.setHeight (bounds.getHeight());
    }
    drawMetallicBackground (g, extendedBounds);
    
    // Draw track (shortened with room for labels at bottom)
    const float trackTop = 20.0f; // Start track lower
    const float trackBottom = bounds.getHeight() - 60.0f; // Leave 60px room for labels at bottom
    const float trackHeight = trackBottom - trackTop;
    auto trackRect = juce::Rectangle<float> (bounds.getCentreX() - trackWidth/2, trackTop, trackWidth, trackHeight);
    drawMetallicTrack (g, trackRect);
    
    // Calculate handle position within the shortened track
    const float value = (float) getValue();
    const float normalizedValue = (value - getMinimum()) / (getMaximum() - getMinimum());
    const float handleY = trackRect.getY() + (1.0f - normalizedValue) * trackRect.getHeight();
    auto handleRect = juce::Rectangle<float> (bounds.getCentreX() - handleSize/2, handleY - handleSize/2, handleSize, handleSize);
    
    // Draw 3D handle
    draw3DHandle (g, handleRect);
    
    // Draw visual markers and labels
    drawMarkers (g, trackRect);
    
    // Standard border treatment: accent border (reduced brightness for sliders)
    g.setColour (accentColor.withAlpha (0.3f));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);
    
    // Peak line (thicker bottom border like meters)
    g.setColour (accentColor.withAlpha (0.6f));
    g.fillRect (juce::Rectangle<float> (bounds.getX(), bounds.getBottom() - 1.0f, bounds.getWidth(), 2.0f));
}

void VerticalSlider3D::draw3DHandle (juce::Graphics& g, juce::Rectangle<float> handleRect)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto accent = lf ? lf->theme.accent : juce::Colour (0xFF5AA9E6);
    const auto shadowDark = lf ? lf->theme.shadowDark : juce::Colour (0xFF1A1C20);
    const auto shadowLight = lf ? lf->theme.shadowLight : juce::Colour (0xFF60646C);
    
    // Create gradient for 3D effect
    juce::ColourGradient gradient (accent.brighter (0.3f), 
                                  juce::Point<float>(handleRect.getCentreX(), handleRect.getY()),
                                  accent.darker (0.3f), 
                                  juce::Point<float>(handleRect.getCentreX(), handleRect.getBottom()),
                                  false);
    gradient.addColour (0.5, accent);
    
    // Draw handle under glow
    g.setColour (accent.withAlpha (0.4f));
    g.fillEllipse (handleRect.expanded (4, 4));
    
    // Draw handle shadow
    g.setColour (shadowDark.withAlpha (0.4f));
    g.fillEllipse (handleRect.translated (2, 2));
    
    // Draw handle body
    g.setGradientFill (gradient);
    g.fillEllipse (handleRect);
    
    // Draw darker interior for better text visibility
    g.setColour (accent.darker (0.4f));
    g.fillEllipse (handleRect.reduced (3));
    
    // Draw subtle highlight
    g.setColour (accent.brighter (0.2f));
    g.fillEllipse (handleRect.reduced (4));
    
    // Draw rim
    g.setColour (accent.darker (0.2f));
    g.drawEllipse (handleRect, 1.0f);
    
    // Draw handle label (I, O, M) - use component name to identify slider type
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (8.0f, juce::Font::bold));
    juce::String handleLabel;
    juce::String componentName = getName();
    if (componentName.contains ("input")) handleLabel = "I";
    else if (componentName.contains ("output")) handleLabel = "O";
    else if (componentName.contains ("mix")) handleLabel = "M";
    
    if (!handleLabel.isEmpty())
    {
        g.drawText (handleLabel, handleRect, juce::Justification::centred);
    }
}

void VerticalSlider3D::drawMarkers (juce::Graphics& g, juce::Rectangle<float> trackRect)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto textColor = lf ? lf->theme.textMuted : juce::Colour (0xFF9A9DA5);
    const auto accentColor = lf ? lf->theme.accent : juce::Colours::cyan;
    
    g.setColour (textColor);
    g.setFont (juce::Font (9.0f, juce::Font::bold));
    
    // Determine marker values based on slider range
    std::vector<float> markerValues;
    std::vector<juce::String> markerLabels;
    
    const float minVal = (float) getMinimum();
    const float maxVal = (float) getMaximum();
    
    if (maxVal <= 12.0f && minVal >= -60.0f) {
        // dB range (Input/Output sliders)
        markerValues = {-60.0f, -40.0f, -20.0f, -10.0f, -6.0f, -3.0f, 0.0f, 3.0f, 6.0f, 12.0f};
        markerLabels = {"-60", "-40", "-20", "-10", "-6", "-3", "0", "+3", "+6", "+12"};
    } else if (maxVal <= 100.0f && minVal >= 0.0f) {
        // Percentage range (Mix slider)
        markerValues = {0.0f, 25.0f, 50.0f, 75.0f, 100.0f};
        markerLabels = {"0%", "25%", "50%", "75%", "100%"};
    } else {
        // Generic range - create 5 markers
        const float range = maxVal - minVal;
        for (int i = 0; i <= 4; ++i) {
            const float val = minVal + (range * i / 4.0f);
            markerValues.push_back (val);
            markerLabels.push_back (juce::String (val, 1));
        }
    }
    
    // Draw markers
    for (size_t i = 0; i < markerValues.size(); ++i) {
        const float markerValue = markerValues[i];
        const juce::String markerLabel = markerLabels[i];
        
        // Calculate position
        const float normalizedPos = (markerValue - minVal) / (maxVal - minVal);
        const float markerY = trackRect.getY() + (1.0f - normalizedPos) * trackRect.getHeight();
        
        // Draw tick mark
        const float tickLength = 8.0f;
        const float tickX = trackRect.getX() - trackRect.getWidth() - 5.0f;
        g.setColour (textColor.withAlpha (0.8f));
        g.drawLine (tickX, markerY, tickX + tickLength, markerY, 1.5f);
        
        // Draw label
        g.setColour (textColor);
        const float labelX = tickX + tickLength + 3.0f;
        const float labelY = markerY - 6.0f;
        g.drawText (markerLabel, labelX, labelY, 40.0f, 12.0f, juce::Justification::left);
        
        // Add subtle accent highlight for key values
        if (markerValue == 0.0f || markerValue == 50.0f || markerValue == 100.0f) {
            g.setColour (accentColor.withAlpha (0.3f));
            g.fillEllipse (tickX - 2.0f, markerY - 1.0f, 4.0f, 2.0f);
        }
    }
}

void VerticalSlider3D::drawMetallicTrack (juce::Graphics& g, juce::Rectangle<float> trackRect)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto panel = lf ? lf->theme.panel : juce::Colour (0xFF3A3D45);
    const auto shadowDark = lf ? lf->theme.shadowDark : juce::Colour (0xFF1A1C20);
    const auto shadowLight = lf ? lf->theme.shadowLight : juce::Colour (0xFF60646C);
    
    // Track background
    g.setColour (panel.darker (0.2f));
    g.fillRoundedRectangle (trackRect, 4.0f);
    
    // Inner shadow
    g.setColour (shadowDark.withAlpha (0.3f));
    g.drawRoundedRectangle (trackRect.reduced (0.5f), 4.0f, 1.0f);
    
    // Highlight
    g.setColour (shadowLight.withAlpha (0.2f));
    g.drawRoundedRectangle (trackRect.reduced (1.0f), 4.0f, 0.5f);
}

void VerticalSlider3D::drawMetallicBackground (juce::Graphics& g, juce::Rectangle<float> backgroundRect)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto panel = lf ? lf->theme.panel : juce::Colour (0xFF3A3D45);
    const auto shadowDark = lf ? lf->theme.shadowDark : juce::Colour (0xFF1A1C20);
    const auto shadowLight = lf ? lf->theme.shadowLight : juce::Colour (0xFF60646C);
    
    // Background fill
    g.setColour (panel);
    g.fillRoundedRectangle (backgroundRect, 6.0f);
    
    // Drop shadow
    juce::DropShadow ds1 (shadowDark.withAlpha (0.4f), 8, { 0, 2 });
    juce::DropShadow ds2 (shadowLight.withAlpha (0.2f), 4, { 0, 1 });
    auto ri = backgroundRect.getSmallestIntegerContainer();
    ds1.drawForRectangle (g, ri);
    ds2.drawForRectangle (g, ri);
    
    // Inner rim
    g.setColour (shadowDark.withAlpha (0.3f));
    g.drawRoundedRectangle (backgroundRect.reduced (1.0f), 6.0f, 1.0f);
}

void VerticalSlider3D::mouseDown (const juce::MouseEvent& e)
{
    isDragging = true;
    lastMousePos = e.position;
    juce::Slider::mouseDown (e);
}

void VerticalSlider3D::mouseDrag (const juce::MouseEvent& e)
{
    if (isDragging)
    {
        const float deltaY = lastMousePos.y - e.position.y;
        const float sensitivity = 0.5f;
        const float newValue = getValue() + deltaY * sensitivity;
        setValue (juce::jlimit (getMinimum(), getMaximum(), (double) newValue));
        lastMousePos = e.position;
    }
    juce::Slider::mouseDrag (e);
}

void VerticalSlider3D::mouseUp (const juce::MouseEvent& e)
{
    isDragging = false;
    juce::Slider::mouseUp (e);
}

void VerticalSlider3D::setSliderStyle (SliderStyle newStyle)
{
    // Force vertical style
    juce::Slider::setSliderStyle (juce::Slider::LinearVertical);
}

//==============================================================
// ControlContainer (panel with subtle depth + title)
ControlContainer::ControlContainer() { setWantsKeyboardFocus (false); }

void ControlContainer::setTitle (const juce::String& t) { containerTitle = t; repaint(); }

void ControlContainer::paint (juce::Graphics& g)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto panel = lf ? lf->theme.panel : juce::Colour (0xFF3A3D45);
    const auto text  = lf ? lf->theme.text  : juce::Colour (0xFFF0F2F5);
    const auto accent= lf ? lf->theme.accent: juce::Colour (0xFF5AA9E6);

    auto r = getLocalBounds().toFloat();
    const float rad = 8.0f;

    if (showBorder)
    {
        // Use custom background color if set, otherwise use theme panel color
        juce::Colour bgColor = useCustomBackgroundColour ? backgroundColour : panel;
        g.setColour (bgColor);
        g.fillRoundedRectangle (r.reduced (3.0f), rad);

        // depth
        juce::DropShadow ds1 ((lf ? lf->theme.shadowDark  : juce::Colour (0xFF1A1C20)).withAlpha (0.6f), 20, { -2, -2 });
        juce::DropShadow ds2 ((lf ? lf->theme.shadowLight : juce::Colour (0xFF60646C)).withAlpha (0.4f),  8, { -1, -1 });
        auto ri = r.reduced (3.0f).getSmallestIntegerContainer();
        ds1.drawForRectangle (g, ri);
        ds2.drawForRectangle (g, ri);

        // inner rim
        g.setColour ((lf ? lf->theme.sh : juce::Colour (0xFF2A2C30)).withAlpha (0.3f));
        g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 1.0f);
    }

    if (showBorder)
    {
        auto border = r.reduced (3.0f);
        // hover halo
        const bool over = isMouseOverOrDragging();
        if (over || hoverActive)
        {
            g.setColour (accent.withAlpha (0.5f));
            g.drawRoundedRectangle (border.expanded (2.0f), rad, 2.0f);
        }
        g.setColour (useCustomBorderColour ? borderColour : accent);
        g.drawRoundedRectangle (border, rad, 1.0f);
    }

    if (containerTitle.isNotEmpty() && showBorder)
    {
        auto title = r.reduced (10.0f).removeFromTop (25);
        g.setColour (text);
        g.setFont (juce::Font (juce::FontOptions (14.0f).withStyle ("Bold")));

        // optional icon
        IconSystem::IconType icon = IconSystem::Speaker;
        if      (containerTitle == "FIELD")  icon = IconSystem::Space;
        else if (containerTitle == "VOLUME") icon = IconSystem::Speaker;
        else if (containerTitle == "EQ")     icon = IconSystem::Tilt;

        IconSystem::drawIcon (g, icon, title.removeFromLeft (20).reduced (2.0f), text);
        g.drawText (containerTitle, title, juce::Justification::centredLeft);
    }
}

//==============================================================
// XYPad (visual upgrade preserved; layout/edges match original)
void XYPad::pushWaveformSample (double l, double r)
{
    waveformL[(size_t) waveformWriteIndex] = l;
    waveformR[(size_t) waveformWriteIndex] = r;
    waveformWriteIndex = (waveformWriteIndex + 1) % waveformBufferSize;
    hasWaveformData = true;

    // Avoid cross-thread repaint; Editor timer will repaint at ~30 Hz.
    // (No repaint here.)
}

int XYPad::getBallAtPosition (juce::Point<float> pos, juce::Rectangle<float> b)
{
    if (!isSplitMode) return 0;

    const float gainScale = juce::jmap (gainValue, -24.0f, 24.0f, 0.5f, 2.0f);
    const float hitR = 15.0f * gainScale;

    juce::Point<float> L (b.getX() + leftPt  * b.getWidth(),  b.getY() + (1.0f - pt.second) * b.getHeight());
    juce::Point<float> R (b.getX() + rightPt * b.getWidth(),  b.getY() + (1.0f - pt.second) * b.getHeight());

    if (pos.getDistanceFrom (L) < hitR) return 1;
    if (pos.getDistanceFrom (R) < hitR) return 2;
    return 0;
}

void XYPad::drag (const juce::MouseEvent& e)
{
    auto r = getLocalBounds().toFloat().reduced (40.0f);
    float x01 = juce::jlimit (0.0f, 1.0f, (e.position.x - r.getX()) / r.getWidth());
    float y01 = juce::jlimit (0.0f, 1.0f, 1.0f - (e.position.y - r.getY()) / r.getHeight());

    if (snapEnabled)
    {
        x01 = std::round (x01 * 20.0f) / 20.0f;
        y01 = std::round (y01 * 10.0f) / 10.0f;
    }

    if (isSplitMode)
    {
        if (isLinked)
        {
            leftPt = rightPt = x01;
            pt.second = y01;
            if (onSplitChange) onSplitChange (leftPt, rightPt, y01);
        }
        else
        {
            if (activeBall == 0) activeBall = getBallAtPosition (e.position, r);
            if (activeBall == 1) { leftPt  = x01; pt.second = y01; if (onBallChange) onBallChange (1, leftPt,  y01); }
            if (activeBall == 2) { rightPt = x01; pt.second = y01; if (onBallChange) onBallChange (2, rightPt, y01); }
            if (onSplitChange) onSplitChange (leftPt, rightPt, pt.second);
        }
    }
    else
    {
        pt = { x01, y01 };
        if (onChange) onChange (x01, y01);
    }

    repaint();
}
void XYPad::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    const float rad = 8.0f;

    // panel
    if (auto* lfPanel = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        g.setColour (lfPanel->theme.panel);
    else
        g.setColour (juce::Colours::darkgrey);
    g.fillRoundedRectangle (r.reduced (3.0f), rad);

    // depth (softer to avoid visible top/bottom bars)
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    juce::DropShadow ds1 ((lf ? lf->theme.shadowDark  : juce::Colours::black).withAlpha (0.35f), 12, { -1, -1 });
    juce::DropShadow ds2 ((lf ? lf->theme.shadowLight : juce::Colours::grey).withAlpha (0.25f),  6, { -1, -1 });
    auto ri = r.reduced (3.0f).getSmallestIntegerContainer();
    ds1.drawForRectangle (g, ri);
    ds2.drawForRectangle (g, ri);

    // rim (lighter)
    g.setColour ((lf ? lf->theme.sh : juce::Colours::black).withAlpha (0.18f));
    g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 0.8f);

    // hover halo
    const auto accent = (lf ? lf->theme.accent : juce::Colours::lightblue);
    auto border = r.reduced (2.0f);
    const bool over = isMouseOverOrDragging();
    g.setColour (accent);
    g.drawRoundedRectangle (border, rad, 2.0f);
    if (over || hoverActive)
    {
        for (int i = 1; i <= 8; ++i)
        {
            const float t = (float) i / 8.0f;
            const float expand = 3.0f + t * 10.0f;
            g.setColour (accent.withAlpha ((1.0f - t) * (isGreenMode ? 0.25f : 0.22f)));
            g.drawRoundedRectangle (border.expanded (expand), rad + expand * 0.4f, 2.0f);
        }
        g.setColour (accent);
        g.drawRoundedRectangle (border, rad, 2.0f);
    }

    // Standard border treatment: accent border (reduced brightness for XY pad)
    g.setColour (accent.withAlpha (0.3f));
    g.drawRoundedRectangle (r, rad, 1.0f);

    // inner content
    auto padBounds = r.reduced (40.0f);

    drawWaveformBackground (g, padBounds);
    drawGrid              (g, padBounds);
    drawFrequencyRegions  (g, padBounds);
    drawEQCurves          (g, padBounds);
    drawBalls             (g, padBounds);
    drawImagingOverlays   (g, padBounds);

    // center crosshair (subtle)
    g.setColour ((lf ? lf->theme.textMuted : juce::Colours::white).withAlpha (0.4f));
    g.drawLine (r.getCentreX(), r.getY() + 40, r.getCentreX(), r.getBottom() - 40, 1.5f);
    g.drawLine (r.getX() + 40, r.getCentreY(), r.getRight() - 40, r.getCentreY(), 1.5f);
}

void XYPad::drawImagingOverlays (juce::Graphics& g, juce::Rectangle<float> b)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto gridCol = lf ? lf->theme.hl.withAlpha (0.30f)    : juce::Colours::grey.withAlpha (0.30f);
    auto textCol = lf ? lf->theme.textMuted.withAlpha(.8f): juce::Colours::lightgrey.withAlpha(.8f);
    auto acc     = lf ? lf->theme.accent.withAlpha(0.85f) : juce::Colours::lightblue.withAlpha(0.85f);

    auto xAtHz = [&] (float hz)
    {
        const float minHz = 20.0f, maxHz = 20000.0f;
        float t = (float) (std::log10 (juce::jlimit(minHz, maxHz, hz) / minHz) / std::log10 (maxHz / minHz));
        return juce::jmap (t, 0.0f, 1.0f, b.getX(), b.getRight());
    };

    // 1) Three-band crossovers (clamped like DSP)
    {
        float lo = juce::jlimit (40.0f, 400.0f, xoverLoHz);
        float hi = juce::jlimit (800.0f, 6000.0f, xoverHiHz);
        if (hi <= lo) hi = juce::jlimit (lo + 10.0f, 6000.0f, hi);
        const float xLo = xAtHz (lo);
        const float xHi = xAtHz (hi);

        auto drawDashed = [&] (float x)
        {
            juce::Path p; p.startNewSubPath (x, b.getY()); p.lineTo (x, b.getBottom());
            const float dashes[] = { 5.0f, 4.0f };
            juce::Path dashed; juce::PathStrokeType (1.2f).createDashedStroke (dashed, p, dashes, 2);
            // Slightly brighter guides
            g.setColour (gridCol.withAlpha (0.8f));
            g.strokePath (dashed, juce::PathStrokeType (1.6f));
        };
        drawDashed (xLo);
        drawDashed (xHi);

        // band tags with accent-blue backgrounds at the top of the pad
        auto loRect  = juce::Rectangle<float> (b.getX(), b.getY() + 4.0f, xLo - b.getX(), 16.0f);
        auto midRect = juce::Rectangle<float> (xLo,      b.getY() + 4.0f, xHi - xLo,      16.0f);
        auto hiRect  = juce::Rectangle<float> (xHi,      b.getY() + 4.0f, b.getRight()-xHi,16.0f);

        auto badge = [&] (juce::Rectangle<float> r, const juce::String& txt)
        {
            auto bg = (lf ? lf->theme.accent : juce::Colours::cornflowerblue).withAlpha (0.35f);
            g.setColour (bg);
            g.fillRoundedRectangle (r.reduced (2.0f), 6.0f);
            g.setColour (bg.darker (0.35f));
            g.drawRoundedRectangle (r.reduced (2.0f), 6.0f, 1.0f);
            g.setColour (lf ? lf->theme.text : juce::Colours::white);
            g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
            g.drawText (txt, r, juce::Justification::centred);
        };
        badge (loRect,  "LO");
        badge (midRect, "MID");
        badge (hiRect,  "HI");
    }

    // 2) True M/S rotation renderer (energy circle + rotated basis + S-curve)
    if (lf)
    {
        // Slightly smaller ring: 45% of pad height is the radius
        const float radius = b.getHeight() * 0.45f;
        const float side   = radius * 2.0f;
        auto rotRect = juce::Rectangle<float> (0.0f, 0.0f, side, side).withCentre (b.getCentre());
        lf->drawRotationPad (g, rotRect, rotationDeg, asym,
                             lf->theme.accent, lf->theme.text, lf->theme.panel);
    }

    // 4) Shuffler width strip (bottom, 3x taller)
    {
        auto band = b.removeFromBottom (36.0f);
        const float xX = xAtHz (juce::jlimit (150.0f, 2000.0f, shufXHz));

        auto widthH = [&] (float pct)
        {
            pct = juce::jlimit (50.0f, 200.0f, pct);
            return juce::jmap (pct, 50.0f, 200.0f, band.getHeight() * 0.2f, band.getHeight());
        };

        // baseline @100%
        g.setColour (gridCol.withAlpha (0.9f));
        const float yBase = band.getBottom() - widthH (100.0f);
        g.drawLine (band.getX(), yBase, band.getRight(), yBase, 1.0f);

        // left segment (Lo%)
        g.setColour (acc.withAlpha (0.25f));
        g.fillRect (juce::Rectangle<float> (band.getX(), band.getBottom() - widthH (shufLoPct), xX - band.getX(), widthH (shufLoPct)));
        // right segment (Hi%)
        g.setColour (acc.withAlpha (0.35f));
        g.fillRect (juce::Rectangle<float> (xX, band.getBottom() - widthH (shufHiPct), band.getRight() - xX, widthH (shufHiPct)));

        // crossover tick and optional full-height dotted discovery guide
        g.setColour (gridCol.withAlpha (0.8f));
        g.drawLine (xX, band.getY(), xX, band.getBottom(), 1.0f);
    }
}
// ---- grid / frequency regions / EQ / balls ----
// Minimal implementations to satisfy drawing helpers used by XYPad::paint
void XYPad::drawGrid (juce::Graphics& g, juce::Rectangle<float> b)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto border = lf ? lf->theme.sh : juce::Colours::darkgrey;
    const auto grid   = lf ? lf->theme.hl.withAlpha (0.22f) : juce::Colours::grey.withAlpha (0.22f);
    const auto sub    = lf ? lf->theme.hl.withAlpha (0.10f) : juce::Colours::grey.withAlpha (0.10f);

    g.setColour (border);
    g.drawRoundedRectangle (b, 6.0f, 1.0f);

    // Pan subgrid (every 5 units across -50..0..+50)
    // Map -50..+50 to left..right; ticks every 5
    for (int p = -50; p <= 50; p += 5)
    {
        const float t = (float) (p + 50) / 100.0f; // 0..1
        const float x = juce::jmap (t, 0.0f, 1.0f, b.getX(), b.getRight());
        g.setColour ((p % 10 == 0) ? grid : sub);
        g.drawLine (x, b.getY(), x, b.getBottom(), (p % 10 == 0) ? 1.0f : 0.6f);
        // Top/bottom tick marks every 5 for extra legibility
        if (p % 10 != 0)
        {
            const float tickH = 6.0f;
            g.setColour (sub);
            g.drawLine (x, b.getY(),              x, b.getY() + tickH,        1.0f);
            g.drawLine (x, b.getBottom() - tickH, x, b.getBottom(),           1.0f);
        }
        if (p % 10 == 0)
        {
            // labels at top
            juce::String lbl;
            if (p < 0)      lbl = juce::String (std::abs(p)) + "L";
            else if (p > 0) lbl = juce::String (p) + "R";
            else            lbl = "0";
            g.setColour (lf ? lf->theme.textMuted.withAlpha (0.8f) : juce::Colours::lightgrey.withAlpha (0.8f));
            g.setFont (juce::Font (juce::FontOptions (10.0f).withStyle ("Bold")));
            g.drawText (lbl, juce::Rectangle<int> ((int) (x - 14), (int) (b.getY() - 14), 28, 12), juce::Justification::centred);
        }
    }
    // A few horizontal guides (quarters)
    for (int j = 1; j < 4; ++j)
    {
        const float y = juce::jmap ((float) j, 0.0f, 4.0f, b.getY(), b.getBottom());
        g.setColour (j == 2 ? grid : sub);
        g.drawLine (b.getX(), y, b.getRight(), y, j == 2 ? 1.0f : 0.6f);
    }

    // Frequency scale markers (low→high left-to-right)
    g.setColour (lf ? lf->theme.textMuted.withAlpha (0.35f) : juce::Colours::white.withAlpha (0.35f));
    const float yLabel = b.getBottom() + 12.0f;
    auto drawHz = [&] (float hz)
    {
        const float t = (float) (std::log10 (hz / 20.0f) / 3.0);
        const float x = juce::jmap (juce::jlimit (0.0f, 1.0f, t), 0.0f, 1.0f, b.getX(), b.getRight());
        g.drawLine (x, b.getBottom(), x, b.getBottom() - 6.0f, 1.0f);
        juce::String label;
        if      (hz >= 1000.0f) label = juce::String (hz / 1000.0f, 1) + "k";
        else                    label = juce::String ((int) hz);
        g.drawText (label, juce::Rectangle<int> ((int) x - 20, (int) yLabel, 40, 12), juce::Justification::centred);
    };
    for (float hz : { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f })
        drawHz (hz);
}

void XYPad::drawFrequencyRegions (juce::Graphics& g, juce::Rectangle<float> b)
{
    // Shaded log-spaced bands to differentiate Hz regions
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto base1 = lf ? lf->theme.base : juce::Colours::darkgrey;
    auto base2 = lf ? lf->theme.panel: juce::Colours::grey;
    base1 = base1.withAlpha (0.06f);
    base2 = base2.withAlpha (0.10f);

    const float minHz = 20.0f, maxHz = 20000.0f;
    auto xAtHz = [&] (float hz)
    {
        const float t = (float) (std::log10 (juce::jlimit (minHz, maxHz, hz) / minHz) / std::log10 (maxHz / minHz));
        return juce::jmap (t, 0.0f, 1.0f, b.getX(), b.getRight());
    };

    // Define region boundaries (approx): 20, 60, 200, 800, 3k, 8k, 20k
    float marks[] = { 20.0f, 60.0f, 200.0f, 800.0f, 3000.0f, 8000.0f, 20000.0f };
    for (int i = 0; i < 6; ++i)
    {
        float x1 = xAtHz (marks[i]);
        float x2 = xAtHz (marks[i+1]);
        auto region = juce::Rectangle<float> (x1, b.getY(), x2 - x1, b.getHeight());
        g.setColour ((i % 2 == 0) ? base1 : base2);
        g.fillRect (region);
    }
}

void XYPad::drawWaveformBackground (juce::Graphics& g, juce::Rectangle<float> b)
{
    if (!hasWaveformData) return;
    const int N = waveformBufferSize;
    const int stride = 2; // downsample for slower, more readable motion
    const int P = juce::jmax (2, (N - 1) / stride + 1);
    const float dx = b.getWidth() / (float) (P - 1);

    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto leftCol  = lf ? lf->theme.accent.withAlpha (0.40f) : juce::Colours::lightblue.withAlpha (0.40f);
    const auto rightCol = lf ? lf->theme.text.withAlpha (0.35f)   : juce::Colours::white.withAlpha (0.35f);

    auto drawBuffer = [&] (const std::array<double, waveformBufferSize>& buf, juce::Colour col)
    {
        juce::Path p;
        p.preallocateSpace (P * 3);
        float x = b.getX();
        // Left-to-right: oldest on left, newest on right
        const int startIdx = waveformWriteIndex; // oldest sample position
        int pointIndex = 0;
        for (int i = 0; i < N; i += stride)
        {
            const int idx = (startIdx + i) % N;
            const float y = juce::jmap ((float) buf[(size_t) idx], -1.0f, 1.0f, b.getBottom(), b.getY());
            if (pointIndex == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
            x += dx;
            ++pointIndex;
        }
        // glow: outer soft + core line
        g.setColour (col.withAlpha (0.15f));
        g.strokePath (p, juce::PathStrokeType (10.0f));
        g.setColour (col.withAlpha (0.30f));
        g.strokePath (p, juce::PathStrokeType (5.0f));
        g.setColour (col.withAlpha (0.75f));
        g.strokePath (p, juce::PathStrokeType (1.2f));
    };

    drawBuffer (waveformL, leftCol);
    drawBuffer (waveformR, rightCol);
}

namespace VizEQ {
struct Biquad {
    double b0=1, b1=0, b2=0, a1=0, a2=0;
    inline void normalize(double a0){ b0/=a0; b1/=a0; b2/=a0; a1/=a0; a2/=a0; }
    inline double magDB(double w) const {
        const double c1=std::cos(w), s1=std::sin(w);
        const double c2=std::cos(2*w), s2=std::sin(2*w);
        const double NR=b0 + b1*c1 + b2*c2;
        const double NI=     b1*s1 + b2*s2;
        const double DR=1.0 + a1*c1 + a2*c2;
        const double DI=     a1*s1 + a2*s2;
        const double m2=(NR*NR+NI*NI)/(DR*DR+DI*DI);
        return 20.0*std::log10(std::max(1e-12, std::sqrt(m2)));
    }
};
constexpr double kPI = juce::MathConstants<double>::pi;
constexpr double kSqrt2Inv = 0.7071067811865476;
inline Biquad lowpassRBJ(double Fs, double f0, double Q=kSqrt2Inv){
    const double w0=2.0*kPI*f0/Fs, c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/(2.0*Q);
    double b0=(1.0-c)*0.5, b1=1.0-c, b2=(1.0-c)*0.5;
    double a0=1.0+alpha, a1=-2.0*c, a2=1.0-alpha;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b; }
inline Biquad highpassRBJ(double Fs, double f0, double Q=kSqrt2Inv){
    const double w0=2.0*kPI*f0/Fs, c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/(2.0*Q);
    double b0=(1.0+c)*0.5, b1=-(1.0+c), b2=(1.0+c)*0.5;
    double a0=1.0+alpha, a1=-2.0*c, a2=1.0-alpha;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b; }
inline Biquad lowshelfRBJ(double Fs,double f0,double GdB,double S=1.0){
    const double A=std::pow(10.0,GdB/40.0); const double w0=2.0*kPI*f0/Fs; const double c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/2.0 * std::sqrt((A + 1.0/A)*(1.0/S - 1.0) + 2.0);
    const double twoRtA_alpha = 2.0*std::sqrt(A)*alpha;
    double b0=A*((A+1)-(A-1)*c + twoRtA_alpha);
    double b1=2*A*((A-1)-(A+1)*c);
    double b2=A*((A+1)-(A-1)*c - twoRtA_alpha);
    double a0=   (A+1)+(A-1)*c + twoRtA_alpha;
    double a1=-2*((A-1)+(A+1)*c);
    double a2=   (A+1)+(A-1)*c - twoRtA_alpha;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b; }
inline Biquad highshelfRBJ(double Fs,double f0,double GdB,double S=1.0){
    const double A=std::pow(10.0,GdB/40.0); const double w0=2.0*kPI*f0/Fs; const double c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/2.0 * std::sqrt((A + 1.0/A)*(1.0/S - 1.0) + 2.0);
    const double twoRtA_alpha = 2.0*std::sqrt(A)*alpha;
    double b0=A*((A+1)+(A-1)*c + twoRtA_alpha);
    double b1=-2*A*((A-1)+(A+1)*c);
    double b2=A*((A+1)+(A-1)*c - twoRtA_alpha);
    double a0=   (A+1)-(A-1)*c + twoRtA_alpha;
    double a1= 2*((A-1)-(A+1)*c);
    double a2=   (A+1)-(A-1)*c - twoRtA_alpha;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b; }
inline Biquad peakingRBJ_Q(double Fs,double f0,double GdB,double Q){
    const double A=std::pow(10.0,GdB/40.0); const double w0=2.0*kPI*f0/Fs; const double c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/(2.0*std::max(1e-6,Q));
    double b0=1.0+alpha*A, b1=-2.0*c, b2=1.0-alpha*A;
    double a0=1.0+alpha/A, a1=-2.0*c, a2=1.0-alpha/A;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b; }
inline double softPix(double dB, double knee=6.0, double dBmax=18.0){
    const double s = (1.0 - std::exp(-std::abs(dB)/knee)) * dBmax;
    return dB>=0.0 ? s : -s;
}
} // namespace VizEQ
void XYPad::drawEQCurves (juce::Graphics& g, juce::Rectangle<float> b)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto hpLpCol   = lf ? lf->theme.eq.hp     : juce::Colours::lightblue;
    const auto airCol    = lf ? lf->theme.eq.air    : juce::Colours::white;
    const auto tiltCol   = lf ? lf->theme.eq.tilt   : juce::Colours::orange;
    const auto bassCol   = lf ? lf->theme.eq.bass   : juce::Colours::green;
    const auto scoopCol  = lf ? lf->theme.eq.scoop  : juce::Colours::purple;
    const auto monoShade = lf ? lf->theme.eq.monoShade : juce::Colours::black.withAlpha (0.15f);

    // Shade mono region
    if (monoHzValue > 20.0f)
    {
        const float minHz = 20.0f, maxHz = 20000.0f;
        const float t = (float) (std::log10 (juce::jlimit (minHz, maxHz, monoHzValue) / minHz) / std::log10 (maxHz / minHz));
        const float xMono = juce::jmap (t, 0.0f, 1.0f, b.getX(), b.getRight());
        g.setColour (monoShade);
        g.fillRect (juce::Rectangle<float> (b.getX(), b.getY(), xMono - b.getX(), b.getHeight()));
    }

    // High-res sampling along width for smooth curves
    const int N = juce::jmax (192, (int) b.getWidth());
    auto freqAt = [] (float t01)
    {
        const float minHz = 20.0f, maxHz = 20000.0f;
        return minHz * std::pow (maxHz / minHz, t01);
    };

    auto toY = [&] (float db)
    {
        // Slightly more exaggerated for visibility
        const float scale = 6.0f;
        return juce::jlimit (b.getY(), b.getBottom(), b.getCentreY() - db * scale);
    };

    juce::Path hpLp, hpFill, lpFill, airP, airFill, tiltP, tiltFill, bassP, bassFill, scoopP, scoopFill;
    // Reserve to reduce per-frame allocations in paint
    const int reservePts = juce::jmax (N * 3, 256);
    hpLp.preallocateSpace (reservePts);
    hpFill.preallocateSpace (reservePts);
    lpFill.preallocateSpace (reservePts);
    airP.preallocateSpace (reservePts);
    airFill.preallocateSpace (reservePts);
    tiltP.preallocateSpace (reservePts);
    tiltFill.preallocateSpace (reservePts);
    bassP.preallocateSpace (reservePts);
    bassFill.preallocateSpace (reservePts);
    scoopP.preallocateSpace (reservePts);
    scoopFill.preallocateSpace (reservePts);
    for (int i = 0; i < N; ++i)
    {
        const float t01 = (float) i / (float) (N - 1);
        const float x    = juce::jmap (t01, b.getX(), b.getRight());
        const float hz   = freqAt (t01);

        // Build RBJ biquads for current parameters
        const double Fs = vizSampleRate > 0.0 ? vizSampleRate : 48000.0;
        // Skip HP/LP visual influence when they are at neutral extremes to avoid phantom curvature at 20/20000
        const bool hpNeutral = hpValue <= 20.0f;
        const bool lpNeutral = lpValue >= 20000.0f;
        // Shelves use S; Tilt optionally inherits S
        auto bBass  = VizEQ::lowshelfRBJ (Fs, juce::jlimit (20.0f, 20000.0f, bassFreqValue),  bassValue,  shelfShapeS);
        auto bAir   = VizEQ::highshelfRBJ(Fs, juce::jlimit (20.0f, 20000.0f, airFreqValue),   airValue,   shelfShapeS);
        const float tiltS = tiltUsesShelfS ? shelfShapeS : 0.90f;
        auto bTiltLo= VizEQ::lowshelfRBJ (Fs, juce::jlimit (20.0f, 20000.0f, tiltFreqValue), +0.5f*tiltValue, tiltS);
        auto bTiltHi= VizEQ::highshelfRBJ(Fs, juce::jlimit (20.0f, 20000.0f, tiltFreqValue), -0.5f*tiltValue, tiltS);
        
        // HP/LP use Q (global or per-filter); neutral at extremes uses sqrt2 for flatness
        const double qHP = qLink ? (double) filterQGlobal : (double) hpQ;
        const double qLP = qLink ? (double) filterQGlobal : (double) lpQ;
        auto bHP    = hpNeutral ? VizEQ::highpassRBJ(Fs, 20.0, VizEQ::kSqrt2Inv)
                                : VizEQ::highpassRBJ (Fs, juce::jlimit (20.0f, 20000.0f, hpValue), qHP);
        auto bLP    = lpNeutral ? VizEQ::lowpassRBJ (Fs, 20000.0, VizEQ::kSqrt2Inv)
                                : VizEQ::lowpassRBJ  (Fs, juce::jlimit (20.0f, 20000.0f, lpValue), qLP);
        // Peak bell for scoop/boost: adapt Q from shelf shape S (wider at low S, tighter at high S)
        const double qPeak = juce::jlimit (0.5, 2.0, juce::jmap ((double) shelfShapeS, 0.25, 1.50, 0.5, 2.0));
        auto bPeak  = VizEQ::peakingRBJ_Q(Fs, juce::jlimit (20.0f, 20000.0f, scoopFreqValue), scoopValue, qPeak);

        const double w = 2.0 * juce::MathConstants<double>::pi * (double)hz / Fs;
        const float hpDb   = hpNeutral ? 0.0f : (float) bHP.magDB (w);
        const float lpDb   = lpNeutral ? 0.0f : (float) bLP.magDB (w);
        const float airDb  = (float) bAir.magDB(w);
        const float bassDb = (float) bBass.magDB(w);
        const float tiltDb = (float) (bTiltLo.magDB(w) + bTiltHi.magDB(w));
        const float scoopDb= (float) bPeak.magDB(w);

        const float yHP  = toY ((float) VizEQ::softPix (hpDb + lpDb));
        const float yAir = toY ((float) VizEQ::softPix (airDb));
        const float yTlt = toY ((float) VizEQ::softPix (tiltDb));
        const float yBas = toY ((float) VizEQ::softPix (bassDb));
        const float yScp = toY ((float) VizEQ::softPix (scoopDb));

        if (i == 0)
        {
            hpLp .startNewSubPath (x, yHP);
            airP .startNewSubPath (x, yAir);
            tiltP.startNewSubPath (x, yTlt);
            bassP.startNewSubPath (x, yBas);
            scoopP.startNewSubPath (x, yScp);
            // start fill paths along top to draw vertical gradient later
            hpFill .startNewSubPath (x, yHP);
            lpFill .startNewSubPath (x, yHP);
            airFill.startNewSubPath (x, yAir);
            tiltFill.startNewSubPath (x, yTlt);
            bassFill.startNewSubPath (x, yBas);
            scoopFill.startNewSubPath (x, yScp);
        }
        else
        {
            hpLp .lineTo (x, yHP);
            airP .lineTo (x, yAir);
            tiltP.lineTo (x, yTlt);
            bassP.lineTo (x, yBas);
            scoopP.lineTo (x, yScp);
            hpFill .lineTo (x, yHP);
            lpFill .lineTo (x, yHP);
            airFill.lineTo (x, yAir);
            tiltFill.lineTo (x, yTlt);
            bassFill.lineTo (x, yBas);
            scoopFill.lineTo (x, yScp);
        }
    }

    auto stroke = [&] (const juce::Path& path, juce::Colour base)
    {
        g.setColour (base.withAlpha (0.12f)); g.strokePath (path, juce::PathStrokeType (10.0f));
        g.setColour (base.withAlpha (0.28f)); g.strokePath (path, juce::PathStrokeType (5.0f));
        g.setColour (base.withAlpha (0.95f)); g.strokePath (path, juce::PathStrokeType (2.0f));
    };

    // Close fills to bottom for gradient area and draw subtle vertical gradients per-curve
    auto fillGradient = [&] (juce::Path& topPath, juce::Colour base)
    {
        juce::Path fill = topPath;
        fill.lineTo (b.getRight(), b.getBottom());
        fill.lineTo (b.getX(),     b.getBottom());
        fill.closeSubPath();
        juce::ColourGradient grad (base.withAlpha (0.25f), b.getX(), b.getY(), base.withAlpha (0.02f), b.getX(), b.getBottom(), false);
        g.setGradientFill (grad);
        g.fillPath (fill);
    };

    fillGradient (hpFill,  hpLpCol);
    fillGradient (bassFill,bassCol);
    fillGradient (airFill, airCol);
    fillGradient (tiltFill,tiltCol);
    fillGradient (scoopFill,scoopCol);

    stroke (hpLp,  hpLpCol);
    stroke (bassP, bassCol);
    stroke (airP,  airCol);
    // dashed tilt
    {
        juce::Path dashed;
        const float dashes[] = { 6.0f, 4.0f };
        juce::PathStrokeType (2.0f).createDashedStroke (dashed, tiltP, dashes, 2);
        g.setColour (tiltCol.withAlpha (0.12f)); g.strokePath (dashed, juce::PathStrokeType (10.0f));
        g.setColour (tiltCol.withAlpha (0.25f)); g.strokePath (dashed, juce::PathStrokeType (5.0f));
        g.setColour (tiltCol.withAlpha (0.95f)); g.strokePath (dashed, juce::PathStrokeType (2.0f));
    }
    stroke (scoopP, scoopCol);

    // Mono cutoff visualization (filter-accurate shading with stronger distinction and side-curve)
    if (monoHzValue > 20.0f)
    {
        auto* lf2 = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        const auto baseEq = lf2 ? lf2->theme.eq.hp : juce::Colours::lightblue;

        const float minHz = 20.0f, maxHz = 20000.0f;
        const float Fc = juce::jlimit (minHz, maxHz, monoHzValue);

        // Slope order from 6/12/24 -> 1/2/4
        const int order = (monoSlopeDbPerOct <= 6 ? 1 : monoSlopeDbPerOct <= 12 ? 2 : 4);
        const juce::Colour tint = (order == 1 ? baseEq.brighter (0.25f)
                                              : order == 2 ? baseEq
                                                           : baseEq.darker (0.25f));

        auto xAtHz = [&] (float hz)
        {
            const float t = (float) (std::log10 (hz / minHz) / std::log10 (maxHz / minHz));
            return juce::jmap (juce::jlimit (0.0f, 1.0f, t), 0.0f, 1.0f, b.getX(), b.getRight());
        };

        // Shade by Butterworth magnitude: |H(jw)| = 1/sqrt(1+(w/wc)^(2N))
        // Increase distinction between orders by non-linear alpha mapping
        const int cols = juce::jmax (192, (int) b.getWidth());
        juce::Path sideCurve; bool sideStarted = false;
        for (int i = 0; i < cols; ++i)
        {
            const float t01 = (float) i / (float) (cols - 1);
            const float hz  = 20.0f * std::pow (1000.0f, t01 * 3.0f);
            const float ratio = hz / Fc;
            const float mag = 1.0f / std::sqrt (1.0f + std::pow (juce::jmax (ratio, 1.0e-6f), (float) (2 * order)));

            // Mono weight ~ |H_lp|. Use exponent and scaling per-order to exaggerate visual separation
            const float monoWeight = mag; // 0..1
            const float shape = (order == 1 ? 0.85f : order == 2 ? 1.10f : 1.45f);
            const float alpha = juce::jlimit (0.0f, 1.0f, 0.06f + 0.70f * std::pow (monoWeight, shape));

            const float x = xAtHz (hz);
            g.setColour (tint.withAlpha (alpha * (hz <= Fc ? 0.85f : 0.6f)));
            g.fillRect (juce::Rectangle<float> (x, b.getY(), 2.0f, b.getHeight()));

            // Optional dashed curve: stereo width multiplier ~ |1 - H_lp|
            const float sideWeight = juce::jlimit (0.0f, 1.0f, 1.0f - mag);
            const float y = b.getY() + (1.0f - sideWeight) * b.getHeight();
            if (!sideStarted) { sideCurve.startNewSubPath (x, y); sideStarted = true; }
            else              { sideCurve.lineTo (x, y); }
        }

        const float xFc = xAtHz (Fc);
        g.setColour (tint.withAlpha (0.80f));
        g.drawLine (xFc, b.getY(), xFc, b.getBottom(), 1.4f);

        // Draw dashed side curve on top
        {
            juce::Path dashed;
            const float dashes[] = { 6.0f, 4.0f };
            juce::PathStrokeType (2.0f).createDashedStroke (dashed, sideCurve, dashes, 2);
            g.setColour (tint.withAlpha (0.55f));
            g.strokePath (dashed, juce::PathStrokeType (1.8f));
        }
    }

    // dB scale labels on left (match curve pixel mapping using softPix)
    {
        g.setColour ((lf ? lf->theme.textMuted : juce::Colours::lightgrey).withAlpha (0.8f));
        g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
        const float dBs[] = { +18.0f, +12.0f, +6.0f, 0.0f, -6.0f, -12.0f, -18.0f };
        for (float d : dBs)
        {
            // Use the same visual mapping as curves so peaks align with tick longitudes
            const float y = toY ((float) VizEQ::softPix ((double) d, 6.0, 18.0));
            g.drawText (juce::String ((int) d) + " dB", juce::Rectangle<int> ((int) b.getX() - 44, (int) (y - 7), 40, 14), juce::Justification::centredRight);
            // small tick
            g.fillRect (juce::Rectangle<float> (b.getX() - 6.0f, y - 0.5f, 4.0f, 1.0f));
        }
    }
}

void XYPad::drawBalls (juce::Graphics& g, juce::Rectangle<float> b)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto accent = lf ? lf->theme.accent : juce::Colours::lightblue;
    const auto text   = lf ? lf->theme.text   : juce::Colours::white;

    const float r = 8.0f;
    const float cx = b.getX() + pt.first * b.getWidth();
    const float cy = b.getY() + (1.0f - pt.second) * b.getHeight();

    if (!isSplitMode)
    {
        g.setColour ((lf ? lf->theme.shadowDark : juce::Colours::black).withAlpha (0.4f));
        g.fillEllipse (cx - r + 2.0f, cy - r + 2.0f, r * 2.0f, r * 2.0f);
        g.setColour (accent);
        g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);
        g.setColour (text.withAlpha (0.7f));
        g.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 1.2f);
        // Reverb depth rings: subtle expanding rings based on spaceValue
        if (spaceValue > 0.001f)
        {
            const float maxRadius = r * (1.0f + 1.5f * spaceValue);
            const int rings = 3;
            for (int i = 1; i <= rings; ++i)
            {
                const float t = (float) i / (float) rings;
                const float rr = juce::jmap (t, 0.0f, 1.0f, r * 1.2f, maxRadius);
                g.setColour (accent.withAlpha (0.18f * (1.0f - t)));
                g.drawEllipse (cx - rr, cy - rr, rr * 2.0f, rr * 2.0f, 1.2f);
            }
        }
        return;
    }

    // Split mode: left/right balls
    const float lx = b.getX() + leftPt  * b.getWidth();
    const float rx = b.getX() + rightPt * b.getWidth();
    const float y  = cy;

    g.setColour ((lf ? lf->theme.shadowDark : juce::Colours::black).withAlpha (0.4f));
    g.setColour ((lf ? lf->theme.shadowDark : juce::Colours::black).withAlpha (0.4f));
    g.fillEllipse (lx - r + 2.0f, y - r + 2.0f, r * 2.0f, r * 2.0f);
    g.fillEllipse (rx - r + 2.0f, y - r + 2.0f, r * 2.0f, r * 2.0f);

    g.setColour (accent);
    g.fillEllipse (lx - r, y - r, r * 2.0f, r * 2.0f);
    g.setColour (text.withAlpha (0.7f));
    g.fillEllipse (rx - r, y - r, r * 2.0f, r * 2.0f);

    g.setColour (text.withAlpha (0.7f));
    g.drawEllipse (lx - r, y - r, r * 2.0f, r * 2.0f, 1.2f);
    g.drawEllipse (rx - r, y - r, r * 2.0f, r * 2.0f, 1.2f);
}
// ------------------------------------------------
/* ===================== Editor ===================== */
MyPluginAudioProcessorEditor::MyPluginAudioProcessorEditor (MyPluginAudioProcessor& p)
: AudioProcessorEditor (&p), proc (p), presetManager (proc.apvts, nullptr)
{
    // Log: Editor constructor started
    // TEMPORARILY DISABLE file logging to test if this is causing the crash
    // juce::File f = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Field_CrashLog.txt");
    // f.appendText("Editor Ctor: STARTED\n", false, false, "\n");
    // Populate factory presets directly from app-data JSON (single source of truth)
    {
        const auto presetsFile = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                                  .getChildFile ("Field/Presets/delay_presets.json");
        if (presetsFile.existsAsFile())
        {
            const auto text = presetsFile.loadFileAsString();
            const auto root = juce::JSON::parse (text);
            auto importOne = [this](const juce::var& v)
            {
                if (! v.isObject()) return;
                if (auto* obj = v.getDynamicObject())
                {
                    LibraryPreset pr;
                    pr.meta.name        = obj->getProperty ("name").toString();
                    pr.meta.description = obj->getProperty ("desc").toString();
                    pr.meta.hint        = obj->getProperty ("hint").toString();
                    pr.meta.author      = "Factory";
                    pr.meta.category    = "Delay";
                    if (auto tags = obj->getProperty ("tags"); tags.isArray())
                        for (auto& t : *tags.getArray()) pr.meta.tags.add (t.toString());
                    if (auto params = obj->getProperty ("params"); params.isObject())
                        if (auto* pv = params.getDynamicObject())
                            for (auto& kv : pv->getProperties())
                                pr.params.set (kv.name.toString(), kv.value);
                    if (pr.meta.name.isNotEmpty()) presetStore.addFactoryPreset (pr);
                }
            };
            if (root.isArray())
            {
                for (const auto& it : *root.getArray()) importOne (it);
            }
            else if (auto* d = root.getDynamicObject())
            {
                auto arr = d->getProperty ("presets");
                if (arr.isArray()) for (const auto& it : *arr.getArray()) importOne (it);
            }
        }
    }
    presetStore.scan();
    DBG("[PresetStore] after add+scan: " << presetStore.getAll().size() << " presets");
    // Build knob cells once after all sliders/labels are created
    buildCells();
    // Calculate minimum size based on layout requirements
    const float s = 1.0f;
    const int lPx  = Layout::dp ((float) Layout::knobPx (Layout::Knob::L),  s);
    const int xlPx = Layout::dp ((float) Layout::knobPx (Layout::Knob::XL), s);
    const int swW  = Layout::dp ((int) (Layout::ALGO_SWITCH_W * Layout::ALGO_SWITCH_W_RATIO), s);
    // Calculate minimum width needed for all controls
    const int numItems = 1 + 1 + 1 + 5 + 3; // pan, space, switch, duck(5), gain/drive/mix(3)
    const int gaps = numItems - 1;
    const int gapS = Layout::dp (Layout::GAP_S, s);
    // Compute minimum delay card width based on 7 columns using same cell width and inner gap
    const int cellW_delay_min = lPx + Layout::dp (8, s);
    const int delayColsMin = 7;
    const int delayCardWMin = delayColsMin * cellW_delay_min + gapS * (delayColsMin - 1) + Layout::dp (Layout::GAP, s);
    // Compute minimum motion grid width (second divider + gap + 4 motion cells)
    const int motionColsMin = 4;
    const int motionCellWMin = lPx + Layout::dp (8, s);
    const int motionDividerWMin = Layout::dp (8, s);
    const int motionAreaWMin = motionDividerWMin + Layout::dp (Layout::GAP, s) + motionColsMin * motionCellWMin;
    const int calculatedMinWidth = xlPx + lPx + swW + 5*lPx + 3*lPx + gaps * gapS
                                   + Layout::dp (Layout::PAD, s) * 2
                                   + delayCardWMin + Layout::dp (Layout::GAP, s)
                                   + motionAreaWMin;
    
    // Calculate minimum height based on current architecture: visuals + 2x16 controls
    const int headerH = Layout::dp (50, s);
    const int xyMinH = Layout::dp (Layout::XY_MIN_H, s);
    const int metersH = Layout::dp (84, s);
    const int bottomReserveMin = Layout::dp (6, s) + Layout::dp (22, s);
    const int gapH = Layout::dp (Layout::GAP, s);
    const auto gridMetrics = ControlGridMetrics::compute (baseWidth, baseHeight);
    const int controlsHMin = gridMetrics.controlsH; // 2 rows of the flat grid
    const int calculatedMinHeight = headerH + juce::jmax (xyMinH, metersH) + gapH + controlsHMin + Layout::dp (Layout::PAD, s) + bottomReserveMin;
    
    // Store resize constraints
    // Allow some narrowing vs content width, but never below a conservative floor
    const int minWidthAllowed = juce::jmax (800, (int) std::round ((float) baseWidth * 0.5f));
    const int minWidthFloor   = Layout::BP_WIDE; // breakpoint for wide layouts (protects 4x16 flats)
    const int proposedMinW    = juce::jmin (calculatedMinWidth, minWidthAllowed);
    this->minWidth = juce::jmax (minWidthFloor, proposedMinW);
    this->minHeight = calculatedMinHeight;
    this->maxWidth = 3000;
    this->maxHeight = 2000;
    
    // Set minimum size constraints
    setResizeLimits (minWidth, minHeight, maxWidth, maxHeight);
    
    // Set initial size (respecting minimums). Prefer baseWidth over full content width.
    const int initialWidth = juce::jmax (baseWidth, minWidth);
    const int initialHeight = juce::jmax (baseHeight, calculatedMinHeight);
    setSize (initialWidth, initialHeight);
    // Initial layout deferred until layoutReady is true
    // Defer one tick to ensure LookAndFeel and attachments settle, then repaint
    juce::MessageManager::callAsync ([this]
    {
        if (spaceSwitchCell)
        {
            spaceSwitchCell->resized();
            spaceSwitchCell->repaint();
        }
        spaceAlgorithmSwitch.repaint();
        repaint();
    });
    lnf.theme.accent = juce::Colour (0xFF5AA9E6); // ocean default
    lnf.setupColours();
    setLookAndFeel (&lnf);

    // Drive editor heartbeat at 30 Hz (UI updates throttled within timer)
    startTimerHz (30);
    uiTimerHzCurrent = 30;
    addMouseListener (this, true); // receive events from all children

    // History removed

    // History group dividers removed

    // Options menu (oversampling) with per-mode tint
    addAndMakeVisible (optionsButton);
    optionsButton.onClick = [this]
    {
        const bool wasOn = optionsButton.getToggleState();
        optionsButton.setToggleState (true, juce::dontSendNotification);
        optionsButton.repaint();

        TintMenuLNFEx menuLnf; menuLnf.defaultTint = lnf.theme.accent; menuLnf.hideChecks = true;
        menuLnf.setColour (juce::PopupMenu::textColourId, lnf.theme.text);

        int curIdx = 0, numChoices = 1;
        if (auto* rp = proc.apvts.getParameter ("os_mode"))
            if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (rp)) { curIdx = cp->getIndex(); numChoices = cp->choices.size(); }

        showTintedMenu (optionsButton, menuLnf,
            // BUILD
            [this, curIdx, numChoices] (juce::PopupMenu& m, TintMenuLNFEx& lnfEx)
            {
                m.addSectionHeader ("Oversampling");
                struct Row { int id; const char* text; juce::Colour tint; bool enabled; };
                juce::Array<Row> rows;
                rows.add ({ 1, "1x (Off)",          lnf.theme.textMuted, true });
                rows.add ({ 2, "2x (High Quality)", lnf.theme.eq.bass.withAlpha (0.95f),   numChoices > 1 });   // vibrant green
                rows.add ({ 3, "4x (Ultra)",        lnf.theme.accent.withHue   (lnf.theme.accent.getHue() + 0.08f).withSaturation (0.9f), numChoices > 2 });
                rows.add ({ 4, "8x (Max)",          lnf.theme.accent.withHue   (lnf.theme.accent.getHue() - 0.08f).withBrightness (0.95f), numChoices > 3 });
                rows.add ({ 5, "16x (Extreme)",     lnf.theme.eq.scoop.withAlpha (0.95f), numChoices > 4 });    // bold magenta/plum

                lnfEx.itemTints.clear();
                for (int i = 0; i < rows.size(); ++i)
                {
                    m.addItem (rows[i].id, rows[i].text, rows[i].enabled, i == curIdx);
                    lnfEx.itemTints.add (rows[i].tint);
                }
            },
            // RESULT
            [this, wasOn, numChoices] (int r)
            {
                // restore button active state to real OS>1×
                if (auto* rp = proc.apvts.getParameter ("os_mode"))
                    if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (rp))
                        optionsButton.setToggleState (cp->getIndex() > 0, juce::dontSendNotification);
                    else optionsButton.setToggleState (wasOn, juce::dontSendNotification);
                optionsButton.repaint();

                if (r == 0) return;
                if (r < 1 || r > numChoices) return;
                if (auto* p = proc.apvts.getParameter ("os_mode"))
                {
                    const float norm = numChoices > 1 ? (float) (r - 1) / (float) (numChoices - 1) : 0.0f;
                    p->beginChangeGesture(); p->setValueNotifyingHost (norm); p->endChangeGesture();
                }
            });
    };

    // Tint Options button based on oversampling choice
    auto applyOptionsTint = [this]
    {
        int sel = 0;
        if (auto* rp = proc.apvts.getParameter ("os_mode"))
            if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (rp)) sel = cp->getIndex();
        // Map index to label and tint
        juce::Colour tint = lnf.theme.textMuted;
        juce::String label = "1x";
        switch (sel)
        {
            case 0: tint = lnf.theme.textMuted; label = "1x"; break;
            case 1: tint = lnf.theme.eq.bass.withAlpha (0.95f); label = "2x"; break;
            case 2: tint = lnf.theme.accent.withHue (lnf.theme.accent.getHue() + 0.08f).withSaturation (0.9f); label = "4x"; break;
            case 3: tint = lnf.theme.accent.withHue (lnf.theme.accent.getHue() - 0.08f).withBrightness (0.95f); label = "8x"; break;
            case 4: tint = lnf.theme.eq.scoop.withAlpha (0.95f); label = "16x"; break;
        }
        optionsButton.getProperties().set ("accentOverrideARGB", (int) tint.getARGB());
        optionsButton.getProperties().set ("iconOverrideARGB", (int) tint.getARGB());
        optionsButton.getProperties().set ("labelText", label);
        optionsButton.setToggleState (true, juce::dontSendNotification);
        optionsButton.repaint();
    };
    applyOptionsTint();
    osSelect.onChange = [this, applyOptionsTint]
    {
        applyOptionsTint();
    };
    // Also listen to APVTS os_mode directly to avoid drift
    if (!osModeParamAttach)
    {
        if (auto* p = proc.apvts.getParameter ("os_mode"))
        {
            osModeParamAttach = std::make_unique<juce::ParameterAttachment>(*p, [applyOptionsTint](float){ applyOptionsTint(); }, nullptr);
        }
    }

    // Help button → FAQ dialog
    addAndMakeVisible (helpButton);
    helpButton.onClick = [this]
    {
        struct HelpFAQComponent : public juce::Component
        {
            HelpFAQComponent (FieldLNF& l) : lnf(l)
            {
                addAndMakeVisible (text);
                text.setReadOnly (true);
                text.setMultiLine (true);
                text.setScrollbarsShown (true);
                text.setCaretVisible (false);
                text.setFont (juce::Font (juce::FontOptions (14.0f)));
                text.setText (
                    "FIELD — FAQ\n\n"
                    "Q: How do I change color modes?\n"
                    "A: Click the palette button in the header to cycle Ocean → Green → Pink → Yellow → Grey.\n\n"
                    "Q: Where are colors defined?\n"
                    "A: All colors live in FieldLookAndFeel (FieldLNF::theme). Components never hardcode colors.\n\n"
                    "Q: Why don't knobs move when I resize?\n"
                    "A: Sizing happens in resized() only; layout is responsive via Layout::dp().\n\n"
                    "Q: How do I reset a control?\n"
                    "A: Double-click most knobs/sliders to reset to default.\n\n"
                    "Q: Where are presets saved?\n"
                    "A: In your user data folder under the plugin's presets directory.\n\n"
                );
            }
            void resized() override
            {
                text.setBounds (getLocalBounds().reduced (12));
            }
            void paint (juce::Graphics& g) override
            {
                g.fillAll (lnf.theme.panel);
                g.setColour (lnf.theme.sh);
                g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 6.0f, 1.0f);
            }
            juce::TextEditor text;
            FieldLNF& lnf;
        };

        auto* content = new HelpFAQComponent (lnf);
        content->setSize (600, 400);

        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned (content);
        opts.dialogTitle = "Help / FAQ";
        opts.componentToCentreAround = this;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = true;
        (void) opts.launchAsync();
    };

    // Bypass (attach to param if present)
    addAndMakeVisible (bypassButton);
    bypassButton.onClick = [this]
    {
        if (auto* p = proc.apvts.getParameter ("bypass"))
            p->setValueNotifyingHost (bypassButton.getToggleState() ? 1.0f : 0.0f);
    };

    // History panel removed

    // Header Undo/Redo removed

    // Color mode cycle (Ocean → Green → Pink → Yellow → Grey)
    addAndMakeVisible (colorModeButton);
    addAndMakeVisible (tooltipsButton);
    colorModeButton.setTooltip (FieldLNF::getThemeName (lnf.currentVariant));
    colorModeButton.onClick = [this]
    {
        // Determine current by accent; rotate deterministically through variants
        using TV = FieldLNF::ThemeVariant;
        static TV order[] = { TV::Ocean, TV::Green, TV::Pink, TV::Yellow, TV::Grey };
        auto currentAccent = lnf.theme.accent.getARGB();
        int idx = 0;
        if (currentAccent == juce::Colour (0xFF5AA9E6).getARGB()) idx = 0; // Ocean
        else if (currentAccent == juce::Colour (0xFF5AA95A).getARGB()) idx = 1; // Green
        else if (currentAccent == juce::Colour (0xFFE91E63).getARGB()) idx = 2; // Pink
        else if (currentAccent == juce::Colour (0xFFFFC107).getARGB()) idx = 3; // Yellow
        else if (currentAccent == juce::Colour (0xFF9EA3AA).getARGB()) idx = 4; // Grey
        idx = (idx + 1) % 5;
        lnf.setTheme (order[idx]);
        colorModeButton.setTooltip (FieldLNF::getThemeName (order[idx]));
        // Propagate to components that cache green flag
        const bool greenNow = (order[idx] == TV::Green);
        spaceKnob.setGreenMode (greenNow);
        spaceAlgorithmSwitch.setGreenMode (greenNow);
        pad.setGreenMode (greenNow);
        repaint();
    };

    tooltipsButton.setTooltip ("Tooltip Assistant");
    tooltipsButton.setToggleState (tooltipAssistantOn_, juce::dontSendNotification);
    tooltipsButton.onClick = [this]
    {
        tooltipAssistantOn_ = tooltipsButton.getToggleState();
        tooltipsButton.repaint();
        tooltipBubble.setVisible (false);
    };

    // Full screen (top-level window kiosk toggle; restore original bounds)
    addAndMakeVisible (fullScreenButton);
    fullScreenButton.onClick = [this]
    {
        const bool on = fullScreenButton.getToggleState();

        if (auto* tlw = getTopLevelComponent())
        {
            if (auto* rw = dynamic_cast<juce::ResizableWindow*>(tlw))
            {
                if (on)
                {
                    // Save current window bounds to restore later
                    savedBounds = rw->getBounds();
                    rw->setFullScreen (true);
                }
                else
                {
                    rw->setFullScreen (false);
                    if (!savedBounds.isEmpty())
                        rw->setBounds (savedBounds);
                }
                return;
            }
        }

        // Fallback: if no top-level resizable window is accessible, do nothing to avoid bad states
        // Reset the toggle to off if we couldn't enter fullscreen safely
        if (on)
            fullScreenButton.setToggleState (false, juce::dontSendNotification);
    };

    // Link + Snap
    addAndMakeVisible (linkButton);
    linkButton.onClick = [this]
    {
        linkButton.setToggleState (!linkButton.getToggleState(), juce::dontSendNotification);
        pad.setLinked (linkButton.getToggleState());
    };
    addAndMakeVisible (snapButton);
    snapButton.setToggleState (false, juce::dontSendNotification); // default OFF per your note
    snapButton.onClick = [this]
    {
        const bool on = !snapButton.getToggleState();
        snapButton.setToggleState (on, juce::dontSendNotification);
        pad.setSnapEnabled (on);
    };

    // Presets UI
    // Preset field + arrows
    addAndMakeVisible (presetField);
    presetField.setButtonText ("Search presets…");
    presetField.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    presetField.setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (prevPresetButton);
    addAndMakeVisible (nextPresetButton);

    prevPresetButton.onClick = [this]
    {
        // no-op: legacy preset combo removed
    };
    nextPresetButton.onClick = [this]
    {
        // no-op: legacy preset combo removed
    };
    // Open command-palette when clicking the preset field; arrows remain for prev/next only
    presetField.onClick = [this]
    {
        static PresetRegistry presetRegistry; // lifetime across openings
        presetRegistry.reloadAll();
        PresetCommandPalette::show(
            presetRegistry, presetField,
            // Apply
            [this](const PresetEntry& e){
                // Convert NamedValueSet to APVTS via PresetManager
                LibraryPreset tmp; tmp.meta.id = e.id; tmp.meta.name = e.name; tmp.params = e.params;
                presetManager.applyPresetAtomic (tmp);
                presetNameLabel.setText (e.name, juce::dontSendNotification);
            },
            // Load to slot
            [this](const PresetEntry& e, bool toA){ LibraryPreset tmp; tmp.params = e.params; presetManager.loadToSlot (tmp, toA); },
            // Star (persist favorite)
            [reg=&presetRegistry](const PresetEntry& e, bool fav){ reg->setFavorite (e.id, fav); },
            // Save As
            [this](juce::String name, juce::StringArray tags, juce::String cat){ auto pr = presetManager.currentAsPreset (name, cat, tags, "User preset", "", "You"); presetStore.saveUserPreset (pr); presetStore.scan(); },
            presetField.getButtonText()
        );
    };

    // Removed savePresetButton handler
    // A/B + copy
    addAndMakeVisible (abButtonA);
    addAndMakeVisible (abButtonB);
    addAndMakeVisible (copyButton);
    abButtonA.setToggleState (true, juce::dontSendNotification);
    abButtonB.setToggleState (false, juce::dontSendNotification);
    abButtonA.onClick = [this]{ if (!abButtonA.getToggleState()) toggleABState(); };
    abButtonB.onClick = [this]{ if (!abButtonB.getToggleState()) toggleABState(); };
    copyButton.onClick = [this]
    {
        juce::PopupMenu m; m.addItem (1, "Copy A to B"); m.addItem (2, "Copy B to A");
        m.showMenuAsync (juce::PopupMenu::Options(), [this](int r)
        {
            if (r == 1) { copyState (true);  pasteState (false); }
            if (r == 2) { copyState (false); pasteState (true);  }
        });
    };

    // Split toggle
    addAndMakeVisible (splitToggle);
    splitToggle.onToggleChange = [this] (bool split)
    {
        pad.setSplitMode (split);
        linkButton.setVisible (split);
        panKnob.setVisible (!split);
        panValue.setVisible (!split);
        panKnobLeft .setVisible (split);
        panKnobRight.setVisible (split);
        panValueLeft .setVisible (split);
        panValueRight.setVisible (split);
        resized();
    };
    splitToggle.setToggleState (false, juce::dontSendNotification);
    linkButton.setVisible (false);
    // Multi-pane dock (XY, Dynamic EQ, Imager) + shade overlay
    panes = std::make_unique<PaneManager> (proc, proc.apvts.state, &getLookAndFeel(), pad);
    addAndMakeVisible (*panes);
    panes->setSampleRate (proc.getSampleRate());
    // keep-warm removed; no pane warm-up needed
    // Default to XY view on startup
    panes->setActive (PaneID::XY, true);
    // Spectrum removed; Dynamic EQ pane owns its analyzer styling

    xyShade = std::make_unique<ShadeOverlay> (lnf);
    addAndMakeVisible (*xyShade);
    xyShade->onAmountChanged = [this](float a)
    {
        if (panes) panes->setActiveShade (a);
        proc.apvts.state.setProperty ("ui_shade_active", a, nullptr);
    };
    if (panes && xyShade)
        xyShade->setAmount (panes->getActiveShade(), false);
    if (panes)
        panes->onActivePaneChanged = [this](PaneID){ if (xyShade) xyShade->setAmount (panes->getActiveShade(), false); };

    // Containers
    addAndMakeVisible (mainControlsContainer); mainControlsContainer.setTitle (""); mainControlsContainer.setShowBorder (false);
    addAndMakeVisible (panKnobContainer);      panKnobContainer.setTitle ("");     panKnobContainer.setShowBorder (true);
    addAndMakeVisible (spaceKnobContainer);    spaceKnobContainer.setTitle (""); spaceKnobContainer.setShowBorder (true);
    addAndMakeVisible (volumeContainer);       volumeContainer.setTitle ("");   volumeContainer.setShowBorder (true);
    // Delay container will be removed; lay out directly on right side
    // addAndMakeVisible (delayContainer);        delayContainer.setTitle ("");     delayContainer.setShowBorder (true);
    // Row containers for EQ/Image are no longer used
    addAndMakeVisible (MainContentContainer);  MainContentContainer.setTitle ("");    MainContentContainer.setShowBorder (false);
    addAndMakeVisible (rightSlidersContainer);   rightSlidersContainer.setTitle ("");     rightSlidersContainer.setShowBorder (false);
    addAndMakeVisible (metersContainer);       metersContainer.setTitle ("");         metersContainer.setShowBorder (false);
    
    
    // Add 3D vertical sliders to rightSlidersContainer
    rightSlidersContainer.addAndMakeVisible (inputSlider);
    rightSlidersContainer.addAndMakeVisible (outputSlider);
    rightSlidersContainer.addAndMakeVisible (mixSlider);
    
    
    
    // Configure sliders (ranges and values already set in VerticalSlider3D constructor)
    inputSlider.setTextValueSuffix (" dB");
    inputSlider.setLookAndFeel (&lnf);
    
    outputSlider.setTextValueSuffix (" dB");
    outputSlider.setLookAndFeel (&lnf);
    
    mixSlider.setRange (0.0, 100.0, 0.1);  // Mix slider has different range
    mixSlider.setValue (100.0);
    mixSlider.setTextValueSuffix (" %");
    mixSlider.setLookAndFeel (&lnf);
    
    // Add small units display to sliders
    inputSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 15);
    outputSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 15);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 15);
    
    // Set component names for handle labels
    inputSlider.setName ("inputSlider");
    outputSlider.setName ("outputSlider");
    mixSlider.setName ("mixSlider");
    
    // Style labels with knobcell color background
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto sh = lf ? lf->theme.sh : juce::Colour (0xFF2A2C30);
    
    auto labelStyle = [&](juce::Label& label) {
        label.setColour (juce::Label::backgroundColourId, sh.withAlpha (0.15f));
        label.setColour (juce::Label::textColourId, juce::Colours::white);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font (10.0f, juce::Font::bold));
    };
    

    // Width group (image row, bottom-right): invisible container + placeholder slots for spanning grid
    addChildComponent (widthGroupContainer);
    widthGroupContainer.setTitle ("");
    widthGroupContainer.setShowBorder (true);
    widthGroupContainer.setVisible (false);
    widthGroupContainer.setInterceptsMouseClicks (false, false);
    
    // Gain+Drive+Mix group (volume row): invisible container to horizontally arrange GAIN/DRIVE/MIX (all XL)
    addChildComponent (gainMixGroupContainer);
    gainMixGroupContainer.setTitle ("");
    gainMixGroupContainer.setShowBorder (false);
    gainMixGroupContainer.setVisible (false);
    gainMixGroupContainer.setInterceptsMouseClicks (false, false);
    addChildComponent (gainMixSlot1); gainMixSlot1.setInterceptsMouseClicks (false, false); gainMixSlot1.setVisible (false);
    addChildComponent (gainMixSlot2); gainMixSlot2.setInterceptsMouseClicks (false, false); gainMixSlot2.setVisible (false);

    // Ducking group (Depth, Attack, Release, Threshold) – invisible container
    addChildComponent (duckGroupContainer);
    duckGroupContainer.setTitle ("");
    duckGroupContainer.setShowBorder (false);
    duckGroupContainer.setVisible (false);
    duckGroupContainer.setInterceptsMouseClicks (false, false);
    addChildComponent (duckSlot1); duckSlot1.setInterceptsMouseClicks (false, false); duckSlot1.setVisible (false);
    addChildComponent (duckSlot2); duckSlot2.setInterceptsMouseClicks (false, false); duckSlot2.setVisible (false);
    addChildComponent (duckSlot3); duckSlot3.setInterceptsMouseClicks (false, false); duckSlot3.setVisible (false);
    addChildComponent (widthGroupSlot1); widthGroupSlot1.setInterceptsMouseClicks (false, false); widthGroupSlot1.setVisible (false);
    addChildComponent (widthGroupSlot2); widthGroupSlot2.setInterceptsMouseClicks (false, false); widthGroupSlot2.setVisible (false);
    addChildComponent (widthGroupSlot3); widthGroupSlot3.setInterceptsMouseClicks (false, false); widthGroupSlot3.setVisible (false);

    // Volume row unified group (pan, reverb, algo switch, duck group, gain/drive/mix)
    addChildComponent (volGroupContainer);
    volGroupContainer.setTitle("");
    volGroupContainer.setShowBorder(false);
    volGroupContainer.setVisible(false);
    volGroupContainer.setInterceptsMouseClicks(false, false);
    addChildComponent (volSlot1); volSlot1.setInterceptsMouseClicks(false,false); volSlot1.setVisible(false);
    addChildComponent (volSlot2); volSlot2.setInterceptsMouseClicks(false,false); volSlot2.setVisible(false);
    addChildComponent (volSlot3); volSlot3.setInterceptsMouseClicks(false,false); volSlot3.setVisible(false);
    addChildComponent (volSlot4); volSlot4.setInterceptsMouseClicks(false,false); volSlot4.setVisible(false);
    addChildComponent (volSlot5); volSlot5.setInterceptsMouseClicks(false,false); volSlot5.setVisible(false);
    addChildComponent (volSlot6); volSlot6.setInterceptsMouseClicks(false,false); volSlot6.setVisible(false);
    addChildComponent (volSlot7); volSlot7.setInterceptsMouseClicks(false,false); volSlot7.setVisible(false);

    // Split-pan overlay container
    addAndMakeVisible (panSplitContainer);
    panSplitContainer.setVisible (false);
    panSplitContainer.setInterceptsMouseClicks (false, false);

    // Sliders/knobs
    // NOTE: These names/IDs match your processor (original)
    auto style = [this](juce::Slider& s, bool main = false)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        // Align with FieldLNF ticks: π → π + 2π
        constexpr float kStart = juce::MathConstants<float>::pi;
        constexpr float kEnd   = juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi;
        s.setRotaryParameters (kStart, kEnd, true);
        s.setLookAndFeel (&lnf);
    };

    // main rotary
    for (juce::Slider* slider : { &width,&tilt,&monoHz,&hpHz,&lpHz,&satDrive,&satMix,&air,&bass,&scoop,
                              &widthLo,&widthMid,&widthHi,&xoverLoHz,&xoverHiHz,&rotationDeg,&asymmetry,&shufLoPct,&shufHiPct,&shufXHz,
                              &shelfShapeS,&filterQ })
    {
        addAndMakeVisible (*slider);
        style (*slider);
        slider->setVelocityBasedMode (true);
        slider->setVelocityModeParameters (0.85, 1, 0.0, true);
        slider->setMouseDragSensitivity (140);
        slider->addListener (this);
    }
    addAndMakeVisible (gain); style (gain); gain.addListener (this);

    // micro sliders (freq) + HP/LP Q minis
    for (juce::Slider* slider : { &tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider, &hpQSlider, &lpQSlider })
    {
        addAndMakeVisible (*slider);
        slider->setSliderStyle (juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider->setMouseDragSensitivity (140);
        slider->setVelocityBasedMode (false);
        slider->setSliderSnapsToMousePosition (false);
        slider->setDoubleClickReturnValue (true, 0.0);
        slider->setLookAndFeel (&lnf);
        slider->addListener (this);
    }

    // link buttons (will be placed inside a cell later)
    addAndMakeVisible (tiltLinkSButton);
    addAndMakeVisible (qLinkButton);
    tiltLinkSButton.setLookAndFeel (&lnf);
    qLinkButton.setLookAndFeel (&lnf);
    tiltLinkSButton.setButtonText ("Tilt uses S");
    qLinkButton.setButtonText (""); // visual icon-only; state-driven paint

    // Ducking advanced knobs (3 generics + custom ratio)
    for (juce::Slider* slider : { &duckAttack, &duckRelease, &duckThreshold })
    {
        addAndMakeVisible (*slider);
        slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        constexpr float kStart = juce::MathConstants<float>::pi;
        constexpr float kEnd   = juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi;
        slider->setRotaryParameters (kStart, kEnd, true);
        slider->setLookAndFeel (&lnf);
        slider->addListener (this);
    }
    addAndMakeVisible (duckRatio);
    duckRatio.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    duckRatio.setLookAndFeel (&lnf);
    duckRatio.addListener (this);

    // pan + reverb + ducking
    addAndMakeVisible (panKnob);      style (panKnob, true);     panKnob.setRange (-1.0, 1.0, 0.01); panKnob.setOverlayEnabled (false); panKnob.addListener (this);
    addAndMakeVisible (panKnobLeft);  style (panKnobLeft, true); panKnobLeft.setRange (-1.0, 1.0, 0.01); panKnobLeft.setOverlayEnabled (true);  panKnobLeft.setLabel ("L"); panKnobLeft.addListener (this);
    addAndMakeVisible (panKnobRight); style (panKnobRight, true);panKnobRight.setRange(-1.0, 1.0, 0.01); panKnobRight.setOverlayEnabled (true); panKnobRight.setLabel ("R"); panKnobRight.addListener (this);

    panKnob.setVisible (true);
    panKnobLeft.setVisible (false);
    panKnobRight.setVisible (false);

    // Legacy Group 1 reverb depth knob is no longer used; keep hidden
    style (spaceKnob, true); spaceKnob.addListener (this); spaceKnob.setVisible (false);
    addAndMakeVisible (duckingKnob); style (duckingKnob);   duckingKnob.addListener (this);

    // values
    for (juce::Label* l : { &gainValue,&widthValue,&tiltValue,&monoValue,&hpValue,&lpValue,&satDriveValue,&satMixValue,&airValue,&bassValue,&scoopValue,&shelfShapeValue,&filterQValue,
                             &panValue,&panValueLeft,&panValueRight,&spaceValue,&duckingValue,&duckAttackValue,&duckReleaseValue,&duckThresholdValue,&duckRatioValue,
                             &tiltFreqValue,&scoopFreqValue,&bassFreqValue,&airFreqValue,
                             &widthLoValue,&widthMidValue,&widthHiValue,&xoverLoValue,&xoverHiValue,
                             &rotationValue,&asymValue,&shufLoValue,&shufHiValue,&shufXValue,
                             &delayTimeValue,&delayFeedbackValue,&delayWetValue,&delaySpreadValue,&delayWidthValue,&delayModRateValue,&delayModDepthValue,&delayWowflutterValue,&delayJitterValue,
                             &delayHpValue,&delayLpValue,&delayTiltValue,&delaySatValue,&delayDiffusionValue,&delayDiffuseSizeValue,
                             &delayDuckDepthValue,&delayDuckAttackValue,&delayDuckReleaseValue,&delayDuckThresholdValue,&delayDuckRatioValue,&delayDuckLookaheadValue })
    {
        addAndMakeVisible (*l);
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::Font (juce::FontOptions (15.0f * scaleFactor).withStyle ("Bold")));
        l->setColour (juce::Label::textColourId, lnf.theme.text);
        l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    }
    // Live EQ curve updates while turning knobs
    auto addLiveRepaint = [this](juce::Slider& s)
    {
        s.onValueChange = [this]
        {
            pad.setTiltValue  ((float) tilt .getValue());
            pad.setHPValue    ((float) hpHz .getValue());
            pad.setLPValue    ((float) lpHz .getValue());
            pad.setAirValue   ((float) air  .getValue());
            pad.setBassValue  ((float) bass .getValue());
            pad.setScoopValue ((float) scoop.getValue());
            pad.setTiltFreqValue  ((float) tiltFreqSlider.getValue());
            pad.setScoopFreqValue ((float) scoopFreqSlider.getValue());
            pad.setBassFreqValue  ((float) bassFreqSlider.getValue());
            pad.setAirFreqValue   ((float) airFreqSlider.getValue());
            pad.setMonoValue      ((float) monoHz.getValue());
            pad.repaint();
        };
    };
    for (juce::Slider* slider : { &tilt,&hpHz,&lpHz,&air,&bass,&scoop,&tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider,&monoHz,&shelfShapeS,&filterQ,&hpQSlider,&lpQSlider })
    {
        addLiveRepaint (*slider);
        slider->onDragStart = [this]{ proc.setIsEditing (true); };
        slider->onDragEnd   = [this]{ proc.setIsEditing (false); };
    }

    // slider names (for LNF knob labels)
    gain.setName ("GAIN"); width.setName ("WIDTH"); tilt.setName ("TILT"); monoHz.setName ("MONO");
    hpHz.setName ("HP Hz"); lpHz.setName ("LP Hz"); satDrive.setName ("DRIVE"); satMix.setName ("MIX");
    air.setName ("AIR"); bass.setName ("BASS"); scoop.setName ("SCOOP"); spaceKnob.setName ("REVERB");
    shelfShapeS.setName ("Shape"); filterQ.setName ("Q");
    duckingKnob.setName ("DUCK"); panKnob.setName ("PAN"); panKnobLeft.setName ("PAN L"); panKnobRight.setName ("PAN R");
    duckAttack.setName ("ATT"); duckRelease.setName ("REL"); duckThreshold.setName ("THR"); duckRatio.setName ("RAT");
    tiltFreqSlider.setName ("TILT F"); scoopFreqSlider.setName ("SCOOP F"); bassFreqSlider.setName ("BASS F"); airFreqSlider.setName ("AIR F");
    // Imaging knob labels
    widthLo.setName ("W LO"); widthMid.setName ("W MID"); widthHi.setName ("W HI");
    xoverLoHz.setName ("XO LO"); xoverHiHz.setName ("XO HI");
    rotationDeg.setName ("ROT"); asymmetry.setName ("ASYM");
    shufLoPct.setName ("SHUF LO"); shufHiPct.setName ("SHUF HI"); shufXHz.setName ("SHUF XO");

    // HP/LP value label precision: integer Hz
    hpHz.setNumDecimalPlacesToDisplay (0);
    lpHz.setNumDecimalPlacesToDisplay (0);

    // Imaging static text labels under knobs (hidden to avoid duplication with value labels)
    for (auto* l : { &widthLoName,&widthMidName,&widthHiName,&xoverLoName,&xoverHiName,&rotationName,&asymName,&shufLoName,&shufHiName,&shufXName })
    {
        l->setVisible (false);
        l->setInterceptsMouseClicks (false, false);
    }
    widthLoName.setText ("", juce::dontSendNotification);
    widthMidName.setText("", juce::dontSendNotification);
    widthHiName.setText ("", juce::dontSendNotification);
    xoverLoName.setText ("", juce::dontSendNotification);
    xoverHiName.setText ("", juce::dontSendNotification);
    rotationName.setText("", juce::dontSendNotification);
    asymName.setText   ("", juce::dontSendNotification);
    shufLoName.setText ("", juce::dontSendNotification);
    shufHiName.setText ("", juce::dontSendNotification);
    shufXName.setText  ("", juce::dontSendNotification);
    // Delay controls initialization
    for (juce::Slider* slider : { &delayTime, &delayFeedback, &delayWet, &delaySpread, &delayWidth, &delayModRate, &delayModDepth, &delayWowflutter, &delayJitter, &delayPreDelay,
                                  &delayHp, &delayLp, &delayTilt, &delaySat, &delayDiffusion, &delayDiffuseSize,
                                  &delayDuckDepth, &delayDuckAttack, &delayDuckRelease, &delayDuckThreshold, &delayDuckRatio, &delayDuckLookahead })
    {
        addAndMakeVisible (*slider);
        style (*slider);
        slider->addListener (this);
    }
    
    // Delay combo boxes
    for (juce::ComboBox* combo : { &delayMode, &delayTimeDiv, &delayDuckSource, &delayGridFlavor, &delayFilterType })
    {
        addAndMakeVisible (*combo);
        combo->setLookAndFeel (&lnf);
        combo->addListener (this);
    }
    
    // Delay toggle buttons
    for (juce::ToggleButton* button : { &delayEnabled, &delaySync, &delayKillDry, &delayFreeze, &delayPingpong, &delayDuckPost, &delayDuckLinkGlobal })
    {
        addAndMakeVisible (*button);
        button->setLookAndFeel (&lnf);
        button->addListener (this);
    }

    // Delay row-1 explicit captions (ensure text visible in SwitchCell)
    delayEnabled.setButtonText (""); delayEnabled.setToggleState (delayEnabled.getToggleState(), juce::dontSendNotification);
    delaySync.setButtonText ("");      delaySync.setToggleState (delaySync.getToggleState(), juce::dontSendNotification);
    delayPingpong.setButtonText (""); delayPingpong.setToggleState (delayPingpong.getToggleState(), juce::dontSendNotification);
    delayFreeze.setButtonText ("");  delayFreeze.setToggleState (delayFreeze.getToggleState(), juce::dontSendNotification);
    delayKillDry.setButtonText (""); delayKillDry.setToggleState (delayKillDry.getToggleState(), juce::dontSendNotification);
    delayMode.setTextWhenNothingSelected ("Mode");
    // Populate Delay Mode items from APVTS choice parameter so popup shows options
    if (auto* delayModeParam = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter("delay_mode")))
    {
        delayMode.clear();
        for (int i = 0; i < delayModeParam->choices.size(); ++i)
            delayMode.addItem (delayModeParam->choices[i], i + 1);
        // Reflect current value without firing callbacks; attachment will keep in sync
        delayMode.setSelectedId (delayModeParam->getIndex() + 1, juce::dontSendNotification);
        // Per-item tints (Digital, Analog, Tape)
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            juce::Array<juce::Colour> tints;
            tints.add (lnf.theme.accent.withAlpha (0.85f));                                   // Digital: accent
            tints.add (lnf.theme.eq.tilt.withAlpha (0.90f));                                   // Analog: orange tilt
            tints.add (lnf.theme.eq.bass.withHue (lnf.theme.eq.bass.getHue() - 0.10f));        // Tape: warmer green
            lf->setPopupItemTints (tints);
        }
        delayMode.getProperties().set ("tintedSelected", true);
    }

    // Populate Delay Duck Source items from APVTS choice parameter so popup shows options
    if (auto* duckSrcParam = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter("delay_duck_source")))
    {
        delayDuckSource.clear();
        for (int i = 0; i < duckSrcParam->choices.size(); ++i)
            delayDuckSource.addItem (duckSrcParam->choices[i], i + 1);
        delayDuckSource.setSelectedId (duckSrcParam->getIndex() + 1, juce::dontSendNotification);
        // Per-item tints (Pre, Post, External)
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            juce::Array<juce::Colour> tints;
            tints.add (lnf.theme.eq.hp.withAlpha (0.95f));                                     // Pre: HP blue
            tints.add (lnf.theme.accent.withHue (lnf.theme.accent.getHue() + 0.12f));          // Post: cyan shift
            tints.add (lnf.theme.eq.scoop.withAlpha (0.95f));                                  // External: plum
            lf->setPopupItemTints (tints);
        }
        delayDuckSource.getProperties().set ("tintedSelected", true);
    }
    
    // Delay value labels
    for (juce::Label* l : { &delayTimeValue, &delayFeedbackValue, &delayWetValue, &delaySpreadValue, &delayWidthValue, &delayModRateValue, &delayModDepthValue, &delayWowflutterValue, &delayJitterValue, &delayPreDelayValue,
                            &delayHpValue, &delayLpValue, &delayTiltValue, &delaySatValue, &delayDiffusionValue, &delayDiffuseSizeValue,
                            &delayDuckDepthValue, &delayDuckAttackValue, &delayDuckReleaseValue, &delayDuckThresholdValue, &delayDuckRatioValue, &delayDuckLookaheadValue })
    {
        addAndMakeVisible (*l);
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::Font (juce::FontOptions (13.0f * scaleFactor).withStyle ("Bold")));
        l->setColour (juce::Label::textColourId, lnf.theme.text);
        l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    }
    
    // Motion value labels
    for (int i = 0; i < 20; ++i)
    {
        addAndMakeVisible (motionValuesGroup2[i]);
        motionValuesGroup2[i].setJustificationType (juce::Justification::centred);
        motionValuesGroup2[i].setFont (juce::Font (juce::FontOptions (13.0f * scaleFactor).withStyle ("Bold")));
        motionValuesGroup2[i].setColour (juce::Label::textColourId, lnf.theme.text);
        motionValuesGroup2[i].setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        motionValuesGroup2[i].setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    }
    
    // Delay name labels
    for (auto* l : { &delayTimeName, &delayFeedbackName, &delayWetName, &delaySpreadName, &delayWidthName, &delayModRateName, &delayModDepthName, &delayWowflutterName, &delayJitterName, &delayPreDelayName,
                     &delayHpName, &delayLpName, &delayTiltName, &delaySatName, &delayDiffusionName, &delayDiffuseSizeName,
                     &delayDuckDepthName, &delayDuckAttackName, &delayDuckReleaseName, &delayDuckThresholdName, &delayDuckRatioName, &delayDuckLookaheadName })
    {
        l->setVisible (false);
        l->setInterceptsMouseClicks (false, false);
        l->setText ("", juce::dontSendNotification);
    }
    
    // Set delay control names
    delayTime.setName ("TIME"); delayFeedback.setName ("FB"); delayWet.setName ("WET"); delaySpread.setName ("SPREAD");
    delayWidth.setName ("WIDTH"); delayModRate.setName ("MOD RATE"); delayModDepth.setName ("MOD DEPTH");
    delayWowflutter.setName ("WOW"); delayJitter.setName ("JITTER");
    delayHp.setName ("HP"); delayLp.setName ("LP"); delayTilt.setName ("TILT"); delaySat.setName ("SAT");
    delayDiffusion.setName ("DIFF"); delayDiffuseSize.setName ("SIZE");
    delayDuckDepth.setName ("DEPTH"); delayDuckAttack.setName ("ATT"); delayDuckRelease.setName ("REL");
    delayDuckThreshold.setName ("THR"); delayDuckRatio.setName ("DUCK RAT"); delayDuckLookahead.setName ("LA");
    delayPreDelay.setName ("PRE");
    
    // Set Motion control names
    motionDummiesGroup2[2].setName ("RATE"); motionDummiesGroup2[3].setName ("DEPTH"); motionDummiesGroup2[4].setName ("PHASE");
    motionDummiesGroup2[5].setName ("SPREAD"); motionDummiesGroup2[6].setName ("ELEV"); motionDummiesGroup2[7].setName ("BOUNCE");
    motionDummiesGroup2[8].setName ("JITTER"); motionDummiesGroup2[12].setName ("HOLD"); motionDummiesGroup2[13].setName ("SENS");
    motionDummiesGroup2[14].setName ("OFFSET"); motionDummiesGroup2[15].setName ("FRONT"); motionDummiesGroup2[16].setName ("DOPPLER");
    motionDummiesGroup2[17].setName ("SEND"); motionDummiesGroup2[19].setName ("BASS");

    // seed value labels with current values
    sliderValueChanged (&width);
    sliderValueChanged (&tilt);
    sliderValueChanged (&monoHz);
    sliderValueChanged (&hpHz);
    sliderValueChanged (&lpHz);
    sliderValueChanged (&satDrive);
    sliderValueChanged (&satMix);
    sliderValueChanged (&air);
    sliderValueChanged (&bass);
    sliderValueChanged (&scoop);
    sliderValueChanged (&panKnob);
    sliderValueChanged (&panKnobLeft);
    sliderValueChanged (&panKnobRight);
    sliderValueChanged (&spaceKnob);
    sliderValueChanged (&duckingKnob);
    sliderValueChanged (&tiltFreqSlider);
    sliderValueChanged (&scoopFreqSlider);
    sliderValueChanged (&bassFreqSlider);
    sliderValueChanged (&airFreqSlider);
    sliderValueChanged (&gain);
    updateMutedKnobVisuals();

    // Mono Maker slope & audition controls
    // Keep legacy ComboBox hidden for APVTS attachment; drive it from switch
    addChildComponent (monoSlopeChoice);
    monoSlopeChoice.addItem ("6",  1);
    monoSlopeChoice.addItem ("12", 2);
    monoSlopeChoice.addItem ("24", 3);

    // Center group: init punch mode choices
    centerPunchMode.addItem ("toSides",  1);
    centerPunchMode.addItem ("toCenter", 2);
    centerPunchMode.getProperties().set ("tintedSelected", true);

    // Center group: ensure rotary knobs and no text boxes (avoid duplicate labels)
    auto initCenterKnob = [this] (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        // Match standard tick system (12/3/6/9 o'clock): π to π + 2π and our LNF
        constexpr float kStart = juce::MathConstants<float>::pi;
        constexpr float kEnd   = juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi;
        s.setRotaryParameters (kStart, kEnd, true);
        s.setLookAndFeel (&lnf);
    };
    initCenterKnob (centerPromDb);
    initCenterKnob (centerFocusLoHz);
    initCenterKnob (centerFocusHiHz);
    initCenterKnob (centerPunchAmt01);
    initCenterKnob (centerPhaseAmt01);
    initCenterKnob (centerLockDb);
    // Provide names for LNF to draw knob-centered labels (match left-group behavior)
    centerPromDb.setName    ("CNTR");
    centerFocusLoHz.setName ("LO");
    centerFocusHiHz.setName ("HI");
    centerPunchAmt01.setName("PUNCH");
    centerPhaseAmt01.setName("PHASE");
    centerLockDb.setName    ("LOCK dB");
    if (!monoSlopeSwitch)
        monoSlopeSwitch = std::make_unique<MonoSlopeSwitch>();
    addAndMakeVisible (*monoSlopeSwitch);
    monoSlopeSwitch->onChange = [this](int idx)
    {
        monoSlopeChoice.setSelectedItemIndex (juce::jlimit (0, 2, idx), juce::NotificationType::sendNotification);
    };
    addAndMakeVisible (monoAuditionButton);
    monoAuditionButton.setButtonText ("AUD");

    // Imaging attachments
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, IDs::widthLo,         widthLo));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, IDs::widthMid,        widthMid));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, IDs::widthHi,         widthHi));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, IDs::xoverLoHz,      xoverLoHz));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, IDs::xoverHiHz,      xoverHiHz));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, IDs::rotationDeg,     rotationDeg));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, IDs::asymmetry,        asymmetry));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, IDs::shufLoPct,  shufLoPct));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, IDs::shufHiPct,  shufHiPct));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, IDs::shufXHz, shufXHz));

    // All children created; allow layout from now on
    layoutReady = true;

    // reverb algo switch
    addAndMakeVisible (spaceAlgorithmSwitch);
    spaceAlgorithmSwitch.setGreenMode (isGreenMode);
    if (auto* c = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter ("space_algo")))
    {
        const int maxIdx = juce::jmax (0, c->choices.size() - 1);
        const int idx = juce::jlimit (0, maxIdx, (int) c->getIndex());
        spaceAlgorithmSwitch.setOrientation (SpaceAlgorithmSwitch::Orientation::Vertical);
        spaceAlgorithmSwitch.setSpacing (4.0f);
        spaceAlgorithmSwitch.setAlgorithmFromParameter (idx);
        // Ensure labels show Room/Plate/Hall regardless of saved state
        spaceAlgorithmSwitch.setLabels (juce::StringArray { "Room", "Plate", "Hall" });
        currentAlgorithm = idx;
        pad.setSpaceAlgorithm (idx);
    }
    spaceAlgorithmSwitch.onAlgorithmChange = [this](int a)
    {
        if (auto* c = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter ("space_algo")))
        {
            const int maxIdx = juce::jmax (0, c->choices.size() - 1);
            const int idx = juce::jlimit (0, maxIdx, a);
            const float norm = c->convertTo0to1 ((float) idx);
            c->beginChangeGesture(); c->setValueNotifyingHost (norm); c->endChangeGesture();
            currentAlgorithm = idx;
            pad.setSpaceAlgorithm (idx);
        }
    };

    // header divider
    addAndMakeVisible (splitDivider);
    splitDivider.setVisible (true);

    // XY callbacks -> AVTS
    auto refreshXYOverlays = [this]
    {
        pad.setMixValue   (proc.apvts.getRawParameterValue ("sat_mix")->load());
        pad.setDriveValue (proc.apvts.getRawParameterValue ("sat_drive_db")->load());
        pad.setTiltValue  (proc.apvts.getRawParameterValue ("tilt")->load());
        pad.setHPValue    (proc.apvts.getRawParameterValue ("hp_hz")->load());
        pad.setLPValue    (proc.apvts.getRawParameterValue ("lp_hz")->load());
        pad.setAirValue   (proc.apvts.getRawParameterValue ("air_db")->load());
        pad.setBassValue  (proc.apvts.getRawParameterValue ("bass_db")->load());
        pad.setScoopValue (proc.apvts.getRawParameterValue ("scoop")->load());
        pad.setTiltFreqValue  (proc.apvts.getRawParameterValue ("tilt_freq")->load());
        pad.setScoopFreqValue (proc.apvts.getRawParameterValue ("scoop_freq")->load());
        pad.setBassFreqValue  (proc.apvts.getRawParameterValue ("bass_freq")->load());
        pad.setAirFreqValue   (proc.apvts.getRawParameterValue ("air_freq")->load());
        pad.setWidthValue (proc.apvts.getRawParameterValue ("width")->load());
        pad.setPanValue   (proc.apvts.getRawParameterValue ("pan")->load());
        pad.setGainValue  (proc.apvts.getRawParameterValue ("gain_db")->load());
    };

    pad.onChange = [this, refreshXYOverlays](float x01, float y01)
    {
        if (auto* split = proc.apvts.getParameter ("split_mode")) { split->beginChangeGesture(); split->setValueNotifyingHost (0.0f); split->endChangeGesture(); }
        if (auto* pan   = proc.apvts.getParameter ("pan"))        { pan  ->beginChangeGesture(); pan  ->setValueNotifyingHost (x01);  pan  ->endChangeGesture(); }
        if (auto* dep   = proc.apvts.getParameter ("depth"))      { dep  ->beginChangeGesture(); dep  ->setValueNotifyingHost (y01);  dep  ->endChangeGesture(); }
        refreshXYOverlays();
    };
    pad.onSplitChange = [this, refreshXYOverlays](float l01, float r01, float y01)
    {
        if (auto* split = proc.apvts.getParameter ("split_mode")) { split->beginChangeGesture(); split->setValueNotifyingHost (1.0f); split->endChangeGesture(); }
        if (auto* pL = proc.apvts.getParameter ("pan_l")) { pL->beginChangeGesture(); pL->setValueNotifyingHost (l01); pL->endChangeGesture(); }
        if (auto* pR = proc.apvts.getParameter ("pan_r")) { pR->beginChangeGesture(); pR->setValueNotifyingHost (r01); pR->endChangeGesture(); }
        if (auto* dep= proc.apvts.getParameter ("depth")) { dep->beginChangeGesture(); dep->setValueNotifyingHost (y01); dep->endChangeGesture(); }
        refreshXYOverlays();
    };
    // listeners for split overlay % etc.
    panKnobLeft.addListener (this);
    panKnobRight.addListener (this);
    // attachments (deduped; removed accidental duplicates you had for bass/scoop/depth)
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::gain,       gain));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::inputGain,  inputSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::outputGain, outputSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::mix,       mixSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::width,         width));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::tilt,          tilt));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::monoHz,       monoHz));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::hpHz,         hpHz));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::lpHz,         lpHz));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::satDriveDb,  satDrive));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::satMix,       satMix));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::airDb,        air));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::bassDb,       bass));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::scoop,         scoop));
    // Rebind ducking controls to Reverb Engine parameters
    attachments.push_back (std::make_unique<SA> (proc.apvts, ReverbIDs::duckDepthDb, duckingKnob));
    attachments.push_back (std::make_unique<SA> (proc.apvts, ReverbIDs::duckAtkMs,   duckAttack));
    attachments.push_back (std::make_unique<SA> (proc.apvts, ReverbIDs::duckRelMs,   duckRelease));
    attachments.push_back (std::make_unique<SA> (proc.apvts, ReverbIDs::duckThrDb,   duckThreshold));
    attachments.push_back (std::make_unique<SA> (proc.apvts, ReverbIDs::duckRatio,   duckRatio));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "pan",           panKnob));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "pan_l",         panKnobLeft));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "pan_r",         panKnobRight));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "depth",         spaceKnob));
    // Mono maker APVTS attachments
    comboAttachments .push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "mono_slope_db_oct", monoSlopeChoice));
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>   (proc.apvts, "mono_audition",      monoAuditionButton));

    // Center group attachments
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::centerPromDb,        centerPromDb));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::centerFocusLoHz,       centerFocusLoHz));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::centerFocusHiHz,       centerFocusHiHz));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::centerPunchAmt01,     centerPunchAmt01));
    comboAttachments .push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, IDs::centerPunchMode,   centerPunchMode));
    // Legacy Phase Recovery parameters removed - using new Phase Alignment system
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>   (proc.apvts, IDs::centerLockOn,      centerLockOn));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::centerLockDb,       centerLockDb));

    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::tiltFreq,     tiltFreqSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::scoopFreq,    scoopFreqSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::bassFreq,     bassFreqSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::airFreq,      airFreqSlider));
    // New EQ params
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::eqShelfShape, shelfShapeS));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::eqFilterQ,    filterQ));
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, IDs::tiltLinkS, tiltLinkSButton));
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, IDs::eqQLink,   qLinkButton));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::hpQ, hpQSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, IDs::lpQ, lpQSlider));

    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "bypass", bypassButton));
    comboAttachments .push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "os_mode", osSelect));
    
    // Delay parameter attachments - with safety checks
    auto safeDelayAttachment = [&](const char* paramId, auto&& attachment) {
        if (proc.apvts.getParameter(paramId) != nullptr) {
            attachment();
        }
    };
    
    safeDelayAttachment("delay_enabled", [&]() { buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_enabled", delayEnabled)); });
    safeDelayAttachment("delay_mode", [&]() { comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "delay_mode", delayMode)); });
    safeDelayAttachment("delay_sync", [&]() { buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_sync", delaySync)); });
    safeDelayAttachment("delay_grid_flavor", [&]() { comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "delay_grid_flavor", delayGridFlavor)); });
    safeDelayAttachment("delay_time_ms", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_time_ms", delayTime)); });
    safeDelayAttachment("delay_time_div", [&]() { comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "delay_time_div", delayTimeDiv)); });
    safeDelayAttachment("delay_feedback_pct", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_feedback_pct", delayFeedback)); });
    safeDelayAttachment("delay_wet", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_wet", delayWet)); });
    safeDelayAttachment("delay_kill_dry", [&]() { buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_kill_dry", delayKillDry)); });
    safeDelayAttachment("delay_freeze", [&]() { buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_freeze", delayFreeze)); });
    safeDelayAttachment("delay_pingpong", [&]() { buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_pingpong", delayPingpong)); });
    safeDelayAttachment("delay_crossfeed_pct", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_crossfeed_pct", delaySpread)); });
    safeDelayAttachment("delay_stereo_spread_pct", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_stereo_spread_pct", delaySpread)); });
    safeDelayAttachment("delay_width", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_width", delayWidth)); });
    safeDelayAttachment("delay_mod_rate_hz", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_mod_rate_hz", delayModRate)); });
    safeDelayAttachment("delay_mod_depth_ms", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_mod_depth_ms", delayModDepth)); });
    safeDelayAttachment("delay_wowflutter", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_wowflutter", delayWowflutter)); });
    safeDelayAttachment("delay_jitter_pct", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_jitter_pct", delayJitter)); });
    safeDelayAttachment("delay_hp_hz", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_hp_hz", delayHp)); });
    safeDelayAttachment("delay_lp_hz", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_lp_hz", delayLp)); });
    safeDelayAttachment("delay_tilt_db", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_tilt_db", delayTilt)); });
    safeDelayAttachment("delay_sat", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_sat", delaySat)); });
    safeDelayAttachment("delay_diffusion", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_diffusion", delayDiffusion)); });
    safeDelayAttachment("delay_diffuse_size_ms", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_diffuse_size_ms", delayDiffuseSize)); });
    safeDelayAttachment("delay_duck_source", [&]() { comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "delay_duck_source", delayDuckSource)); });
    safeDelayAttachment("delay_duck_post", [&]() { buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_duck_post", delayDuckPost)); });
    safeDelayAttachment("delay_duck_depth", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_duck_depth", delayDuckDepth)); });
    safeDelayAttachment("delay_duck_attack_ms", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_duck_attack_ms", delayDuckAttack)); });
    safeDelayAttachment("delay_duck_release_ms", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_duck_release_ms", delayDuckRelease)); });
    safeDelayAttachment("delay_duck_threshold_db", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_duck_threshold_db", delayDuckThreshold)); });
    safeDelayAttachment("delay_duck_ratio", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_duck_ratio", delayDuckRatio)); });
    safeDelayAttachment("delay_duck_lookahead_ms", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_duck_lookahead_ms", delayDuckLookahead)); });
    safeDelayAttachment("delay_pre_delay_ms", [&]() { attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_pre_delay_ms", delayPreDelay)); });
    safeDelayAttachment("delay_filter_type", [&]() { comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "delay_filter_type", delayFilterType)); });
    safeDelayAttachment("delay_duck_link_global", [&]() { buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_duck_link_global", delayDuckLinkGlobal)); });

            // Create Motion ComboBoxes and Buttons
            #include "motion/MotionIDs.h"
            using namespace motion;
            
            // ComboBoxes: Panner (0), Path (1), Quantize (9), Mode (10)
            motionComboBoxes[0].addItemList(choiceListPanner(), 1);
            motionComboBoxes[1].addItemList(choiceListPath(), 1);
            motionComboBoxes[2].addItemList(choiceListQuant(), 1);
            motionComboBoxes[3].addItemList(choiceListMode(), 1);
            
            // Explicitly set Panner ComboBox defaults (no chevron state)
            motionComboBoxes[0].getProperties().set ("forceSelectedText", true);
            motionComboBoxes[0].getProperties().set ("defaultTextWhenEmpty", "P1");
            motionComboBoxes[0].setSelectedId(1, juce::dontSendNotification); // P1 = index 0, but ComboBox uses 1-based IDs
            
            // Configure Motion ComboBoxes like Delay ComboBoxes
            for (juce::ComboBox* combo : { &motionComboBoxes[0], &motionComboBoxes[1], &motionComboBoxes[2], &motionComboBoxes[3] })
            {
                addAndMakeVisible (*combo);
                combo->setLookAndFeel (&lnf);
                combo->addListener (this);
                combo->getProperties().set ("tintedSelected", true);
            }
            
            // Special configuration for Panner ComboBox to remove chevron
            motionComboBoxes[0].setTextWhenNothingSelected("");
            
            // Configure Motion sliders like Delay sliders
            for (int i = 0; i < 20; ++i)
            {
                addAndMakeVisible (motionDummiesGroup2[i]);
                style (motionDummiesGroup2[i]);
                motionDummiesGroup2[i].addListener (this);
            }
            
            // Configure Motion buttons like Delay buttons
            for (juce::ToggleButton* button : { &motionButtons[0], &motionButtons[1], &motionButtons[2] })
            {
                addAndMakeVisible (*button);
                button->setLookAndFeel (&lnf);
                button->addListener (this);
            }
            
            // Set up per-item tints for Motion ComboBoxes (following Delay ComboBox pattern)
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                // Panner ComboBox tints (P1, P2, Link) - Blue family
                juce::Array<juce::Colour> pannerTints;
                pannerTints.add (juce::Colour (0xFF64B5F6)); // P1 - Light Blue
                pannerTints.add (juce::Colour (0xFF42A5F5)); // P2 - Medium Blue  
                pannerTints.add (juce::Colour (0xFF2196F3)); // Link - Dark Blue
                lf->setPopupItemTints (pannerTints);
                motionComboBoxes[0].getProperties().set ("tintedSelected", true);
                
                // Path ComboBox tints (Circle, Figure-8, Bounce, Arc, Spiral, Polygon, Random Walk, User Shape) - Rainbow
                juce::Array<juce::Colour> pathTints;
                pathTints.add (lnf.theme.accent.withHue (lnf.theme.accent.getHue() - 0.33f).withSaturation (0.85f)); // Circle - warm red/orange
                pathTints.add (lnf.theme.eq.tilt.withAlpha (0.95f));   // Figure-8 - Orange
                pathTints.add (lnf.theme.accent.withHue (lnf.theme.accent.getHue() + 0.16f).withBrightness (0.95f)); // Bounce - Yellow
                pathTints.add (lnf.theme.eq.bass);                     // Arc - Green
                pathTints.add (lnf.theme.accent.withHue (lnf.theme.accent.getHue() + 0.12f)); // Spiral - Cyan-ish
                pathTints.add (lnf.theme.eq.hp);                       // Polygon - Blue
                pathTints.add (lnf.theme.eq.scoop);                    // Random Walk - Purple/Plum
                pathTints.add (lnf.theme.sh.brighter (0.6f));          // User Shape - Neutral brownish via shadow tint
                lf->setPopupItemTints (pathTints);
                motionComboBoxes[1].getProperties().set ("tintedSelected", true);
                
                // Quantize ComboBox tints (Off, 1/1, 1/2, 1/4, 1/8, 1/16, 1/32, Triplet, Dotted) - Purple family
                juce::Array<juce::Colour> quantTints;
                quantTints.add (lnf.theme.textMuted); // Off - Grey
                auto baseScoop = lnf.theme.eq.scoop;
                quantTints.add (baseScoop.withAlpha (0.75f));
                quantTints.add (baseScoop.withBrightness (juce::jlimit (0.0f, 1.0f, baseScoop.getBrightness() * 1.05f)));
                quantTints.add (baseScoop);
                quantTints.add (baseScoop.withSaturation (juce::jlimit (0.0f, 1.0f, baseScoop.getSaturation() * 1.05f)));
                quantTints.add (baseScoop.withHue (baseScoop.getHue() - 0.03f));
                quantTints.add (baseScoop.withHue (baseScoop.getHue() - 0.06f));
                quantTints.add (baseScoop.withHue (baseScoop.getHue() - 0.09f));
                quantTints.add (baseScoop.withHue (baseScoop.getHue() - 0.12f));
                lf->setPopupItemTints (quantTints);
                motionComboBoxes[2].getProperties().set ("tintedSelected", true);
                
                // Mode ComboBox tints (Free, Sync, Input Env, Sidechain, One-Shot) - Green family
                juce::Array<juce::Colour> modeTints;
                modeTints.add (lnf.theme.eq.bass.withAlpha (0.80f)); // Free
                modeTints.add (lnf.theme.eq.bass);                   // Sync
                modeTints.add (lnf.theme.eq.bass.withSaturation (juce::jlimit (0.0f, 1.0f, lnf.theme.eq.bass.getSaturation() * 0.9f))); // Input Env
                modeTints.add (lnf.theme.eq.bass.darker (0.10f));    // Sidechain
                modeTints.add (lnf.theme.eq.bass.darker (0.20f));    // One-Shot
                lf->setPopupItemTints (modeTints);
                motionComboBoxes[3].getProperties().set ("tintedSelected", true);
            }
            
            // Create Motion SwitchCell wrappers (following Delay group pattern)
            const juce::StringArray comboLabels = {"Panner", "Path", "Quant", "Mode"};
            for (int i = 0; i < 4; ++i) {
                if (!motionComboCells[i]) {
                    motionComboCells[i] = std::make_unique<SwitchCell>(motionComboBoxes[i]);
                    motionComboCells[i]->setCaption(comboLabels[i]);
                    // Apply same green border treatment as other Motion items
                    motionComboCells[i]->getProperties().set ("motionPurpleBorder", true);
                    motionComboCells[i]->getProperties().set ("metallic", true);
                    motionComboCells[i]->setShowBorder(true);
                }
                // Now lives only in Group 1 grid; do not add to bottomAltPanel
            }
            
            const juce::StringArray buttonLabels = {"Enable", "Retrig", "Anchor", "HeadSafe"};
            for (int i = 0; i < 4; ++i) {
                if (!motionButtonCells[i]) {
                    if (i == 0) {
                        // Add Enable icon to Motion Enable, same as Delay Enable
                        motionButtons[0].getProperties().set ("iconType", (int) IconSystem::Power);
                        motionButtons[0].setComponentID ("motionEnabled");
                    } else if (i == 1) {
                        // Retrig icon styling to match system icons
                        motionButtons[1].getProperties().set ("iconType", (int) IconSystem::Retrig);
                        motionButtons[1].setComponentID ("motionRetrig");
                    } else if (i == 2) {
                        // Anchor icon styling to match system icons
                        motionButtons[2].getProperties().set ("iconType", (int) IconSystem::Anchor);
                        motionButtons[2].setComponentID ("motionAnchor");
                    }
                    motionButtonCells[i] = std::make_unique<SwitchCell>(motionButtons[i]);
                    motionButtonCells[i]->setCaption(buttonLabels[i]);
                    // Apply same green border treatment as other Motion items
                    motionButtonCells[i]->getProperties().set ("motionPurpleBorder", true);
                    motionButtonCells[i]->getProperties().set ("metallic", true);
                    motionButtonCells[i]->setShowBorder(true);
                }
                // Now lives only in Group 1 grid; do not add to bottomAltPanel
            }
            
            // Motion parameter attachments (6x4 grid: 24 total) - Global parameters first
            buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, motion::id::enable, motionButtons[0]));
            
            // Ensure panner parameter is set to P1 before creating attachment
            if (auto* pannerParam = proc.apvts.getParameter(motion::id::panner_select)) {
                pannerParam->setValueNotifyingHost(0.0f); // P1 = 0.0f for choice parameter
            }
            
            comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, motion::id::panner_select, motionComboBoxes[0]));
            attachments.push_back (std::make_unique<SA> (proc.apvts, motion::id::bass_floor_hz, motionDummiesGroup2[22]));
            attachments.push_back (std::make_unique<SA> (proc.apvts, motion::id::occlusion, motionDummiesGroup2[23]));
            buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, motion::id::headphone_safe, motionButtons[3]));
            
            // Initialize motion parameter attachments based on current panner selection
            int initialPannerSelect = 0; // Default to P1 (we already set the parameter above)
            updateMotionParameterAttachments(initialPannerSelect);

    // parameter listeners (host→UI)
    proc.apvts.addParameterListener ("space_algo", this);
    proc.apvts.addParameterListener ("split_mode", this);
    proc.apvts.addParameterListener ("pan",        this);
    proc.apvts.addParameterListener ("depth",      this);
    proc.apvts.addParameterListener ("mono_slope_db_oct", this);
    // EQ shape/Q visual linkage (live updates)
    proc.apvts.addParameterListener ("eq_shelf_shape", this);
    proc.apvts.addParameterListener ("eq_q_link",      this);
    proc.apvts.addParameterListener ("eq_filter_q",    this);
    proc.apvts.addParameterListener ("hp_q",           this);
    // Motion panner selection for dynamic parameter switching
    proc.apvts.addParameterListener (motion::id::panner_select, this);
    proc.apvts.addParameterListener ("lp_q",           this);
    proc.apvts.addParameterListener ("tilt_link_s",    this);
    // Imaging overlays
    proc.apvts.addParameterListener ("xover_lo_hz",    this);
    proc.apvts.addParameterListener ("xover_hi_hz",    this);
    proc.apvts.addParameterListener ("rotation_deg",   this);
    proc.apvts.addParameterListener ("asymmetry",      this);
    proc.apvts.addParameterListener ("shuffler_lo_pct", this);
    proc.apvts.addParameterListener ("shuffler_hi_pct", this);
    proc.apvts.addParameterListener ("shuffler_xover_hz", this);

    // audio callbacks -> panes
    proc.onAudioSample   = [this](float L, float R) { if (panes) panes->onAudioSample (L, R); };
    proc.onAudioBlock    = [this](const float* L, const float* R, int n) { if (panes) panes->onAudioBlock (L, R, n); };
    proc.onAudioBlockPre = [this](const float* L, const float* R, int n) { if (panes) panes->onAudioBlockPre (L, R, n); };
    pad.setSampleRate (proc.getSampleRate());

    // Keybindings for panes and keep-warm
    struct LocalKeyListener : public juce::KeyListener {
        PaneManager* mgr;
        explicit LocalKeyListener (PaneManager* m) : mgr (m) {}
        bool keyPressed (const juce::KeyPress& k, juce::Component*) override
        {
            if (!mgr) return false;
            if (k.getTextCharacter()=='1') { mgr->setActive (PaneID::XY, true);       return true; }
            if (k.getTextCharacter()=='2') { mgr->setActive (PaneID::DynEQ, true); return true; }
            if (k.getTextCharacter()=='3') { mgr->setActive (PaneID::Imager, true);   return true; }
            // keep-warm toggle removed (K/k)
            return false;
        }
    };
    keyListener.reset (new LocalKeyListener (panes.get()));
    addKeyListener (keyListener.get());

    // divider line component
    addAndMakeVisible (splitDivider);

    // sync XY with current values
    syncXYPadWithParameters();

    // pointer cursors on interactive children
    applyGlobalCursorPolicy();

    startTimerHz (20);

    // Image row group
    addChildComponent (imgGroupContainer);
    imgGroupContainer.setTitle("");
    imgGroupContainer.setShowBorder(false);
    imgGroupContainer.setVisible(false);
    imgGroupContainer.setInterceptsMouseClicks(false, false);
    // Row 2 group (Reverb, switch, Ducking group, Delay group)
    addChildComponent (volGroupContainer2);
    volGroupContainer2.setTitle("");
    volGroupContainer2.setShowBorder(false);
    volGroupContainer2.setVisible(false);
    volGroupContainer2.setInterceptsMouseClicks(false, false);
    // Mono group container
    addAndMakeVisible (monoGroupContainer);
    monoGroupContainer.setTitle ("");
    monoGroupContainer.setShowBorder (true);
    // Reverb group container
    // Reverb row container no longer used; layout directly
    
    // Re-enable resized() call since crash is happening after constructor
    resized();
}

void MyPluginAudioProcessorEditor::buildCells()
{
    // Row 1
    if (!widthCell)   widthCell   = std::make_unique<KnobCell>(width,    widthValue,    "WIDTH");
    if (!widthLoCell) widthLoCell = std::make_unique<KnobCell>(widthLo,  widthLoValue,  "W LO");
    if (!widthMidCell)widthMidCell= std::make_unique<KnobCell>(widthMid, widthMidValue, "W MID");
    if (!widthHiCell) widthHiCell = std::make_unique<KnobCell>(widthHi,  widthHiValue,  "W HI");
    if (!gainCell)    gainCell    = std::make_unique<KnobCell>(gain,     gainValue,     "GAIN");
    if (!satDriveCell)satDriveCell= std::make_unique<KnobCell>(satDrive, satDriveValue, "DRIVE");
    if (!satMixCell)  satMixCell  = std::make_unique<KnobCell>(satMix,   satMixValue,   "MIX");
    if (!monoCell)    monoCell    = std::make_unique<KnobCell>(monoHz,   monoValue,     "MONO");
    // Legacy spaceCell (REVERB) removed from Group 1 row; Reverb amount lives in Group 2 as WET
    if (!spaceCell)    spaceCell    = std::make_unique<KnobCell>(spaceKnob,    spaceValue,    "REVERB");
    if (!duckCell)     duckCell     = std::make_unique<KnobCell>(duckingKnob,  duckingValue,  "DUCK");
    if (!duckAttCell)  duckAttCell  = std::make_unique<KnobCell>(duckAttack,   duckAttackValue,   "ATT");
    if (!duckRelCell)  duckRelCell  = std::make_unique<KnobCell>(duckRelease,  duckReleaseValue,  "REL");
    if (!duckThrCell)  duckThrCell  = std::make_unique<KnobCell>(duckThreshold,duckThresholdValue,"THR");
    if (!duckRatCell)  duckRatCell  = std::make_unique<KnobCell>(duckRatio,    duckRatioValue,    "RAT");

    if (!bassCell)     bassCell     = std::make_unique<KnobCell>(bass,  bassValue,  "BASS");
    if (!airCell)      airCell      = std::make_unique<KnobCell>(air,   airValue,   "AIR");
    if (!tiltCell)     tiltCell     = std::make_unique<KnobCell>(tilt,  tiltValue,  "TILT");
    if (!scoopCell)    scoopCell    = std::make_unique<KnobCell>(scoop, scoopValue, "SCOOP");
    if (!hpCell)       hpCell       = std::make_unique<KnobCell>(hpHz,  hpValue,    "HP Hz");
    if (!lpCell)       lpCell       = std::make_unique<KnobCell>(lpHz,  lpValue,    "LP Hz");

    if (!xoverLoCell)  xoverLoCell  = std::make_unique<KnobCell>(xoverLoHz, xoverLoValue, "XO LO");
    if (!xoverHiCell)  xoverHiCell  = std::make_unique<KnobCell>(xoverHiHz, xoverHiValue, "XO HI");
    if (!rotationCell) rotationCell = std::make_unique<KnobCell>(rotationDeg, rotationValue, "ROT");
    if (!asymCell)     asymCell     = std::make_unique<KnobCell>(asymmetry,   asymValue,     "ASYM");
    if (!shufLoCell)   shufLoCell   = std::make_unique<KnobCell>(shufLoPct,   shufLoValue,   "SHUF LO");
    if (!shufHiCell)   shufHiCell   = std::make_unique<KnobCell>(shufHiPct,   shufHiValue,   "SHUF HI");
    if (!shufXCell)    shufXCell    = std::make_unique<KnobCell>(shufXHz,     shufXValue,    "SHUF XO");

    if (!delayTimeCell)      delayTimeCell       = std::make_unique<KnobCell>(delayTime,      delayTimeValue,      "TIME");
    if (!delayFeedbackCell)  delayFeedbackCell   = std::make_unique<KnobCell>(delayFeedback,  delayFeedbackValue,  "FB");
    if (!delayWetCell)       delayWetCell        = std::make_unique<KnobCell>(delayWet,       delayWetValue,       "WET");
    if (!delaySpreadCell)    delaySpreadCell     = std::make_unique<KnobCell>(delaySpread,    delaySpreadValue,    "SPREAD");
    if (!delayWidthCell)     delayWidthCell      = std::make_unique<KnobCell>(delayWidth,     delayWidthValue,     "WIDTH");
    if (!delayModRateCell)   delayModRateCell    = std::make_unique<KnobCell>(delayModRate,   delayModRateValue,   "RATE");
    if (!delayModDepthCell)  delayModDepthCell   = std::make_unique<KnobCell>(delayModDepth,  delayModDepthValue,  "DEPTH");
    if (!delayWowflutterCell)delayWowflutterCell = std::make_unique<KnobCell>(delayWowflutter,delayWowflutterValue,"WOW");
    if (!delayJitterCell)    delayJitterCell     = std::make_unique<KnobCell>(delayJitter,    delayJitterValue,    "JITTER");
    if (!delayHpCell)        delayHpCell         = std::make_unique<KnobCell>(delayHp,        delayHpValue,        "HP");
    if (!delayLpCell)        delayLpCell         = std::make_unique<KnobCell>(delayLp,        delayLpValue,        "LP");
    if (!delayTiltCell)      delayTiltCell       = std::make_unique<KnobCell>(delayTilt,      delayTiltValue,      "TILT");
    if (!delaySatCell)       delaySatCell        = std::make_unique<KnobCell>(delaySat,       delaySatValue,       "SAT");
    if (!delayDiffusionCell) delayDiffusionCell  = std::make_unique<KnobCell>(delayDiffusion, delayDiffusionValue, "DIFF");
    if (!delayDiffuseSizeCell)delayDiffuseSizeCell= std::make_unique<KnobCell>(delayDiffuseSize, delayDiffuseSizeValue, "SIZE");
    if (!delayDuckDepthCell) delayDuckDepthCell  = std::make_unique<KnobCell>(delayDuckDepth, delayDuckDepthValue, "DEPTH");
    if (!delayDuckAttackCell)delayDuckAttackCell = std::make_unique<KnobCell>(delayDuckAttack,delayDuckAttackValue,"ATT");
    if (!delayDuckReleaseCell)delayDuckReleaseCell=std::make_unique<KnobCell>(delayDuckRelease,delayDuckReleaseValue,"REL");
    if (!delayJitterCell) delayJitterCell = std::make_unique<KnobCell>(delayJitter, delayJitterValue, "JITTER");
    if (!delayDuckRatioCell) delayDuckRatioCell = std::make_unique<KnobCell>(delayDuckRatio, delayDuckRatioValue, "RAT");
    
    // Log: Editor constructor complete
    // TEMPORARILY DISABLE file logging to test if this is causing the crash
    // juce::File f = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Field_CrashLog.txt");
    // f.appendText("Editor Ctor: COMPLETE\n", false, false, "\n");
}

MyPluginAudioProcessorEditor::~MyPluginAudioProcessorEditor()
{
    // Editor destructor - restore crash logging for debugging
    juce::File f = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Field_CrashLog.txt");
    f.appendText("Editor Destructor: STARTED\n", false, false, "\n");
    
    // Cancel AsyncUpdater to prevent use-after-free
    motionBinding.cancelPendingUpdate();
    f.appendText("Editor Destructor: AsyncUpdater cancelled\n", false, false, "\n");
    
    // Detach APVTS attachments BEFORE any controls are destroyed
    attachments.clear();
    buttonAttachments.clear();
    comboAttachments.clear();
    motionSliderAttachments.clear();
    motionButtonAttachments.clear();
    motionComboAttachments.clear();
    f.appendText("Editor Destructor: APVTS attachments cleared\n", false, false, "\n");

    // Stop editor timer early
    stopTimer();
    f.appendText("Editor Destructor: Editor timer stopped\n", false, false, "\n");

    // Remove key listener safely
    if (keyListener)
    {
        removeKeyListener (keyListener.get());
        keyListener.reset();
    }
    f.appendText("Editor Destructor: Key listener removed\n", false, false, "\n");

    // Clear audio->UI callbacks to prevent use-after-free from audio thread
    proc.onAudioSample   = nullptr;
    proc.onAudioBlock    = nullptr;
    proc.onAudioBlockPre = nullptr;
    f.appendText("Editor Destructor: Audio callbacks cleared\n", false, false, "\n");

    // Remove all parameter listeners that were added in the ctor
    proc.apvts.removeParameterListener ("space_algo", this);
    proc.apvts.removeParameterListener ("split_mode", this);
    proc.apvts.removeParameterListener ("pan",        this);
    proc.apvts.removeParameterListener ("depth",      this);
    proc.apvts.removeParameterListener ("mono_slope_db_oct", this);
    proc.apvts.removeParameterListener ("eq_shelf_shape", this);
    proc.apvts.removeParameterListener ("eq_q_link",      this);
    proc.apvts.removeParameterListener ("eq_filter_q",    this);
    proc.apvts.removeParameterListener ("hp_q",           this);
    proc.apvts.removeParameterListener (motion::id::panner_select, this);
    proc.apvts.removeParameterListener ("lp_q",           this);
    proc.apvts.removeParameterListener ("tilt_link_s",    this);
    proc.apvts.removeParameterListener ("xover_lo_hz",    this);
    proc.apvts.removeParameterListener ("xover_hi_hz",    this);
    proc.apvts.removeParameterListener ("rotation_deg",   this);
    proc.apvts.removeParameterListener ("asymmetry",      this);
    proc.apvts.removeParameterListener ("shuffler_lo_pct", this);
    proc.apvts.removeParameterListener ("shuffler_hi_pct", this);
    proc.apvts.removeParameterListener ("shuffler_xover_hz", this);
    f.appendText("Editor Destructor: Parameter listeners removed\n", false, false, "\n");

    // Detach UI listeners from knobs
    panKnobLeft.removeListener (this);
    panKnobRight.removeListener (this);
    f.appendText("Editor Destructor: UI listeners removed\n", false, false, "\n");

    // Ensure PaneManager timers and children are torn down before editor memory goes away
    panes.reset();
    f.appendText("Editor Destructor: PaneManager reset\n", false, false, "\n");

    // ensure A holds final state if user ended on B
    if (!isStateA) { saveCurrentState(); stateA = stateB; }
    f.appendText("Editor Destructor: State saved\n", false, false, "\n");

    setLookAndFeel (nullptr);
    f.appendText("Editor Destructor: LookAndFeel detached\n", false, false, "\n");
    
    f.appendText("Editor Destructor: COMPLETE\n", false, false, "\n");
}
void MyPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Paint method - removed logging to prevent file I/O on every paint
    
    // background gradient (toned down and tinted with accent color)
    auto full = getLocalBounds();
    juce::Colour top    = lnf.theme.sh.interpolatedWith(lnf.theme.accent, 0.08f); // Subtle accent tint
    juce::Colour mid    = lnf.theme.hl.darker(0.15f).interpolatedWith(lnf.theme.accent, 0.12f); // Accent tint on middle
    juce::Colour bottom = lnf.theme.sh.interpolatedWith(lnf.theme.accent, 0.08f); // Subtle accent tint
    juce::ColourGradient bg (top, (float) full.getCentreX(), (float) full.getY(),
                             bottom, (float) full.getCentreX(), (float) full.getBottom(), false);
    bg.addColour (0.85, mid);
    g.setGradientFill (bg);
    g.fillAll();

    // logo + tagline
    auto header = getLocalBounds().removeFromTop ((int) (100 * scaleFactor));
    const int leftInset = Layout::dp (20, scaleFactor);
    auto logoArea = juce::Rectangle<int> (header.getX() + leftInset, header.getY() + Layout::dp (4, scaleFactor),
                                          header.getWidth(), (int) (30 * scaleFactor + 2));
    
    // Enhanced FIELD logo with shadow and glow effects
    drawHeaderFieldLogo(g, logoArea.toFloat());

    // version - position after the actual logo width
    const juce::String ver = " v" + juce::String (JUCE_STRINGIFY (JucePlugin_VersionString));
    juce::Font vfont (juce::FontOptions (juce::jmax (9, (int) std::round (8 * scaleFactor))));
    g.setFont (vfont);
    g.setColour (lnf.theme.textMuted);
    
    // Calculate actual logo width and position version after it
    const float actualLogoWidth = juce::jmin(logoArea.getHeight() * 0.8f, 30.0f) * 2.5f;
    const int vx = logoArea.getX() + (int) actualLogoWidth + Layout::dp (8, scaleFactor);
    const int vy = logoArea.getY() + (logoArea.getHeight() - vfont.getHeight()) * 0.5f + 1;
    g.drawText (ver, juce::Rectangle<int> (vx, vy, 120, (int) vfont.getHeight() + 2), juce::Justification::centredLeft);

    // tagline
    g.setColour (lnf.theme.textMuted);
    g.setFont (juce::Font (juce::FontOptions (13.0f * scaleFactor).withStyle ("Bold")));
    g.drawText ("Spatial Audio Processor",
                juce::Rectangle<int> (logoArea.getX(), logoArea.getBottom() + Layout::dp (2, scaleFactor),
                                     header.getWidth(), (int) (14 * scaleFactor + 2)),
                juce::Justification::centredLeft);

    // resize handle
    auto bounds = getLocalBounds();
    auto resizeArea = bounds.removeFromRight (20).removeFromBottom (20);
    g.setColour (lnf.theme.textMuted);
    for (int i = 0; i < 3; ++i)
    {
        int off = i * 4;
        g.drawLine (resizeArea.getRight() - 8 - off, resizeArea.getBottom() - 4 - off,
                    resizeArea.getRight() - 4 - off, resizeArea.getBottom() - 8 - off, 1.0f);
    }
}

void MyPluginAudioProcessorEditor::drawHeaderFieldLogo (juce::Graphics& g, juce::Rectangle<float> area) const
{
    // Calculate logo size for header (smaller than shade overlay)
    const float logoHeight = juce::jmin(area.getHeight() * 0.8f, 30.0f);
    const float logoWidth = logoHeight * 2.5f; // FIELD is wider than tall
    
    // Center the logo in the header area
    const float logoX = area.getX();
    const float logoY = area.getCentreY() - logoHeight * 0.5f;
    const auto logoRect = juce::Rectangle<float>(logoX, logoY, logoWidth, logoHeight);
    
    // Create bold font for header logo
    juce::Font logoFont(juce::FontOptions(logoHeight * 0.8f).withStyle("Bold"));
    g.setFont(logoFont);
    
    // Enhanced shadow system for header (stronger effects)
    const int shadowLayers = 8; // Increased for stronger effect
    for (int i = shadowLayers; i > 0; --i)
    {
        const float shadowOffset = (float)i * 2.0f; // Increased offset for stronger effect
        const float shadowAlpha = (1.0f - (float)i / shadowLayers) * 0.7f; // Increased alpha for stronger effect
        
        // Outer accent glow
        g.setColour(lnf.theme.accent.withAlpha(shadowAlpha * 0.8f));
        g.drawText("FIELD", logoRect.translated(shadowOffset, shadowOffset), 
                  juce::Justification::centredLeft);
        
        // Dark shadow for depth
        g.setColour(juce::Colours::black.withAlpha(shadowAlpha * 0.9f));
        g.drawText("FIELD", logoRect.translated(shadowOffset * 0.5f, shadowOffset * 0.5f), 
                  juce::Justification::centredLeft);
    }
    
    // Enhanced gradient effect for header (stronger)
    juce::ColourGradient logoGradient(
        lnf.theme.accent.brighter(0.6f), logoRect.getX(), logoRect.getY(),
        lnf.theme.accent.darker(0.3f), logoRect.getX(), logoRect.getBottom(), false);
    logoGradient.addColour(0.5f, lnf.theme.accent);
    
    g.setGradientFill(logoGradient);
    g.drawText("FIELD", logoRect, juce::Justification::centredLeft);
    
    // Enhanced highlight for header (stronger)
    g.setColour(lnf.theme.accent.brighter(0.7f).withAlpha(0.9f));
    g.drawText("FIELD", logoRect, juce::Justification::centredLeft);
    
    // Final white highlight for shine (stronger)
    g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.drawText("FIELD", logoRect, juce::Justification::centredLeft);
}

void MyPluginAudioProcessorEditor::performLayout()
{
    if (!layoutReady) return;

    const float s = juce::jmax (0.5f, scaleFactor);
    // Use height-driven scale for vertical metrics to decouple bottom spacing from width changes
    const float sv = juce::jlimit (0.5f, 2.0f, (float) getHeight() / (float) baseHeight);
    const int bottomReserve = Layout::dp (6, sv) + Layout::dp (22, sv);
    auto r = getLocalBounds().reduced (Layout::dp (Layout::PAD, s)).withTrimmedBottom (bottomReserve);
    
    
    // Legacy value-label placement helper removed; KnobCell in Managed mode
    // positions value labels directly below the knob in its resized().

    // 1) wood bar controls (reduced height)
    auto woodBar = r.removeFromTop (Layout::dp (50, s));
    juce::Grid header;
    header.rowGap = juce::Grid::Px (Layout::dp (4, s));
    header.columnGap = juce::Grid::Px (0);
    header.alignContent = juce::Grid::AlignContent::center;
    header.justifyContent = juce::Grid::JustifyContent::center;
    header.alignItems = juce::Grid::AlignItems::center;
    header.justifyItems = juce::Grid::JustifyItems::center;
    header.templateRows = { juce::Grid::TrackInfo (juce::Grid::Fr (1)) };

    // Compute dynamic left header width based on painted logo size
    juce::Font logoFont (juce::FontOptions (26.0f * s).withStyle ("Bold"));
    const int logoTextW = (int) logoFont.getStringWidthFloat ("FIELD");
    const int leftInset = Layout::dp (20, s);
    const int bypassW   = Layout::dp (56, s);
    const int leftPaddingAfterLogo = Layout::dp (120, s); // increased gap between logo and bypass
    const int leftHeaderW = juce::jmax (Layout::dp (240, s), leftInset + logoTextW + leftPaddingAfterLogo + bypassW);

    header.templateColumns = {
        juce::Grid::TrackInfo (juce::Grid::Px (leftHeaderW)),         // left: reserve for painted logo + bypass
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (60, s))),  // spacer between left and center controls
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (360, s))), // center: preset field
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))),  // prev
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))),  // next
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))),  // A
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))),  // B
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))),  // copy
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (16, s))),  // spacer
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))),  // snap (moved left of split)
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (16, s))),  // spacer left of split
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (120, s))), // split
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))),  // link (center group)
        juce::Grid::TrackInfo (juce::Grid::Fr (1)),                   // spacer before right utilities
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (176, s))), // transport clock (right group)
        // history group removed (divider, undo, redo, history button)
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))),  // color mode (right)
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))),  // tooltips (right)
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s)))   // fullscreen (right)
    };

    const int h = Layout::dp (24, s);
    auto sizeBtn = [&](juce::Component& c, int w){ c.setSize (w, h); };

    sizeBtn (bypassButton,       Layout::dp (56, s));
    // Build left header group (bypass only)
    if (headerLeftGroup.getParentComponent() != this) addAndMakeVisible (headerLeftGroup);
    if (bypassButton.getParentComponent() != &headerLeftGroup) headerLeftGroup.addAndMakeVisible (bypassButton);
    headerLeftGroup.setBounds (0, 0, leftHeaderW, h);
    // Place bypass just after the painted logo text + padding
    bypassButton.setTopLeftPosition (leftInset + logoTextW + leftPaddingAfterLogo, 0);
    // savePresetButton removed
    sizeBtn (abButtonA,          Layout::dp (40, s));
    sizeBtn (abButtonB,          Layout::dp (40, s));
    sizeBtn (copyButton,         Layout::dp (40, s));
    sizeBtn (prevPresetButton,   Layout::dp (40, s));
    sizeBtn (nextPresetButton,   Layout::dp (40, s));
    presetNameLabel.setMinimumHorizontalScale (0.8f);
    presetNameLabel.setText ("Presets", juce::dontSendNotification);
    presetNameLabel.setJustificationType (juce::Justification::centredLeft);
    splitToggle.setSize (Layout::dp (120, s), Layout::dp (28, s));
    sizeBtn (linkButton,         Layout::dp (40, s));
    sizeBtn (snapButton,         Layout::dp (40, s));
    // Transport clock label styling and sizing (larger, right-aligned)
    {
        if (transportClockLabel.getParentComponent() != this) addAndMakeVisible (transportClockLabel);
        transportClockLabel.setJustificationType (juce::Justification::centredRight);
        transportClockLabel.setInterceptsMouseClicks (false, false);
        transportClockLabel.setText ("00:00.000", juce::dontSendNotification);
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            transportClockLabel.setColour (juce::Label::textColourId, lf->theme.text);
        }
        // Larger font
        transportClockLabel.setFont (juce::Font (juce::FontOptions (18.0f * s).withStyle ("Bold")));
        const int clockW = Layout::dp (176, s);
        transportClockLabel.setSize (clockW, h);
    }
    // history controls removed
    sizeBtn (colorModeButton,    Layout::dp (40, s));
    sizeBtn (tooltipsButton,     Layout::dp (40, s));
    sizeBtn (fullScreenButton,   Layout::dp (40, s));
    sizeBtn (optionsButton,      Layout::dp (40, s));

    header.items = {
        juce::GridItem (headerLeftGroup),
        juce::GridItem(), // spacer after bypass
        juce::GridItem (presetField),
        juce::GridItem (prevPresetButton),
        juce::GridItem (nextPresetButton),
        juce::GridItem (abButtonA),
        juce::GridItem (abButtonB),
        juce::GridItem (copyButton),
        juce::GridItem(), // spacer left of divider
        juce::GridItem (snapButton),
        juce::GridItem(), // spacer left of split
        juce::GridItem (splitToggle),
        juce::GridItem (linkButton),
        juce::GridItem(), // spacer before right utilities
        juce::GridItem (transportClockLabel),
        juce::GridItem (colorModeButton),
        juce::GridItem (tooltipsButton),
        juce::GridItem (fullScreenButton),
    };

    auto headerArea = woodBar.reduced (Layout::dp (Layout::GAP, s), Layout::dp (6, s))
                             .withTrimmedBottom (Layout::dp (8, s))
                             .withTrimmedTop (Layout::dp (2, s));
    header.performLayout (headerArea);

    // Tooltip bubble menu callback
    tooltipBubble.onMenu = [this](juce::Point<int> where)
    {
        juce::PopupMenu m; m.setLookAndFeel (&lnf);
        m.addSectionHeader ("Tooltip Options");
        m.addItem (1, "Open DYN_EQ Tooltips Doc");
        m.addItem (2, "Turn Assistant Off", tooltipAssistantOn_);
        m.showMenuAsync (juce::PopupMenu::Options().withTargetScreenArea (juce::Rectangle<int> (where.x, where.y, 1, 1)),
            [this](int r){ if (r == 1) { juce::URL::createWithoutParsing ("file://docs/notes/DYN_EQ_TOOLTIPS.md").launchInDefaultBrowser(); }
                           if (r == 2) { tooltipAssistantOn_ = false; tooltipsButton.setToggleState (false, juce::dontSendNotification); tooltipBubble.setVisible (false); repaint(); } });
    };

    if (! tooltipBubble.isOnDesktop()) addChildComponent (tooltipBubble);

    // history panel removed

    // options + phase mode at bottom-left; help to bottom-right; bottom-center panel toggle
    {
        auto bounds = getLocalBounds();
        const int padding = Layout::dp (8, s);
        const int btnW = Layout::dp (40, s);
        const int btnH = h;
        const int leftY = bounds.getBottom() - btnH - padding;
        optionsButton.setBounds (bounds.getX() + padding, leftY, btnW, btnH);

        // Place Phase button as last in the left group
        if (!phaseModeParamAttach)
        {
            if (auto* p = proc.apvts.getParameter ("phase_mode"))
            {
                phaseModeParamAttach = std::make_unique<juce::ParameterAttachment>(*p, [this](float v)
                {
                    const int idx = (int) juce::roundToInt (v * 3.0f);
                    phaseModeButton.setToggleState (idx != 0, juce::dontSendNotification);
                    phaseModeButton.repaint();
                }, nullptr);
            }
        }
        phaseModeButton.setBounds (optionsButton.getRight() + Layout::dp (8, s), leftY, btnW, btnH);
        addAndMakeVisible (phaseModeButton);
        // Shade per mode (Zero keeps inactive visual; menu shows selection)
        auto applyPhaseTint = [this]
        {
            int cur = 0;
            if (auto* p = proc.apvts.getParameter ("phase_mode"))
                if (auto* cp = dynamic_cast<juce::AudioParameterChoice*>(p)) cur = cp->getIndex();
             // Mode → colour tint (including Zero grey)
             juce::Colour tint = lnf.theme.textMuted; // Zero default
             juce::String label = "Z";
             switch (cur)
             {
                case 1: tint = lnf.theme.accent.withHue (lnf.theme.accent.getHue() + 0.12f); label = "N"; break;
                case 2: tint = lnf.theme.eq.hp;                                            label = "H"; break;
                case 3: tint = lnf.theme.accent.withHue (lnf.theme.accent.getHue() - 0.18f); label = "F"; break;
             }
             phaseModeButton.getProperties().set ("accentOverrideARGB", (int) tint.getARGB());
             phaseModeButton.getProperties().set ("iconOverrideARGB",   (int) tint.getARGB());
             phaseModeButton.getProperties().set ("labelText", label);
             // Always show solid style, including Zero (grey)
             phaseModeButton.setToggleState (true, juce::dontSendNotification);
             phaseModeButton.repaint();
        };
        applyPhaseTint();
        if (!phaseModeParamAttach)
        {
            if (auto* p = proc.apvts.getParameter ("phase_mode"))
            {
                phaseModeParamAttach = std::make_unique<juce::ParameterAttachment>(*p, [applyPhaseTint](float){ applyPhaseTint(); }, nullptr);
            }
        }
        phaseModeButton.onClick = [this, applyPhaseTint]
        {
            int cur = 0;
            if (auto* p = proc.apvts.getParameter ("phase_mode"))
                if (auto* cp = dynamic_cast<juce::AudioParameterChoice*>(p)) cur = cp->getIndex();

            TintMenuLNFEx menuLnf; menuLnf.defaultTint = lnf.theme.accent; menuLnf.hideChecks = true;
            menuLnf.setColour (juce::PopupMenu::textColourId, lnf.theme.text);

            showTintedMenu (phaseModeButton, menuLnf,
                // BUILD
                [this, cur] (juce::PopupMenu& m, TintMenuLNFEx& lnfEx)
                {
                    m.addSectionHeader ("Phase");
                    struct Row { int id; const char* text; juce::Colour tint; bool ticked; };
                    juce::Array<Row> rows;
                rows.add ({ 1, "Zero-latency",    lnf.theme.textMuted,          cur == 0 });
                rows.add ({ 2, "Natural-phase",   lnf.theme.accent.withHue (lnf.theme.accent.getHue() + 0.12f), cur == 1 });
                rows.add ({ 3, "Hybrid Linear",   lnf.theme.eq.hp,               cur == 2 });
                rows.add ({ 4, "Full Linear",     lnf.theme.accent.withHue (lnf.theme.accent.getHue() - 0.18f), cur == 3 });

                    lnfEx.itemTints.clear();
                    for (auto& r : rows) { m.addItem (r.id, r.text, true, r.ticked); lnfEx.itemTints[r.id] = r.tint; }
                },
                // RESULT
                [this, applyPhaseTint] (int r)
                {
                    if (r < 1 || r > 4) return;
                    if (auto* p = proc.apvts.getParameter ("phase_mode"))
                    {
                        p->beginChangeGesture();
                        p->setValueNotifyingHost ((float) (r - 1) / 3.0f);
                        p->endChangeGesture();
                    }
                    applyPhaseTint();
                });
        };

        // Quality button next to Phase
        if (!qualityParamAttach)
        {
            if (auto* p = proc.apvts.getParameter ("quality"))
            {
                qualityParamAttach = std::make_unique<juce::ParameterAttachment>(*p, [this](float)
                {
                    // tint is updated on demand below
                    qualityButton.repaint();
                }, nullptr);
            }
        }
        qualityButton.setBounds (phaseModeButton.getRight() + Layout::dp (8, s), leftY, btnW, btnH);
        addAndMakeVisible (qualityButton);
        auto applyQualityTint = [this]
        {
            int cur = 1; // default Standard
            if (auto* p = proc.apvts.getParameter ("quality"))
                if (auto* cp = dynamic_cast<juce::AudioParameterChoice*>(p)) cur = cp->getIndex();
            // Distinct palette from Oversampling and Phase
            juce::Colour tint = lnf.theme.textMuted; // Eco default (Grey)
            juce::String label = "E";
            switch (cur)
            {
                case 0: tint = lnf.theme.textMuted;            label = "E"; break; // Eco
                case 1: tint = lnf.theme.eq.bass;              label = "S"; break; // Standard
                case 2: tint = lnf.theme.eq.scoop;             label = "H"; break; // High
            }
            qualityButton.getProperties().set ("accentOverrideARGB", (int) tint.getARGB());
            qualityButton.getProperties().set ("iconOverrideARGB",   (int) tint.getARGB());
            qualityButton.getProperties().set ("labelText", label);
            qualityButton.setToggleState (true, juce::dontSendNotification);
            qualityButton.repaint();
        };
        applyQualityTint();
        if (!qualityParamAttach)
        {
            if (auto* p = proc.apvts.getParameter ("quality"))
            {
                qualityParamAttach = std::make_unique<juce::ParameterAttachment>(*p, [applyQualityTint](float){ applyQualityTint(); }, nullptr);
            }
        }
        qualityButton.onClick = [this, applyQualityTint]
        {
            int cur = 1;
            if (auto* p = proc.apvts.getParameter ("quality"))
                if (auto* cp = dynamic_cast<juce::AudioParameterChoice*>(p)) cur = cp->getIndex();

            TintMenuLNFEx menuLnf; menuLnf.defaultTint = lnf.theme.accent; menuLnf.hideChecks = true;
            menuLnf.setColour (juce::PopupMenu::textColourId, lnf.theme.text);

            showTintedMenu (qualityButton, menuLnf,
                // BUILD
                [this, cur] (juce::PopupMenu& m, TintMenuLNFEx& lnfEx)
                {
                    m.addSectionHeader ("Quality");
                    struct Row { int id; const char* text; juce::Colour tint; bool ticked; };
                    juce::Array<Row> rows;
                    rows.add ({ 1, "Eco",      lnf.theme.textMuted,        cur == 0 });
                    rows.add ({ 2, "Standard", lnf.theme.eq.bass,          cur == 1 });
                    rows.add ({ 3, "High",     lnf.theme.eq.scoop,         cur == 2 });
                    lnfEx.itemTints.clear();
                    for (auto& r : rows) { m.addItem (r.id, r.text, true, r.ticked); lnfEx.itemTints[r.id] = r.tint; }
                },
                // RESULT
                [this, applyQualityTint] (int r)
                {
                    if (r < 1 || r > 3) return;
                    if (auto* p = proc.apvts.getParameter ("quality"))
                    {
                        p->beginChangeGesture();
                        // map 1..3 -> 0..2
                        const int idx = r - 1;
                        if (auto* cp = dynamic_cast<juce::AudioParameterChoice*>(p))
                        {
                            const int n = cp->choices.size();
                            const float norm = n > 1 ? (float) idx / (float) (n - 1) : 0.0f;
                            p->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, norm));
                        }
                        else
                        {
                            p->setValueNotifyingHost ((float) idx / 2.0f);
                        }
                        p->endChangeGesture();
                    }
                    applyQualityTint();
                });
        };

        // Hidden position: panel bottom sits well below the bottom bar so it never clips it visually
        // Derive bottom bar boundary from actual control rectangles (no hardcoding)
        juce::Rectangle<int> bottomBarRect = optionsButton.getBounds();
        bottomBarRect = bottomBarRect.getUnion (phaseModeButton.getBounds());
        bottomBarRect = bottomBarRect.getUnion (helpButton.getBounds());
        bottomBarRect = bottomBarRect.getUnion (bottomAreaToggle.getBounds());
        const int bottomBarTop    = bottomBarRect.getY();

        // Help to bottom-right (left of resize grip)
        const int helpX = bounds.getRight() - Layout::dp (24, s) - btnW;
        const int helpY = leftY;
        helpButton.setBounds (helpX, helpY, btnW, btnH);
        addAndMakeVisible (helpButton);

        // Bottom-center toggle removed; free scrolling via viewport replaces this trigger
    }

    // divider left of Snap (Divider | Snap | Split)
    {
        auto b = snapButton.getBounds();
        const int lineH = Layout::dp (24, s);
        const int gapX  = Layout::dp (8, s);
        auto cy = b.getCentreY();
        splitDivider.setBounds (b.getX() - gapX - 1, cy - lineH/2, 4, lineH);
        splitDivider.toFront (false);
    }
    // 2) main XY area + vertical meters on right side
    {
        // Pre-compute row heights to reserve exact space for rows below, so XY/meters respond consistently
        const int lPx_rsv       = Layout::dp ((float) Layout::knobPx (Layout::Knob::L), sv);
        const int labelBand_rsv = Layout::dp (Layout::LABEL_BAND_EXTRA, sv);
        const int containerH_rsv = lPx_rsv + labelBand_rsv + Layout::dp (Layout::LABEL_BAND_EXTRA, sv);
        const int rowsTotalH_rsv = containerH_rsv * 4; // four uniform rows

        // Meters take the full right side strip (carve from full remaining area first)
        // Use grid-derived width so meters do not get too wide on large windows
        const int lPx_rs     = Layout::dp ((float) Layout::knobPx (Layout::Knob::L), s);
        const int cellW_rs   = lPx_rs + Layout::dp (8, s);
        const int colW_m     = juce::jlimit (Layout::dp (24, s), Layout::dp (56, s), juce::roundToInt (cellW_rs * 0.75f));
        const int corrW_m    = juce::jmax (Layout::dp (10, s), juce::roundToInt (colW_m * 0.5f)); // CORR is half width of others
        const int inter_m    = juce::jmax (1, Layout::dp (Layout::GAP_S, s) / 2); // tighter spacing between columns
        const int outerPadM_X= juce::jmax (1, Layout::dp (Layout::GAP_S, s));     // small side padding
        const int outerPadM_Y= Layout::dp (Layout::GAP, sv);                       // match left container vertical pad for bottom align
        const int targetStripW = colW_m * 2 + corrW_m + inter_m * 2 + outerPadM_X * 2; // IO | LR | CORR(half)
        const int metersStripW = juce::jlimit (Layout::dp (96, s), Layout::dp (240, s), targetStripW);
        // Split the remaining area: left meters container, main content container, and right sliders container
        auto metersArea = r.removeFromLeft (metersStripW);
        auto rightSlidersArea = r.removeFromRight (metersStripW); // Same width as meters for now
        auto leftArea   = r; // whatever remains after carving both meter areas
        metersArea = metersArea.reduced (outerPadM_X, outerPadM_Y);
        rightSlidersArea = rightSlidersArea.reduced (outerPadM_X, outerPadM_Y);
        leftArea   = leftArea  .reduced (Layout::dp (Layout::GAP, s), Layout::dp (Layout::GAP, sv));
        // Ensure all containers share the same top/bottom so bottoms align
        metersArea.setY (leftArea.getY());
        metersArea.setHeight (leftArea.getHeight());
        rightSlidersArea.setY (leftArea.getY());
        rightSlidersArea.setHeight (leftArea.getHeight());
        
        // Ensure the rightSlidersArea has a minimum size
        if (rightSlidersArea.getWidth() < 100) {
            rightSlidersArea.setWidth (100);
            DBG("WARNING: rightSlidersArea too narrow, forcing width to 100");
        }
        if (rightSlidersArea.getHeight() < 100) {
            rightSlidersArea.setHeight (100);
            DBG("WARNING: rightSlidersArea too short, forcing height to 100");
        }
        
        MainContentContainer.setBounds (leftArea);
        rightSlidersContainer.setBounds (rightSlidersArea);
        rightSlidersContainer.setVisible (true);
        rightSlidersContainer.toFront (false);
        metersContainer.setBounds       (metersArea);
        
        // Layout the 3D vertical sliders in rightSlidersContainer
        auto sliderArea = rightSlidersArea.reduced (10);
        const int sliderWidth = sliderArea.getWidth() / 3;
        const int sliderHeight = sliderArea.getHeight() - 40; // Leave space for labels
        
        // Debug: Check if the area is too small
        if (sliderWidth < 10 || sliderHeight < 10) {
            DBG("WARNING: Slider area too small! sliderWidth=" << sliderWidth << ", sliderHeight=" << sliderHeight);
        }
        
        // Debug: Force minimum slider sizes to ensure visibility
        const int minSliderWidth = juce::jmax (sliderWidth, 50);
        const int minSliderHeight = juce::jmax (sliderHeight, 100);
        DBG("Forced slider sizes: width=" << minSliderWidth << ", height=" << minSliderHeight);
        
        // Align sliders to match meters (full container height)
        const int containerWidth = rightSlidersContainer.getWidth();
        const int containerHeight = rightSlidersContainer.getHeight();
        const int sliderGap = 8; // 8px gap between sliders
        const int reducedSliderWidth = minSliderWidth - 8; // Reduce width to allow for gaps
        const int totalSliderWidth = (reducedSliderWidth * 3) + (sliderGap * 2);
        const int startX = (containerWidth - totalSliderWidth) / 2;
        const int startY = 0; // Start at top of container
        
        // Use full container height to match meters
        const int fullSliderHeight = containerHeight;
        const int sliderStartY = startY;
        
        inputSlider.setBounds (startX, sliderStartY, reducedSliderWidth, fullSliderHeight);
        outputSlider.setBounds (startX + reducedSliderWidth + sliderGap, sliderStartY, reducedSliderWidth, fullSliderHeight);
        mixSlider.setBounds (startX + (reducedSliderWidth + sliderGap) * 2, sliderStartY, reducedSliderWidth, fullSliderHeight);
        
        
        // Debug: Force sliders to be visible and repaint
        inputSlider.setVisible (true);
        outputSlider.setVisible (true);
        mixSlider.setVisible (true);
        inputSlider.repaint();
        outputSlider.repaint();
        mixSlider.repaint();
        
        
        
        
        
        // Force main editor repaint to show containers
        repaint();
        
        // Labels will be styled with knobcell background in their paint method
        
        // Sliders are already visible from addAndMakeVisible in constructor
        
        // Debug: Add some logging to see what's happening
        DBG("rightSlidersArea (right sliders): " << rightSlidersArea.toString());
        DBG("metersArea (left meters): " << metersArea.toString());
        DBG("sliderArea: " << sliderArea.toString());
        DBG("sliderWidth: " << sliderWidth << ", sliderHeight: " << sliderHeight);
        DBG("inputSlider bounds: " << inputSlider.getBounds().toString());
        DBG("inputSlider bounds: " << inputSlider.getBounds().toString());
        DBG("outputSlider bounds: " << outputSlider.getBounds().toString());
        DBG("mixSlider bounds: " << mixSlider.getBounds().toString());
        DBG("minSliderWidth: " << minSliderWidth << ", minSliderHeight: " << minSliderHeight);
        DBG("rightSlidersContainer child count: " << rightSlidersContainer.getNumChildComponents());
        
        // Debug: Test if sliders are actually inside the container bounds
        DBG("Slider positioning test:");
        DBG("  Container bounds: " << rightSlidersContainer.getBounds().toString());
        DBG("  Slider area: " << sliderArea.toString());
        // (Reverted) bottom toggle remains a direct child of the editor; positioned earlier

        // Allocate the top area to full height (legacy rows disabled)
        const int mainH = juce::jmax (Layout::dp (Layout::XY_MIN_H, s), r.getHeight());
        auto main = r.removeFromTop (mainH);
        
        // Visual dock takes the pad area exactly matching MainContentContainer width
        auto padLocal = MainContentContainer.getLocalBounds()
                           .removeFromTop (mainH)
                           .reduced (Layout::dp (Layout::GAP, s), Layout::dp (Layout::GAP, sv));
        if (panes)
        {
            if (panes->getParentComponent() != &MainContentContainer) MainContentContainer.addAndMakeVisible (*panes);
            panes->setBounds (padLocal);
        }
        if (xyShade)
        {
            // Keep ShadeOverlay as direct child of main editor for setAlwaysOnTop to work
            if (xyShade->getParentComponent() != this) addAndMakeVisible (*xyShade);
            // Extend ShadeOverlay bounds to include tab area above the pad
            // Convert padLocal (relative to MainContentContainer) to main editor coordinates
            auto extendedBounds = padLocal;
            const int tabHeight = 40;  // Tab height from PaneManager
            const int handleOffset = -10;  // Further reduced space for handle above tabs (was 20px, now -10px)
            extendedBounds.setY(extendedBounds.getY() - tabHeight - handleOffset);
            extendedBounds.setHeight(extendedBounds.getHeight() + tabHeight + handleOffset);
            
            // Convert to main editor coordinates by adding MainContentContainer position
            extendedBounds.setX(extendedBounds.getX() + MainContentContainer.getX());
            extendedBounds.setY(extendedBounds.getY() + MainContentContainer.getY());
            
            xyShade->setBounds (extendedBounds);
            // setAlwaysOnTop(true) in constructor should make it appear above everything
        }

        // Hide center container if present
        phaseCenterContainer.setVisible (false);

        // Layout meters stack: [Corr] [IO (vertical In/Out)] [LR (vertical)] side-by-side inside their panels
        // Mount meters as children of metersContainer to isolate z-order and clipping
        if (corrMeter.getParentComponent() != &metersContainer) metersContainer.addAndMakeVisible (corrMeter);
        if (lrMeters.getParentComponent()   != &metersContainer) metersContainer.addAndMakeVisible (lrMeters);
        if (ioMeters.getParentComponent()   != &metersContainer) metersContainer.addAndMakeVisible (ioMeters);
        auto mB = metersContainer.getLocalBounds();
        // Lay out three tight columns: IO | LR | CORR (CORR half width), minimal spacing
        int x = mB.getX();
        const int h = mB.getHeight();
        const int meterBottomPadding = Layout::dp (8, s); // Add padding to bottom of meters
        auto ioCol   = juce::Rectangle<int> (x, mB.getY(), colW_m, h - meterBottomPadding); x += colW_m + inter_m;
        auto lrCol   = juce::Rectangle<int> (x, mB.getY(), colW_m, h - meterBottomPadding); x += colW_m + inter_m;
        auto corrCol = juce::Rectangle<int> (x, mB.getY(), corrW_m, h - meterBottomPadding);
        ioMeters.setBounds   (ioCol);
        lrMeters.setBounds   (lrCol);
        corrMeter.setBounds  (corrCol);
    }

    // Predeclare legacy layout metrics (used only if legacy path is compiled/executed)
    const int lPx       = Layout::dp ((float) Layout::knobPx (Layout::Knob::L), s);
    const int valuePxCommon = Layout::dp (14, s);
    const int labelGapCommon = Layout::dp (2, s); // Reduced from 4 to 2 for tighter spacing
    const int containerHeight = lPx + labelGapCommon + valuePxCommon;
    const int rowH1 = containerHeight, rowH2 = containerHeight, rowH3 = containerHeight, rowH4 = containerHeight;
    juce::Rectangle<int> row1, row2, row3, row4;

    // Legacy controls are disabled; skip legacy layout entirely
    if (false)
    {
        const int gapI      = Layout::dp (Layout::GAP_S, s);
        const int dividerW  = Layout::dp (8, s); // global divider column width (thicker + more spacing)
        // microH no longer used (minis integrated into cells)

        // Compute desired delay column width, but do not carve a separate right-hand container.
        // We'll integrate delay into the same 4-row system and just reserve a right strip later.
        const int delayCols = 7;
        const int cellW_right = lPx + Layout::dp (8, s);
        const int delayCardW = delayCols * cellW_right + Layout::dp (Layout::PAD, s);
        juce::Rectangle<int> delayArea; // to be computed after rows are defined
        // Capture the full rows area (left column) for overlay sizing
        auto rowsArea = r; // rows live directly below the panes (after main area was removed)
        row1 = r.removeFromTop (rowH1);
        row2 = r.removeFromTop (rowH2);
        row3 = r.removeFromTop (rowH3);
        row4 = r.removeFromTop (rowH4);
        // Align left content container bottom to the bottom of Row 4 to remove extra space
        {
            const int rowsBottom = row4.getBottom();
            auto lc = MainContentContainer.getBounds();
            lc.setBottom (rowsBottom);
            MainContentContainer.setBounds (lc);
            // Align meters container bottom to the same baseline
            auto mc = metersContainer.getBounds();
            mc.setBottom (rowsBottom);
            metersContainer.setBounds (mc);
        }
        // Re-layout meter columns now that meters container height is final
        {
            const int lPx_rs   = Layout::dp ((float) Layout::knobPx (Layout::Knob::L), s);
            const int cellW_rs = lPx_rs + Layout::dp (8, s);
            const int colW_m   = juce::jlimit (Layout::dp (24, s), Layout::dp (56, s), juce::roundToInt (cellW_rs * 0.75f));
            const int corrW_m  = juce::jmax (Layout::dp (10, s), juce::roundToInt (colW_m * 0.5f));
            const int inter_m  = juce::jmax (1, Layout::dp (Layout::GAP_S, s) / 2);
            auto mB = metersContainer.getLocalBounds();
            int x = mB.getX();
            const int h = mB.getHeight();
            const int meterBottomPadding = Layout::dp (8, s); // Add padding to bottom of meters
            auto ioCol   = juce::Rectangle<int> (x, mB.getY(), colW_m, h - meterBottomPadding); x += colW_m + inter_m;
            auto lrCol   = juce::Rectangle<int> (x, mB.getY(), colW_m, h - meterBottomPadding); x += colW_m + inter_m;
            auto corrCol = juce::Rectangle<int> (x, mB.getY(), corrW_m, h - meterBottomPadding);
            ioMeters.setBounds   (ioCol);
            lrMeters.setBounds   (lrCol);
            corrMeter.setBounds  (corrCol);
        }
    }
    // Alternate bottom panel (slides over bottom rows when enabled)
    if (true)
    {
        if (controlsViewport.getParentComponent() == &MainContentContainer)
            MainContentContainer.removeChildComponent (&controlsViewport);
        group1Container.setVisible (false);
        group2Container.setVisible (false);
    }
    if (false)
    {

        // Reserve a rectangle covering bottom rows (1..4), just below the XY pad
        // Stop position (active): panel bottom sits above the bottom bar by a small gap
        // Hidden position: panel bottom sits well below the bottom bar so it never clips it visually
        // Derive bottom bar boundary from actual control rectangles (no hardcoding)
        juce::Rectangle<int> bottomBarRect = optionsButton.getBounds();
        bottomBarRect = bottomBarRect.getUnion (phaseModeButton.getBounds());
        bottomBarRect = bottomBarRect.getUnion (helpButton.getBounds());
        bottomBarRect = bottomBarRect.getUnion (bottomAreaToggle.getBounds());
        const int bottomBarTop    = bottomBarRect.getY();
        const int bottomBarBottom = bottomBarRect.getBottom();
        const int stackTop        = row1.getY();
        const int activeGapPx     = Layout::dp (20,  s);  // keep a tight visual gap when active
        const int hiddenGapPx     = Layout::dp (100, s);  // move panel further DOWN when hidden
        overlayActiveBaseline  = juce::jmax (stackTop, bottomBarTop - activeGapPx);
        overlayHiddenBaseline  = bottomBarBottom + hiddenGapPx;
        overlayHeightPx        = juce::jmax (0, overlayActiveBaseline - stackTop);

        // Mount viewport over the 4 control rows area and stack Group 1 (top) and Group 2 (below)
        const int totalRowsH_local = rowH1 + rowH2 + rowH3 + rowH4;
        controlRowsHeightPx = totalRowsH_local;
        auto rowsLocalRect = MainContentContainer.getLocalBounds().removeFromBottom (totalRowsH_local);
        if (controlsViewport.getParentComponent() != &MainContentContainer)
            MainContentContainer.addAndMakeVisible (controlsViewport);
        controlsViewport.setBounds (rowsLocalRect);
        controlsViewport.setScrollBarsShown (true, false);
        controlsViewport.setInterceptsMouseClicks (true, true);
        // Theme the vertical scrollbar with accent
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            auto& vsb = controlsViewport.getVerticalScrollBar();
            vsb.setColour (juce::ScrollBar::thumbColourId, lf->theme.accent);
            vsb.setColour (juce::ScrollBar::trackColourId, lf->theme.panel.darker (0.10f));
        }
        if (group1Container.getParentComponent() != &controlsContent)
            controlsContent.addAndMakeVisible (group1Container);
        if (group2Container.getParentComponent() != &controlsContent)
            controlsContent.addAndMakeVisible (group2Container);
        group1Container.setBounds (juce::Rectangle<int> (0, 0, rowsLocalRect.getWidth(), totalRowsH_local));
        group2Container.setBounds (juce::Rectangle<int> (0, totalRowsH_local, rowsLocalRect.getWidth(), totalRowsH_local));
        controlsContent.setBounds (juce::Rectangle<int> (0, 0, rowsLocalRect.getWidth(), totalRowsH_local * 2));
        controlsViewport.setViewedComponent (&controlsContent, false);

        // Remove reliance on bottomAreaToggle; viewport scrolling will replace this trigger

        // Motion Engine: removed from Group 2. Lives only in Group 1 flat grid.
        // Group 2 content local bounds (will be used in viewport stack)
        auto b = juce::Rectangle<int> (0, 0, group2Container.getWidth(), totalRowsH_local);
        // Delay group positioned directly in Group 2 panel (no container)
        const int availableWLocal = b.getWidth();
        const int cellWBase = lPx + Layout::dp (8, s);
        const int cellWFit  = juce::jmax (1, availableWLocal / 16); // two 8-col blocks (Delay + Reverb)
        const int cellW     = juce::jmin (cellWBase, cellWFit);
        const int delayGroupX = b.getX();
        const int delayGroupW = 8 * cellW; // 8 columns for delay items
        const int delayGroupH = b.getHeight();
        const int delayGroupY = 0;
        
        // Create and layout delay items in exact 4x7 order
        const int valuePx = Layout::dp (14, s);
        const int labelGap = Layout::dp (4, s);
        const int delayCellW = cellW;
        
        // Build overlay contents once; mark dirty for reflow when size/scale changes
        if (!overlayContentsBuilt)
        {
            // Create delay switch cells
            if (!delayEnabledCell) { delayEnabled.setComponentID ("delayEnabled"); delayEnabled.getProperties().set ("iconType", (int) IconSystem::Power); delayEnabledCell = std::make_unique<SwitchCell> (delayEnabled); delayEnabledCell->setCaption ("Enable"); delayEnabledCell->setDelayTheme (true); }
            if (!delayModeCell)    { delayModeCell    = std::make_unique<SwitchCell> (delayMode);    delayModeCell->setCaption ("Mode"); delayMode.getProperties().set ("iconOnly", true); delayModeCell->setDelayTheme (true); }
            if (!delaySyncCell)    { delaySync.getProperties().set ("iconType", (int) IconSystem::Link); delaySyncCell    = std::make_unique<SwitchCell> (delaySync);    delaySyncCell->setCaption ("Sync"); delaySyncCell->setDelayTheme (true); }
            if (!delayGridFlavorSegments) delayGridFlavorSegments = std::make_unique<Segmented3Control>(proc.apvts, "delay_grid_flavor", juce::StringArray{ "S", "D", "T" });
            if (!delayGridFlavorCell) { delayGridFlavorCell = std::make_unique<SwitchCell> (*delayGridFlavorSegments); delayGridFlavorCell->setCaption ("Feel"); delayGridFlavorCell->setDelayTheme (true); }
            if (!delayPingpongCell)   { delayPingpong.getProperties().set ("iconType", (int) IconSystem::Stereo); delayPingpongCell = std::make_unique<SwitchCell> (delayPingpong); delayPingpongCell->setCaption ("Ping-Pong"); delayPingpongCell->setDelayTheme (true); }
            if (!delayFreezeCell)     { delayFreeze.getProperties().set ("iconType", (int) IconSystem::Snowflake); delayFreezeCell  = std::make_unique<SwitchCell> (delayFreeze);   delayFreezeCell->setCaption ("Freeze"); delayFreezeCell->setDelayTheme (true); }
            if (!delayKillDryCell)    { delayKillDry.getProperties().set ("iconType", (int) IconSystem::Mix); delayKillDryCell = std::make_unique<SwitchCell> (delayKillDry);  delayKillDryCell->setCaption ("Wet Only"); delayKillDry.setTooltip ("Wet Only: Removes the dry signal from the output (effects only)"); delayKillDryCell->setDelayTheme (true); }
            if (!delayFilterTypeCell) { delayFilterTypeCell = std::make_unique<SwitchCell> (delayFilterType); delayFilterTypeCell->setCaption ("Filter"); delayFilterTypeCell->setDelayTheme (true); }
            if (!delayDuckSourceCell) { delayDuckSourceCell = std::make_unique<SwitchCell> (delayDuckSource); delayDuckSourceCell->setCaption ("Duck Source"); delayDuckSourceCell->setDelayTheme (true); }
            if (!delayDuckPostCell)   { delayDuckPost.getProperties().set ("iconType", (int) IconSystem::RightArrow); delayDuckPostCell = std::make_unique<SwitchCell> (delayDuckPost); delayDuckPostCell->setCaption ("Post"); delayDuckPostCell->setDelayTheme (true); }

            // Create delay knob cells
            if (!delayTimeCell)      delayTimeCell       = std::make_unique<KnobCell>(delayTime,      delayTimeValue,      "TIME");
            if (!delayFeedbackCell)  delayFeedbackCell   = std::make_unique<KnobCell>(delayFeedback,  delayFeedbackValue,  "FB");
            if (!delayWetCell)       delayWetCell        = std::make_unique<KnobCell>(delayWet,       delayWetValue,       "WET");
            if (!delaySpreadCell)    delaySpreadCell     = std::make_unique<KnobCell>(delaySpread,    delaySpreadValue,    "SPREAD");
            if (!delayWidthCell)     delayWidthCell      = std::make_unique<KnobCell>(delayWidth,     delayWidthValue,     "WIDTH");
            if (!delayModRateCell)   delayModRateCell    = std::make_unique<KnobCell>(delayModRate,   delayModRateValue,   "RATE");
            if (!delayModDepthCell)  delayModDepthCell   = std::make_unique<KnobCell>(delayModDepth,  delayModDepthValue,  "DEPTH");
            if (!delayWowflutterCell)delayWowflutterCell = std::make_unique<KnobCell>(delayWowflutter,delayWowflutterValue,"WOW");
            if (!delayJitterCell)    delayJitterCell     = std::make_unique<KnobCell>(delayJitter,    delayJitterValue,    "JITTER");
            if (!delayHpCell)        delayHpCell         = std::make_unique<KnobCell>(delayHp,        delayHpValue,        "HP");
            if (!delayLpCell)        delayLpCell         = std::make_unique<KnobCell>(delayLp,        delayLpValue,        "LP");
            if (!delayTiltCell)      delayTiltCell       = std::make_unique<KnobCell>(delayTilt,      delayTiltValue,      "TILT");
            if (!delaySatCell)       delaySatCell        = std::make_unique<KnobCell>(delaySat,       delaySatValue,       "SAT");
            if (!delayDiffusionCell) delayDiffusionCell  = std::make_unique<KnobCell>(delayDiffusion, delayDiffusionValue, "DIFF");
            if (!delayDiffuseSizeCell)delayDiffuseSizeCell= std::make_unique<KnobCell>(delayDiffuseSize, delayDiffuseSizeValue, "SIZE");
            if (!delayDuckDepthCell) delayDuckDepthCell  = std::make_unique<KnobCell>(delayDuckDepth, delayDuckDepthValue, "DEPTH");
            if (!delayDuckAttackCell)delayDuckAttackCell = std::make_unique<KnobCell>(delayDuckAttack,delayDuckAttackValue,"ATT");
            if (!delayDuckReleaseCell)delayDuckReleaseCell=std::make_unique<KnobCell>(delayDuckRelease,delayDuckReleaseValue,"REL");
            if (!delayDuckThresholdCell) delayDuckThresholdCell = std::make_unique<KnobCell>(delayDuckThreshold, delayDuckThresholdValue, "THR");
            if (!delayDuckLookaheadCell) delayDuckLookaheadCell = std::make_unique<KnobCell>(delayDuckLookahead, delayDuckLookaheadValue, "LA");
            if (!delayDuckRatioCell) delayDuckRatioCell = std::make_unique<KnobCell>(delayDuckRatio, delayDuckRatioValue, "RAT");
            if (!delayPreDelayCell) delayPreDelayCell = std::make_unique<KnobCell>(delayPreDelay, delayPreDelayValue, "PRE");

            // Add all delay cells directly to the Group 2 panel (once)
            for (auto* c : { delayEnabledCell.get(), delayModeCell.get(), delaySyncCell.get(), delayGridFlavorCell.get(), delayPingpongCell.get(), delayFreezeCell.get(), delayKillDryCell.get(), delayFilterTypeCell.get(), delayDuckSourceCell.get(), delayDuckPostCell.get() })
                if (c) { group2Container.addAndMakeVisible (*c); c->setShowBorder (true); }
            for (auto* c : { delayTimeCell.get(), delayFeedbackCell.get(), delayWetCell.get(), delaySpreadCell.get(), delayWidthCell.get(), delayModRateCell.get(), delayModDepthCell.get(), delayWowflutterCell.get(), delayJitterCell.get(), delayPreDelayCell.get(), delayHpCell.get(), delayLpCell.get(), delayTiltCell.get(), delaySatCell.get(), delayDiffusionCell.get(), delayDiffuseSizeCell.get(), delayDuckDepthCell.get(), delayDuckAttackCell.get(), delayDuckReleaseCell.get(), delayDuckThresholdCell.get(), delayDuckLookaheadCell.get(), delayDuckRatioCell.get() })
                if (c) { group2Container.addAndMakeVisible (*c); c->setShowBorder (true); }

            overlayContentsBuilt = true;
            overlayLayoutDirty = true;
        }
        if (!delayEnabledCell) { delayEnabled.setComponentID ("delayEnabled"); delayEnabled.getProperties().set ("iconType", (int) IconSystem::Power); delayEnabledCell = std::make_unique<SwitchCell> (delayEnabled); delayEnabledCell->setCaption ("Enable"); delayEnabledCell->setDelayTheme (true); }
        if (!delayModeCell)    { delayModeCell    = std::make_unique<SwitchCell> (delayMode);    delayModeCell->setCaption ("Mode"); delayMode.getProperties().set ("iconOnly", true); delayModeCell->setDelayTheme (true); }
        if (!delaySyncCell)    { delaySync.getProperties().set ("iconType", (int) IconSystem::Link); delaySyncCell    = std::make_unique<SwitchCell> (delaySync);    delaySyncCell->setCaption ("Sync"); delaySyncCell->setDelayTheme (true); }
        if (!delayGridFlavorSegments) delayGridFlavorSegments = std::make_unique<Segmented3Control>(proc.apvts, "delay_grid_flavor", juce::StringArray{ "S", "D", "T" });
        if (!delayGridFlavorCell) { delayGridFlavorCell = std::make_unique<SwitchCell> (*delayGridFlavorSegments); delayGridFlavorCell->setCaption ("Feel"); delayGridFlavorCell->setDelayTheme (true); }
        if (!delayPingpongCell)   { delayPingpong.getProperties().set ("iconType", (int) IconSystem::Stereo); delayPingpongCell = std::make_unique<SwitchCell> (delayPingpong); delayPingpongCell->setCaption ("Ping-Pong"); delayPingpongCell->setDelayTheme (true); }
        if (!delayFreezeCell)     { delayFreeze.getProperties().set ("iconType", (int) IconSystem::Snowflake); delayFreezeCell  = std::make_unique<SwitchCell> (delayFreeze);   delayFreezeCell->setCaption ("Freeze"); delayFreezeCell->setDelayTheme (true); }
        if (!delayKillDryCell)    { delayKillDry.getProperties().set ("iconType", (int) IconSystem::Mix); delayKillDryCell = std::make_unique<SwitchCell> (delayKillDry);  delayKillDryCell->setCaption ("Wet Only"); delayKillDry.setTooltip ("Wet Only: Removes the dry signal from the output (effects only)"); delayKillDryCell->setDelayTheme (true); }
        if (!delayFilterTypeCell) { delayFilterTypeCell = std::make_unique<SwitchCell> (delayFilterType); delayFilterTypeCell->setCaption ("Filter"); delayFilterTypeCell->setDelayTheme (true); }
        if (!delayDuckSourceCell) { delayDuckSourceCell = std::make_unique<SwitchCell> (delayDuckSource); delayDuckSourceCell->setCaption ("Duck Source"); delayDuckSourceCell->setDelayTheme (true); }
        if (!delayDuckPostCell)   { delayDuckPost.getProperties().set ("iconType", (int) IconSystem::RightArrow); delayDuckPostCell = std::make_unique<SwitchCell> (delayDuckPost); delayDuckPostCell->setCaption ("Post"); delayDuckPostCell->setDelayTheme (true); }
        
        // Create delay knob cells
        if (!delayTimeCell)      delayTimeCell       = std::make_unique<KnobCell>(delayTime,      delayTimeValue,      "TIME");
        if (!delayFeedbackCell)  delayFeedbackCell   = std::make_unique<KnobCell>(delayFeedback,  delayFeedbackValue,  "FB");
        if (!delayWetCell)       delayWetCell        = std::make_unique<KnobCell>(delayWet,       delayWetValue,       "WET");
        if (!delaySpreadCell)    delaySpreadCell     = std::make_unique<KnobCell>(delaySpread,    delaySpreadValue,    "SPREAD");
        if (!delayWidthCell)     delayWidthCell      = std::make_unique<KnobCell>(delayWidth,     delayWidthValue,     "WIDTH");
        if (!delayModRateCell)   delayModRateCell    = std::make_unique<KnobCell>(delayModRate,   delayModRateValue,   "RATE");
        if (!delayModDepthCell)  delayModDepthCell   = std::make_unique<KnobCell>(delayModDepth,  delayModDepthValue,  "DEPTH");
        if (!delayWowflutterCell)delayWowflutterCell = std::make_unique<KnobCell>(delayWowflutter,delayWowflutterValue,"WOW");
        if (!delayJitterCell)    delayJitterCell     = std::make_unique<KnobCell>(delayJitter,    delayJitterValue,    "JITTER");
        if (!delayHpCell)        delayHpCell         = std::make_unique<KnobCell>(delayHp,        delayHpValue,        "HP");
        if (!delayLpCell)        delayLpCell         = std::make_unique<KnobCell>(delayLp,        delayLpValue,        "LP");
        if (!delayTiltCell)      delayTiltCell       = std::make_unique<KnobCell>(delayTilt,      delayTiltValue,      "TILT");
        if (!delaySatCell)       delaySatCell        = std::make_unique<KnobCell>(delaySat,       delaySatValue,       "SAT");
        if (!delayDiffusionCell) delayDiffusionCell  = std::make_unique<KnobCell>(delayDiffusion, delayDiffusionValue, "DIFF");
        if (!delayDiffuseSizeCell)delayDiffuseSizeCell= std::make_unique<KnobCell>(delayDiffuseSize, delayDiffuseSizeValue, "SIZE");
        if (!delayDuckDepthCell) delayDuckDepthCell  = std::make_unique<KnobCell>(delayDuckDepth, delayDuckDepthValue, "DEPTH");
        if (!delayDuckAttackCell)delayDuckAttackCell = std::make_unique<KnobCell>(delayDuckAttack,delayDuckAttackValue,"ATT");
        if (!delayDuckReleaseCell)delayDuckReleaseCell=std::make_unique<KnobCell>(delayDuckRelease,delayDuckReleaseValue,"REL");
        if (!delayDuckThresholdCell) delayDuckThresholdCell = std::make_unique<KnobCell>(delayDuckThreshold, delayDuckThresholdValue, "THR");
        if (!delayDuckLookaheadCell) delayDuckLookaheadCell = std::make_unique<KnobCell>(delayDuckLookahead, delayDuckLookaheadValue, "LA");
        if (!delayDuckRatioCell) delayDuckRatioCell = std::make_unique<KnobCell>(delayDuckRatio, delayDuckRatioValue, "RAT");
        if (!delayPreDelayCell) delayPreDelayCell = std::make_unique<KnobCell>(delayPreDelay, delayPreDelayValue, "PRE");
        
        // Apply metrics to all delay cells
        for (auto* c : { delayTimeCell.get(), delayFeedbackCell.get(), delayWetCell.get(), delaySpreadCell.get(), delayWidthCell.get(), delayModRateCell.get(), delayModDepthCell.get(), delayWowflutterCell.get(), delayJitterCell.get(), delayPreDelayCell.get(), delayHpCell.get(), delayLpCell.get(), delayTiltCell.get(), delaySatCell.get(), delayDiffusionCell.get(), delayDiffuseSizeCell.get(), delayDuckDepthCell.get(), delayDuckAttackCell.get(), delayDuckReleaseCell.get(), delayDuckThresholdCell.get(), delayDuckLookaheadCell.get(), delayDuckRatioCell.get() })
        {
            if (c) {
                c->setMetrics (lPx, valuePx, labelGap);
                c->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
                c->setValueLabelGap (labelGap);
            }
        }
        
        // Add all delay cells directly to the Group 2 panel
        for (auto* c : { delayEnabledCell.get(), delayModeCell.get(), delaySyncCell.get(), delayGridFlavorCell.get(), delayPingpongCell.get(), delayFreezeCell.get(), delayKillDryCell.get(), delayFilterTypeCell.get(), delayDuckSourceCell.get(), delayDuckPostCell.get() })
            if (c) { group2Container.addAndMakeVisible (*c); c->setShowBorder (true); }
            
        for (auto* c : { delayTimeCell.get(), delayFeedbackCell.get(), delayWetCell.get(), delaySpreadCell.get(), delayWidthCell.get(), delayModRateCell.get(), delayModDepthCell.get(), delayWowflutterCell.get(), delayJitterCell.get(), delayPreDelayCell.get(), delayHpCell.get(), delayLpCell.get(), delayTiltCell.get(), delaySatCell.get(), delayDiffusionCell.get(), delayDiffuseSizeCell.get(), delayDuckDepthCell.get(), delayDuckAttackCell.get(), delayDuckReleaseCell.get(), delayDuckThresholdCell.get(), delayDuckLookaheadCell.get(), delayDuckRatioCell.get() })
            if (c) { group2Container.addAndMakeVisible (*c); c->setShowBorder (true); }
        
        // Layout delay items in exact 4x7 order (only when dirty)
        if (overlayLayoutDirty)
        {
        juce::Grid delayGrid;
        delayGrid.rowGap = juce::Grid::Px (0);
        delayGrid.columnGap = juce::Grid::Px (0);
        delayGrid.templateRows = { juce::Grid::Px (containerHeight), juce::Grid::Px (containerHeight), juce::Grid::Px (containerHeight), juce::Grid::Px (containerHeight) };
        delayGrid.templateColumns = {
            juce::Grid::Px (delayCellW), juce::Grid::Px (delayCellW), juce::Grid::Px (delayCellW), juce::Grid::Px (delayCellW),
            juce::Grid::Px (delayCellW), juce::Grid::Px (delayCellW), juce::Grid::Px (delayCellW), juce::Grid::Px (delayCellW)
        };
        delayGrid.items = {
            // Row 1 — Switches/Combos (8 items)
            juce::GridItem (*delayEnabledCell).withArea (1,1),
            juce::GridItem (*delayModeCell).withArea (1,2),
            juce::GridItem (*delaySyncCell).withArea (1,3),
            juce::GridItem (*delayGridFlavorCell).withArea (1,4),
            juce::GridItem (*delayPingpongCell).withArea (1,5),
            juce::GridItem (*delayFreezeCell).withArea (1,6),
            juce::GridItem (*delayFilterTypeCell).withArea (1,7),
            juce::GridItem (*delayKillDryCell).withArea (1,8),
            
            // Row 2 — Time & Mod Essentials (8 items)
            juce::GridItem (*delayTimeCell).withArea (2,1),
            juce::GridItem (*delayFeedbackCell).withArea (2,2),
            juce::GridItem (*delayWetCell).withArea (2,3),
            juce::GridItem (*delayModRateCell).withArea (2,4),
            juce::GridItem (*delayModDepthCell).withArea (2,5),
            juce::GridItem (*delaySpreadCell).withArea (2,6),
            juce::GridItem (*delayWidthCell).withArea (2,7),
            juce::GridItem (*delayPreDelayCell).withArea (2,8),
            
            // Row 3 — Tone & Reverb (8 items)
            juce::GridItem (*delaySatCell).withArea (3,1),
            juce::GridItem (*delayDiffusionCell).withArea (3,2),
            juce::GridItem (*delayDiffuseSizeCell).withArea (3,3),
            juce::GridItem (*delayHpCell).withArea (3,4),
            juce::GridItem (*delayLpCell).withArea (3,5),
            juce::GridItem (*delayTiltCell).withArea (3,6),
            juce::GridItem (*delayWowflutterCell).withArea (3,7),
            juce::GridItem (*delayJitterCell).withArea (3,8),
            
            // Row 4 — Ducking Cluster (8 items)
            juce::GridItem (*delayDuckSourceCell).withArea (4,1),
            juce::GridItem (*delayDuckPostCell).withArea (4,2),
            juce::GridItem (*delayDuckThresholdCell).withArea (4,3),
            juce::GridItem (*delayDuckDepthCell).withArea (4,4),
            juce::GridItem (*delayDuckAttackCell).withArea (4,5),
            juce::GridItem (*delayDuckReleaseCell).withArea (4,6),
            juce::GridItem (*delayDuckLookaheadCell).withArea (4,7),
            juce::GridItem (*delayDuckRatioCell).withArea (4,8)
        };
        
        // Perform layout within delay group area (no outer reduction/padding) in group2Container local coords
        auto delayBounds = juce::Rectangle<int>(delayGroupX, delayGroupY, delayGroupW, delayGroupH);
        delayGrid.performLayout(delayBounds);
        }

        // Delay visuals are rendered in the top Delay tab via PaneManager, not in Group 2
        
        // Apply delay theme to all cells
        auto lightenDelayCell = [] (KnobCell* kc) {
            if (!kc) return;
            kc->getProperties().set ("panelBrighten", 0.10);
            kc->getProperties().set ("borderBrighten", 0.15);
            kc->getProperties().set ("delayThemeBorderTextGrey", true);
            kc->repaint();
        };
        for (auto* kc : { delayTimeCell.get(), delayFeedbackCell.get(), delayWetCell.get(), delayModRateCell.get(), delayModDepthCell.get(), delaySpreadCell.get(), delayWidthCell.get(),
                           delaySatCell.get(), delayDiffusionCell.get(), delayDiffuseSizeCell.get(), delayHpCell.get(), delayLpCell.get(), delayTiltCell.get(), delayWowflutterCell.get(),
                           delayJitterCell.get(), delayPreDelayCell.get(), delayDuckThresholdCell.get(), delayDuckDepthCell.get(), delayDuckAttackCell.get(), delayDuckReleaseCell.get(), delayDuckLookaheadCell.get(), delayDuckRatioCell.get() })
            lightenDelayCell (kc);

        // Reverb controls group to the right of Delay group in Group 2 (flattened 4x16 grid; 8 columns used)
        {
            const int reverbGroupX = delayGroupX + delayGroupW;
            const int reverbGroupY = 0;
            const int availableW  = b.getRight() - reverbGroupX;
            const int reverbCellW = cellW;
            const int targetReverbW = reverbCellW * 8; // 8 columns
            const int reverbGroupW = juce::jmin (availableW, targetReverbW);
            const int reverbGroupH = delayGroupH;

            static std::unique_ptr<class ReverbControlsPanel> rvPanel;
            if (!rvPanel)
                rvPanel = std::make_unique<ReverbControlsPanel> (proc.apvts);

            // DynEQ sub-pane is created but not laid out here; wiring is deferred

            // Mirror Delay metrics for Reverb grid so KnobCells match exactly
            const int valuePx2 = Layout::dp (14, s);
            const int labelGap2 = Layout::dp (4, s);
            const int delayCellW = cellW;
            rvPanel->setCellMetrics (lPx, valuePx2, labelGap2, delayCellW);
            rvPanel->setRowHeightPx (containerHeight);

            // Add cells directly to Group 2 (no container)
            juce::Array<KnobCell*> rvCells;
            rvPanel->collectCells (rvCells);
            auto ensureAdd = [this](juce::Component* c)
            {
                if (!c) return;
                if (c->getParentComponent() != &group2Container)
                    group2Container.addAndMakeVisible (*c);
                if (auto* kc = dynamic_cast<KnobCell*>(c))
                {
                    kc->setShowBorder (true);
                    kc->getProperties().set ("reverbMaroonBorder", true);
                }
                else if (auto* sc = dynamic_cast<SwitchCell*>(c))
                {
                    sc->getProperties().set ("reverbMaroonBorder", true);
                }
            };

            // Create Enable (switch), Algorithm (switch-like), Wet Only (switch) styled like Delay/Motion
            static juce::ToggleButton reverbEnable;
            static std::unique_ptr<SwitchCell> reverbEnableCell;
            if (!reverbEnableCell) {
                reverbEnable.setComponentID ("reverb_enable");
                reverbEnable.getProperties().set ("iconType", (int) IconSystem::Power);
                reverbEnableCell = std::make_unique<SwitchCell> (reverbEnable);
                reverbEnableCell->setCaption ("Enable");
                reverbEnableCell->setDelayTheme (false);
                buttonAttachments.push_back (std::make_unique<ButtonAttachment> (proc.apvts, ReverbIDs::enabled, reverbEnable));
            }

            static juce::ComboBox reverbAlgo;
            static std::unique_ptr<SwitchCell> reverbAlgoCell;
            if (!reverbAlgoCell) {
                if (auto* ch = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter(ReverbIDs::algo))) {
                    reverbAlgo.clear(); for (int i = 0; i < ch->choices.size(); ++i) reverbAlgo.addItem (ch->choices[i], i + 1);
                    reverbAlgo.setSelectedId (ch->getIndex() + 1, juce::dontSendNotification);
                    comboAttachments.push_back (std::make_unique<ComboAttachment> (proc.apvts, ReverbIDs::algo, reverbAlgo));
                }
                reverbAlgo.getProperties().set ("iconOnly", true);
                reverbAlgoCell = std::make_unique<SwitchCell> (reverbAlgo);
                reverbAlgoCell->setCaption ("Algo");
                reverbAlgoCell->setDelayTheme (false);
            }

            static juce::ToggleButton reverbWetOnly;
            static std::unique_ptr<SwitchCell> reverbWetOnlyCell;
            if (!reverbWetOnlyCell) {
                reverbWetOnly.setComponentID ("reverb_wet_only");
                reverbWetOnly.getProperties().set ("iconType", (int) IconSystem::Mix);
                reverbWetOnlyCell = std::make_unique<SwitchCell> (reverbWetOnly);
                reverbWetOnlyCell->setCaption ("Wet Only");
                reverbWetOnlyCell->setDelayTheme (false);
                buttonAttachments.push_back (std::make_unique<ButtonAttachment> (proc.apvts, ReverbIDs::killDry, reverbWetOnly));
            }

            group2Container.addAndMakeVisible (*reverbEnableCell);
            group2Container.addAndMakeVisible (*reverbAlgoCell);
            group2Container.addAndMakeVisible (*reverbWetOnlyCell);
            reverbEnableCell->getProperties().set ("reverbMaroonBorder", true);
            reverbAlgoCell  ->getProperties().set ("reverbMaroonBorder", true);
            reverbWetOnlyCell->getProperties().set ("reverbMaroonBorder", true);

            // Additional knobs (new params): SIZE, DECAY XOVER LO/HI, BLOOM, DISTANCE
            static std::unique_ptr<KnobCell> sizeCell, dreqXLoCell, dreqXHiCell, bloomCell, distanceCell;
            static juce::Slider sizeS, dreqXLoS, dreqXHiS, bloomS, distanceS;
            static juce::Label  sizeV, dreqXLoV, dreqXHiV, bloomV, distanceV;
            auto attachSlider = [this, &s, lPx](auto& aStore, const char* id, juce::Slider& sl, juce::Label& val, const juce::String& cap, std::unique_ptr<KnobCell>& cell)
            {
                if (!cell)
                {
                    sl.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
                    sl.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
                    sl.setRotaryParameters (juce::MathConstants<float>::pi,
                                            juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi,
                                            true);
                    sl.setName (cap);
                    cell = std::make_unique<KnobCell> (sl, val, cap);
                    cell->setMetrics (lPx, Layout::dp (14, s), Layout::dp (4, s));
                    cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
                    cell->setValueLabelGap (Layout::dp (4, s));
                    bottomAltPanel.addAndMakeVisible (*cell);
                    aStore.push_back (std::make_unique<SliderAttachment> (proc.apvts, id, sl));
                }
            };
            attachSlider (attachments, ReverbIDs::sizePct,       sizeS,     sizeV,     "SIZE",      sizeCell);
            attachSlider (attachments, ReverbIDs::dreqXoverLoHz, dreqXLoS,  dreqXLoV,  "DEC XO LO", dreqXLoCell);
            attachSlider (attachments, ReverbIDs::dreqXoverHiHz, dreqXHiS,  dreqXHiV,  "DEC XO HI", dreqXHiCell);
            attachSlider (attachments, ReverbIDs::bloomPct,      bloomS,    bloomV,    "BLOOM",     bloomCell);
            attachSlider (attachments, ReverbIDs::distancePct,   distanceS, distanceV, "DIST",      distanceCell);

            // Freeze switch (Row 4, Col 2)
            static juce::ToggleButton reverbFreeze;
            static std::unique_ptr<SwitchCell> reverbFreezeCell;
            if (!reverbFreezeCell)
            {
                reverbFreeze.setComponentID ("reverb_freeze");
                reverbFreeze.getProperties().set ("iconType", (int) IconSystem::Snowflake);
                reverbFreezeCell = std::make_unique<SwitchCell> (reverbFreeze);
                reverbFreezeCell->setCaption ("Freeze");
                group2Container.addAndMakeVisible (*reverbFreezeCell);
                buttonAttachments.push_back (std::make_unique<ButtonAttachment> (proc.apvts, ReverbIDs::freeze, reverbFreeze));
            }

            juce::Grid reverbGrid;
            reverbGrid.rowGap = juce::Grid::Px (0);
            reverbGrid.columnGap = juce::Grid::Px (0);
            reverbGrid.templateRows = { juce::Grid::Px (containerHeight), juce::Grid::Px (containerHeight), juce::Grid::Px (containerHeight), juce::Grid::Px (containerHeight) };
            reverbGrid.templateColumns = {
                juce::Grid::Px (delayCellW), juce::Grid::Px (delayCellW), juce::Grid::Px (delayCellW),
                juce::Grid::Px (delayCellW), juce::Grid::Px (delayCellW), juce::Grid::Px (delayCellW),
                juce::Grid::Px (delayCellW), juce::Grid::Px (delayCellW)
            };
            // Explicit placement into 4x8 grid per spec
            juce::Array<juce::GridItem> items;
            // Row 1: Enable, PRE, ER LVL, Algo, ER DENS, ER WIDTH, WET ONLY, SIZE
            ensureAdd (reverbEnableCell.get()); items.add (juce::GridItem (*reverbEnableCell)  .withArea (1, 1));
            if (rvCells.size() > 0 && rvCells[0]) { ensureAdd (rvCells[0]); items.add (juce::GridItem (*rvCells[0]).withArea (1, 2)); } // PRE
            if (rvCells.size() > 1 && rvCells[1]) { ensureAdd (rvCells[1]); items.add (juce::GridItem (*rvCells[1]).withArea (1, 3)); } // ER LVL
            ensureAdd (reverbAlgoCell.get());    items.add (juce::GridItem (*reverbAlgoCell)    .withArea (1, 4));
            if (rvCells.size() > 3 && rvCells[3]) { ensureAdd (rvCells[3]); items.add (juce::GridItem (*rvCells[3]).withArea (1, 5)); } // ER DENS
            if (rvCells.size() > 4 && rvCells[4]) { ensureAdd (rvCells[4]); items.add (juce::GridItem (*rvCells[4]).withArea (1, 6)); } // ER WIDTH
            ensureAdd (reverbWetOnlyCell.get()); items.add (juce::GridItem (*reverbWetOnlyCell) .withArea (1, 7));
            if (sizeCell) { ensureAdd (sizeCell.get()); items.add (juce::GridItem (*sizeCell)      .withArea (1, 8)); }

            // Row 2: DIF, MOD DEP, MOD RATE, HP, LP, TILT, EQ MIX, DECAY XOVER LO
            if (rvCells.size() > 7  && rvCells[7])  { ensureAdd (rvCells[7]);  items.add (juce::GridItem (*rvCells[7]) .withArea (2, 1)); } // DIF
            if (rvCells.size() > 8  && rvCells[8])  { ensureAdd (rvCells[8]);  items.add (juce::GridItem (*rvCells[8]) .withArea (2, 2)); } // MOD DEP
            if (rvCells.size() > 9  && rvCells[9])  { ensureAdd (rvCells[9]);  items.add (juce::GridItem (*rvCells[9]) .withArea (2, 3)); } // MOD RATE
            if (rvCells.size() > 10 && rvCells[10]) { ensureAdd (rvCells[10]); items.add (juce::GridItem (*rvCells[10]).withArea (2, 4)); } // HP
            if (rvCells.size() > 11 && rvCells[11]) { ensureAdd (rvCells[11]); items.add (juce::GridItem (*rvCells[11]).withArea (2, 5)); } // LP
            if (rvCells.size() > 12 && rvCells[12]) { ensureAdd (rvCells[12]); items.add (juce::GridItem (*rvCells[12]).withArea (2, 6)); } // TILT
            if (rvCells.size() > 13 && rvCells[13]) { ensureAdd (rvCells[13]); items.add (juce::GridItem (*rvCells[13]).withArea (2, 7)); } // EQ MIX
            if (dreqXLoCell) { ensureAdd (dreqXLoCell.get()); items.add (juce::GridItem (*dreqXLoCell).withArea (2, 8)); }

            // Row 3: ER→TAIL, LOW×, MID×, HIGH×, WIDTH, WET, DECAY (RT60), BLOOM
            if (rvCells.size() > 14 && rvCells[14]) { ensureAdd (rvCells[14]); items.add (juce::GridItem (*rvCells[14]).withArea (3, 1)); } // ER→TAIL
            if (rvCells.size() > 15 && rvCells[15]) { ensureAdd (rvCells[15]); items.add (juce::GridItem (*rvCells[15]).withArea (3, 2)); } // LOW×
            if (rvCells.size() > 16 && rvCells[16]) { ensureAdd (rvCells[16]); items.add (juce::GridItem (*rvCells[16]).withArea (3, 3)); } // MID×
            if (rvCells.size() > 17 && rvCells[17]) { ensureAdd (rvCells[17]); items.add (juce::GridItem (*rvCells[17]).withArea (3, 4)); } // HIGH×
            if (rvCells.size() > 18 && rvCells[18]) { ensureAdd (rvCells[18]); items.add (juce::GridItem (*rvCells[18]).withArea (3, 5)); } // WIDTH
            if (rvCells.size() > 19 && rvCells[19]) { ensureAdd (rvCells[19]); items.add (juce::GridItem (*rvCells[19]).withArea (3, 6)); } // WET
            if (rvCells.size() > 5  && rvCells[5])  { ensureAdd (rvCells[5]);  items.add (juce::GridItem (*rvCells[5]) .withArea (3, 7)); } // DECAY
            if (bloomCell) { ensureAdd (bloomCell.get()); items.add (juce::GridItem (*bloomCell)    .withArea (3, 8)); }

            // Row 4: DISTANCE, FREEZE, DUCK, ATT, REL, THR, RAT, DECAY XOVER HI
            auto setDuckMetrics = [&](KnobCell* kc)
            {
                if (!kc) return;
                kc->setMetrics (lPx, valuePx2, labelGap2);
                kc->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
                kc->setValueLabelGap (labelGap2);
            };
            setDuckMetrics (duckCell.get());
            setDuckMetrics (duckAttCell.get());
            setDuckMetrics (duckRelCell.get());
            setDuckMetrics (duckThrCell.get());
            setDuckMetrics (duckRatCell.get());
            if (distanceCell) { ensureAdd (distanceCell.get()); items.add (juce::GridItem (*distanceCell).withArea (4, 1)); }
            ensureAdd (reverbFreezeCell.get()); items.add (juce::GridItem (*reverbFreezeCell).withArea (4, 2));
            if (duckCell)    { ensureAdd (duckCell.get());    items.add (juce::GridItem (*duckCell)   .withArea (4, 3)); }
            if (duckAttCell) { ensureAdd (duckAttCell.get()); items.add (juce::GridItem (*duckAttCell).withArea (4, 4)); }
            if (duckRelCell) { ensureAdd (duckRelCell.get()); items.add (juce::GridItem (*duckRelCell).withArea (4, 5)); }
            if (duckThrCell) { ensureAdd (duckThrCell.get()); items.add (juce::GridItem (*duckThrCell).withArea (4, 6)); }
            if (duckRatCell) { ensureAdd (duckRatCell.get()); items.add (juce::GridItem (*duckRatCell).withArea (4, 7)); }
            if (dreqXHiCell) { ensureAdd (dreqXHiCell.get()); items.add (juce::GridItem (*dreqXHiCell).withArea (4, 8)); }
            reverbGrid.items = std::move (items);

            // Center a fixed-width 7-column block inside the available group width (no outer reduction/padding)
            auto rb = juce::Rectangle<int>(reverbGroupX, reverbGroupY, reverbGroupW, reverbGroupH);
            const int fixedW = delayCellW * 8;
            auto rbCentered = rb.withWidth (juce::jmin (rb.getWidth(), fixedW));
            rbCentered.setX (rb.getX() + (rb.getWidth() - rbCentered.getWidth())); // right-align to match group placement
            reverbGrid.performLayout (rbCentered);
            overlayLayoutDirty = false;
        }
    }
    // ---------------- Group 1: flat 4x16 contiguous grid -----------------------
    {
        const int valuePx = Layout::dp (14, s);
        const int labelGap = Layout::dp (4, s);
        const int cellWTarget = lPx + Layout::dp (8, s);
        const int availableWLocal = group1Container.getWidth();
        const int cellWFit = juce::jmax (1, availableWLocal / 16);
        const int cellW     = juce::jmin (cellWTarget, cellWFit);

        // Compute a single bounds covering rows 1-4 across the full left content width (container-local)
        const int totalRowsH = rowH1 + rowH2 + rowH3 + rowH4;
        juce::Rectangle<int> group1BoundsLocal = juce::Rectangle<int>(0, 0, group1Container.getWidth(), totalRowsH);

        // Ensure cells/components exist and have metrics
        {
            // Left: BASS, HP, Q, Q LINK, LP
            const int miniStripW = Layout::dp (90, s);
            const int miniBarH   = Layout::dp (12, s);
            const int valueGap   = Layout::dp (6,  s);
            bassCell ->setMetrics (lPx, valuePx, 0, miniStripW);
            bassCell ->setMiniPlacementRight (true);
            bassCell ->setMiniThicknessPx (miniBarH);
            bassCell ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            bassCell ->setValueLabelGap (valueGap);
            bassCell ->setMiniWithLabel (&bassFreqSlider, &bassFreqValue, miniStripW);
            bassFreqSlider.getProperties().set ("micro", true);

            if (!hpCell) hpCell = std::make_unique<KnobCell>(hpHz, hpValue, "HP");
            if (!lpCell) lpCell = std::make_unique<KnobCell>(lpHz, lpValue, "LP");
            if (!filterQCell)  filterQCell  = std::make_unique<KnobCell>(filterQ, filterQValue, "Q");
            if (!qClusterCell) qClusterCell = std::make_unique<KnobCell>(qClusterDummySlider, qClusterDummyValue, "Q LINK");
            for (auto* kc : { hpCell.get(), lpCell.get(), filterQCell.get(), qClusterCell.get() })
                if (kc) { kc->setMetrics (lPx, valuePx, labelGap); kc->setValueLabelMode (KnobCell::ValueLabelMode::Managed); kc->setValueLabelGap (labelGap); }
            qClusterCell->setShowKnob (false);
            qClusterCell->setMiniPlacementRight (true);
            qClusterCell->setMiniThicknessPx (Layout::dp (12, s));
            qClusterCell->setAuxAsBars (true);
            qClusterCell->setAuxComponents ({ &qLinkButton, &hpQSlider, &lpQSlider }, Layout::dp (90, s));
            hpQSlider.getProperties().set ("micro", true);
            lpQSlider.getProperties().set ("micro", true);

            // Left Row2: AIR + WIDTH cluster
            airCell    ->setMetrics (lPx, valuePx, 0, miniStripW);
            airCell    ->setMiniPlacementRight (true);
            airCell    ->setMiniThicknessPx (miniBarH);
            airCell    ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            airCell    ->setValueLabelGap (valueGap);
            airCell    ->setMiniWithLabel (&airFreqSlider, &airFreqValue, miniStripW);
            airFreqSlider.getProperties().set ("micro", true);
            for (auto* kc : { widthCell.get(), widthLoCell.get(), widthMidCell.get(), widthHiCell.get() })
                if (kc) { kc->setMetrics (lPx, valuePx, labelGap); kc->setValueLabelMode (KnobCell::ValueLabelMode::Managed); kc->setValueLabelGap (labelGap); }

            // Left Row3: TILT + XO/SHUF
            tiltCell ->setMetrics (lPx, valuePx, 0, miniStripW);
            tiltCell ->setMiniPlacementRight (true);
            tiltCell ->setMiniThicknessPx (miniBarH);
            tiltCell ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            tiltCell ->setValueLabelGap (valueGap);
            tiltCell ->setMiniWithLabel (&tiltFreqSlider, &tiltFreqValue, miniStripW);
            tiltFreqSlider.getProperties().set ("micro", true);
            if (!xoverLoCell)  xoverLoCell  = std::make_unique<KnobCell>(xoverLoHz, xoverLoValue, "XO LO");
            if (!xoverHiCell)  xoverHiCell  = std::make_unique<KnobCell>(xoverHiHz, xoverHiValue, "XO HI");
            for (auto* kc : { xoverLoCell.get(), xoverHiCell.get(), shufLoCell.get(), shufHiCell.get() })
                if (kc) { kc->setMetrics (lPx, valuePx, labelGap); kc->setValueLabelMode (KnobCell::ValueLabelMode::Managed); kc->setValueLabelGap (labelGap); }

            // Left Row4: SCOOP + imaging
            const int scoopMiniW = Layout::dp (90, s);
            const int scoopMiniH = Layout::dp (12, s);
            scoopCell->setMetrics (lPx, valuePx, 0, scoopMiniW);
            scoopCell->setMiniPlacementRight (true);
            scoopCell->setMiniThicknessPx (scoopMiniH);
            scoopCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            scoopCell->setValueLabelGap (valueGap);
            scoopCell->setMiniWithLabel (&scoopFreqSlider, &scoopFreqValue, scoopMiniW);
            scoopFreqSlider.getProperties().set ("micro", true);
            if (!shelfShapeCell) shelfShapeCell = std::make_unique<KnobCell>(shelfShapeS, shelfShapeValue, "Shape");
            for (auto* kc : { rotationCell.get(), asymCell.get(), shufXCell.get(), shelfShapeCell.get() })
                if (kc) { kc->setMetrics (lPx, valuePx, 0); kc->setValueLabelMode (KnobCell::ValueLabelMode::Managed); kc->setValueLabelGap (Layout::dp (6, s)); }

            // Center rows 1-4
            if (!satDriveCell) satDriveCell = std::make_unique<KnobCell>(satDrive, satDriveValue, "DRIVE");
            if (!panCell)      panCell      = std::make_unique<KnobCell>(panKnob,  panValue,        "");
            if (!wetOnlyCell)
            {
                wetOnlyToggle.getProperties().set ("iconType", (int) IconSystem::Mix);
                wetOnlyToggle.setButtonText ("");
                wetOnlyCell = std::make_unique<SwitchCell> (wetOnlyToggle);
                wetOnlyCell->setCaption ("Wet Only");
            }
            for (auto* kc : { gainCell.get(), satMixCell.get(), satDriveCell.get(), panCell.get() })
                if (kc) { kc->setMetrics (lPx, valuePx, labelGap); kc->setValueLabelMode (KnobCell::ValueLabelMode::Managed); kc->setValueLabelGap (labelGap); kc->getProperties().set ("metallic", true); }
            if (wetOnlyCell) wetOnlyCell->getProperties().set ("metallic", true);
            if (monoCell)
            {
                monoCell->setMetrics (lPx, valuePx, labelGap, Layout::dp (24, s));
                monoCell->setMiniPlacementRight (true);
                monoCell->setMiniThicknessPx (juce::jmax (8, Layout::dp (12, s)));
                if (monoSlopeSwitch != nullptr)
                {
                    monoCell->setAuxComponents ({ monoSlopeSwitch.get(), &monoAuditionButton }, juce::jmax (32, Layout::dp (90, s)));
                    monoCell->setAuxWeights ({ 2.0f, 1.0f });
                    monoCell->setAuxAsBars (false);
                    monoSlopeSwitch->setVisible (true);
                    monoSlopeSwitch->toFront (false);
                }
                monoCell->getProperties().set ("metallic", true);
            }

            // Center R3/R4 cells
            if (!centerPromCell)     centerPromCell     = std::make_unique<KnobCell>(centerPromDb,     centerPromVal,     "");
            if (!centerFocusLoCell)  centerFocusLoCell  = std::make_unique<KnobCell>(centerFocusLoHz,  centerFocusLoVal,  "");
            if (!centerFocusHiCell)  centerFocusHiCell  = std::make_unique<KnobCell>(centerFocusHiHz,  centerFocusHiVal,  "");
            if (!centerPunchAmtCell) centerPunchAmtCell = std::make_unique<KnobCell>(centerPunchAmt01, centerPunchAmtVal, "");
            if (!centerPunchModeCell) { centerPunchModeCell = std::make_unique<SwitchCell>(centerPunchMode); centerPunchModeCell->setCaption ("PUNCH MODE"); }
            if (!centerPhaseRecCell)  { centerPhaseRecCell  = std::make_unique<SwitchCell>(centerPhaseRecOn); centerPhaseRecCell->setCaption ("PHASE REC"); }
            if (!centerPhaseAmtCell)  centerPhaseAmtCell  = std::make_unique<KnobCell>(centerPhaseAmt01, centerPhaseAmtVal, "");
            if (!centerLockOnCell)    { centerLockOnCell    = std::make_unique<SwitchCell>(centerLockOn); centerLockOnCell->setCaption ("CNTR LOCK"); }
            if (!centerLockDbCell)    centerLockDbCell    = std::make_unique<KnobCell>(centerLockDb, centerLockDbVal, "");
            for (auto* kc : { centerPromCell.get(), centerFocusLoCell.get(), centerFocusHiCell.get(), centerPunchAmtCell.get(), centerPhaseAmtCell.get(), centerLockDbCell.get() })
                if (kc) { kc->setMetrics (lPx, valuePx, labelGap); kc->getProperties().set ("metallic", true); kc->setValueLabelMode (KnobCell::ValueLabelMode::Managed); kc->setValueLabelGap (labelGap); }
            for (auto* sc : { centerPunchModeCell.get(), centerPhaseRecCell.get(), centerLockOnCell.get() })
                if (sc) { sc->getProperties().set ("metallic", true); }
        }

        // Make visible (Group 1 into group1Container)
        auto addVis = [this](juce::Component* c)
        {
            if (!c) return;
            if (c->getParentComponent() != &group1Container)
                group1Container.addAndMakeVisible (*c);
        };
        addVis (bassCell.get());
        addVis (hpCell.get());
        addVis (filterQCell.get());
        addVis (qClusterCell.get());
        addVis (lpCell.get());
        addVis (airCell.get());
        addVis (widthCell.get());
        addVis (widthLoCell.get());
        addVis (widthMidCell.get());
        addVis (widthHiCell.get());
        addVis (tiltCell.get());
        addVis (xoverLoCell.get());
        addVis (shufLoCell.get());
        addVis (shufHiCell.get());
        addVis (xoverHiCell.get());
        addVis (scoopCell.get());
        addVis (rotationCell.get());
        addVis (asymCell.get());
        addVis (shufXCell.get());
        addVis (shelfShapeCell.get());
        addVis (gainCell.get());
        addVis (satMixCell.get());
        addVis (satDriveCell.get());
        addVis (panCell.get());
        addVis (wetOnlyCell ? wetOnlyCell.get() : nullptr);
        addVis (monoCell ? monoCell.get() : nullptr);
        addVis (centerPunchAmtCell.get());
        addVis (centerPromCell.get());
        addVis (centerFocusLoCell.get());
        addVis (centerFocusHiCell.get());
        addVis (centerPunchModeCell.get());
        addVis (centerPhaseRecCell.get());
        addVis (centerPhaseAmtCell.get());
        addVis (centerLockOnCell.get());

        // Prepare Motion controls (reparent from Group 2 container if needed)
        auto reparent = [this](juce::Component* c)
        {
            if (c == nullptr) return;
            if (c->getParentComponent() == &group2Container)
                group2Container.removeChildComponent (c);
            if (c->getParentComponent() != &group1Container)
                group1Container.addAndMakeVisible (*c);
        };
        auto setKMetrics = [lPx, s](KnobCell* kc)
        {
            if (kc) kc->setMetrics (lPx, Layout::dp (14, s), Layout::dp (4, s));
        };

        const bool group2Raised  = bottomAltTargetOn || (bottomAltSlide01 > 0.01f);

        // Ensure Motion knob cells exist (now live only in Group 1)
        {
            const juce::StringArray motionLabels = {
                "Enable", "Panner", "Path", "Rate", "Depth", "Phase",
                "Spread", "Elev", "Bounce", "Jitter", "Quant", "Swing",
                "Mode", "Retrig", "Hold", "Sens", "Offset", "Inertia",
                "Front", "Doppler", "Send", "Anchor", "Bass", "Occlusion"
            };
            for (int i = 0; i < 24; ++i)
            {
                if (!motionCellsGroup2[i])
                {
                    motionValuesGroup2[i].setText ("", juce::dontSendNotification);
                    motionCellsGroup2[i] = std::make_unique<KnobCell>(motionDummiesGroup2[i], motionValuesGroup2[i], motionLabels[i]);
                    motionCellsGroup2[i]->getProperties().set ("motionPurpleBorder", true);
                    motionCellsGroup2[i]->getProperties().set ("metallic", true);

                    // Configure slider styles for dummy controls
                    if (i == 0) // Enable (button)
                    {
                        motionDummiesGroup2[i].setSliderStyle (juce::Slider::LinearHorizontal);
                        motionDummiesGroup2[i].setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
                    }
                    else if (i == 1 || i == 2 || i == 10 || i == 12) // Combo-backed
                    {
                        motionDummiesGroup2[i].setSliderStyle (juce::Slider::LinearHorizontal);
                        motionDummiesGroup2[i].setTextBoxStyle (juce::Slider::TextBoxLeft, false, 60, 18);
                    }
                    else if (i == 13 || i == 21) // Button-backed
                    {
                        motionDummiesGroup2[i].setSliderStyle (juce::Slider::LinearHorizontal);
                        motionDummiesGroup2[i].setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
                    }
                    else // Knobs
                    {
                        motionDummiesGroup2[i].setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
                        motionDummiesGroup2[i].setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
                        motionDummiesGroup2[i].setRotaryParameters (
                            juce::MathConstants<float>::pi,
                            juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi,
                            true);
                        // Ensure LNF draws captions: set slider name from our labels
                        motionDummiesGroup2[i].setName (motionLabels[i]);
                    }
                }
                // Apply metrics and value label behavior for all motion knob cells
                setKMetrics (motionCellsGroup2[i].get());
                motionCellsGroup2[i]->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
                motionCellsGroup2[i]->setValueLabelGap (Layout::dp (4, s));
            }
        }

        // Flat grid: 4 rows x 16 columns (6 left + 4 center + 6 motion)
        juce::Grid g; g.rowGap = juce::Grid::Px (0); g.columnGap = juce::Grid::Px (0);
        g.templateRows = { juce::Grid::Px (containerHeight), juce::Grid::Px (containerHeight), juce::Grid::Px (containerHeight), juce::Grid::Px (containerHeight) };
        g.templateColumns = {
            juce::Grid::Px (cellW), juce::Grid::Px (cellW), juce::Grid::Px (cellW), juce::Grid::Px (cellW), juce::Grid::Px (cellW), juce::Grid::Px (cellW),
            juce::Grid::Px (cellW), juce::Grid::Px (cellW), juce::Grid::Px (cellW), juce::Grid::Px (cellW), juce::Grid::Px (cellW), juce::Grid::Px (cellW),
            juce::Grid::Px (cellW), juce::Grid::Px (cellW), juce::Grid::Px (cellW), juce::Grid::Px (cellW)
        };

        juce::Array<juce::GridItem> items;
        // Row 1: Left (1-6)
        items.add (juce::GridItem (*bassCell)     .withArea (1, 1, 2, 3)); // spans 1-2
        items.add (juce::GridItem (*hpCell)       .withArea (1, 3));
        items.add (juce::GridItem (*filterQCell)  .withArea (1, 4));
        items.add (juce::GridItem (*qClusterCell) .withArea (1, 5));
        items.add (juce::GridItem (*lpCell)       .withArea (1, 6));
        // Row 1: Center (7-10)
        items.add (juce::GridItem (*gainCell)     .withArea (1, 7));
        items.add (juce::GridItem (*satMixCell)   .withArea (1, 8));
        items.add (juce::GridItem (*satDriveCell) .withArea (1, 9));
        if (wetOnlyCell) items.add (juce::GridItem (*wetOnlyCell).withArea (1, 10));
        // Row 1: Motion (11-16)
        reparent (motionButtonCells[0].get());
        reparent (motionComboCells[0].get());
        reparent (motionComboCells[1].get());
        for (int idx : {3,4,5}) { if (motionCellsGroup2[idx]) { reparent (motionCellsGroup2[idx].get()); setKMetrics (motionCellsGroup2[idx].get()); } }
        items.add (juce::GridItem (*motionButtonCells[0]).withArea (1,11));
        items.add (juce::GridItem (*motionComboCells[0]) .withArea (1,12));
        items.add (juce::GridItem (*motionComboCells[1]) .withArea (1,13));
        items.add (juce::GridItem (*motionCellsGroup2[3]).withArea (1,14));
        items.add (juce::GridItem (*motionCellsGroup2[4]).withArea (1,15));
        items.add (juce::GridItem (*motionCellsGroup2[5]).withArea (1,16));

        // Row 2: Left (1-6)
        items.add (juce::GridItem (*airCell)      .withArea (2, 1, 3, 3)); // spans 1-2
        items.add (juce::GridItem (*widthCell)    .withArea (2, 3));
        items.add (juce::GridItem (*widthLoCell)  .withArea (2, 4));
        items.add (juce::GridItem (*widthMidCell) .withArea (2, 5));
        items.add (juce::GridItem (*widthHiCell)  .withArea (2, 6));
        // Row 2: Center (7-10)
        items.add (juce::GridItem (*panCell)      .withArea (2, 7, 3, 9)); // spans 7-8
        if (monoCell) items.add (juce::GridItem (*monoCell).withArea (2, 9, 3, 11)); // spans 9-10
        // Row 2: Motion (11-16)
        for (int idx : {6,7,8,9,11}) { if (motionCellsGroup2[idx]) { reparent (motionCellsGroup2[idx].get()); setKMetrics (motionCellsGroup2[idx].get()); } }
        reparent (motionComboCells[2].get());
        items.add (juce::GridItem (*motionCellsGroup2[6]) .withArea (2,11));
        items.add (juce::GridItem (*motionCellsGroup2[7]) .withArea (2,12));
        items.add (juce::GridItem (*motionCellsGroup2[8]) .withArea (2,13));
        items.add (juce::GridItem (*motionCellsGroup2[9]) .withArea (2,14));
        items.add (juce::GridItem (*motionComboCells[2])  .withArea (2,15));
        items.add (juce::GridItem (*motionCellsGroup2[11]).withArea (2,16));

        // Row 3: Left (1-6)
        items.add (juce::GridItem (*tiltCell)     .withArea (3, 1, 4, 3)); // spans 1-2
        items.add (juce::GridItem (*xoverLoCell)  .withArea (3, 3));
        items.add (juce::GridItem (*shufLoCell)   .withArea (3, 4));
        items.add (juce::GridItem (*shufHiCell)   .withArea (3, 5));
        items.add (juce::GridItem (*xoverHiCell)  .withArea (3, 6));
        // Row 3: Center (7-10)
        items.add (juce::GridItem (*centerPunchAmtCell).withArea (3, 7));
        items.add (juce::GridItem (*centerPromCell)    .withArea (3, 8));
        items.add (juce::GridItem (*centerFocusLoCell) .withArea (3, 9));
        items.add (juce::GridItem (*centerFocusHiCell) .withArea (3,10));
        // Row 3: Motion (11-16)
        for (int idx : {14,15,16,17}) { if (motionCellsGroup2[idx]) { reparent (motionCellsGroup2[idx].get()); setKMetrics (motionCellsGroup2[idx].get()); } }
        reparent (motionComboCells[3].get());
        reparent (motionButtonCells[1].get());
        items.add (juce::GridItem (*motionComboCells[3])  .withArea (3,11));
        items.add (juce::GridItem (*motionButtonCells[1]).withArea (3,12));
        items.add (juce::GridItem (*motionCellsGroup2[14]).withArea (3,13));
        items.add (juce::GridItem (*motionCellsGroup2[15]).withArea (3,14));
        items.add (juce::GridItem (*motionCellsGroup2[16]).withArea (3,15));
        items.add (juce::GridItem (*motionCellsGroup2[17]).withArea (3,16));

        // Row 4: Left (1-6)
        items.add (juce::GridItem (*scoopCell)    .withArea (4, 1, 5, 3)); // spans 1-2
        items.add (juce::GridItem (*rotationCell) .withArea (4, 3));
        items.add (juce::GridItem (*asymCell)     .withArea (4, 4));
        items.add (juce::GridItem (*shufXCell)    .withArea (4, 5));
        items.add (juce::GridItem (*shelfShapeCell).withArea (4, 6));
        // Row 4: Center (7-10)
        items.add (juce::GridItem (*centerPunchModeCell).withArea (4, 7));
        items.add (juce::GridItem (*centerPhaseRecCell) .withArea (4, 8));
        items.add (juce::GridItem (*centerPhaseAmtCell) .withArea (4, 9));
        items.add (juce::GridItem (*centerLockOnCell)   .withArea (4,10));
        // Row 4: Motion (11-16)
        for (int idx : {18,19,20,22,23}) { if (motionCellsGroup2[idx]) { reparent (motionCellsGroup2[idx].get()); setKMetrics (motionCellsGroup2[idx].get()); } }
        reparent (motionButtonCells[2].get());
        items.add (juce::GridItem (*motionCellsGroup2[18]).withArea (4,11));
        items.add (juce::GridItem (*motionCellsGroup2[19]).withArea (4,12));
        items.add (juce::GridItem (*motionCellsGroup2[20]).withArea (4,13));
        items.add (juce::GridItem (*motionButtonCells[2]).withArea (4,14));
        items.add (juce::GridItem (*motionCellsGroup2[22]).withArea (4,15));
        items.add (juce::GridItem (*motionCellsGroup2[23]).withArea (4,16));

        g.items = std::move (items);
        g.performLayout (group1BoundsLocal);

        // Motion visibility is managed solely by the Group 1 flat grid

        // UX: disable Phase Amt when Phase Rec is off; clamp label precision
        const bool phaseOn = centerPhaseRecOn.getToggleState();
        centerPhaseAmt01.setEnabled (phaseOn);
        auto setValText = [] (juce::Label& lbl, double v, int digits)
        {
            lbl.setText (juce::String (v, digits), juce::dontSendNotification);
        };
        setValText (centerPromVal,      centerPromDb.getValue(), 1);
        setValText (centerFocusLoVal,   centerFocusLoHz.getValue(), 0);
        setValText (centerFocusHiVal,   centerFocusHiHz.getValue(), 0);
        setValText (centerPunchAmtVal,  centerPunchAmt01.getValue() * 100.0, 0);
        setValText (centerPhaseAmtVal,  centerPhaseAmt01.getValue() * 100.0, 0);
        setValText (centerLockDbVal,    centerLockDb.getValue(), 1);

        // Initialize Motion Engine value labels (KnobCell will manage placement)
        auto placeMotion = [&] (int idx)
        {
            if (idx >= 0 && idx < (int) motionCellsGroup2.size())
                if (motionCellsGroup2[idx])
                {
                    // Initialize label text once from current slider value
                    auto set = [] (juce::Label& lbl, const juce::String& t){ lbl.setText (t, juce::dontSendNotification); };
                    auto pct = [] (double v){ return juce::String (v, 0) + "%"; };
                    auto Hz  = [] (double v){ return v < 10.0 ? juce::String (v, 2) + " Hz" : juce::String (v, 0) + " Hz"; };
                    int i = idx;
                    if      (i == 3)  set (motionValuesGroup2[i], Hz  (motionDummiesGroup2[i].getValue()));      // Rate
                    else if (i == 4)  set (motionValuesGroup2[i], pct (motionDummiesGroup2[i].getValue()));      // Depth
                    else if (i == 5)  set (motionValuesGroup2[i], juce::String (motionDummiesGroup2[i].getValue(), 2) + "°"); // Phase
                    else if (i == 6)  set (motionValuesGroup2[i], pct (motionDummiesGroup2[i].getValue()));      // Spread
                    else if (i == 7)  set (motionValuesGroup2[i], juce::String (motionDummiesGroup2[i].getValue(), 2));       // Elev
                    else if (i == 8)  set (motionValuesGroup2[i], pct (motionDummiesGroup2[i].getValue()));      // Bounce
                    else if (i == 9)  set (motionValuesGroup2[i], pct (motionDummiesGroup2[i].getValue()));      // Jitter
                    else if (i == 11) set (motionValuesGroup2[i], pct (motionDummiesGroup2[i].getValue()));      // Swing
                    else if (i == 14) set (motionValuesGroup2[i], juce::String (motionDummiesGroup2[i].getValue(), 2) + " ms"); // Hold
                    else if (i == 15) set (motionValuesGroup2[i], pct (motionDummiesGroup2[i].getValue()));      // Sens
                    else if (i == 16) set (motionValuesGroup2[i], juce::String (motionDummiesGroup2[i].getValue(), 2) + "°"); // Offset
                    else if (i == 17) set (motionValuesGroup2[i], juce::String (motionDummiesGroup2[i].getValue(), 2) + " ms"); // Inertia
                    else if (i == 18) set (motionValuesGroup2[i], juce::String (motionDummiesGroup2[i].getValue(), 2));        // Front
                    else if (i == 19) set (motionValuesGroup2[i], pct (motionDummiesGroup2[i].getValue()));      // Doppler
                    else if (i == 20) set (motionValuesGroup2[i], pct (motionDummiesGroup2[i].getValue()));      // Send
                    else if (i == 22) set (motionValuesGroup2[i], Hz  (motionDummiesGroup2[i].getValue()));      // Bass Floor
                    else if (i == 23) set (motionValuesGroup2[i], pct (motionDummiesGroup2[i].getValue()));      // Occlusion
                }
        };
        // Knob-backed Motion indices (skip combos/buttons)
        for (int idx : { 3,4,5,6,7,8,9,11,14,15,16,17,18,19,20,22,23 })
            placeMotion (idx);
    }

    // Legacy Row 2 reverb group removed; Group 2 Reverb grid now owns reverb/ducking UI
    // Delay theme applied in Group 2
}

void MyPluginAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    noteUserInteraction();
    const int grip = 16;
    if (e.position.x > getWidth() - grip && e.position.y > getHeight() - grip)
    {
        isResizing = true;
        resizeStart = e.getPosition();
        originalBounds = getBounds();
    }
}

void MyPluginAudioProcessorEditor::mouseDrag (const juce::MouseEvent& e)
{
    noteUserInteraction();
    if (!isResizing) return;
    auto d = e.getPosition() - resizeStart;
    
    // Calculate new size
    int newWidth = originalBounds.getWidth() + d.x;
    int newHeight = originalBounds.getHeight() + d.y;
    
    // Apply minimum size constraints
    newWidth = juce::jmax (newWidth, minWidth);
    newHeight = juce::jmax (newHeight, minHeight);
    
    // Apply maximum size constraints
    newWidth = juce::jmin (newWidth, maxWidth);
    newHeight = juce::jmin (newHeight, maxHeight);
    
    // Do not maintain aspect ratio by default; hold Shift to lock aspect
    const bool maintainAspectRatio = e.mods.isShiftDown();
    
    if (maintainAspectRatio)
    {
        const float aspectRatio = (float)baseWidth / (float)baseHeight;
        if (std::abs(d.x) > std::abs(d.y))
        {
            // Width changed more, adjust height to maintain ratio
            newHeight = (int)(newWidth / aspectRatio);
        }
        else
        {
            // Height changed more, adjust width to maintain ratio
            newWidth = (int)(newHeight * aspectRatio);
        }
        
        // Re-apply constraints after aspect ratio adjustment
        newWidth = juce::jlimit (minWidth, maxWidth, newWidth);
        newHeight = juce::jlimit (minHeight, maxHeight, newHeight);
        
        // If constraints broke the aspect ratio, adjust the other dimension
        const float currentRatio = (float)newWidth / (float)newHeight;
        const float targetRatio = aspectRatio;
        const float ratioError = std::abs(currentRatio - targetRatio);
        
        if (ratioError > 0.01f) // Allow small tolerance
        {
            if (currentRatio > targetRatio)
            {
                // Too wide, reduce width
                newWidth = (int)(newHeight * aspectRatio);
                newWidth = juce::jlimit (minWidth, maxWidth, newWidth);
            }
            else
            {
                // Too tall, reduce height  
                newHeight = (int)(newWidth / aspectRatio);
                newHeight = juce::jlimit (minHeight, maxHeight, newHeight);
            }
        }
    }
    
    setBounds (originalBounds.withSize (newWidth, newHeight));
}

void MyPluginAudioProcessorEditor::mouseUp (const juce::MouseEvent& e)
{
    noteUserInteraction();
    juce::ignoreUnused (e);
    isResizing = false;
}

void MyPluginAudioProcessorEditor::resized()
{
    // Calculate scale factor from both width and height; use the smaller to ensure uniform downsizing
    const float wScale = (float)getWidth()  / (float)baseWidth;
    const float hScale = (float)getHeight() / (float)baseHeight;
    scaleFactor = juce::jmin (wScale, hScale);

    // Ensure scale factor stays within reasonable bounds
    scaleFactor = juce::jlimit (0.5f, 2.0f, scaleFactor);
    
    // Call the existing layout code with the calculated scale factor
    if (!layoutReady) return;
    performLayout();
}

void MyPluginAudioProcessorEditor::mouseMove (const juce::MouseEvent& e)
{
    if (! tooltipAssistantOn_) { tooltipBubble.setVisible (false); return; }
    auto withinHeader = getLocalBounds().removeFromTop (static_cast<int> (60 * scaleFactor));
    if (! withinHeader.contains (e.position.toInt())) { tooltipBubble.setVisible (false); lastTooltipTarget = nullptr; return; }

    // Consider a few primary header controls
    juce::Component* targets[] { &colorModeButton, &tooltipsButton, &fullScreenButton };
    juce::String tip; juce::Component* hit = nullptr;
    for (auto* c : targets)
    {
        auto rel = e.getEventRelativeTo (c);
        if (c->isVisible() && c->getBounds().contains (rel.getPosition().toInt())) { hit = c; break; }
    }
    if (hit == nullptr) { tooltipBubble.setVisible (false); lastTooltipTarget = nullptr; return; }

    if (hit != lastTooltipTarget)
    {
        lastTooltipTarget = hit;
        if (hit == &colorModeButton) tip = "Cycle colour theme";
        else if (hit == &tooltipsButton) tip = "Tooltip Assistant – compact, contextual hints";
        else if (hit == &fullScreenButton) tip = "Toggle fullscreen";

        tooltipBubble.setText (tip);
        auto anchor = hit->getBounds();
        anchor.setPosition (hit->getParentComponent()->getBounds().getPosition() + anchor.getPosition());
        tooltipBubble.setAnchor (anchor);
        tooltipBubble.setVisible (true);
        tooltipBubble.toFront (false);
    }
}

// Repaint waveform from UI thread at ~30 Hz
void MyPluginAudioProcessorEditor::timerCallback()
{
    if (! isShowing()) return;
    // Adaptive timer: burst to 60 Hz for ~150 ms after any user interaction, then 30 Hz
    const auto now = juce::Time::getMillisecondCounter();
    const bool burst = (now - lastUserInteractionMs) <= 150;
    const int targetHz = burst ? 60 : 30;
    if (targetHz != uiTimerHzCurrent)
    {
        uiTimerHzCurrent = targetHz;
        startTimerHz (uiTimerHzCurrent);
    }
    // Throttle heavy UI work to reduce message-thread contention (combobox/popup lag)
    static int uiTick = 0; ++uiTick;
    const bool doHeavyUi = (uiTick % 3) == 0; // ~6-7 Hz when timer is 20 Hz
    // If a modal component (PopupMenu/ComboBox list) is open, skip most UI work to keep interaction snappy
    if (juce::ModalComponentManager::getInstance()->getNumModalComponents() > 0)
    {
        if ((uiTick % 6) != 0) return; // ~3 Hz minimal maintenance
    }
    // history removed
    // Update ducking meter overlay on knob; idle when Reverb wet is zero (Reverb Engine)
    float grDb = proc.getReverbDuckGrDb();
    bool reverbEnabled = false;
    float reverbWet = 0.0f;
    if (auto* pOn  = proc.apvts.getRawParameterValue (ReverbIDs::enabled))    reverbEnabled = pOn->load() > 0.5f;
    if (auto* pWet = proc.apvts.getRawParameterValue (ReverbIDs::wetMix01))   reverbWet     = pWet->load();
    if (!reverbEnabled || reverbWet <= 0.0001f) grDb = 0.0f;
    duckingKnob.setCurrentGrDb (grDb);

    // Grey-out ATT/REL/THR/RAT when DuckDepthDb=0 or Reverb inactive; also grey the Algo switch
    const bool reverbActive = reverbEnabled && (reverbWet > 0.0001f);
    float duckDepthDb = 0.0f;
    if (auto* pDepth = proc.apvts.getRawParameterValue (ReverbIDs::duckDepthDb)) duckDepthDb = pDepth->load();
    const bool duckActive = (duckDepthDb > 0.0001f) && reverbActive;
    // spaceAlgorithmSwitch greying now follows Reverb Engine state
    spaceAlgorithmSwitch.setAlpha (reverbActive ? 1.0f : 0.35f);
    spaceAlgorithmSwitch.setMuted (!reverbActive);
    
    // Update motion panel visual state with sequence tracking and idle fallback
    if (panes) {
        motion::VisualState visualState;
        // Motion Engine is now handled by FieldChain template
        auto seq = 0; // Motion visual state will be handled by FieldChain
        
        // Only update if we have a new sequence number (prevents unnecessary repaints)
        static uint32_t lastSeq = 0;
        static int staleMs = 0;
        
        if (seq == lastSeq) {
            staleMs += 16; // ~60Hz timer
            if (staleMs > 200) { // 200ms timeout
                // Synthesize a static pose from APVTS (no DSP needed)
                visualState = synthesizeVisualFromParams();
            }
        } else {
            staleMs = 0;
        }
        
        if (visualState.seq != lastSeq || staleMs > 200) {
            lastSeq = visualState.seq;
            panes->setMotionVisualState(visualState);
        }
    }
    duckAttack.setMuted (!duckActive);
    duckRelease.setMuted(!duckActive);
    duckThreshold.setMuted(!duckActive);
    duckRatio.setMuted(!duckActive);
    // DUCK knob follows overall duckActive (grey when depth=0 or reverb=0)
    duckingKnob.setMuted (!duckActive);
    if (doHeavyUi)
    {
        spaceAlgorithmSwitch.repaint();
        duckingKnob.repaint();
        duckAttack.repaint();
        duckRelease.repaint();
        duckThreshold.repaint();
        duckRatio.repaint();
        pad.repaint();
    }

    // Update transport clock label (host/standalone song time)
    {
        const double tSec = proc.getTransportTimeSeconds();
        const bool isPlaying = proc.isTransportPlaying();
        // Format as mm:SS.mmm similar to Ableton's transport
        auto formatTime = [] (double seconds)
        {
            if (seconds < 0.0) seconds = 0.0;
            const int totalMs = (int) std::llround (seconds * 1000.0);
            const int ms = totalMs % 1000;
            const int totalSec = totalMs / 1000;
            const int sec = totalSec % 60;
            const int min = (totalSec / 60);
            return juce::String::formatted ("%02d:%02d.%03d", min, sec, ms);
        };
        // Only update text when host transport supplies a position; otherwise hold last
    if (doHeavyUi)
        transportClockLabel.setText (formatTime (tSec), juce::dontSendNotification);
        // Dim when stopped
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            auto col = isPlaying ? lf->theme.text : lf->theme.textMuted;
            transportClockLabel.setColour (juce::Label::textColourId, col);
        }
    }

    // Drive bottom panel slide animation (move-only; no full relayout)
    if (doHeavyUi)
    {
        bool changed = false;
        if (bottomAltTargetOn && bottomAltSlide01 < 1.0f) { bottomAltSlide01 = juce::jmin (1.0f, bottomAltSlide01 + 0.06f); changed = true; }
        else if (!bottomAltTargetOn && bottomAltSlide01 > 0.0f) { bottomAltSlide01 = juce::jmax (0.0f, bottomAltSlide01 - 0.06f); changed = true; }
        if (changed) updateGroup2OverlayDuringSlide();
    }
}

 

void MyPluginAudioProcessorEditor::paintOverChildren (juce::Graphics& g)
{
    juce::ignoreUnused (g);
    // No extra overlay on top of children for now
}

// Move-only animation of Group 2 overlay; avoid reflow during slide
void MyPluginAudioProcessorEditor::updateGroup2OverlayDuringSlide()
{
    // Compute eased slide progress for visual feel
    const float appearThresh = 0.10f;
    const float t0 = juce::jlimit (0.0f, 1.0f, (bottomAltSlide01 - appearThresh) / juce::jmax (0.0001f, 1.0f - appearThresh));
    const float effSlide = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::pi * t0);

    // Animate between cached baselines; keep bounds fixed to overlay rect
    const int bottomY = juce::roundToInt (juce::jmap (effSlide, 0.0f, 1.0f, (float) overlayHiddenBaseline, (float) overlayActiveBaseline));
    const int curTop  = bottomY - juce::roundToInt ((float) overlayHeightPx * effSlide);

    // Set visibility based on threshold
    const bool showPanel = effSlide > 0.001f;
    bottomAltPanel.setVisible (showPanel);
    if (!showPanel) return;

    // We keep the overlay sized to overlayLocalRect, and slide its clip region via setBounds() within that local rect
    // Move by adjusting Y within the overlayLocalRect height (top anchored by curTop relative to MainContentContainer rows stack top)
    // Here we compute a translation within the same width and height
    auto base = overlayLocalRect;
    const int desiredTop = curTop; // curTop is relative to MainContentContainer
    const int deltaY = desiredTop - base.getY();
    base.translate (0, deltaY);
    bottomAltPanel.setBounds (base);
    bottomAltPanel.toFront (true);
    bottomAreaToggle.toFront (true);
}

void MyPluginAudioProcessorEditor::setScaleFactor (float newScale)
{
    scaleFactor = juce::jlimit (0.5f, 2.0f, newScale);
    resized();
    repaint();
}
void MyPluginAudioProcessorEditor::sliderValueChanged (juce::Slider* s)
{
    auto set = [](juce::Label& l, const juce::String& t){ l.setText (t, juce::dontSendNotification); };
    auto Hz  = [](double v){ return juce::String (v, 1) + " Hz"; };
    auto dB  = [](double v){ return juce::String (v, 1) + " dB"; };
    auto pct = [](double v){ return juce::String (v, 0) + "%"; };

    if (s == &gain) {
        set (gainValue, dB (gain.getValue()));
        pad.setGainValue ((float) gain.getValue()); // feed XY for split-ball hit radius
    }
    else if (s == &width) {
        const double pctVal = juce::jlimit (0.0, 1000.0, width.getValue() * 100.0); // 1.0 -> 100%
        set (widthValue, juce::String (pctVal, 0) + "%");
    }
    else if (s == &tilt)   set (tiltValue, juce::String (tilt.getValue(), 1));
    else if (s == &monoHz) set (monoValue, Hz (monoHz.getValue()));
    else if (s == &hpHz)   set (hpValue,   Hz (hpHz.getValue()));
    else if (s == &lpHz)   set (lpValue,   Hz (lpHz.getValue()));
    else if (s == &satDrive) set (satDriveValue, dB (satDrive.getValue()));
    else if (s == &satMix)   set (satMixValue, pct (juce::jmap (satMix.getValue(), satMix.getMinimum(), satMix.getMaximum(), 0.0, 100.0)));
    else if (s == &air)    set (airValue, dB (air.getValue()));
    else if (s == &bass)   set (bassValue, dB (bass.getValue()));
    else if (s == &scoop)  {
        set (scoopValue, juce::String (scoop.getValue(), 1));
        // Dynamic label: Boost for >0, Scoop for <0, 0 shows Scoop
        scoop.setName (scoop.getValue() > 0.0 ? "BOOST" : "SCOOP");
    }
    else if (s == &panKnob){ set (panValue, juce::String (panKnob.getValue(), 2)); pad.setPanValue ((float) panKnob.getValue()); }
    else if (s == &panKnobLeft || s == &panKnobRight)
    {
        set (panValueLeft,  juce::String (panKnobLeft.getValue(), 2));
        set (panValueRight, juce::String (panKnobRight.getValue(), 2));
        panKnob.setSplitPercentage ((float) juce::jmap (panKnobLeft.getValue(),  -1.0, 1.0, 0.0, 100.0),
                                    (float) juce::jmap (panKnobRight.getValue(), -1.0, 1.0, 0.0, 100.0));
        panKnob.repaint();
    }
    else if (s == &spaceKnob)   { set (spaceValue, juce::String (spaceKnob.getValue(), 2)); pad.setSpaceValue ((float) spaceKnob.getValue()); }
    else if (s == &duckingKnob) {
        // Display 0–100% depth
        const double pctVal = juce::jmap (duckingKnob.getValue(), duckingKnob.getMinimum(), duckingKnob.getMaximum(), 0.0, 100.0);
        set (duckingValue, juce::String (pctVal, 0) + "%");
    }
    else if (s == &duckAttack)  set (duckAttackValue, juce::String (duckAttack.getValue(), 0) + " ms");
    else if (s == &duckRelease) set (duckReleaseValue, juce::String (duckRelease.getValue(), 0) + " ms");
    else if (s == &duckThreshold) set (duckThresholdValue, juce::String (duckThreshold.getValue(), 0) + " dB");
    else if (s == &duckRatio)     set (duckRatioValue, juce::String (duckRatio.getValue(), 1) + ":1");
    else if (s == &duckAttack)  set (duckAttackValue, juce::String (duckAttack.getValue(), 0) + " ms");
    else if (s == &duckRelease) set (duckReleaseValue, juce::String (duckRelease.getValue(), 0) + " ms");
    else if (s == &duckThreshold) set (duckThresholdValue, juce::String (duckThreshold.getValue(), 0) + " dB");

    else if (s == &tiltFreqSlider)  set (tiltFreqValue,  Hz (tiltFreqSlider.getValue()));
    else if (s == &scoopFreqSlider) set (scoopFreqValue, Hz (scoopFreqSlider.getValue()));
    else if (s == &bassFreqSlider)  set (bassFreqValue,  Hz (bassFreqSlider.getValue()));
    else if (s == &airFreqSlider)   set (airFreqValue,   Hz (airFreqSlider.getValue()));
    else if (s == &shelfShapeS)     { set (shelfShapeValue, juce::String (shelfShapeS.getValue(), 2)); shelfShapeS.getProperties().set ("S_value", (double) shelfShapeS.getValue()); shelfShapeS.repaint(); }
    else if (s == &filterQ)         set (filterQValue,    juce::String (filterQ.getValue(), 2));
    // Q minis update their own badges; do not overwrite HP/LP knob value labels

    // Imaging value labels with units
    else if (s == &widthLo)  set (widthLoValue,  juce::String (juce::roundToInt (widthLo.getValue()  * 100.0))  + "%");
    else if (s == &widthMid) set (widthMidValue, juce::String (juce::roundToInt (widthMid.getValue() * 100.0)) + "%");
    else if (s == &widthHi)  set (widthHiValue,  juce::String (juce::roundToInt (widthHi.getValue()  * 100.0))  + "%");
    else if (s == &xoverLoHz) set (xoverLoValue, juce::String ((int) xoverLoHz.getValue()) + " Hz");
    else if (s == &xoverHiHz) set (xoverHiValue, juce::String ((int) xoverHiHz.getValue()) + " Hz");
    else if (s == &rotationDeg) set (rotationValue, juce::String (rotationDeg.getValue(), 1) + "°");
    else if (s == &asymmetry)   set (asymValue,     juce::String (juce::roundToInt (asymmetry.getValue() * 100.0)) + "%");
    else if (s == &shufLoPct)   set (shufLoValue,   juce::String (juce::roundToInt (shufLoPct.getValue())) + "%");
    else if (s == &shufHiPct)   set (shufHiValue,   juce::String (juce::roundToInt (shufHiPct.getValue())) + "%");
    else if (s == &shufXHz)     set (shufXValue,    juce::String ((int) shufXHz.getValue()) + " Hz");
    
    // Delay controls
    else if (s == &delayTime) set (delayTimeValue, juce::String ((int) delayTime.getValue()) + " ms");
    else if (s == &delayFeedback) set (delayFeedbackValue, juce::String ((int) delayFeedback.getValue()) + "%");
    else if (s == &delayWet) set (delayWetValue, pct (juce::jmap (delayWet.getValue(), delayWet.getMinimum(), delayWet.getMaximum(), 0.0, 100.0)));
    else if (s == &delaySpread) set (delaySpreadValue, juce::String ((int) delaySpread.getValue()) + "%");
    else if (s == &delayWidth) set (delayWidthValue, juce::String (juce::roundToInt (delayWidth.getValue() * 100.0)) + "%");
    else if (s == &delayModRate) set (delayModRateValue, Hz (delayModRate.getValue()));
    else if (s == &delayModDepth) set (delayModDepthValue, juce::String (delayModDepth.getValue(), 1) + " ms");
    else if (s == &delayWowflutter) set (delayWowflutterValue, pct (juce::jmap (delayWowflutter.getValue(), delayWowflutter.getMinimum(), delayWowflutter.getMaximum(), 0.0, 100.0)));
    else if (s == &delayJitter) set (delayJitterValue, juce::String (delayJitter.getValue(), 1) + "%");
    else if (s == &delayHp) set (delayHpValue, Hz (delayHp.getValue()));
    else if (s == &delayLp) set (delayLpValue, Hz (delayLp.getValue()));
    else if (s == &delayTilt) set (delayTiltValue, dB (delayTilt.getValue()));
    else if (s == &delaySat) set (delaySatValue, pct (juce::jmap (delaySat.getValue(), delaySat.getMinimum(), delaySat.getMaximum(), 0.0, 100.0)));
    else if (s == &delayDiffusion) set (delayDiffusionValue, pct (juce::jmap (delayDiffusion.getValue(), delayDiffusion.getMinimum(), delayDiffusion.getMaximum(), 0.0, 100.0)));
    else if (s == &delayDiffuseSize) set (delayDiffuseSizeValue, juce::String ((int) delayDiffuseSize.getValue()) + " ms");
    else if (s == &delayDuckDepth) set (delayDuckDepthValue, pct (juce::jmap (delayDuckDepth.getValue(), delayDuckDepth.getMinimum(), delayDuckDepth.getMaximum(), 0.0, 100.0)));
    else if (s == &delayDuckAttack) set (delayDuckAttackValue, juce::String ((int) delayDuckAttack.getValue()) + " ms");
    else if (s == &delayDuckRelease) set (delayDuckReleaseValue, juce::String ((int) delayDuckRelease.getValue()) + " ms");
    else if (s == &delayDuckThreshold) set (delayDuckThresholdValue, juce::String ((int) delayDuckThreshold.getValue()) + " dB");
    else if (s == &delayDuckRatio) set (delayDuckRatioValue, juce::String (delayDuckRatio.getValue(), 1) + ":1");
    else if (s == &delayDuckLookahead) set (delayDuckLookaheadValue, juce::String ((int) delayDuckLookahead.getValue()) + " ms");
    else if (s == &delayPreDelay) set (delayPreDelayValue, juce::String ((int) delayPreDelay.getValue()) + " ms");

    // Motion controls value label updates (indices aligned with attachments/refresh)
    else if (s == &motionDummiesGroup2[3])  set (motionValuesGroup2[3],  Hz (motionDummiesGroup2[3].getValue()));      // Rate
    else if (s == &motionDummiesGroup2[4])  set (motionValuesGroup2[4],  pct (motionDummiesGroup2[4].getValue()));     // Depth
    else if (s == &motionDummiesGroup2[5])  set (motionValuesGroup2[5],  juce::String (motionDummiesGroup2[5].getValue(), 2) + "°"); // Phase
    else if (s == &motionDummiesGroup2[6])  set (motionValuesGroup2[6],  pct (motionDummiesGroup2[6].getValue()));     // Spread
    else if (s == &motionDummiesGroup2[7])  set (motionValuesGroup2[7],  juce::String (motionDummiesGroup2[7].getValue(), 2));   // Elev Bias
    else if (s == &motionDummiesGroup2[8])  set (motionValuesGroup2[8],  pct (motionDummiesGroup2[8].getValue()));     // Bounce
    else if (s == &motionDummiesGroup2[9])  set (motionValuesGroup2[9],  pct (motionDummiesGroup2[9].getValue()));     // Jitter
    else if (s == &motionDummiesGroup2[11]) set (motionValuesGroup2[11], pct (motionDummiesGroup2[11].getValue()));    // Swing
    else if (s == &motionDummiesGroup2[14]) set (motionValuesGroup2[14], juce::String (motionDummiesGroup2[14].getValue(), 2) + " ms"); // Hold
    else if (s == &motionDummiesGroup2[15]) set (motionValuesGroup2[15], pct (motionDummiesGroup2[15].getValue()));    // Sens
    else if (s == &motionDummiesGroup2[16]) set (motionValuesGroup2[16], juce::String (motionDummiesGroup2[16].getValue(), 2) + "°"); // Offset
    else if (s == &motionDummiesGroup2[17]) set (motionValuesGroup2[17], juce::String (motionDummiesGroup2[17].getValue(), 2) + " ms"); // Inertia
    else if (s == &motionDummiesGroup2[18]) set (motionValuesGroup2[18], juce::String (motionDummiesGroup2[18].getValue(), 2));  // Front Bias
    else if (s == &motionDummiesGroup2[19]) set (motionValuesGroup2[19], pct (motionDummiesGroup2[19].getValue()));    // Doppler
    else if (s == &motionDummiesGroup2[20]) set (motionValuesGroup2[20], pct (motionDummiesGroup2[20].getValue()));    // Motion Send
    else if (s == &motionDummiesGroup2[22]) set (motionValuesGroup2[22], Hz (motionDummiesGroup2[22].getValue()));     // Bass Floor
    else if (s == &motionDummiesGroup2[23]) set (motionValuesGroup2[23], pct (motionDummiesGroup2[23].getValue()));    // Occlusion

    // Refresh muted visuals when any control changes
    updateMutedKnobVisuals();
}
void MyPluginAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    // Handle motion panner ComboBox changes
    if (comboBox == &motionComboBoxes[0]) { // Panner ComboBox
        // Redundant safety: force rebind and visual refresh so the UI flips immediately
        motionBinding.trigger();
        refreshMotionControlValues();
        repaint();
    }
}

void MyPluginAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    // Handle button clicks if needed
}

void MyPluginAudioProcessorEditor::applyGlobalCursorPolicy()
{
    auto setCursor = [] (juce::Component& c, auto& ref) -> void
    {
        const bool interactive = dynamic_cast<juce::Button*>(&c)
                                 || dynamic_cast<juce::Slider*>(&c)
                                 || dynamic_cast<juce::ComboBox*>(&c)
                                 || dynamic_cast<ToggleSwitch*>(&c)
                                 || dynamic_cast<XYPad*>(&c);
        if (interactive) c.setMouseCursor (juce::MouseCursor::PointingHandCursor);
        for (int i = 0; i < c.getNumChildComponents(); ++i) ref (*c.getChildComponent (i), ref);
    };
    setCursor (*this, setCursor);
}

void MyPluginAudioProcessorEditor::updateMutedKnobVisuals()
{
    auto setMutedIf = [] (juce::Slider& s, bool muted)
    {
        s.getProperties().set ("muted", muted);
        s.repaint();
    };

    // Neutral/default states considered inactive (greyed): tilt=0, scoop=0, gain=0, width=1, monoHz=0, hp=20, lp=20000, air=0, bass=0, ducking=0, etc.
    setMutedIf (gain,    std::abs ((float) gain.getValue()) < 0.0001f);
    setMutedIf (width,   std::abs ((float) width.getValue() - 1.0f) < 0.0001f);
    setMutedIf (tilt,    std::abs ((float) tilt.getValue()) < 0.0001f);
    setMutedIf (monoHz,  (float) monoHz.getValue() <= 0.0001f);
    setMutedIf (hpHz,    (float) hpHz.getValue() <= 20.0f + 0.001f);
    setMutedIf (lpHz,    (float) lpHz.getValue() >= 20000.0f - 0.001f);
    setMutedIf (air,     std::abs ((float) air.getValue()) < 0.0001f);
    setMutedIf (bass,    std::abs ((float) bass.getValue()) < 0.0001f);
    setMutedIf (scoop,   std::abs ((float) scoop.getValue()) < 0.0001f);
    setMutedIf (spaceKnob, std::abs ((float) spaceKnob.getValue()) < 0.0001f);
    setMutedIf (panKnob,  std::abs ((float) panKnob.getValue())  < 0.0001f);
    setMutedIf (satDrive, std::abs ((float) satDrive.getValue()) < 0.0001f);
    // Mix: only greyed when 0% (default is 100% but should NOT be greyed)
    setMutedIf (satMix,   (float) satMix.getValue() <= 0.0001f);
    // EQ Q link logic handled elsewhere; ensure Q shows greyed ring when unlinked
    if (auto* pLink = proc.apvts.getRawParameterValue ("eq_q_link"))
    {
        const bool link = pLink->load() >= 0.5f;
        filterQ.getProperties().set ("muted", !link);
        filterQ.repaint();
    }
}
void MyPluginAudioProcessorEditor::parameterChanged (const juce::String& id, float nv)
{
    // Bounce to message thread for UI updates during automation/offline render
    if (id == "pan" || id == "depth" || id == "split_mode")
    {
        const float v = nv;
        juce::MessageManager::callAsync ([this, id, v]
        {
            if      (id == "pan")        { pad.setPanValue   (v); }
            else if (id == "depth")      { pad.setSpaceValue (v); }
            else if (id == "split_mode") { pad.setSplitMode  (v >= 0.5f); resized(); }
        });
    }
    else if (id == "mono_slope_db_oct")
    {
        const int idx = (int) std::round (nv); // 0,1,2 from choice
        const int slope = (idx == 0 ? 6 : idx == 1 ? 12 : 24);
        juce::MessageManager::callAsync ([this, slope]
        {
            pad.setMonoSlopeDbPerOct (slope);
        });
    }
    else if (id == "eq_shelf_shape" || id == "eq_q_link" || id == "eq_filter_q"
          || id == "hp_q" || id == "lp_q" || id == "tilt_link_s")
    {
        juce::MessageManager::callAsync ([this]
        {
            syncXYPadWithParameters();
        });
    }
    else if (id == "xover_lo_hz" || id == "xover_hi_hz" || id == "rotation_deg" || id == "asymmetry"
          || id == "shuffler_lo_pct" || id == "shuffler_hi_pct" || id == "shuffler_xover_hz")
    {
        juce::MessageManager::callAsync ([this]
        {
            syncXYPadWithParameters();
        });
    }
    else if (id == motion::id::panner_select)
    {
        // Trigger async rebinding on message thread (no races, no dangles)
        motionBinding.trigger();
    }
}

void MyPluginAudioProcessorEditor::updatePresetDisplay() { /* hook to PresetManager */ }

// --- A/B state helpers (unchanged from your version, trimmed) ---
void MyPluginAudioProcessorEditor::saveCurrentState()
{
    std::map<juce::String, float> s;
    s["gain_db"]  = (float) gain.getValue();
    s["width"]    = (float) width.getValue();
    s["tilt"]     = (float) tilt.getValue();
    s["mono_hz"]  = (float) monoHz.getValue();
    s["hp_hz"]    = (float) hpHz.getValue();
    s["lp_hz"]    = (float) lpHz.getValue();
    s["sat_drive_db"] = (float) satDrive.getValue();
    s["sat_mix"]  = (float) satMix.getValue();
    s["air_db"]   = (float) air.getValue();
    s["bass_db"]  = (float) bass.getValue();
    s["scoop"]    = (float) scoop.getValue();
    s["pan"]      = (float) panKnob.getValue();
    s["depth"]    = (float) spaceKnob.getValue();
    s["ducking"]  = (float) duckingKnob.getValue();
    if (isStateA) stateA = std::move (s); else stateB = std::move (s);
}

static void applyStateToSlider (juce::Slider& s, float v) { s.setValue (v, juce::sendNotificationSync); }

void MyPluginAudioProcessorEditor::loadState (bool A)
{
    const auto& s = A ? stateA : stateB;
    if (s.empty()) return;

    if (auto it = s.find ("gain_db");  it != s.end()) applyStateToSlider (gain, it->second);
    if (auto it = s.find ("width");    it != s.end()) applyStateToSlider (width, it->second);
    if (auto it = s.find ("tilt");     it != s.end()) applyStateToSlider (tilt, it->second);
    if (auto it = s.find ("mono_hz");  it != s.end()) applyStateToSlider (monoHz, it->second);
    if (auto it = s.find ("hp_hz");    it != s.end()) applyStateToSlider (hpHz, it->second);
    if (auto it = s.find ("lp_hz");    it != s.end()) applyStateToSlider (lpHz, it->second);
    if (auto it = s.find ("sat_drive_db"); it != s.end()) applyStateToSlider (satDrive, it->second);
    if (auto it = s.find ("sat_mix");  it != s.end()) applyStateToSlider (satMix, it->second);
    if (auto it = s.find ("air_db");   it != s.end()) applyStateToSlider (air, it->second);
    if (auto it = s.find ("bass_db");  it != s.end()) applyStateToSlider (bass, it->second);
    if (auto it = s.find ("scoop");    it != s.end()) applyStateToSlider (scoop, it->second);
    if (auto it = s.find ("pan");      it != s.end()) applyStateToSlider (panKnob, it->second);
    if (auto it = s.find ("depth");    it != s.end()) applyStateToSlider (spaceKnob, it->second);
    if (auto it = s.find ("ducking");  it != s.end()) applyStateToSlider (duckingKnob, it->second);
}

void MyPluginAudioProcessorEditor::toggleABState()
{
    saveCurrentState();
    isStateA = !isStateA;
    abButtonA.setToggleState (isStateA,   juce::dontSendNotification);
    abButtonB.setToggleState (!isStateA,  juce::dontSendNotification);
    loadState (isStateA);
}

void MyPluginAudioProcessorEditor::copyState (bool fromA) { clipboardState = fromA ? stateA : stateB; }
void MyPluginAudioProcessorEditor::pasteState (bool toA)  { if (!clipboardState.empty()) { (toA ? stateA : stateB) = clipboardState; loadState (toA); } }

void MyPluginAudioProcessorEditor::syncXYPadWithParameters()
{
    pad.setPanValue   ((float) panKnob.getValue());
    pad.setSpaceValue ((float) spaceKnob.getValue());
    pad.setWidthValue ((float) width.getValue());
    pad.setTiltValue  ((float) tilt .getValue());
    pad.setHPValue    ((float) hpHz .getValue());
    pad.setLPValue    ((float) lpHz .getValue());
    // initialize mono slope for XY visualization
    {
        int slopeGuess = 12;
        auto txt = monoSlopeChoice.getText();
        if      (txt == "6")  slopeGuess = 6;
        else if (txt == "12") slopeGuess = 12;
        else if (txt == "24") slopeGuess = 24;
        pad.setMonoSlopeDbPerOct (slopeGuess);
    }
    pad.setAirValue   ((float) air  .getValue());
    pad.setBassValue  ((float) bass .getValue());
    pad.setScoopValue ((float) scoop.getValue());
    pad.setGainValue  ((float) gain .getValue());

    // Push EQ S/Q and link states from APVTS to XYPad and UI
    if (auto* pS = proc.apvts.getRawParameterValue ("eq_shelf_shape"))
        {
            const float S = (float) pS->load();
            pad.setShelfShapeS (S);
            shelfShapeS.getProperties().set ("S_value", (double) S); // expose to LNF for warning segment
            shelfShapeS.repaint();
        }
    if (auto* pLink = proc.apvts.getRawParameterValue ("eq_q_link"))
    {
        const bool link = pLink->load() >= 0.5f;
        pad.setQLink (link);
        // Reflect minis enabled state + muted visuals
        hpQSlider.setEnabled (!link);
        lpQSlider.setEnabled (!link);
        hpQSlider.getProperties().set ("muted", link);
        lpQSlider.getProperties().set ("muted", link);
        filterQ.getProperties().set ("muted", !link); // unlinked => grey ring on global Q
        if (link)
        {
            if (auto* pQ = proc.apvts.getRawParameterValue ("eq_filter_q"))
                pad.setFilterQ ((float) pQ->load());
        }
        else
        {
            if (auto* pHP = proc.apvts.getRawParameterValue ("hp_q")) pad.setHPQ ((float) pHP->load());
            if (auto* pLP = proc.apvts.getRawParameterValue ("lp_q")) pad.setLPQ ((float) pLP->load());
        }
    }
    if (auto* pTiltS = proc.apvts.getRawParameterValue ("tilt_link_s"))
        pad.setTiltUseS (pTiltS->load() >= 0.5f);

    // Imaging/shuffler overlays
    pad.setXoverLoHz   ((float) xoverLoHz.getValue());
    pad.setXoverHiHz   ((float) xoverHiHz.getValue());
    pad.setRotationDeg ((float) rotationDeg.getValue());
    pad.setAsymmetry   ((float) asymmetry.getValue());
    pad.setShuffler    ((float) shufLoPct.getValue(), (float) shufHiPct.getValue(), (float) shufXHz.getValue());
    updateMutedKnobVisuals();
}
void MyPluginAudioProcessorEditor::updateMotionParameterAttachments(int pannerSelect)
{
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    // Store the current attachment counts to know which ones to clear later
    size_t initialAttachmentCount = attachments.size();
    size_t initialComboCount = comboAttachments.size();
    size_t initialButtonCount = buttonAttachments.size();
    
    // Rebuild motion-only attachment buckets deterministically
    motionSliderAttachments.clear();
    motionComboAttachments.clear();
    motionButtonAttachments.clear();

    // Create new attachments based on selected panner
    if (pannerSelect == 1) { // P2 mode
        motionComboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, motion::id::p2_path, motionComboBoxes[1]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_rate_hz,    motionDummiesGroup2[3]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_depth_pct,  motionDummiesGroup2[4]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_phase_deg,  motionDummiesGroup2[5]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_spread_pct, motionDummiesGroup2[6]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_elev_bias,  motionDummiesGroup2[7]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_shape_bounce, motionDummiesGroup2[8]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_jitter_amt, motionDummiesGroup2[9]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_swing_pct,  motionDummiesGroup2[11]));
        motionComboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, motion::id::p2_quantize_div, motionComboBoxes[2]));
        motionComboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, motion::id::p2_mode,        motionComboBoxes[3]));
        motionButtonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, motion::id::p2_retrig,       motionButtons[1]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_hold_ms,    motionDummiesGroup2[14]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_sens,       motionDummiesGroup2[15]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_phase_deg,  motionDummiesGroup2[16]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_inertia_ms, motionDummiesGroup2[17]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_front_bias, motionDummiesGroup2[18]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_doppler_amt,motionDummiesGroup2[19]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_motion_send,motionDummiesGroup2[20]));
        motionButtonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, motion::id::p2_anchor_enable, motionButtons[2]));
    } else { // P1 mode or Link mode
        motionComboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, motion::id::p1_path, motionComboBoxes[1]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_rate_hz,    motionDummiesGroup2[3]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_depth_pct,  motionDummiesGroup2[4]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_phase_deg,  motionDummiesGroup2[5]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_spread_pct, motionDummiesGroup2[6]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_elev_bias,  motionDummiesGroup2[7]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_shape_bounce, motionDummiesGroup2[8]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_jitter_amt, motionDummiesGroup2[9]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_swing_pct,  motionDummiesGroup2[11]));
        motionComboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, motion::id::p1_quantize_div, motionComboBoxes[2]));
        motionComboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, motion::id::p1_mode,        motionComboBoxes[3]));
        motionButtonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, motion::id::p1_retrig,       motionButtons[1]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_hold_ms,    motionDummiesGroup2[14]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_sens,       motionDummiesGroup2[15]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_phase_deg,  motionDummiesGroup2[16]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_inertia_ms, motionDummiesGroup2[17]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_front_bias, motionDummiesGroup2[18]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_doppler_amt,motionDummiesGroup2[19]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_motion_send,motionDummiesGroup2[20]));
        motionButtonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, motion::id::p1_anchor_enable, motionButtons[2]));
    }
}

void MyPluginAudioProcessorEditor::updateMotionParameterAttachmentsOnMessageThread()
{
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;

    // 1) Clear and rebuild motion-only attachments
    motionSliderAttachments.clear();
    motionComboAttachments.clear();
    motionButtonAttachments.clear();

    // 2) Get current panner selection
    const int pannerSelect = (int)std::round(proc.apvts.getRawParameterValue(motion::id::panner_select)->load());

    // 3) Bind controls to P1 or P2 parameters based on selection
    if (pannerSelect == 1) { // P2 mode
        motionComboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, motion::id::p2_path, motionComboBoxes[1]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_rate_hz,    motionDummiesGroup2[3]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_depth_pct,  motionDummiesGroup2[4]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_phase_deg,  motionDummiesGroup2[5]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_spread_pct, motionDummiesGroup2[6]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_elev_bias,  motionDummiesGroup2[7]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_shape_bounce, motionDummiesGroup2[8]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_jitter_amt, motionDummiesGroup2[9]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_swing_pct,  motionDummiesGroup2[11]));
        motionComboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, motion::id::p2_quantize_div, motionComboBoxes[2]));
        motionComboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, motion::id::p2_mode,        motionComboBoxes[3]));
        motionButtonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, motion::id::p2_retrig,       motionButtons[1]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_hold_ms,    motionDummiesGroup2[14]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_sens,       motionDummiesGroup2[15]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_phase_deg,  motionDummiesGroup2[16]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_inertia_ms, motionDummiesGroup2[17]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_front_bias, motionDummiesGroup2[18]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_doppler_amt,motionDummiesGroup2[19]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p2_motion_send,motionDummiesGroup2[20]));
        motionButtonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, motion::id::p2_anchor_enable, motionButtons[2]));
    } else { // P1 or Link (bind to P1)
        motionComboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, motion::id::p1_path, motionComboBoxes[1]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_rate_hz,    motionDummiesGroup2[3]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_depth_pct,  motionDummiesGroup2[4]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_phase_deg,  motionDummiesGroup2[5]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_spread_pct, motionDummiesGroup2[6]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_elev_bias,  motionDummiesGroup2[7]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_shape_bounce, motionDummiesGroup2[8]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_jitter_amt, motionDummiesGroup2[9]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_swing_pct,  motionDummiesGroup2[11]));
        motionComboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, motion::id::p1_quantize_div, motionComboBoxes[2]));
        motionComboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, motion::id::p1_mode,        motionComboBoxes[3]));
        motionButtonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, motion::id::p1_retrig,       motionButtons[1]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_hold_ms,    motionDummiesGroup2[14]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_sens,       motionDummiesGroup2[15]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_phase_deg,  motionDummiesGroup2[16]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_inertia_ms, motionDummiesGroup2[17]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_front_bias, motionDummiesGroup2[18]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_doppler_amt,motionDummiesGroup2[19]));
        motionSliderAttachments.push_back(std::make_unique<SA>(proc.apvts, motion::id::p1_motion_send,motionDummiesGroup2[20]));
        motionButtonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, motion::id::p1_anchor_enable, motionButtons[2]));
    }
    // 4) Refresh all motion control values to reflect the new parameter bindings
    refreshMotionControlValues();
    
    // 5) Pull fresh visual state and repaint immediately (don't wait for timer tick)
    motion::VisualState visualState;
    // Motion Engine is now handled by FieldChain template
    if (false) { // Motion visual state will be handled by FieldChain
        if (panes) {
            panes->setMotionVisualState(visualState);
        }
    }
}

motion::VisualState MyPluginAudioProcessorEditor::synthesizeVisualFromParams()
{
    motion::VisualState vs{};
    
    // Get current panner selection
    const int pannerSelect = (int)std::round(proc.apvts.getRawParameterValue(motion::id::panner_select)->load());
    vs.active = (pannerSelect == 0) ? motion::ActiveSel::P1 : 
               (pannerSelect == 1) ? motion::ActiveSel::P2 : motion::ActiveSel::Link;
    vs.link = (vs.active == motion::ActiveSel::Link);
    
    // Get global parameters
    vs.enable = proc.apvts.getRawParameterValue(motion::id::enable)->load() > 0.5f;
    vs.occlusion = proc.apvts.getRawParameterValue(motion::id::occlusion)->load();
    vs.headphoneSafe = proc.apvts.getRawParameterValue(motion::id::headphone_safe)->load() > 0.5f;
    vs.bassFloorHz = proc.apvts.getRawParameterValue(motion::id::bass_floor_hz)->load();
    
    // Helper to get parameter value safely
    auto getParam = [&](const char* id) -> float {
        if (auto* param = proc.apvts.getRawParameterValue(id)) {
            return param->load();
        }
        return 0.0f;
    };
    
    // Synthesize P1 pose from parameters
    vs.p1.rateHz = getParam(motion::id::p1_rate_hz);
    vs.p1.depth = getParam(motion::id::p1_depth_pct);
    vs.p1.spread = getParam(motion::id::p1_spread_pct);
    vs.p1.pathType = (int)getParam(motion::id::p1_path);
    vs.p1.phaseDeg = getParam(motion::id::p1_phase_deg);
    vs.p1.elevBias = getParam(motion::id::p1_elev_bias);
    vs.p1.bounce = getParam(motion::id::p1_shape_bounce);
    vs.p1.jitter = getParam(motion::id::p1_jitter_amt);
    vs.p1.swing = getParam(motion::id::p1_swing_pct);
    vs.p1.quantizeDiv = (int)getParam(motion::id::p1_quantize_div);
    vs.p1.mode = (int)getParam(motion::id::p1_mode);
    vs.p1.retrig = getParam(motion::id::p1_retrig) > 0.5f;
    vs.p1.holdMs = getParam(motion::id::p1_hold_ms);
    vs.p1.sens = getParam(motion::id::p1_sens);
    vs.p1.inertia = getParam(motion::id::p1_inertia_ms);
    vs.p1.frontBias = getParam(motion::id::p1_front_bias);
    vs.p1.doppler = getParam(motion::id::p1_doppler_amt);
    vs.p1.motionSend = getParam(motion::id::p1_motion_send);
    vs.p1.anchor = getParam(motion::id::p1_anchor_enable) > 0.5f;
    
    // Simple static pose calculation (depth -> radius, phase -> azimuth)
    vs.p1.radius = vs.p1.depth;
    vs.p1.azimuth = vs.p1.phaseDeg / 180.0f; // -1..+1
    vs.p1.elevation = vs.p1.elevBias;
    vs.p1.x = vs.p1.radius * std::cos(juce::MathConstants<float>::halfPi * vs.p1.azimuth);
    vs.p1.y = vs.p1.radius * std::sin(juce::MathConstants<float>::halfPi * vs.p1.azimuth);
    
    // Synthesize P2 pose from parameters
    vs.p2.rateHz = getParam(motion::id::p2_rate_hz);
    vs.p2.depth = getParam(motion::id::p2_depth_pct);
    vs.p2.spread = getParam(motion::id::p2_spread_pct);
    vs.p2.pathType = (int)getParam(motion::id::p2_path);
    vs.p2.phaseDeg = getParam(motion::id::p2_phase_deg);
    vs.p2.elevBias = getParam(motion::id::p2_elev_bias);
    vs.p2.bounce = getParam(motion::id::p2_shape_bounce);
    vs.p2.jitter = getParam(motion::id::p2_jitter_amt);
    vs.p2.swing = getParam(motion::id::p2_swing_pct);
    vs.p2.quantizeDiv = (int)getParam(motion::id::p2_quantize_div);
    vs.p2.mode = (int)getParam(motion::id::p2_mode);
    vs.p2.retrig = getParam(motion::id::p2_retrig) > 0.5f;
    vs.p2.holdMs = getParam(motion::id::p2_hold_ms);
    vs.p2.sens = getParam(motion::id::p2_sens);
    vs.p2.inertia = getParam(motion::id::p2_inertia_ms);
    vs.p2.frontBias = getParam(motion::id::p2_front_bias);
    vs.p2.doppler = getParam(motion::id::p2_doppler_amt);
    vs.p2.motionSend = getParam(motion::id::p2_motion_send);
    vs.p2.anchor = getParam(motion::id::p2_anchor_enable) > 0.5f;
    
    // Simple static pose calculation (depth -> radius, phase -> azimuth)
    vs.p2.radius = vs.p2.depth;
    vs.p2.azimuth = vs.p2.phaseDeg / 180.0f; // -1..+1
    vs.p2.elevation = vs.p2.elevBias;
    vs.p2.x = vs.p2.radius * std::cos(juce::MathConstants<float>::halfPi * vs.p2.azimuth);
    vs.p2.y = vs.p2.radius * std::sin(juce::MathConstants<float>::halfPi * vs.p2.azimuth);
    
    // Set sequence to 0 to indicate synthesized state
    vs.seq = 0;
    
    return vs;
}

void MyPluginAudioProcessorEditor::refreshMotionControlValues()
{
    // Helper functions for formatting values
    auto Hz = [](double v) { return juce::String(v, 1) + " Hz"; };
    auto pct = [](double v) { return juce::String(v, 1) + "%"; };
    auto set = [](juce::Label& l, const juce::String& s) { l.setText(s, juce::dontSendNotification); };
    
    // Get current panner selection to determine which parameters to read
    const int pannerSelect = (int)std::round(proc.apvts.getRawParameterValue(motion::id::panner_select)->load());
    
    // Helper to get parameter value safely
    auto getParam = [&](const char* id) -> float {
        if (auto* param = proc.apvts.getRawParameterValue(id)) {
            return param->load();
        }
        return 0.0f;
    };
    
    // Refresh all motion control values based on current panner selection
    if (pannerSelect == 1) { // P2 mode
        set(motionValuesGroup2[3],  Hz(getParam(motion::id::p2_rate_hz)));     // Rate
        set(motionValuesGroup2[4],  pct(getParam(motion::id::p2_depth_pct)));    // Depth
        set(motionValuesGroup2[5],  juce::String(getParam(motion::id::p2_phase_deg), 1) + "°"); // Phase
        set(motionValuesGroup2[6],  pct(getParam(motion::id::p2_spread_pct)));    // Spread
        set(motionValuesGroup2[7],  juce::String(getParam(motion::id::p2_elev_bias), 1));  // Elev Bias
        set(motionValuesGroup2[8],  pct(getParam(motion::id::p2_shape_bounce)));    // Bounce
        set(motionValuesGroup2[9],  pct(getParam(motion::id::p2_jitter_amt)));    // Jitter
        set(motionValuesGroup2[11], pct(getParam(motion::id::p2_swing_pct)));   // Swing
        set(motionValuesGroup2[14], juce::String(getParam(motion::id::p2_hold_ms), 1) + " ms"); // Hold
        set(motionValuesGroup2[15], pct(getParam(motion::id::p2_sens)));   // Sens
        set(motionValuesGroup2[16], juce::String(getParam(motion::id::p2_phase_deg), 1) + "°"); // Offset (using phase)
        set(motionValuesGroup2[17], juce::String(getParam(motion::id::p2_inertia_ms), 1) + " ms"); // Inertia
        set(motionValuesGroup2[18], juce::String(getParam(motion::id::p2_front_bias), 1)); // Front Bias
        set(motionValuesGroup2[19], pct(getParam(motion::id::p2_doppler_amt)));   // Doppler
        set(motionValuesGroup2[20], pct(getParam(motion::id::p2_motion_send)));   // Motion Send
    } else { // P1 mode or Link mode
        set(motionValuesGroup2[3],  Hz(getParam(motion::id::p1_rate_hz)));     // Rate
        set(motionValuesGroup2[4],  pct(getParam(motion::id::p1_depth_pct)));    // Depth
        set(motionValuesGroup2[5],  juce::String(getParam(motion::id::p1_phase_deg), 1) + "°"); // Phase
        set(motionValuesGroup2[6],  pct(getParam(motion::id::p1_spread_pct)));    // Spread
        set(motionValuesGroup2[7],  juce::String(getParam(motion::id::p1_elev_bias), 1));  // Elev Bias
        set(motionValuesGroup2[8],  pct(getParam(motion::id::p1_shape_bounce)));    // Bounce
        set(motionValuesGroup2[9],  pct(getParam(motion::id::p1_jitter_amt)));    // Jitter
        set(motionValuesGroup2[11], pct(getParam(motion::id::p1_swing_pct)));   // Swing
        set(motionValuesGroup2[14], juce::String(getParam(motion::id::p1_hold_ms), 1) + " ms"); // Hold
        set(motionValuesGroup2[15], pct(getParam(motion::id::p1_sens)));   // Sens
        set(motionValuesGroup2[16], juce::String(getParam(motion::id::p1_phase_deg), 1) + "°"); // Offset (using phase)
        set(motionValuesGroup2[17], juce::String(getParam(motion::id::p1_inertia_ms), 1) + " ms"); // Inertia
        set(motionValuesGroup2[18], juce::String(getParam(motion::id::p1_front_bias), 1)); // Front Bias
        set(motionValuesGroup2[19], pct(getParam(motion::id::p1_doppler_amt)));   // Doppler
        set(motionValuesGroup2[20], pct(getParam(motion::id::p1_motion_send)));   // Motion Send
    }
    
    // Global parameters (same for both panners)
    set(motionValuesGroup2[22], Hz(getParam(motion::id::bass_floor_hz)));    // Bass Floor
    set(motionValuesGroup2[23], pct(getParam(motion::id::occlusion)));   // Occlusion
    
    // Force repaint of all motion controls to show updated values
    for (int i = 0; i < 24; ++i) {
        motionDummiesGroup2[i].repaint();
        motionValuesGroup2[i].repaint();
    }
}

// end