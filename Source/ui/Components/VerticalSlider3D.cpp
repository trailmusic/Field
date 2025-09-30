#include "VerticalSlider3D.h"
#include "../../Core/FieldLookAndFeel.h"

VerticalSlider3D::VerticalSlider3D()
{
    setSliderStyle (juce::Slider::LinearVertical);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
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
    auto accentColor = lf->theme.accent;
    
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
    
    // Draw track (leave room for bottom labels)
    const float trackTop = 20.0f; // Start track lower
    const float trackBottom = bounds.getHeight() - 40.0f; // Leave room for bottom labels
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
    
    // Draw bottom value label
    drawBottomLabel (g, bounds);
    
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
    const auto accent = lf->theme.accent;
    const auto shadowDark = lf->theme.shadowDark;
    const auto shadowLight = lf->theme.shadowLight;
    
    // Create gradient for 3D effect
    juce::ColourGradient gradient (accent.brighter (0.3f), 
                                  juce::Point<float>(handleRect.getCentreX(), handleRect.getY()),
                                  accent.darker (0.3f), 
                                  juce::Point<float>(handleRect.getCentreX(), handleRect.getBottom()),
                                  false);
    gradient.addColour (0.5, accent);
    
    // Draw handle shadow
    g.setColour (shadowDark.withAlpha (0.4f));
    g.fillRoundedRectangle (handleRect.translated (2, 2), 4.0f);
    
    // Draw handle body
    g.setGradientFill (gradient);
    g.fillRoundedRectangle (handleRect, 4.0f);
    
    // Draw accent color outer frame first (smaller frame)
    g.setColour (accent);
    g.fillRoundedRectangle (handleRect.reduced (1), 1.0f);
    
    // Draw grey interior fill on top (more room for labels)
    g.setColour (lf->theme.meters.panelDark.darker (0.5f));
    g.fillRoundedRectangle (handleRect.reduced (3), 2.0f);
    
    // Draw rim
    g.setColour (accent.darker (0.2f));
    g.drawRoundedRectangle (handleRect, 4.0f, 1.0f);
    
    // Draw handle label (I, O, M) - use component name to identify slider type
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (7.0f, juce::Font::bold));
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
    const auto textColor = lf->theme.textMuted;
    const auto accentColor = lf->theme.accent;
    
    g.setColour (textColor);
    g.setFont (juce::Font (7.0f, juce::Font::bold));
    
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
        // Percentage range (Mix slider) - no % symbols to save space
        markerValues = {0.0f, 25.0f, 50.0f, 75.0f, 100.0f};
        markerLabels = {"0", "25", "50", "75", "100"};
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
        g.setFont (juce::Font (7.0f, juce::Font::bold));
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
    
    // Track background using meter colors
    g.setColour (lf->theme.meters.trackBase);
    g.fillRoundedRectangle (trackRect, 4.0f);
    
    // Inner shadow
    g.setColour (lf->theme.meters.trackBorder);
    g.drawRoundedRectangle (trackRect.reduced (0.5f), 4.0f, 1.0f);
    
    // Highlight
    g.setColour (lf->theme.meters.trackBase.withAlpha (0.3f));
    g.drawRoundedRectangle (trackRect.reduced (1.0f), 4.0f, 0.5f);
}

void VerticalSlider3D::drawMetallicBackground (juce::Graphics& g, juce::Rectangle<float> backgroundRect)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto panel = lf->theme.meters.panelMedium;
    const auto shadowDark = lf->theme.shadowDark;
    const auto shadowLight = lf->theme.shadowLight;
    
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

void VerticalSlider3D::drawBottomLabel (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    if (!lf) return;
    
    const auto textColor = lf->theme.textMuted;
    g.setColour (textColor);
    g.setFont (juce::Font (7.0f, juce::Font::bold));
    
    // Format the current value based on slider type
    juce::String valueText;
    const float currentValue = getValue();
    juce::String componentName = getName();
    
    if (componentName.contains ("mix")) {
        // Mix slider: show percentage without % symbol
        valueText = juce::String (juce::roundToInt (currentValue));
    } else {
        // Input/Output sliders: show dB value
        valueText = juce::String (currentValue, 1);
    }
    
    // Draw at bottom of slider
    const float labelY = bounds.getHeight() - 25.0f;
    const float labelHeight = 15.0f;
    g.drawText (valueText, bounds.getX(), labelY, bounds.getWidth(), labelHeight, juce::Justification::centred);
}

void VerticalSlider3D::setSliderStyle (SliderStyle newStyle)
{
    // Force vertical style
    juce::Slider::setSliderStyle (juce::Slider::LinearVertical);
}
