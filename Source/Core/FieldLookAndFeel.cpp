#include "FieldLookAndFeel.h"
#include "IconSystem.h"
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

void FieldLNF::drawRotationPad (juce::Graphics& g, juce::Rectangle<float> b,
                                float rotationDeg, float a,
                                juce::Colour accent, juce::Colour text, juce::Colour panel) const
{
    b = b.reduced (4.0f);
    const auto c   = b.getCentre();
    const float r  = 0.5f * std::min (b.getWidth(), b.getHeight()) - 4.0f;

    // Energy circle (ring only, no container background)
    g.setColour (panel.brighter (0.25f));
    g.drawEllipse (c.x - r, c.y - r, 2*r, 2*r, 1.6f);

    // Basis vectors (screen Y goes down)
    const float th = juce::degreesToRadians (rotationDeg);
    auto u = juce::Point<float> ( std::cos (th), -std::sin (th)); // M'
    auto v = juce::Point<float> ( std::sin (th),  std::cos (th)); // S'

    auto line = [&] (juce::Point<float> dir, juce::Colour col, float w)
    {
        juce::Path p;
        p.startNewSubPath (c + (-r) * dir);
        p.lineTo          (c + ( r) * dir);
        g.setColour (col);
        g.strokePath (p, juce::PathStrokeType (w, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    };

    // Original M/S (faint)
    line ({0,-1}, panel.darker (0.10f), 1.0f);
    line ({1, 0}, panel.darker (0.10f), 1.0f);

    // Rotated M'/S' (bold)
    line (u, accent.withAlpha (0.85f), 2.0f);
    line (v, accent.withAlpha (0.55f), 1.6f);

    // Orthonormal S-curve (cubic Bézier)
    const float baseH = r * 0.75f;
    const float bend  = 1.0f;
    const float aCl   = juce::jlimit (-1.0f, 1.0f, a);
    const float h1    = baseH * (1.0f + 0.35f * aCl);
    const float h2    = baseH * (1.0f - 0.35f * aCl);

    const auto P0 = c + (-r) * u;
    const auto P3 = c + ( r) * u;
    const auto P1 = P0 + ( bend * h1) * v; // +S'
    const auto P2 = P3 + (-bend * h2) * v; // -S'

    juce::Path S; S.startNewSubPath (P0); S.cubicTo (P1, P2, P3);

    // Gradient shade the asymmetry lobes inside the ring
    {
        juce::Graphics::ScopedSaveState save (g);
        // Clip to circle
        juce::Path circ; circ.addEllipse (c.x - r, c.y - r, 2*r, 2*r);
        g.reduceClipRegion (circ);

        // Linear gradient along S' axis (v). Bias alpha by asymmetry sign/magnitude
        const float intensity = 0.26f * std::abs (aCl); // heavier tint
        const bool positive   = (aCl >= 0.0f);
        const float aNear = positive ? 0.06f : intensity;
        const float aFar  = positive ? intensity : 0.06f;

        auto p0 = c - v * r;
        auto p1 = c + v * r;
        juce::ColourGradient grad (accent.withAlpha (aNear), p0.x, p0.y,
                                   accent.withAlpha (aFar),  p1.x, p1.y, false);
        g.setGradientFill (grad);
        g.fillEllipse (c.x - r, c.y - r, 2*r, 2*r);
    }

    // Draw S curve on top (solid)
    g.setColour (accent.withAlpha (0.85f));
    g.strokePath (S, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Labels M' / S' outside the rim (smaller font)
    g.setColour (text.withAlpha (0.70f));
    g.setFont (juce::Font (juce::FontOptions (10.0f).withStyle ("Bold")));
    auto labelAt = [&] (juce::String t, juce::Point<float> dir, float d)
    {
        auto p = c + dir * d;
        g.drawFittedText (t, juce::Rectangle<int> ((int) (p.x - 12), (int) (p.y - 8), 24, 16), juce::Justification::centred, 1);
    };
    labelAt ("M", u, r + 12.0f);
    labelAt ("S", v, r + 12.0f);

    // Angle tick on rim (dual-ended)
    auto rimTick = [&] (float ang, juce::Colour col)
    {
        juce::Point<float> d (std::cos (ang), -std::sin (ang));
        auto p0 = c + d * (r - 6.0f);
        auto p1 = c + d * (r);
        g.setColour (col);
        g.drawLine ({ p0, p1 }, 2.0f);
        g.drawLine ({ c - d * (r - 6.0f), c - d * r }, 2.0f);
    };
    rimTick (th, accent.withAlpha (0.8f));
}

void FieldLNF::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                             int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float> (0, 0, (float) width, (float) height).reduced (2.0f);
    auto accent = theme.accent;

    // Background: mimic SwitchCell panel (mode cell style)
    drawNeoPanel (g, r, 5.0f);

    // Determine selected text and optional per-item tint
    const int selIdx = box.getSelectedItemIndex(); // 0-based
    juce::String selText = box.getText();
    // If requested (for specific boxes), or if label text is empty while we do have a selection,
    // pull the text directly from the selected item to avoid showing the chevron placeholder.
    const bool forceSelectedText = (bool) box.getProperties().getWithDefault ("forceSelectedText", false);
    if ((forceSelectedText || selText.isEmpty()) && selIdx >= 0)
        selText = box.getItemText (selIdx);

    // Optional default text when there is no selection at all
    if (selText.isEmpty() && selIdx < 0)
    {
        const juce::String defText = box.getProperties().getWithDefault ("defaultTextWhenEmpty", juce::var()).toString();
        if (defText.isNotEmpty())
            selText = defText;
    }
    juce::Colour selTint = (selIdx >= 0 && selIdx < popupItemTints.size()) ? popupItemTints.getReference (selIdx) : accent;

    // If nothing selected, show chevron-only (iconOnly behavior)
    if (selText.isEmpty())
    {
        const bool over = box.isMouseOver (true);
        const bool down = isButtonDown;
        juce::Colour arrow = over ? accent : theme.text;
        if (down) arrow = arrow.brighter (0.2f);

        auto icon = r.reduced (6.0f);
        const float cx = icon.getCentreX();
        const float cy = icon.getCentreY();
        const float w  = juce::jmin (icon.getWidth(), icon.getHeight()) * 0.50f;
        juce::Path chevron;
        chevron.startNewSubPath (cx - w * 0.45f, cy - w * 0.10f);
        chevron.lineTo       (cx,               cy + w * 0.25f);
        chevron.lineTo       (cx + w * 0.45f,   cy - w * 0.10f);
        // Subtle shadow under arrow for weight
        g.setColour (juce::Colours::black.withAlpha (0.18f));
        g.strokePath (chevron, juce::PathStrokeType (3.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        // Main arrow
        g.setColour (arrow.withAlpha (0.98f));
        g.strokePath (chevron, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Inner glow on hover
        if (over)
        {
            g.setColour (accent.withAlpha (0.10f));
            g.fillRoundedRectangle (icon, 5.0f);
        }
        return;
    }

    // When selected, render the label centered with its tint and hide chevron
    g.setColour (selTint.withAlpha (0.92f));
    auto inner = r.reduced (6.0f);
    selText = selText.toUpperCase();
    g.setFont (getComboBoxFont (box).boldened());
    g.drawFittedText (selText, inner.toNearestInt(), juce::Justification::centred, 1);
}

void FieldLNF::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    // Hide default label text when using iconOnly or tintedSelected rendering
    const bool iconOnly      = (bool) box.getProperties().getWithDefault ("iconOnly", false);
    const bool tintedSelected = (bool) box.getProperties().getWithDefault ("tintedSelected", false);
    if (iconOnly || tintedSelected)
    {
        label.setText ("", juce::dontSendNotification);
        label.setBounds (0, 0, 0, 0);
        label.setVisible (false);
        return;
    }
    LookAndFeel_V4::positionComboBoxText (box, label);
}

// PopupMenu styling to match tinted option/phase menus
void FieldLNF::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    // Reset tint index at start of a new menu paint
    popupPaintIndex = 0;
    auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xFF2C2F35), r.getTopLeft(), juce::Colour (0xFF24272B), r.getBottomRight(), false));
    g.fillRect (r);
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawRoundedRectangle (r.reduced (1.0f), 5.0f, 1.0f);
}

void FieldLNF::drawPopupMenuSeparator (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    auto r = area.toFloat().reduced (10.0f, 0.0f);
    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.fillRect (juce::Rectangle<float> (r.getX(), r.getCentreY() - 0.5f, r.getWidth(), 1.0f));
}

void FieldLNF::drawPopupMenuSectionHeader (juce::Graphics& g, const juce::Rectangle<int>& area,
                                           const juce::String& sectionName)
{
    auto r = area.toFloat().reduced (8.0f, 4.0f);
    g.setColour (juce::Colours::white.withAlpha (0.60f));
    g.setFont (juce::Font (juce::FontOptions (12.5f)).withExtraKerningFactor (0.02f).boldened());
    g.drawFittedText (sectionName.toUpperCase(), r.toNearestInt(), juce::Justification::centredLeft, 1);
}

void FieldLNF::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                                  bool isSeparator, bool /*isActive*/, bool isHighlighted, bool isTicked,
                                  bool /*hasSubMenu*/, const juce::String& text, const juce::String& shortcutKeyText,
                                  const juce::Drawable* /*icon*/, const juce::Colour* textColour)
{
    if (isSeparator) { drawPopupMenuSeparator (g, area); return; }

    auto r = area.toFloat().reduced (4.0f, 2.0f);

    // Pick per-item tint if provided; otherwise theme accent
    juce::Colour tint = theme.accent;
    if (popupPaintIndex >= 0 && popupPaintIndex < popupItemTints.size())
        tint = popupItemTints.getReference (popupPaintIndex);
    if (isHighlighted || isTicked)
    {
        g.setColour (tint.withAlpha (isHighlighted ? 0.90f : 0.65f));
        g.fillRoundedRectangle (r, 4.0f);
        g.setColour (juce::Colours::white.withAlpha (0.10f));
        g.drawRoundedRectangle (r, 4.0f, 1.0f);
    }

    // Advance tint index for next item
    ++popupPaintIndex;

    auto ta = r.reduced (8.0f, 0.0f);
    g.setColour (textColour ? *textColour : theme.text.withAlpha (0.95f));
    {
        auto up = text.toUpperCase();
        g.setFont (juce::Font (juce::FontOptions (14.0f)));
        g.drawFittedText (up, ta.toNearestInt(), juce::Justification::centredLeft, 1);
    }

    if (shortcutKeyText.isNotEmpty())
    {
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.setFont (juce::Font (juce::FontOptions (13.0f)));
        auto rt = ta.removeFromRight (60).toNearestInt();
        g.drawFittedText (shortcutKeyText, rt, juce::Justification::centredRight, 1);
    }
}

void FieldLNF::getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                          int& idealWidth, int& idealHeight)
{
    // Enforce consistent compact spacing matching phase/options menus
    const int h = juce::jmax (standardMenuItemHeight, 22);
    idealHeight = isSeparator ? 8 : h;
    auto f = juce::Font (juce::Font (juce::FontOptions (14.0f)));
    idealWidth = juce::jmax (160, (int) std::ceil (f.getStringWidthFloat (text) + 24.0f));
}

void FieldLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                 float sliderPosProportional, float rotaryStartAngle,
                                 float rotaryEndAngle, juce::Slider& slider)
{
    auto rawBounds = juce::Rectangle<float> (x, y, w, h);
    // Guard against degenerate bounds that can cause stroker issues
    if (rawBounds.getWidth() <= 2.0f || rawBounds.getHeight() <= 2.0f)
        return;
    auto bounds = rawBounds.reduced (6.0f);

    // Uniform hover/active raise effect
    if (slider.isMouseOverOrDragging() || slider.isMouseButtonDown())
        bounds = bounds.expanded (2.0f);

    const auto radius = juce::jmax (1.0f, juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f);
    const auto centre = bounds.getCentre();

    juce::Path body; body.addEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    g.setColour (theme.panel.brighter (0.08f));
    g.fillPath (body);

    juce::ColourGradient grad (theme.hl.withAlpha (0.7f), centre.x - radius * 0.5f, centre.y - radius * 0.6f,
                               theme.sh.withAlpha (0.7f), centre.x + radius * 0.6f, centre.y + radius * 0.7f, false);
    g.setGradientFill (grad);
    g.fillPath (body);

    const float trackThickness = juce::jmax (1.0f, radius * 0.18f);

    // Background ring
    juce::Path ring;
    ring.addCentredArc (centre.x, centre.y, radius * 0.86f, radius * 0.86f, 0.0f,
                        rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (theme.base.darker (0.2f));
    g.strokePath (ring, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value arc, with warning segment for S>1.25 if provided via slider property "S_value"
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const float ringR = radius * 0.86f;
    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, ringR, ringR, 0.0f, rotaryStartAngle, angle, true);
    // Default arc in accent
    g.setColour (theme.accent.withAlpha (0.9f));
    g.strokePath (valueArc, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // If this is the Shape knob and S > 1.25, overlay the excess (1.25..S) in light yellow
    if (auto* prop = slider.getProperties().getVarPointer ("S_value"))
    {
        const double Sval = (double) *prop;
        if (Sval > 1.25)
        {
            const float pWarn     = (float) juce::jlimit (0.0, 1.0, slider.valueToProportionOfLength (1.25f));
            const float startWarn = rotaryStartAngle + pWarn * (rotaryEndAngle - rotaryStartAngle);
            const float endWarn   = rotaryStartAngle + juce::jlimit (0.0f, 1.0f, (float) sliderPosProportional) * (rotaryEndAngle - rotaryStartAngle);
            if (endWarn > startWarn)
            {
                juce::Path warnArc;
                warnArc.addCentredArc (centre.x, centre.y, ringR, ringR, 0.0f, startWarn, endWarn, true);
                g.setColour (juce::Colour (0xFFFFF59D).withAlpha (0.9f)); // light yellow
                g.strokePath (warnArc, juce::PathStrokeType (trackThickness * 0.85f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }
    }

    // Tick marks at quarter points across whatever arc span we're given
    const float tickRadius = radius * 0.86f;
    const float tickSize   = radius * 0.06f;
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

    // Optional muted overlay ring (match duck param greyed style)
    if ((bool) slider.getProperties().getWithDefault ("muted", false))
    {
        g.setColour (theme.panel.withAlpha (0.35f));
        g.fillEllipse (bounds);
        g.setColour (theme.textMuted.withAlpha (0.85f));
        g.drawEllipse (bounds, 1.5f);
    }

    // Knob label in the center (unless suppressed)
    const bool suppress = (bool) slider.getProperties().getWithDefault ("suppressNameLabel", false);
    if (! suppress)
    {
        const juce::String knobName = slider.getName();
        if (knobName.isNotEmpty())
            drawKnobLabel (g, bounds, knobName);
    }
}

void FieldLNF::drawKnobLabel (juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text)
{
    g.setColour (theme.text);
    // Increase font size for better visibility
    const float fontSize = juce::jmax (10.0f, bounds.getHeight() * 0.18f);
    g.setFont (juce::Font (juce::FontOptions (fontSize).withStyle ("Bold")));
    g.drawText (text, bounds.toNearestInt(), juce::Justification::centred);
}

void FieldLNF::drawTabPill (juce::Graphics& g, juce::Rectangle<float> r, bool active) const
{
    auto rr = r.reduced (1.0f);
    const float rad = 9.0f;

    // Base similar to combo/pane pull-down; static border when active
    if (active)
    {
        // Active pill uses a slightly lifted fill and static accent border
        juce::Colour top = theme.panel.brighter (0.10f);
        juce::Colour bot = theme.panel.darker   (0.08f);
        juce::ColourGradient fill (top, rr.getX(), rr.getY(), bot, rr.getX(), rr.getBottom(), false);
        g.setGradientFill (fill);
        g.fillRoundedRectangle (rr, rad);

        // Static accent border
        auto accent = theme.accent.withAlpha (0.95f);
        juce::Path border; border.addRoundedRectangle (rr, rad);
        g.setColour (accent.darker (0.35f));
        g.strokePath (border, juce::PathStrokeType (1.8f));
    }
    else
    {
        // Inactive: solid panel and faint border
        g.setColour (theme.panel);
        g.fillRoundedRectangle (rr, rad);
        g.setColour (juce::Colours::white.withAlpha (0.08f));
        g.drawRoundedRectangle (rr, rad, 1.0f);
    }
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
    const bool inactive = (! slider.isEnabled()) || (bool) slider.getProperties().getWithDefault ("muted", false);
    juce::Colour bgTop = theme.panel.darker (inactive ? 0.45f : 0.30f);
    juce::Colour bgBot = theme.panel.darker (inactive ? 0.25f : 0.10f);
    juce::ColourGradient bgGrad (bgTop, bounds.getX(), bounds.getY(), bgBot, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (bgGrad);
    g.fillRoundedRectangle (bounds.reduced (1.0f), 3.0f);

    // Use JUCE’s skew-aware mapping
    const float t = juce::jlimit (0.0f, 1.0f, (float) slider.valueToProportionOfLength (slider.getValue()));

    // Progress fill (left → thumb)
    auto progress = bounds.reduced (1.0f);
    progress.setWidth (progress.getWidth() * t);

    if (progress.getWidth() > 0.0f)
    {
        juce::Colour cTop = inactive ? theme.textMuted.brighter (0.20f) : theme.accent.brighter (0.20f);
        juce::Colour cBot = inactive ? theme.textMuted                 : theme.accent;
        juce::ColourGradient progressGrad (cTop, progress.getX(), progress.getY(), cBot, progress.getX(), progress.getBottom(), false);
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
    g.setColour (inactive ? theme.textMuted.withAlpha (0.8f) : theme.accent);
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

void FieldLNF::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                 bool isMouseOver, bool isButtonDown)
{
    auto r = button.getLocalBounds().toFloat().reduced (2.0f);
    auto bg = theme.panel;
    auto sh = theme.sh;
    auto hl = theme.hl;
    auto accent = theme.accent;
    // Match inactive fill to machine dropdown panel grey
    auto grey = theme.panel;

    const bool invert = (bool) button.getProperties().getWithDefault ("invertActive", false);
    const bool active = invert ? (! button.getToggleState()) : button.getToggleState();

    juce::Colour fill = active ? accent : grey;
    if (isButtonDown) fill = fill.darker (0.25f);
    else if (isMouseOver) fill = fill.brighter (0.10f);

    // Fill square/rounded rect
    const float cr = 4.0f;
    g.setColour (fill);
    g.fillRoundedRectangle (r, cr);

    // Border contrasts better when active (darker tone), otherwise panel shadow
    g.setColour (active ? fill.darker (0.35f) : sh);
    g.drawRoundedRectangle (r, cr, 1.5f);

    // Icon rendering (iconOnly style via property 'iconType')
    int iconInt = (int) button.getProperties().getWithDefault ("iconType", -1);
    if (iconInt >= 0)
    {
        auto inner = r.reduced (4.0f);
        juce::Colour iconCol = active ? accent : theme.text.withAlpha (0.75f);
        // Shadow pass for weight
        g.setColour (juce::Colours::black.withAlpha (0.18f));
        IconSystem::drawIcon (g, (IconSystem::IconType) iconInt, inner.translated (0.7f, 1.0f), iconCol);
        // Main icon
        g.setColour (iconCol);
        IconSystem::drawIcon (g, (IconSystem::IconType) iconInt, inner, iconCol);
    }
}

void FieldLNF::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                     const juce::Colour& /*backgroundColour*/,
                                     bool isMouseOver, bool isButtonDown)
{
    auto r = button.getLocalBounds().toFloat().reduced (2.0f);
    auto accent = theme.accent;
    auto panel  = theme.panel;

    // Identify special buttons by text; fallback to default look otherwise
    juce::String txt = button.getButtonText().trim();

    bool isLearn = txt.equalsIgnoreCase ("Learn");
    bool isStop  = txt == juce::String::fromUTF8 ("\u25A0") || txt.equalsIgnoreCase ("Stop");
    bool isApply = txt.equalsIgnoreCase ("Apply");

    juce::Colour fill = panel;

    if (isLearn)
    {
        // Learn: inactive by default; when active (property) it subtly blinks and shows countdown on the right
        const bool learnActive = (bool) button.getProperties().getWithDefault ("learn_active", false);
        const double secsLeft  = (double) button.getProperties().getWithDefault ("countdown_secs", 0.0);
        const bool showActive  = learnActive || secsLeft > 0.0;

        if (showActive)
        {
            // Subtle blink of the Machine tab green fill (theme.eq.bass)
            const double t = juce::Time::getMillisecondCounterHiRes() * 0.001;
            const float blink = 0.88f + 0.12f * (0.5f * (1.0f + std::sin (float (t * 2.0 * juce::MathConstants<double>::pi * 0.85))));
            fill = theme.eq.bass.withAlpha (blink);
        }
        else
        {
            // Inactive by default; slight lift on hover
            fill = isMouseOver ? panel.brighter (0.06f) : panel;
            if (isButtonDown) fill = fill.darker (0.12f);
        }
    }
    else if (isStop)
    {
        // Stop: hover accent state, otherwise panel
        fill = isMouseOver ? accent.withAlpha (0.95f) : panel;
        if (isButtonDown) fill = fill.darker (0.12f);
    }
    else if (isApply)
    {
        // Apply button: grey when not chosen/armed; accent when armed
        const bool armed = button.getToggleState() || (bool) button.getProperties().getWithDefault ("apply_active", false);
        const bool disabled = ! button.isEnabled();
        if (disabled || ! armed)
        {
            // Inactive/disabled: render like default panel button
            fill = isMouseOver ? panel.brighter (0.06f) : panel;
            if (isButtonDown) fill = fill.darker (0.12f);
        }
        else
        {
            // Armed/active: solid accent with hover/press variation
            fill = accent;
            if (isMouseOver) fill = fill.brighter (0.10f);
            if (isButtonDown) fill = fill.darker (0.20f);
        }
    }
    else
    {
        // Default
        fill = isMouseOver ? panel.brighter (0.06f) : panel;
        if (isButtonDown) fill = fill.darker (0.12f);
    }

    const float cr = 6.0f;
    g.setColour (fill);
    g.fillRoundedRectangle (r, cr);

    // Border
    g.setColour (fill.darker (0.35f));
    g.drawRoundedRectangle (r, cr, 1.5f);

    // Extra: countdown indicator for Learn
    if (button.getButtonText().trim().equalsIgnoreCase ("Learn"))
    {
        const double secsLeft  = (double) button.getProperties().getWithDefault ("countdown_secs", 0.0);
        if (secsLeft > 0.0)
        {
            auto inner = r.reduced (6.0f, 4.0f);
            // Right-side small badge
            const float badgeW = 28.0f;
            juce::Rectangle<float> badge (inner.getRight() - badgeW, inner.getY(), badgeW, inner.getHeight());
            // Background for readability
            g.setColour (theme.base.withAlpha (0.55f));
            g.fillRoundedRectangle (badge, 4.0f);
            // Text (integer seconds)
            const int s = juce::jmax (0, (int) std::ceil (secsLeft));
            g.setColour (theme.text);
            g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
            g.drawFittedText (juce::String (s) + "s", badge.toNearestInt(), juce::Justification::centred, 1);
        }
    }
}

// Upgraded Metallic Rendering System
void FieldLNF::paintMetal (juce::Graphics& g, const juce::Rectangle<float>& r,
                           const FieldTheme::MetalStops& m, float corner)
{
    // Base gradient with softer break points
    juce::ColourGradient base(m.top, r.getCentreX(), r.getY(),
                              m.bottom, r.getCentreX(), r.getBottom(), false);
    // Add intermediate stops for smoother transitions
    base.addColour(0.3f, m.top.interpolatedWith(m.bottom, 0.3f));
    base.addColour(0.7f, m.top.interpolatedWith(m.bottom, 0.7f));
    g.setGradientFill(base);
    g.fillRoundedRectangle(r, corner);

    // Optional tint (as overlay)
    if (m.tintAlpha > 0.0f) {
        g.setColour(m.tint.withAlpha(m.tintAlpha));
        g.fillRoundedRectangle(r, corner);
    }

    // Sheen band (top third)
    auto sheenH = juce::jlimit(10.0f, 24.0f, r.getHeight() * 0.14f);
    auto sheen  = r.withHeight(sheenH).withY(r.getY() + r.getHeight() * 0.28f);
    juce::Colour cSheen = juce::Colours::white.withAlpha(0.10f); // 10%
    g.setGradientFill(juce::ColourGradient(cSheen, sheen.getX(), sheen.getY(),
                                           juce::Colours::transparentWhite, sheen.getX(), sheen.getBottom(), false));
    g.fillRoundedRectangle(sheen, corner);

    // Static brushed lines (no continuous painting)
    g.setColour(juce::Colours::white.withAlpha(0.02f));
    g.fillRoundedRectangle(r.reduced(2.0f), corner);

    // Grain
    g.setColour(juce::Colours::black.withAlpha(0.045f));
    g.fillRoundedRectangle(r, corner);

    // Vignette
    juce::Path p; p.addRoundedRectangle(r, corner);
    g.setColour(juce::Colours::black.withAlpha(0.14f));
    g.strokePath(p, juce::PathStrokeType(2.0f));
}

// Phase-Specific Metallic System (Deep Cobalt Interference)
void FieldLNF::paintPhaseMetal (juce::Graphics& g, const juce::Rectangle<float>& r, 
                                const FieldLNF::PhaseMetal& m, float corner, float dpi)
{
    // Base gradient with softer break points
    juce::ColourGradient base(m.top, r.getCentreX(), r.getY(),
                              m.bottom, r.getCentreX(), r.getBottom(), false);
    // Add intermediate stops for smoother transitions
    base.addColour(0.3f, m.top.interpolatedWith(m.bottom, 0.3f));
    base.addColour(0.7f, m.top.interpolatedWith(m.bottom, 0.7f));
    g.setGradientFill(base);
    g.fillRoundedRectangle(r, corner);

    // Airy tint overlay
    if (m.airyAlpha > 0.0f) {
        g.setColour(m.airyTint.withAlpha(m.airyAlpha));
        g.fillRoundedRectangle(r, corner);
    }

    // Sheen band (upper third)
    const float sheenH = juce::jlimit (10.0f, 24.0f, r.getHeight() * 0.14f);
    auto sheen = r.withHeight(sheenH).withY (r.getY() + r.getHeight() * 0.28f);
    juce::Colour cSheen = juce::Colours::white.withAlpha (m.sheenAlpha);
    g.setGradientFill (juce::ColourGradient (cSheen, sheen.getX(), sheen.getY(),
                                             juce::Colours::transparentWhite, sheen.getX(), sheen.getBottom(), false));
    g.fillRoundedRectangle (sheen, corner);

    // Static brushed lines (no continuous painting)
    g.setColour (juce::Colours::white.withAlpha (0.02f));
    g.fillRoundedRectangle (r.reduced (2.0f), corner);

    // Static interference motif (no continuous painting)
    // Use a simple static pattern instead of animated lines
    g.setColour (juce::Colour (0xFF8FB4E6).withAlpha (0.03f));
    g.fillRoundedRectangle (r.reduced (2.0f), corner);

    // Bottom multiply (depth)
    g.setColour (m.bottomMul.withAlpha (m.bottomMulAlpha));
    g.fillRoundedRectangle (r.withY (r.getY() + r.getHeight() * 0.75f), corner);

    // Borders
    juce::Path outline; outline.addRoundedRectangle (r, corner);
    g.setColour (juce::Colour (0xFF202226));
    g.strokePath (outline, juce::PathStrokeType (1.0f * dpi));

    g.setColour (juce::Colour (0xFFD7E6FF).withAlpha (0.10f));
    juce::Path innerOutline; innerOutline.addRoundedRectangle (r.reduced (1.0f), corner);
    g.strokePath (innerOutline, juce::PathStrokeType (1.0f * dpi));
}
