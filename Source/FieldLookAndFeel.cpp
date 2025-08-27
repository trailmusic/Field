#include "FieldLookAndFeel.h"
#include <vector>   // for std::vector

void FieldLNF::drawNeoPanel (juce::Graphics& g, juce::Rectangle<float> r, float radius) const
{
    auto inner = r.reduced (3.0f);

    // Main panel background
    g.setColour (theme.panel);
    g.fillRoundedRectangle (inner, radius);

    // Enhanced shadows for more depth (reduced blur for stronger edges)
    juce::DropShadow deepShadow  (theme.shadowDark.withAlpha (0.6f), 12, { -2, -2 });
    juce::DropShadow lightShadow (theme.shadowLight.withAlpha (0.4f),  6, { -1, -1 });

    const auto shadowRect = inner.getSmallestIntegerContainer();
    deepShadow .drawForRectangle (g, shadowRect);
    lightShadow.drawForRectangle (g, shadowRect);

    // Subtle inner rim for inset effect
    g.setColour (theme.sh.withAlpha (0.2f));
    g.drawRoundedRectangle (inner.reduced (1.0f), juce::jmax (0.0f, radius - 1.0f), 1.0f);
}

void FieldLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                 float sliderPosProportional, float rotaryStartAngle,
                                 float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> (x, y, w, h).reduced (6.0f);

    // Uniform hover/active raise effect
    if (slider.isMouseOverOrDragging() || slider.isMouseButtonDown())
        bounds = bounds.expanded (2.0f);

    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();

    juce::Path body; body.addEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    g.setColour (theme.panel.brighter (0.08f));
    g.fillPath (body);

    juce::ColourGradient grad (theme.hl.withAlpha (0.7f), centre.x - radius * 0.5f, centre.y - radius * 0.6f,
                               theme.sh.withAlpha (0.7f), centre.x + radius * 0.6f, centre.y + radius * 0.7f, false);
    g.setGradientFill (grad);
    g.fillPath (body);

    const float trackThickness = radius * 0.18f;

    // Background ring
    juce::Path ring;
    ring.addCentredArc (centre.x, centre.y, radius * 0.86f, radius * 0.86f, 0.0f,
                        rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (theme.base.darker (0.2f));
    g.strokePath (ring, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value arc
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, radius * 0.86f, radius * 0.86f, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (theme.accent.withAlpha (0.9f));
    g.strokePath (valueArc, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Tick marks at quarter points across whatever arc span we're given
    const float tickRadius = radius * 0.86f;
    const float tickSize   = radius * 0.04f;
    const float arcSpan    = rotaryEndAngle - rotaryStartAngle;

    const std::vector<float> tickAngles = {
        rotaryStartAngle,
        rotaryStartAngle + arcSpan * 0.25f,
        rotaryStartAngle + arcSpan * 0.50f,
        rotaryStartAngle + arcSpan * 0.75f,
        rotaryEndAngle
    };

    g.setColour (theme.accent);
    for (const float a : tickAngles)
        g.fillEllipse (centre.x + tickRadius * std::cos (a) - tickSize,
                       centre.y + tickRadius * std::sin (a) - tickSize,
                       tickSize * 2.0f, tickSize * 2.0f);

    // Knob label in the center
    const juce::String knobName = slider.getName();
    if (knobName.isNotEmpty())
        drawKnobLabel (g, bounds, knobName);
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
    // Deliberately NOT trying to infer hover here; keep consistent sizing for the custom gain knob.
    auto bounds = juce::Rectangle<float> (x, y, w, h).reduced (6.0f);

    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();

    juce::Path body; body.addEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    g.setColour (theme.panel.brighter (0.08f));
    g.fillPath (body);

    juce::ColourGradient grad (theme.hl.withAlpha (0.7f), centre.x - radius * 0.5f, centre.y - radius * 0.6f,
                               theme.sh.withAlpha (0.7f), centre.x + radius * 0.6f, centre.y + radius * 0.7f, false);
    g.setGradientFill (grad);
    g.fillPath (body);

    const float trackThickness = radius * 0.18f;

    // Background ring
    juce::Path ring;
    ring.addCentredArc (centre.x, centre.y, radius * 0.86f, radius * 0.86f, 0.0f,
                        rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (theme.base.darker (0.2f));
    g.strokePath (ring, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value arc
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, radius * 0.86f, radius * 0.86f, 0.0f, rotaryStartAngle, angle, true);

    // Blue (accent) progress like other knobs
    g.setColour (theme.accent.withAlpha (0.9f));
    g.strokePath (valueArc, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Tick marks
    const float tickRadius = radius * 0.86f;
    const float tickSize   = radius * 0.04f;
    const float arcSpan    = rotaryEndAngle - rotaryStartAngle;

    const std::vector<float> tickAngles = {
        rotaryStartAngle,
        rotaryStartAngle + arcSpan * 0.25f,
        rotaryStartAngle + arcSpan * 0.50f,
        rotaryStartAngle + arcSpan * 0.75f,
        rotaryEndAngle
    };

    g.setColour (theme.accent);
    for (const float a : tickAngles)
        g.fillEllipse (centre.x + tickRadius * std::cos (a) - tickSize,
                       centre.y + tickRadius * std::sin (a) - tickSize,
                       tickSize * 2.0f, tickSize * 2.0f);

    // Colour delta overlay for gain indication (inner ring)
    if (std::abs (gainDb) > 0.1f)
    {
        juce::Colour deltaColor;
        if (gainDb > 0.1f)
        {
            // Boost: green → yellow-ish
            const float k = juce::jmap (gainDb, 0.0f, 24.0f, 0.0f, 1.0f);
            deltaColor = juce::Colour::fromHSV (0.25f - k * 0.10f, 0.8f, 0.8f, 0.6f);
        }
        else
        {
            // Cut: red range
            const float k = juce::jmap (std::abs (gainDb), 0.0f, 24.0f, 0.0f, 1.0f);
            deltaColor = juce::Colour::fromHSV (0.00f + k * 0.10f, 0.8f, 0.8f, 0.6f);
        }

        g.setColour (deltaColor);
        g.strokePath (valueArc, juce::PathStrokeType (trackThickness * 0.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Outer border hint when the absolute gain is notable
    if (std::abs (gainDb) > 1.0f)
    {
        const float k = juce::jmap (std::abs (gainDb), 1.0f, 24.0f, 0.0f, 1.0f);
        juce::Colour border = (gainDb > 0.0f ? juce::Colours::lime : juce::Colours::red).withAlpha (0.3f * k);
        g.setColour (border);
        g.drawEllipse (centre.x - radius * 0.95f, centre.y - radius * 0.95f, radius * 1.9f, radius * 1.9f, trackThickness * 0.9f);
    }

    drawKnobLabel (g, bounds, "GAIN");
}

void FieldLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h,
                                 float /*sliderPos*/, float /*minSliderPos*/, float /*maxSliderPos*/,
                                 juce::Slider::SliderStyle style, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> (x, y, w, h);

    // For non-horizontal styles, fall back to default look (keeps our custom look focused on the micro sliders)
    // JUCE 8 does not have LinearBarDragThumb; only check for supported styles
    if (style != juce::Slider::LinearHorizontal && style != juce::Slider::LinearBar)
    {
        juce::LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, 0.0f, 0.0f, 0.0f, style, slider);
        return;
    }

    // Background track
    juce::ColourGradient bgGrad (theme.panel.darker (0.3f), bounds.getX(), bounds.getY(),
                                 theme.panel.darker (0.1f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (bgGrad);
    g.fillRoundedRectangle (bounds.reduced (1.0f), 3.0f);

    // Use JUCE’s skew-aware mapping
    const float t = juce::jlimit (0.0f, 1.0f, (float) slider.valueToProportionOfLength (slider.getValue()));

    // Progress fill (left → thumb)
    auto progress = bounds.reduced (1.0f);
    progress.setWidth (progress.getWidth() * t);

    if (progress.getWidth() > 0.0f)
    {
        juce::ColourGradient progressGrad (theme.accent.brighter (0.2f), progress.getX(), progress.getY(),
                                           theme.accent,                  progress.getX(), progress.getBottom(), false);
        g.setGradientFill (progressGrad);
        g.fillRoundedRectangle (progress, 3.0f);
    }

    // Thumb
    const float thumbW = 12.0f;
    const float thumbH = bounds.getHeight() + 8.0f;

    float thumbX = bounds.getX() + (bounds.getWidth() * t) - (thumbW * 0.5f);
    const float thumbY = bounds.getY() - 4.0f;

    // Clamp thumb within visual bounds
    thumbX = juce::jlimit (bounds.getX() - thumbW * 0.5f,
                           bounds.getRight() - thumbW * 0.5f,
                           thumbX);

    // Shadow
    g.setColour (juce::Colours::black.withAlpha (0.3f));
    g.fillRoundedRectangle (thumbX + 1.0f, thumbY + 1.0f, thumbW, thumbH, 5.0f);

    // Thumb body
    juce::ColourGradient thumbGrad (theme.panel.brighter (0.4f), thumbX, thumbY,
                                    theme.panel.brighter (0.1f), thumbX, thumbY + thumbH, false);
    g.setGradientFill (thumbGrad);
    g.fillRoundedRectangle (thumbX, thumbY, thumbW, thumbH, 5.0f);

    // Thumb border
    g.setColour (theme.accent);
    g.drawRoundedRectangle (thumbX, thumbY, thumbW, thumbH, 5.0f, 2.0f);

    // Hover/drag feedback
    const bool isOver  = slider.isMouseOverOrDragging();
    const bool isDown  = slider.isMouseButtonDown();

    if (isOver)
    {
        g.setColour (theme.accent.withAlpha (0.5f));
        g.fillRoundedRectangle (thumbX - 4.0f, thumbY - 4.0f, thumbW + 8.0f, thumbH + 8.0f, 6.0f);

        if (isDown)
        {
            g.setColour (theme.accent.brighter (0.4f).withAlpha (0.7f));
            g.fillRoundedRectangle (thumbX + 1.0f, thumbY + 1.0f, thumbW - 2.0f, thumbH - 2.0f, 4.0f);
        }
    }

    // Subtle interactive outline
    g.setColour (theme.accent.withAlpha (0.10f));
    g.drawRoundedRectangle (bounds.reduced (1.0f), 3.0f, 1.0f);
}

int FieldLNF::getSliderThumbRadius (juce::Slider&)
{
    return 8;
}
