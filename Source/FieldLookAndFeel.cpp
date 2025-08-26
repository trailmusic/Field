#include "FieldLookAndFeel.h"

void FieldLNF::drawNeoPanel (juce::Graphics& g, juce::Rectangle<float> r, float radius) const
{
    auto inner = r.reduced (3.0f);

    // Main panel background
    g.setColour (theme.panel);
    g.fillRoundedRectangle (inner, radius);

    // Enhanced shadows for more depth (reduced blur for stronger edges)
    juce::DropShadow deepShadow (theme.shadowDark.withAlpha (0.6f), 12, { -2, -2 });
    juce::DropShadow lightShadow (theme.shadowLight.withAlpha (0.4f), 6, { -1, -1 });
    juce::DropShadow innerShadow (theme.sh.withAlpha (0.3f), 2, { -1, -1 });
    
    deepShadow.drawForRectangle (g, inner.getSmallestIntegerContainer());
    lightShadow.drawForRectangle (g, inner.getSmallestIntegerContainer());
    
    // Subtle inner shadow for inset effect
    g.setColour (theme.sh.withAlpha (0.2f));
    g.drawRoundedRectangle (inner.reduced (1.0f), radius - 1.0f, 1.0f);
}

void FieldLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                 float sliderPosProportional, float rotaryStartAngle,
                                 float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> (x, y, w, h).reduced (6.0f);
    // Uniform hover/active raise effect for all rotary sliders
    if (slider.isMouseOverOrDragging() || slider.isMouseButtonDown())
        bounds = bounds.expanded(2.0f);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto centre = bounds.getCentre();

    juce::Path p; p.addEllipse (centre.x - radius, centre.y - radius, radius*2, radius*2);
    g.setColour (theme.panel.brighter (0.08f));
    g.fillPath (p);

    juce::ColourGradient grad (theme.hl.withAlpha (0.7f), centre.x - radius*0.5f, centre.y - radius*0.6f,
                               theme.sh.withAlpha (0.7f), centre.x + radius*0.6f, centre.y + radius*0.7f, false);
    g.setGradientFill (grad);
    g.fillPath (p);

    const float thickness = radius * 0.18f;
    juce::Path ring;
    ring.addCentredArc (centre.x, centre.y, radius*0.86f, radius*0.86f, 0.0f,
                        rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (theme.base.darker (0.2f));
    g.strokePath (ring, juce::PathStrokeType (thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    juce::Path val;
    val.addCentredArc (centre.x, centre.y, radius*0.86f, radius*0.86f, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (theme.accent.withAlpha (0.9f));
    g.strokePath (val, juce::PathStrokeType (thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Draw tick mark indicators at specific positions
    const float tickRadius = radius * 0.86f;
    const float tickSize = radius * 0.04f; // Small circle size
    
    // Calculate tick positions for full 360° circle
    // With rotaryStartAngle = -π and rotaryEndAngle = +π, arcSpan = 2π (360°)
    float arcSpan = rotaryEndAngle - rotaryStartAngle;
    
    // Calculate tick angles for full circle: start, quarter, half, three-quarter, full
    std::vector<float> tickAngles = {
        rotaryStartAngle,                    // Start (0.0) - 9 o'clock (-π)
        rotaryStartAngle + arcSpan * 0.25f,  // Quarter (0.25) - 12 o'clock (0)
        rotaryStartAngle + arcSpan * 0.5f,   // Half (0.5) - 3 o'clock (+π/2)
        rotaryStartAngle + arcSpan * 0.75f,  // Three-quarter (0.75) - 6 o'clock (+π)
        rotaryEndAngle                       // Full (1.0) - 9 o'clock again (-π)
    };
    
    for (float tickAngle : tickAngles) {
        float tickX = centre.x + tickRadius * std::cos(tickAngle);
        float tickY = centre.y + tickRadius * std::sin(tickAngle);
        
        g.setColour(theme.accent);
        g.fillEllipse(tickX - tickSize, tickY - tickSize, tickSize * 2, tickSize * 2);
    }

    // Draw knob label in center
    juce::String knobName = slider.getName();
    if (knobName.isNotEmpty()) {
        drawKnobLabel (g, bounds, knobName);
    }

}

void FieldLNF::drawKnobLabel (juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text)
{
    g.setColour (theme.text);
    g.setFont (juce::Font (juce::FontOptions (bounds.getHeight() * 0.12f + 2.0f).withStyle ("Bold")));
    g.drawText (text, bounds.toNearestInt(), juce::Justification::centred);
}

void FieldLNF::drawGainSlider (juce::Graphics& g, int x, int y, int w, int h,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, float gainDb)
{
    auto bounds = juce::Rectangle<float> (x, y, w, h).reduced (6.0f);
    // Uniform hover/active raise effect for gain slider as well
    // Note: we don't have the Slider reference here, so infer via context by expanding slightly if the current context allows hover.
    // Since this is called from a Slider's paint, we can approximate using Desktop mouse position.
    // However, for consistency, we keep the same visual without dependency by skipping; the caller uses our rotary draw pattern.
    // Expand by 2px if the mouse is over the gain slider bounds in screen space.
    {
        auto comp = juce::Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();
        if (comp && comp->isMouseOver(true))
            bounds = bounds.expanded(2.0f);
    }
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto centre = bounds.getCentre();

    juce::Path p; p.addEllipse (centre.x - radius, centre.y - radius, radius*2, radius*2);
    g.setColour (theme.panel.brighter (0.08f));
    g.fillPath (p);

    juce::ColourGradient grad (theme.hl.withAlpha (0.7f), centre.x - radius*0.5f, centre.y - radius*0.6f,
                               theme.sh.withAlpha (0.7f), centre.x + radius*0.6f, centre.y + radius*0.7f, false);
    g.setGradientFill (grad);
    g.fillPath (p);

    const float thickness = radius * 0.18f;
    juce::Path ring;
    ring.addCentredArc (centre.x, centre.y, radius*0.86f, radius*0.86f, 0.0f,
                        rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (theme.base.darker (0.2f));
    g.strokePath (ring, juce::PathStrokeType (thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    juce::Path val;
    val.addCentredArc (centre.x, centre.y, radius*0.86f, radius*0.86f, 0.0f, rotaryStartAngle, angle, true);
    
    // Draw blue progress indicator (like other knobs)
    g.setColour (theme.accent.withAlpha (0.9f));
    g.strokePath (val, juce::PathStrokeType (thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw tick mark indicators at specific positions
    const float tickRadius = radius * 0.86f;
    const float tickSize = radius * 0.04f; // Small circle size
    
    // Calculate tick positions for full 360° circle
    // With rotaryStartAngle = -π and rotaryEndAngle = +π, arcSpan = 2π (360°)
    float arcSpan = rotaryEndAngle - rotaryStartAngle;
    
    // Calculate tick angles for full circle: start, quarter, half, three-quarter, full
    std::vector<float> tickAngles = {
        rotaryStartAngle,                    // Start (0.0) - 9 o'clock (-π)
        rotaryStartAngle + arcSpan * 0.25f,  // Quarter (0.25) - 12 o'clock (0)
        rotaryStartAngle + arcSpan * 0.5f,   // Half (0.5) - 3 o'clock (+π/2)
        rotaryStartAngle + arcSpan * 0.75f,  // Three-quarter (0.75) - 6 o'clock (+π)
        rotaryEndAngle                       // Full (1.0) - 9 o'clock again (-π)
    };
    
    for (float tickAngle : tickAngles) {
        float tickX = centre.x + tickRadius * std::cos(tickAngle);
        float tickY = centre.y + tickRadius * std::sin(tickAngle);
        
        g.setColour(theme.accent);
        g.fillEllipse(tickX - tickSize, tickY - tickSize, tickSize * 2, tickSize * 2);
    }

    // Add color delta overlay for gain indication
    if (std::abs(gainDb) > 0.1f) {
        juce::Colour deltaColor;
        if (gainDb > 0.1f) {
            // Boost: green to yellow gradient
            float intensity = juce::jmap (gainDb, 0.0f, 24.0f, 0.0f, 1.0f);
            deltaColor = juce::Colour::fromHSV (0.25f - intensity * 0.1f, 0.8f, 0.8f, 0.6f);
        } else {
            // Cut: red to purple gradient
            float intensity = juce::jmap (std::abs(gainDb), 0.0f, 24.0f, 0.0f, 1.0f);
            deltaColor = juce::Colour::fromHSV (0.0f + intensity * 0.1f, 0.8f, 0.8f, 0.6f);
        }
        
        // Draw delta indicator as inner ring
        g.setColour (deltaColor);
        g.strokePath (val, juce::PathStrokeType (thickness * 0.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    
    // Additional border ring to show gain amount (thicker)
    if (std::abs(gainDb) > 1.0f) {
        float borderIntensity = juce::jmap (std::abs(gainDb), 1.0f, 24.0f, 0.0f, 1.0f);
        juce::Colour borderColor = gainDb > 0 ? juce::Colours::lime.withAlpha (0.3f * borderIntensity) 
                                             : juce::Colours::red.withAlpha (0.3f * borderIntensity);
        g.setColour (borderColor);
        g.drawEllipse (centre.x - radius*0.95f, centre.y - radius*0.95f, radius*1.9f, radius*1.9f, thickness * 0.9f);
    }
    
    // Draw "GAIN" label in center
    drawKnobLabel (g, bounds, "GAIN");
}

void FieldLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h,
                                 float sliderPos, float minSliderPos, float maxSliderPos,
                                 juce::Slider::SliderStyle style, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, w, h);

    // Background track
    juce::ColourGradient bgGrad(
        theme.panel.darker(0.3f), bounds.getX(), bounds.getY(),
        theme.panel.darker(0.1f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bgGrad);
    g.fillRoundedRectangle(bounds.reduced(1.0f), 3.0f);

    // --- IMPORTANT: use JUCE's skew-aware mapping for proper positioning ---
    // Use JUCE's built-in proportion calculation that respects the NormalisableRange skew
    float t = juce::jlimit(0.0f, 1.0f, (float) slider.valueToProportionOfLength(slider.getValue()));

    // Progress fill (left -> thumb)
    auto progressBounds = bounds.reduced(1.0f);
    progressBounds.setWidth(progressBounds.getWidth() * t);

    if (progressBounds.getWidth() > 0.0f) {
        juce::ColourGradient progressGrad(
            theme.accent.brighter(0.2f), progressBounds.getX(), progressBounds.getY(),
            theme.accent,                progressBounds.getX(), progressBounds.getBottom(), false);
        g.setGradientFill(progressGrad);
        g.fillRoundedRectangle(progressBounds, 3.0f);
    }

    // Thumb
    const float thumbWidth  = 12.0f;
    const float thumbHeight = bounds.getHeight() + 8.0f;

    // Calculate thumb position based on proportion 't' (0-1) across the slider bounds
    float thumbX = bounds.getX() + (bounds.getWidth() * t) - (thumbWidth * 0.5f);
    float thumbY = bounds.getY() - 4.0f;

    // Clamp thumb to visual bounds so it never disappears
    thumbX = juce::jlimit(bounds.getX() - thumbWidth * 0.5f,
                          bounds.getRight() - thumbWidth * 0.5f,
                          thumbX);

    // Shadow
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(thumbX + 1.0f, thumbY + 1.0f, thumbWidth, thumbHeight, 5.0f);

    // Thumb body
    juce::ColourGradient thumbGrad(
        theme.panel.brighter(0.4f), thumbX, thumbY,
        theme.panel.brighter(0.1f), thumbX, thumbY + thumbHeight, false);
    g.setGradientFill(thumbGrad);
    g.fillRoundedRectangle(thumbX, thumbY, thumbWidth, thumbHeight, 5.0f);

    // Thumb border
    g.setColour(theme.accent);
    g.drawRoundedRectangle(thumbX, thumbY, thumbWidth, thumbHeight, 5.0f, 2.0f);

    // Hover/drag feedback
    const bool isMouseOver = slider.isMouseOverOrDragging();
    const bool isMouseDown = slider.isMouseButtonDown();

    if (isMouseOver) {
        g.setColour(theme.accent.withAlpha(0.5f));
        g.fillRoundedRectangle(thumbX - 4.0f, thumbY - 4.0f, thumbWidth + 8.0f, thumbHeight + 8.0f, 6.0f);

        if (isMouseDown) {
            g.setColour(theme.accent.brighter(0.4f).withAlpha(0.7f));
            g.fillRoundedRectangle(thumbX + 1.0f, thumbY + 1.0f, thumbWidth - 2.0f, thumbHeight - 2.0f, 4.0f);
        }
    }

    // Subtle interactive outline
    g.setColour(theme.accent.withAlpha(0.1f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 3.0f, 1.0f);
}

int FieldLNF::getSliderThumbRadius (juce::Slider&)
{
    return 8; // Larger thumb radius for better interaction
}