#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

namespace Layout {
    // global scale helper (uses your scaleFactor)
    inline int dp(float px, float scale) { return juce::roundToInt(px * scale); }

    // rhythm
    constexpr int PAD   = 12;   // outer padding
    constexpr int GAP   = 10;   // gaps between items
    // constexpr int RAD   = 8;    // corner radius (unused)

    // knob sizes
    constexpr int KNOB   = 70;    // standard knob (volume/EQ)
    constexpr int KNOB_L = 150;   // legacy large knob
    constexpr int KNOB_PAN = 180;   // pan knob size (larger)
    constexpr int KNOB_SPACE = 100; // space knob size (larger)
    constexpr int MICRO_W = 60;  // small freq slider width
    constexpr int MICRO_H = 20;  // small freq slider height

    // breakpoints (for wrapping)
    constexpr int BP_WIDE   = 1200;
}

//==============================================================================

ToggleSwitch::ToggleSwitch()
{
    setLabels ("STEREO", "SPLIT");
    sliderValue.setCurrentAndTargetValue (0.0f);
    sliderValue.reset (0.0f, 0.02f); // Even slower toggle animation
}

void ToggleSwitch::setToggleState (bool shouldBeOn, juce::NotificationType notification)
{
    if (isOn != shouldBeOn)
    {
        isOn = shouldBeOn;
        sliderValue.setTargetValue (isOn ? 1.0f : 0.0f);
        
        if (notification == juce::sendNotification && onToggleChange)
            onToggleChange (isOn);
    }
}

void ToggleSwitch::setLabels (const juce::String& offLabel, const juce::String& onLabel)
{
    offText = offLabel;
    onText = onLabel;
    repaint();
}

void ToggleSwitch::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const float cornerRadius = bounds.getHeight() * 0.5f;
    const float sliderRadius = bounds.getHeight() * 0.45f;
    
    // Update slider animation
    sliderValue.setCurrentAndTargetValue (isOn ? 1.0f : 0.0f);
    
    // Draw background track with enhanced styling
    g.setColour (juce::Colour (0xFF1A1D25));
    g.fillRoundedRectangle (bounds, cornerRadius);
    
    // Draw track border with stronger contrast
    g.setColour (juce::Colour (0xFF4A4D55));
    g.drawRoundedRectangle (bounds, cornerRadius, 2.0f);
    
    // Border hover system - color based on mode
    auto mouseScreenPos = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition();
    auto myScreenPos = getScreenPosition().toFloat();
    auto localMouse = mouseScreenPos - myScreenPos;
    if (bounds.contains(localMouse) || hoverActive)
    {
        // Match bypass button blue
        juce::Colour borderColor = juce::Colour (0xFF2196F3);
        g.setColour (borderColor);
        g.drawRoundedRectangle (bounds, cornerRadius, 1.0f);
    }
    
    // Calculate slider position: much smaller travel distance for compact toggle
    const float leftCenterX  = bounds.getX() + bounds.getWidth() * 0.3f;
    const float rightCenterX = bounds.getX() + bounds.getWidth() * 0.7f;
    const float sliderX = juce::jmap (sliderValue.getCurrentValue(), 0.0f, 1.0f,
                                      leftCenterX  - sliderRadius,
                                      rightCenterX - sliderRadius);
    const float sliderY = bounds.getCentreY() - sliderRadius;
    
    // Draw slider with enhanced styling
    juce::Rectangle<float> sliderRect (sliderX, sliderY, sliderRadius * 2.0f, sliderRadius * 2.0f);
    
    // Slider shadow
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillEllipse (sliderRect.translated (2.0f, 2.0f));
    
    // Slider fill: Stereo = blue (match bypass), Split = grey
    g.setColour (isOn ? juce::Colour (0xFF7A7D85) : juce::Colour (0xFF2196F3));
    g.fillEllipse (sliderRect);
    
    // Slider border and Split marker (vertical line when in split/grey state)
    g.setColour (juce::Colour (0xFF9A9DA5));
    g.drawEllipse (sliderRect, 2.0f);
    if (isOn) // split mode (grey)
    {
        g.setColour(juce::Colour (0xFFB0B3B8));
        float cx = sliderRect.getCentreX();
        g.drawLine(cx, sliderRect.getY() + 4.0f, cx, sliderRect.getBottom() - 4.0f, 1.5f);
    }
    

}

void ToggleSwitch::mouseDown (const juce::MouseEvent&)
{
    isMouseDown = true;
    repaint();
}

void ToggleSwitch::mouseUp (const juce::MouseEvent&)
{
    if (isMouseDown)
    {
        setToggleState (!isOn, juce::sendNotification);
        isMouseDown = false;
        repaint();
    }
}

//==============================================================================

ControlContainer::ControlContainer()
{
    containerTitle = "Container";
}

void ControlContainer::setTitle(const juce::String& title)
{
    containerTitle = title;
    repaint();
}

void ControlContainer::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    float radius = 8.0f; // Reduced radius for less curved appearance

    // Grey woodgrain background
    auto woodBounds = r.reduced (3.0f);
    
    // Header base color (darker grey)
    g.setColour (juce::Colour (0xFF2C2F36));
    g.fillRoundedRectangle (woodBounds, radius);
    
    // Removed woodgrain texture overlays for a flat header treatment
    
    // Enhanced shadows for depth - consistent top-left light source
    juce::DropShadow deepShadow (juce::Colour (0xFF1A1C20).withAlpha (0.6f), 20, { -2, -2 });
    juce::DropShadow lightShadow (juce::Colour (0xFF60646C).withAlpha (0.4f), 8, { -1, -1 });
    auto shadowRect = r.reduced (3.0f).getSmallestIntegerContainer();
    deepShadow.drawForRectangle (g, shadowRect);
    lightShadow.drawForRectangle (g, shadowRect);
    
    // subtle inner rim
    g.setColour (juce::Colour (0xFF2A2C30).withAlpha (0.3f));
    g.drawRoundedRectangle (r.reduced (4.0f), radius - 1.0f, 1.0f);

    // Draw borders only if showBorder is true
    if (showBorder)
    {
        // thin border with hover glow - color based on mode
        auto borderBounds = r.reduced (3.0f);
        juce::Colour borderColor = juce::Colour (0xFF5AA9E6); // Default blue
        if (auto* lnf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
            borderColor = lnf->theme.accent;
        }
        g.setColour (borderColor);
        g.drawRoundedRectangle (borderBounds, radius, 1.0f);
        
        // Hover state: thicker glowing border even when child controls are hovered, with delayed fade
        auto mouseScreenPos = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition();
        auto myScreenPos = getScreenPosition().toFloat();
        auto localMouse = mouseScreenPos - myScreenPos;
        if (borderBounds.contains (localMouse) || hoverActive)
        {
            g.setColour (borderColor.withAlpha(0.5f));
            g.drawRoundedRectangle (borderBounds.expanded(2.0f), radius, 2.0f); // Reduced thickness to match other borders
        }
    }
    
    // Draw title with icon only if showBorder is true
    if (showBorder)
    {
        auto titleArea = r.reduced (10.0f).removeFromTop (25);
    
    // Draw icon based on title
    IconSystem::IconType iconType = IconSystem::Speaker; // default
    if (containerTitle == "FIELD") {
        iconType = IconSystem::Space;
    } else if (containerTitle == "VOLUME") {
        iconType = IconSystem::Speaker;
    } else if (containerTitle == "EQ") {
        iconType = IconSystem::Tilt;
    }
    
    // Get title color based on mode
    juce::Colour titleColor = juce::Colour(0xFFF0F2F5); // Default white
    if (auto* lnf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
        titleColor = lnf->theme.text;
    }
    
    // Draw icon
    IconSystem::drawIcon(g, iconType, titleArea.removeFromLeft(20).reduced(2.0f), titleColor);
    
        // Draw title text
        g.setColour (titleColor);
        g.setFont (juce::Font (juce::FontOptions (14.0f).withStyle ("Bold")));
        g.drawText (containerTitle, titleArea, juce::Justification::centredLeft);
    }
}

//==============================================================================

// rowLabel function removed - not used in current layout

void XYPad::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    float radius = 8.0f; // Reduced radius for less curved appearance

    // panel bg - using new lighter colors
    g.setColour (juce::Colour (0xFF454951));
    g.fillRoundedRectangle (r.reduced (3.0f), radius);
    
    // Enhanced shadows for depth - consistent top-left light source
    juce::DropShadow deepShadow (juce::Colour (0xFF1A1C20).withAlpha (0.6f), 20, { -2, -2 });
    juce::DropShadow lightShadow (juce::Colour (0xFF60646C).withAlpha (0.4f), 8, { -1, -1 });
    auto shadowRect = r.reduced (3.0f).getSmallestIntegerContainer();
    deepShadow.drawForRectangle (g, shadowRect);
    lightShadow.drawForRectangle (g, shadowRect);
    
    // subtle inner rim
    g.setColour (juce::Colour (0xFF2A2C30).withAlpha (0.3f));
    g.drawRoundedRectangle (r.reduced (4.0f), radius - 1.0f, 1.0f);

    // Container-like border hover for XYPad - larger border system
    auto padBorder = r.reduced (2.0f);
    juce::Colour borderColor = isGreenMode ? juce::Colour (0xFF5AA95A) : juce::Colour (0xFF5AA9E6);
    g.setColour (borderColor);
    g.drawRoundedRectangle (padBorder, radius, 2.0f);
    auto mouseScreenPos = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition();
    auto myScreenPos = getScreenPosition().toFloat();
    auto localMouse = mouseScreenPos - myScreenPos;
    if (padBorder.contains (localMouse) || hoverActive)
    {
        // Soft glow beneath the border: layered halos with increasing expansion and soft corner radius
        const int glowLayers = 6;
        for (int i = 1; i <= glowLayers; ++i)
        {
            const float t = (float) i / (float) glowLayers; // 0..1
            const float expand = 3.0f + t * 9.0f;            // 3px .. 12px
            const float alpha  = (1.0f - t) * (isGreenMode ? 0.25f : 0.22f);
            g.setColour (borderColor.withAlpha (alpha));
            g.drawRoundedRectangle (padBorder.expanded (expand), radius + expand * 0.4f, 2.0f);
        }

        // Re-draw main border to keep it crisp on top of the glow
        g.setColour (borderColor);
        g.drawRoundedRectangle (padBorder, radius, 2.0f);
    }

    // Draw waveform background (transparent) - always draw, will generate test data if needed
    drawWaveformBackground (g, r.reduced (40.0f));

    // Draw subgrid with numerical markers
    auto padBounds = r.reduced (40.0f);
    drawGrid (g, padBounds); // More space for labels

    // Draw frequency regions with transparent colors
    drawFrequencyRegions(g, padBounds);
    
    // Re-apply hover glow on top of inner content so growth is clearly visible
    {
        auto padBorder2 = r.reduced (2.0f);
        juce::Colour borderColor2 = isGreenMode ? juce::Colour (0xFF5AA95A) : juce::Colour (0xFF5AA9E6);
        auto mouseScreenPos2 = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition();
        auto myScreenPos2 = getScreenPosition().toFloat();
        auto localMouse2 = mouseScreenPos2 - myScreenPos2;
        if (padBorder2.contains (localMouse2) || hoverActive)
        {
            const int glowLayers = 10;
            for (int i = 1; i <= glowLayers; ++i)
            {
                const float t = (float) i / (float) glowLayers;
                const float expand = 4.0f + t * 12.0f;
                const float alpha  = (1.0f - t) * (isGreenMode ? 0.28f : 0.25f);
                g.setColour (borderColor2.withAlpha (alpha));
                g.drawRoundedRectangle (padBorder2.expanded (expand), radius + expand * 0.4f, 2.0f + t * 4.0f);
            }
            // Crisp border on top
            g.setColour (borderColor2);
            g.drawRoundedRectangle (padBorder2, radius, 2.0f);
        }
    }
    
    // Mono indicator: vertical line with gradient to the left (capped at 300Hz)
    if (monoHzValue > 0.0f)
    {
        const float minHz = 20.0f;
        const float maxHz = 300.0f; // Capped at 300Hz
        float hzClamped = juce::jlimit (minHz, maxHz, monoHzValue);
        
        // Calculate position based on the full frequency range (20Hz to 20kHz)
        // but only allow the mono indicator to go up to 300Hz
        const float fullMaxHz = 20000.0f;
        float t = std::log(hzClamped / minHz) / std::log(fullMaxHz / minHz); // 0..1 within full range
        float x = padBounds.getX() + t * padBounds.getWidth();
        
        juce::Colour accent = isGreenMode ? juce::Colour (0xFF5AA95A) : juce::Colour (0xFF5AA9E6);
        juce::ColourGradient grad (accent.withAlpha (0.45f), x, padBounds.getY(),
                                   accent.withAlpha (0.0f), padBounds.getX(), padBounds.getY(), false);
        g.setGradientFill (grad);
        g.fillRect (juce::Rectangle<float> (padBounds.getX(), padBounds.getY(), x - padBounds.getX(), padBounds.getHeight()));
        g.setColour (accent.withAlpha (0.85f));
        g.drawVerticalLine ((int) x, padBounds.getY(), padBounds.getBottom());
    }
    
    // Space indicator: simple transparent circles that grow/shrink with space depth
    if (spaceValue > 0.0f)
    {
        // Get algorithm-specific colors
        juce::Colour spaceColour;
        
        if (isGreenMode) {
            // Green monochromatic palette - distinct tones per algorithm
            switch (spaceAlgorithm) {
                case 0: // Inner
                    spaceColour = juce::Colour(0x605AA95A); // Inner - Green
                    break;
                case 1: // Outer
                    spaceColour = juce::Colour(0x607ACF95); // Outer - Mint green (distinct)
                    break;
                case 2: // Deep
                    spaceColour = juce::Colour(0x604C8F4C); // Deep - Darker green
                    break;
                default:
                    spaceColour = juce::Colour(0x605AA95A);
                    break;
            }
        } else {
            // Standard color palette (match switch colors)
            switch (spaceAlgorithm) {
                case 0: // Inner
                    spaceColour = juce::Colour(0x405AA9E6); // Inner - Blue
                    break;
                case 1: // Outer
                    spaceColour = juce::Colour(0x402EC4B6); // Outer - Teal (distinct)
                    break;
                case 2: // Deep
                    spaceColour = juce::Colour(0x402A1B3D); // Deep - Dark blue/purple
                    break;
                default:
                    spaceColour = juce::Colour(0x405AA9E6);
                    break;
            }
        }
        
        if (isSplitMode)
        {
            // Split mode: individual circles for each ball
            // Left ball space circle
            float leftX = padBounds.getX() + leftPt * padBounds.getWidth();
            float leftY = padBounds.getY() + (1.0f - pt.second) * padBounds.getHeight();
            float leftCircleSize = juce::jmap(spaceValue, 0.0f, 1.0f, 10.0f, 60.0f);
            float leftOpacity = juce::jmap(spaceValue, 0.0f, 1.0f, 0.1f, 0.3f);
            
            g.setColour(spaceColour.withAlpha(leftOpacity));
            g.fillEllipse(leftX - leftCircleSize, leftY - leftCircleSize, leftCircleSize * 2.0f, leftCircleSize * 2.0f);
            
            // Right ball space circle
            float rightX = padBounds.getX() + rightPt * padBounds.getWidth();
            float rightY = padBounds.getY() + (1.0f - pt.second) * padBounds.getHeight();
            float rightCircleSize = juce::jmap(spaceValue, 0.0f, 1.0f, 10.0f, 60.0f);
            float rightOpacity = juce::jmap(spaceValue, 0.0f, 1.0f, 0.1f, 0.3f);
            
            g.setColour(spaceColour.withAlpha(rightOpacity));
            g.fillEllipse(rightX - rightCircleSize, rightY - rightCircleSize, rightCircleSize * 2.0f, rightCircleSize * 2.0f);
        }
        else
        {
            // Stereo mode: single circle at ball position
            float ballX = padBounds.getX() + pt.first * padBounds.getWidth();
            float ballY = padBounds.getY() + (1.0f - pt.second) * padBounds.getHeight();
            float circleSize = juce::jmap(spaceValue, 0.0f, 1.0f, 10.0f, 60.0f);
            float opacity = juce::jmap(spaceValue, 0.0f, 1.0f, 0.1f, 0.3f);
            
            g.setColour(spaceColour.withAlpha(opacity));
            g.fillEllipse(ballX - circleSize, ballY - circleSize, circleSize * 2.0f, circleSize * 2.0f);
        }
    }

    // Center crosshair
    g.setColour (juce::Colour (0xFFB8BDC7).withAlpha (0.4f));
    g.drawLine (r.getCentreX(), r.getY()+40, r.getCentreX(), r.getBottom()-40, 1.5f);
    g.drawLine (r.getX()+40, r.getCentreY(), r.getRight()-40, r.getCentreY(), 1.5f);

    // Draw balls
    drawBalls (g, padBounds);
}

void XYPad::drag (const juce::MouseEvent& e)
{
    auto r = getLocalBounds().toFloat().reduced (40.0f);
    const auto pos = e.position;
    
    if (isSplitMode)
    {
        float x01 = juce::jlimit (0.0f, 1.0f, (pos.x - r.getX()) / r.getWidth());
        float y01 = juce::jlimit (0.0f, 1.0f, 1.0f - (pos.y - r.getY()) / r.getHeight());
        
        if (snapEnabled)
        {
            // Snap X to 5-unit grid (20 divisions), Y to 10% grid (10 divisions)
            x01 = std::round(x01 * 20.0f) / 20.0f;
            y01 = std::round(y01 * 10.0f) / 10.0f;
        }
        
        if (isLinked) {
            // When linked, move both balls together
            leftPt = x01;
            rightPt = x01;
            pt.second = y01;
            repaint();
            if (onSplitChange) onSplitChange (leftPt, rightPt, y01);
        } else {
            // When not linked, determine which ball to move
            if (activeBall == 0) {
                activeBall = getBallAtPosition (pos.toFloat(), r);
            }
            
            if (activeBall == 1) {
                leftPt = x01;
                pt.second = y01;
                repaint();
                if (onBallChange) onBallChange (1, leftPt, y01);
            } else if (activeBall == 2) {
                rightPt = x01;
                pt.second = y01;
                repaint();
                if (onBallChange) onBallChange (2, rightPt, y01);
            }
        }
    }
    else
    {
        float x01 = juce::jlimit (0.0f, 1.0f, (pos.x - r.getX()) / r.getWidth());
        float y01 = juce::jlimit (0.0f, 1.0f, 1.0f - (pos.y - r.getY()) / r.getHeight());
        
        if (snapEnabled)
        {
            x01 = std::round(x01 * 20.0f) / 20.0f;
            y01 = std::round(y01 * 10.0f) / 10.0f;
        }
        pt = { x01, y01 };
        repaint();
        if (onChange) onChange (x01, y01);
    }
}

void XYPad::drawGrid (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour (juce::Colour (0xFFB8BDC7).withAlpha (0.2f));
    
    // Vertical grid lines (stereo placement -50..0..+50 with 5-unit increments)
    const int numDivisionsX = 20; // 5-unit steps across 100 units
    for (int i = 0; i <= numDivisionsX; ++i)
    {
        float x = bounds.getX() + (bounds.getWidth() * (static_cast<float>(i) / static_cast<float>(numDivisionsX)));
        g.drawVerticalLine (static_cast<int>(x), bounds.getY(), bounds.getBottom());
        
        // Numerical markers on bottom (50L .. 0 .. 50R) at every 5 units
        g.setColour (juce::Colour (0xFFB8BDC7).withAlpha (0.6f));
        g.setFont (juce::Font (juce::FontOptions (12.0f)));
        int panUnits = (i - (numDivisionsX / 2)) * 5; // -50 .. 0 .. +50
        juce::String label;
        if (panUnits < 0)
            label = juce::String (std::abs(panUnits)) + "L";
        else if (panUnits > 0)
            label = juce::String (panUnits) + "R";
        else
            label = "0";
        g.drawText (label, static_cast<int>(x - 15), static_cast<int>(bounds.getBottom() + 5), 30, 15, juce::Justification::centred);
        g.setColour (juce::Colour (0xFFB8BDC7).withAlpha (0.2f));
    }
    
    // Horizontal grid lines (depth 0-100%)
    for (int i = 1; i <= 10; ++i)
    {
        float y = bounds.getY() + (bounds.getHeight() * i / 10.0f);
        g.drawHorizontalLine (static_cast<int>(y), bounds.getX(), bounds.getRight());
        
        // dB markers on left side (input level)
        if (i < 10) {
            int dbValue = 60 - (i * 6); // 54, 48, 42, 36, 30, 24, 18, 12, 6 dB (top to bottom)
            g.setColour (juce::Colour (0xFFB8BDC7).withAlpha (0.6f));
            g.setFont (juce::Font (juce::FontOptions (12.0f)));
            g.drawText (juce::String (dbValue) + "dB", static_cast<int>(bounds.getX() - 35), static_cast<int>(y - 7), 30, 14, juce::Justification::centredRight);
            g.setColour (juce::Colour (0xFFB8BDC7).withAlpha (0.2f));
        }
        
        // Space % markers on right side (depth)
        if (i < 10) {
            int spacePercent = i * 10; // 10, 20, 30, 40, 50, 60, 70, 80, 90% (top to bottom)
            g.setColour (juce::Colour (0xFFB8BDC7).withAlpha (0.6f));
            g.setFont (juce::Font (juce::FontOptions (12.0f)));
            g.drawText (juce::String (spacePercent) + "%", static_cast<int>(bounds.getRight() + 5), static_cast<int>(y - 7), 30, 14, juce::Justification::centredLeft);
            g.setColour (juce::Colour (0xFFB8BDC7).withAlpha (0.2f));
        }
    }
    
    // Corner labels
    g.setColour (juce::Colour (0xFFF0F2F5));
    g.setFont (juce::Font (juce::FontOptions (14.0f)));
    g.drawText ("L", static_cast<int>(bounds.getX() - 15), static_cast<int>(bounds.getY() - 20), 30, 20, juce::Justification::centred);
    g.drawText ("R", static_cast<int>(bounds.getRight() - 15), static_cast<int>(bounds.getY() - 20), 30, 20, juce::Justification::centred);
    
    // Emphasize center axes (0 lines) for better visibility
    float centerY = bounds.getCentreY();
    float centerX = bounds.getCentreX();
    g.setColour (juce::Colours::white.withAlpha (0.6f));
    g.drawLine (bounds.getX(), centerY, bounds.getRight(), centerY, 2.0f); // horizontal 0 line (thicker)
    g.drawLine (centerX, bounds.getY(), centerX, bounds.getBottom(), 2.0f); // vertical 0 line (thicker)
    
    // Update grid labels to show proper values
    // Left side: -100 to 0
    // Right side: 0 to +100
    // Center: 0
}

void XYPad::drawBalls (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Calculate visual feedback from gain and width
    float gainScale = juce::jmap (gainValue, -24.0f, 24.0f, 0.5f, 2.0f); // Scale from 0.5f to 2x
    float borderThickness = juce::jmap (widthValue, 0.0f, 10.0f, 1.0f, 8.0f); // Border from 1 to 8px (less aggressive)
    float borderPadding = juce::jmap (widthValue, 0.0f, 10.0f, 0.0f, 6.0f); // Padding from 0 to 6px
    
    if (isSplitMode)
    {
        // Check if balls are stacked (linked and in same position)
        bool ballsStacked = isLinked && std::abs(leftPt - rightPt) < 0.01f;
        
        if (ballsStacked) {
            // Draw stacked balls with visual indicator
            juce::Point<float> centerP (bounds.getX() + leftPt * bounds.getWidth(),
                                       bounds.getY() + (1.0f - pt.second) * bounds.getHeight());
            float ballSize = 10.0f * gainScale;
            
            // Calculate tilt color - match the curve colors exactly
            juce::Colour ballColor;
            if (isGreenMode) {
                // Green monochromatic palette - match curve colors exactly
                if (tiltValue > 0.0f) {
                    // Warm green (positive tilt)
                    float intensity = juce::jmap(tiltValue, 0.0f, 12.0f, 0.0f, 1.0f);
                    ballColor = juce::Colour(0xFF5AA95A).interpolatedWith(juce::Colour(0xFF8CAA00), intensity);
                } else {
                    // Cool green (negative tilt)
                    float intensity = juce::jmap(std::abs(tiltValue), 0.0f, 12.0f, 0.0f, 1.0f);
                    ballColor = juce::Colour(0xFF5AA95A).interpolatedWith(juce::Colour(0xFF68B568), intensity);
                }
            } else {
                // Standard color palette - match ball colors
                if (tiltValue > 0.0f) {
                    float intensity = juce::jmap (tiltValue, 0.0f, 12.0f, 0.0f, 1.0f);
                    float hue = juce::jmap (intensity, 0.0f, 1.0f, 0.15f, 0.0f);
                    ballColor = juce::Colour::fromHSV (hue, 0.7f, 0.7f + intensity * 0.15f, 1.0f);
                } else {
                    float intensity = juce::jmap (std::abs(tiltValue), 0.0f, 12.0f, 0.0f, 1.0f);
                    float hue = juce::jmap (intensity, 0.0f, 1.0f, 0.3f, 0.8f);
                    ballColor = juce::Colour::fromHSV (hue, 0.7f, 0.7f + intensity * 0.15f, 1.0f);
                }
            }
            
            // Draw bottom ball (slightly larger)
            g.setColour (juce::Colours::black.withAlpha (0.35f));
            g.fillEllipse (centerP.x - ballSize * 1.1f, centerP.y - ballSize * 0.6f, ballSize * 2.2f, ballSize * 1.2f);
            g.setColour (ballColor);
            g.fillEllipse (centerP.x - ballSize * 0.8f, centerP.y - ballSize * 0.8f, ballSize * 1.6f, ballSize * 1.6f);
            g.setColour (juce::Colours::white.withAlpha (0.8f));
            g.drawEllipse (centerP.x - ballSize * 1.0f - borderPadding, centerP.y - ballSize * 1.0f - borderPadding, 
                          ballSize * 2.0f + borderPadding * 2.0f, ballSize * 2.0f + borderPadding * 2.0f, borderThickness);
            
            // Draw top ball (slightly smaller and offset)
            g.setColour (juce::Colours::black.withAlpha (0.35f));
            g.fillEllipse (centerP.x - ballSize * 0.9f, centerP.y - ballSize * 1.1f, ballSize * 1.8f, ballSize * 1.2f);
            g.setColour (ballColor);
            g.fillEllipse (centerP.x - ballSize * 0.6f, centerP.y - ballSize * 1.3f, ballSize * 1.2f, ballSize * 1.2f);
            g.setColour (juce::Colours::white.withAlpha (0.8f));
            g.drawEllipse (centerP.x - ballSize * 0.8f - borderPadding, centerP.y - ballSize * 1.5f - borderPadding, 
                          ballSize * 1.6f + borderPadding * 2.0f, ballSize * 1.6f + borderPadding * 2.0f, borderThickness);
            
            // Draw linking indicator (small line between balls)
            g.setColour (juce::Colours::white.withAlpha (0.9f));
            g.drawLine (centerP.x, centerP.y - ballSize * 0.8f, centerP.x, centerP.y - ballSize * 1.3f, 2.0f);
            
            // Progressive width effect with multiple border layers
            if (widthValue > 1.0f) {
                float widthIntensity = juce::jmap (widthValue, 1.0f, 10.0f, 0.0f, 1.0f);
                g.setColour (juce::Colours::white.withAlpha (0.3f * widthIntensity));
                g.drawEllipse (centerP.x - ballSize * 1.3f - borderPadding * 1.5f, centerP.y - ballSize * 1.3f - borderPadding * 1.5f, 
                              ballSize * 2.6f + borderPadding * 3.0f, ballSize * 2.6f + borderPadding * 3.0f, borderThickness * 0.5f);
                
                if (widthValue > 5.0f) {
                    g.setColour (juce::Colours::white.withAlpha (0.2f * widthIntensity));
                    g.drawEllipse (centerP.x - ballSize * 1.5f - borderPadding * 2.0f, centerP.y - ballSize * 1.5f - borderPadding * 2.0f, 
                                  ballSize * 3.0f + borderPadding * 4.0f, ballSize * 3.0f + borderPadding * 4.0f, borderThickness * 0.3f);
                }
            }
        } else {
            // Draw separate balls
            // Draw left ball
            juce::Point<float> leftP (bounds.getX() + leftPt * bounds.getWidth(),
                                      bounds.getY() + (1.0f - pt.second) * bounds.getHeight());
            float leftSize = 10.0f * gainScale;
            g.setColour (juce::Colours::black.withAlpha (0.35f));
            g.fillEllipse (leftP.x - leftSize, leftP.y - leftSize * 0.6f, leftSize * 2, leftSize * 1.2f);
            // Calculate tilt color with more tones (reduced vibrancy) - match curve colors
            juce::Colour ballColor;
            if (isGreenMode) {
                // Green monochromatic palette - match curve colors exactly
                if (tiltValue > 0.0f) {
                    // Warm green (positive tilt)
                    float intensity = juce::jmap(tiltValue, 0.0f, 12.0f, 0.0f, 1.0f);
                    ballColor = juce::Colour(0xFF5AA95A).interpolatedWith(juce::Colour(0xFF8CAA00), intensity);
                } else {
                    // Cool green (negative tilt)
                    float intensity = juce::jmap(std::abs(tiltValue), 0.0f, 12.0f, 0.0f, 1.0f);
                    ballColor = juce::Colour(0xFF5AA95A).interpolatedWith(juce::Colour(0xFF68B568), intensity);
                }
            } else {
                // Standard color palette - match ball colors
                if (tiltValue > 0.0f) {
                    // Warm colors: yellow -> orange -> red -> magenta
                    float intensity = juce::jmap (tiltValue, 0.0f, 12.0f, 0.0f, 1.0f);
                    float hue = juce::jmap (intensity, 0.0f, 1.0f, 0.15f, 0.0f);
                    ballColor = juce::Colour::fromHSV (hue, 0.7f, 0.7f + intensity * 0.15f, 1.0f);
                } else {
                    // Cool colors: green -> cyan -> blue -> purple
                    float intensity = juce::jmap (std::abs(tiltValue), 0.0f, 12.0f, 0.0f, 1.0f);
                    float hue = juce::jmap (intensity, 0.0f, 1.0f, 0.3f, 0.8f);
                    ballColor = juce::Colour::fromHSV (hue, 0.7f, 0.7f + intensity * 0.15f, 1.0f);
                }
            }
            g.setColour (ballColor);
            g.fillEllipse (leftP.x - leftSize * 0.7f, leftP.y - leftSize * 0.7f, leftSize * 1.4f, leftSize * 1.4f);
            g.setColour (juce::Colours::white.withAlpha (0.8f));
            g.drawEllipse (leftP.x - leftSize * 0.9f - borderPadding, leftP.y - leftSize * 0.9f - borderPadding, 
                          leftSize * 1.8f + borderPadding * 2.0f, leftSize * 1.8f + borderPadding * 2.0f, borderThickness);
            
            // Progressive width effect with multiple border layers
            if (widthValue > 1.0f) {
                float widthIntensity = juce::jmap (widthValue, 1.0f, 10.0f, 0.0f, 1.0f);
                g.setColour (juce::Colours::white.withAlpha (0.3f * widthIntensity));
                g.drawEllipse (leftP.x - leftSize * 1.1f - borderPadding * 1.5f, leftP.y - leftSize * 1.1f - borderPadding * 1.5f, 
                              leftSize * 2.2f + borderPadding * 3.0f, leftSize * 2.2f + borderPadding * 3.0f, borderThickness * 0.5f);
                
                if (widthValue > 5.0f) {
                    g.setColour (juce::Colours::white.withAlpha (0.2f * widthIntensity));
                    g.drawEllipse (leftP.x - leftSize * 1.3f - borderPadding * 2.0f, leftP.y - leftSize * 1.3f - borderPadding * 2.0f, 
                                  leftSize * 2.6f + borderPadding * 4.0f, leftSize * 2.6f + borderPadding * 4.0f, borderThickness * 0.3f);
                }
            }
            
            // Draw L label on left ball
            g.setColour (juce::Colour (0xFFF0F2F5));
            g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
            g.drawText ("L", static_cast<int>(leftP.x - 8), static_cast<int>(leftP.y - 8), 16, 16, juce::Justification::centred);
            
            // Draw right ball
            juce::Point<float> rightP (bounds.getX() + rightPt * bounds.getWidth(),
                                       bounds.getY() + (1.0f - pt.second) * bounds.getHeight());
            float rightSize = 10.0f * gainScale;
            g.setColour (juce::Colours::black.withAlpha (0.35f));
            g.fillEllipse (rightP.x - rightSize, rightP.y - rightSize * 0.6f, rightSize * 2, rightSize * 1.2f);
            g.setColour (ballColor);
            g.fillEllipse (rightP.x - rightSize * 0.7f, rightP.y - rightSize * 0.7f, rightSize * 1.4f, rightSize * 1.4f);
            g.setColour (juce::Colours::white.withAlpha (0.8f));
            g.drawEllipse (rightP.x - rightSize * 0.9f - borderPadding, rightP.y - rightSize * 0.9f - borderPadding, 
                          rightSize * 1.8f + borderPadding * 2.0f, rightSize * 1.8f + borderPadding * 2.0f, borderThickness);
            
            // Progressive width effect with multiple border layers
            if (widthValue > 1.0f) {
                float widthIntensity = juce::jmap (widthValue, 1.0f, 10.0f, 0.0f, 1.0f);
                g.setColour (juce::Colours::white.withAlpha (0.3f * widthIntensity));
                g.drawEllipse (rightP.x - rightSize * 1.1f - borderPadding * 1.5f, rightP.y - rightSize * 1.1f - borderPadding * 1.5f, 
                              rightSize * 2.2f + borderPadding * 3.0f, rightSize * 2.2f + borderPadding * 3.0f, borderThickness * 0.5f);
                
                if (widthValue > 5.0f) {
                    g.setColour (juce::Colours::white.withAlpha (0.2f * widthIntensity));
                    g.drawEllipse (rightP.x - rightSize * 1.3f - borderPadding * 2.0f, rightP.y - rightSize * 1.3f - borderPadding * 2.0f, 
                                  rightSize * 2.6f + borderPadding * 4.0f, rightSize * 2.6f + borderPadding * 4.0f, borderThickness * 0.3f);
                }
            }
            
            // Draw R label on right ball
            g.setColour (juce::Colour (0xFFF0F2F5));
            g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
            g.drawText ("R", static_cast<int>(rightP.x - 8), static_cast<int>(rightP.y - 8), 16, 16, juce::Justification::centred);
        }
    }
    else
    {
        // Draw single ball
        juce::Point<float> p (bounds.getX() + pt.first * bounds.getWidth(),
                              bounds.getY() + (1.0f - pt.second) * bounds.getHeight());
        float ballSize = 10.0f * gainScale;
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillEllipse (p.x - ballSize, p.y - ballSize * 0.6f, ballSize * 2, ballSize * 1.2f);
        // Calculate tilt color for single ball with more tones (reduced vibrancy) - match curve colors
        juce::Colour ballColor;
        if (isGreenMode) {
            // Green monochromatic palette - match curve colors exactly
            if (tiltValue > 0.0f) {
                // Warm green (positive tilt)
                float intensity = juce::jmap(tiltValue, 0.0f, 12.0f, 0.0f, 1.0f);
                ballColor = juce::Colour(0xFF5AA95A).interpolatedWith(juce::Colour(0xFF8CAA00), intensity);
            } else {
                // Cool green (negative tilt)
                float intensity = juce::jmap(std::abs(tiltValue), 0.0f, 12.0f, 0.0f, 1.0f);
                ballColor = juce::Colour(0xFF5AA95A).interpolatedWith(juce::Colour(0xFF68B568), intensity);
            }
        } else {
            // Standard color palette - match ball colors
            if (tiltValue > 0.0f) {
                // Warm colors: yellow -> orange -> red -> magenta
                float intensity = juce::jmap (tiltValue, 0.0f, 12.0f, 0.0f, 1.0f);
                float hue = juce::jmap (intensity, 0.0f, 1.0f, 0.15f, 0.0f);
                ballColor = juce::Colour::fromHSV (hue, 0.7f, 0.7f + intensity * 0.15f, 1.0f);
            } else {
                // Cool colors: green -> cyan -> blue -> purple
                float intensity = juce::jmap (std::abs(tiltValue), 0.0f, 12.0f, 0.0f, 1.0f);
                float hue = juce::jmap (intensity, 0.0f, 1.0f, 0.3f, 0.8f);
                ballColor = juce::Colour::fromHSV (hue, 0.7f, 0.7f + intensity * 0.15f, 1.0f);
            }
        }
        g.setColour (ballColor);
        g.fillEllipse (p.x - ballSize * 0.7f, p.y - ballSize * 0.7f, ballSize * 1.4f, ballSize * 1.4f);
        g.setColour (juce::Colours::white.withAlpha (0.8f));
        g.drawEllipse (p.x - ballSize * 0.9f - borderPadding, p.y - ballSize * 0.9f - borderPadding, 
                      ballSize * 1.8f + borderPadding * 2.0f, ballSize * 1.8f + borderPadding * 2.0f, borderThickness);
        
        // Progressive width effect with multiple border layers
        if (widthValue > 1.0f) {
            float widthIntensity = juce::jmap (widthValue, 1.0f, 10.0f, 0.0f, 1.0f);
            g.setColour (juce::Colours::white.withAlpha (0.3f * widthIntensity));
            g.drawEllipse (p.x - ballSize * 1.1f - borderPadding * 1.5f, p.y - ballSize * 1.1f - borderPadding * 1.5f, 
                          ballSize * 2.2f + borderPadding * 3.0f, ballSize * 2.2f + borderPadding * 3.0f, borderThickness * 0.5f);
            
            if (widthValue > 5.0f) {
                g.setColour (juce::Colours::white.withAlpha (0.2f * widthIntensity));
                g.drawEllipse (p.x - ballSize * 1.3f - borderPadding * 2.0f, p.y - ballSize * 1.3f - borderPadding * 2.0f, 
                              ballSize * 2.6f + borderPadding * 4.0f, ballSize * 2.6f + borderPadding * 4.0f, borderThickness * 0.3f);
            }
        }
    }
}

int XYPad::getBallAtPosition (juce::Point<float> pos, juce::Rectangle<float> bounds)
{
    if (!isSplitMode) return 0;
    
    float gainScale = juce::jmap (gainValue, -24.0f, 24.0f, 0.5f, 2.0f);
    float hitRadius = 15.0f * gainScale; // Scale hit radius with ball size
    
    // Check left ball
    juce::Point<float> leftP (bounds.getX() + leftPt * bounds.getWidth(),
                              bounds.getY() + (1.0f - pt.second) * bounds.getHeight());
    if (pos.getDistanceFrom (leftP) < hitRadius) return 1;
    
    // Check right ball
    juce::Point<float> rightP (bounds.getX() + rightPt * bounds.getWidth(),
                               bounds.getY() + (1.0f - pt.second) * bounds.getHeight());
    if (pos.getDistanceFrom (rightP) < hitRadius) return 2;
    
    return 0; // No ball hit
}

void XYPad::setBallPosition (int ballIndex, float x, float y)
{
    if (ballIndex == 1) {
        leftPt = juce::jlimit(0.0f, 1.0f, x);
        pt.second = juce::jlimit(0.0f, 1.0f, y);
    } else if (ballIndex == 2) {
        rightPt = juce::jlimit(0.0f, 1.0f, x);
        pt.second = juce::jlimit(0.0f, 1.0f, y);
    }
    repaint();
}

std::pair<float,float> XYPad::getBallPosition (int ballIndex) const
{
    if (ballIndex == 1) {
        return {leftPt, pt.second};
    } else if (ballIndex == 2) {
        return {rightPt, pt.second};
    }
    return {0.5f, 0.5f}; // Default center position
}

void XYPad::pushWaveformSample (double sampleL, double sampleR)
{
    waveformL[waveformWriteIndex] = sampleL;
    waveformR[waveformWriteIndex] = sampleR;
    waveformWriteIndex = (waveformWriteIndex + 1) % waveformBufferSize;
    hasWaveformData = true;
}

void XYPad::drawWaveformBackground (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Only draw waveforms if we have real audio data
    if (!hasWaveformData) {
        return; // Don't show any waveforms if no real audio
    }
    
    // Use mix value for opacity control (set by editor)
    
    // Draw unified processed signal across entire field with REAL-TIME DSP effects
    g.setColour (juce::Colour (0xFFFF8C00).withAlpha (0.9f * mixValue)); // Orange for processed signal (Drive)
    juce::Path processedPath;
    
    // NEW: Individual visual effects for Gain, Drive, Mix, and Width
    juce::Path gainEffectPath;
    juce::Path driveEffectPath;
    juce::Path mixEffectPath;
    juce::Path widthEffectPath;
    bool gainEffectStarted = false;
    bool driveEffectStarted = false;
    bool mixEffectStarted = false;
    bool widthEffectStarted = false;
    
    for (int x = 0; x < static_cast<int>(bounds.getWidth()); ++x)
    {
        // Live flowing: newest samples on left, oldest on right (FLIPPED DIRECTION)
        int sampleIndex = (waveformWriteIndex - x) % waveformBufferSize;
        if (sampleIndex < 0) sampleIndex += waveformBufferSize;
        
        // Combine left and right channels for unified waveform
        double sampleL = waveformL[sampleIndex];
        double sampleR = waveformR[sampleIndex];
        double sample = (sampleL + sampleR) * 0.5; // Mono mix for unified display
        
        // Apply REAL-TIME DSP effects to the waveform visualization
        // Apply HP/LP filter visualization
        if (hpValue > 20.0f) {
            // Simple HP filter visualization - reduce low frequency content
            float hpAmount = juce::jmap(hpValue, 20.0f, 1000.0f, 0.0f, 0.8f);
            sample *= (1.0f - hpAmount * 0.3f); // Reduce low end
        }
        
        if (lpValue < 20000.0f) {
            // Simple LP filter visualization - reduce high frequency content
            float lpAmount = juce::jmap(lpValue, 20000.0f, 1000.0f, 0.0f, 0.8f);
            sample *= (1.0f - lpAmount * 0.2f); // Reduce high end
        }
        
        // Apply tilt EQ visualization
        if (std::abs(tiltValue) > 0.1f) {
            float tiltAmount = juce::jmap(tiltValue, -12.0f, 12.0f, -0.5f, 0.5f);
            sample *= (1.0f + tiltAmount * 0.3f); // Boost/cut based on tilt
        }
        
        // Apply air band visualization
        if (airValue > 0.1f) {
            float airAmount = juce::jmap(airValue, 0.0f, 6.0f, 0.0f, 0.2f);
            sample *= (1.0f + airAmount * 0.4f); // Boost high frequencies
        }
        
        // Apply drive effect visualization (soft clipping)
        if (driveValue > 0.1f) {
            float driveAmount = juce::jmap(driveValue, 0.0f, 24.0f, 1.0f, 3.0f);
            sample *= driveAmount;
            // Soft clipping visualization
            sample = std::tanh(sample);
            sample /= driveAmount; // Normalize back
        }
        
        // Apply width effect visualization
        if (widthValue != 1.0f) {
            float widthAmount = juce::jmap(widthValue, 0.0f, 10.0f, 0.3f, 2.5f);
            sample *= widthAmount; // Enhance stereo width effect
        }
        
        // Apply pan effect visualization
        if (std::abs(panValue) > 0.1f) {
            float panAmount = juce::jmap(panValue, -1.0f, 1.0f, -0.3f, 0.3f);
            // Pan affects the vertical position - left pan moves up, right pan moves down
            sample += panAmount * 0.2f; // Add pan offset to sample
        }
        
        float y = bounds.getCentreY() + sample * bounds.getHeight() * 0.6f;
        
        // NEW: Calculate individual effect paths
        float gainEffectY = bounds.getCentreY() + (sample * gainValue / 24.0f) * bounds.getHeight() * 0.3f;
        float driveEffectY = bounds.getCentreY() + (std::tanh(sample * driveValue / 12.0f)) * bounds.getHeight() * 0.2f;
        float mixEffectY = bounds.getCentreY() + (sample * mixValue) * bounds.getHeight() * 0.25f;
        float widthEffectY = bounds.getCentreY() + (sample * (widthValue - 1.0f) / 5.0f) * bounds.getHeight() * 0.15f;
        
        if (x == 0) {
            processedPath.startNewSubPath (bounds.getX() + x, y);
            gainEffectPath.startNewSubPath (bounds.getX() + x, gainEffectY);
            driveEffectPath.startNewSubPath (bounds.getX() + x, driveEffectY);
            mixEffectPath.startNewSubPath (bounds.getX() + x, mixEffectY);
            widthEffectPath.startNewSubPath (bounds.getX() + x, widthEffectY);
        } else {
            processedPath.lineTo (bounds.getX() + x, y);
            gainEffectPath.lineTo (bounds.getX() + x, gainEffectY);
            driveEffectPath.lineTo (bounds.getX() + x, driveEffectY);
            mixEffectPath.lineTo (bounds.getX() + x, mixEffectY);
            widthEffectPath.lineTo (bounds.getX() + x, widthEffectY);
        }
    }
    g.strokePath (processedPath, juce::PathStrokeType (1.5f));
    
    // NEW: Draw individual effect paths with different colors
    if (gainValue != 0.0f) {
        juce::Colour gainColor = isGreenMode ? juce::Colour(0xFF5AA95A) : juce::Colour(0xFF00FF00);
        g.setColour (gainColor.withAlpha (0.6f)); // Green for Gain
        g.strokePath (gainEffectPath, juce::PathStrokeType (1.0f));
    }
    
    if (driveValue > 0.1f) {
        juce::Colour driveColor = isGreenMode ? juce::Colour(0xFF68B568) : juce::Colour(0xFFFF0000);
        g.setColour (driveColor.withAlpha (0.6f)); // Red/Green for Drive
        g.strokePath (driveEffectPath, juce::PathStrokeType (1.0f));
    }
    
    if (mixValue > 0.1f) {
        juce::Colour mixColor = isGreenMode ? juce::Colour(0xFF8CAA00) : juce::Colour(0xFF0000FF);
        g.setColour (mixColor.withAlpha (0.6f)); // Blue/Green for Mix
        g.strokePath (mixEffectPath, juce::PathStrokeType (1.0f));
    }
    
    if (widthValue != 1.0f) {
        juce::Colour widthColor = isGreenMode ? juce::Colour(0xFF9B59B6) : juce::Colour(0xFFFFFF00);
        g.setColour (widthColor.withAlpha (0.6f)); // Yellow/Purple for Width
        g.strokePath (widthEffectPath, juce::PathStrokeType (1.0f));
    }
    
    // Draw EQ curves for HP, LP, and Air filters
    drawEQCurves (g, bounds);
}

void XYPad::drawEQCurves (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Only draw EQ curves if filters are active
    bool hasHP = hpValue > 20.0f;
    bool hasLP = lpValue < 20000.0f;
    bool hasAir = airValue > 0.1f;
    bool hasBass = bassValue > 0.1f;
    bool hasScoop = std::abs(scoopValue) > 0.1f;
    bool hasTilt = std::abs(tiltValue) > 0.1f;
    
    if (!hasHP && !hasLP && !hasAir && !hasBass && !hasScoop && !hasTilt) return;
    
    // Set up frequency range (20Hz to 20kHz, log scale) - consistent with frequency regions
    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;
    const int numPoints = static_cast<int>(bounds.getWidth());
    
    // Simplified spectral analysis - just a gentle wave for EQ curves
    std::vector<float> spectralResponse(numPoints, 0.0f);
    if (hasWaveformData) {
        analyzeSpectralResponse(spectralResponse, bounds.getWidth());
    }
    
    // Draw HP/LP filter curve (combined) with gradient fill and simplified spectral response
    if (hasHP || hasLP) {
        juce::Path hpLpPath;
        juce::Path hpLpFillPath;
        juce::Path spectralPath;
        bool hpLpPathStarted = false;
        bool hpLpFillStarted = false;
        bool spectralPathStarted = false;
        
        for (int x = 0; x < numPoints; ++x)
        {
            // Calculate frequency for this x position (logarithmic scale)
            float freqRatio = static_cast<float>(x) / static_cast<float>(numPoints - 1);
            float freq = minFreq * std::pow(maxFreq / minFreq, freqRatio);
            
            // Calculate total gain at this frequency for HP/LP only
            float totalGainDb = 0.0f;
            
            // High-pass filter response (proper 2nd order Butterworth response)
            if (hasHP)
            {
                float normalizedFreq = freq / hpValue;
                float response = 1.0f / std::sqrt(1.0f + std::pow(normalizedFreq, -4.0f)); // 2nd order Butterworth
                totalGainDb += 20.0f * std::log10(response);
            }
            
            // Low-pass filter response (proper 2nd order Butterworth response)
            if (hasLP)
            {
                float normalizedFreq = freq / lpValue;
                float response = 1.0f / std::sqrt(1.0f + std::pow(normalizedFreq, 4.0f)); // 2nd order Butterworth
                totalGainDb += 20.0f * std::log10(response);
            }
            
            // Limit gain range for visualization
            totalGainDb = juce::jlimit(-60.0f, 24.0f, totalGainDb);
            
            // Convert gain to y position (center = 0dB, up = positive gain, down = negative gain)
            float gainRatio = (totalGainDb + 24.0f) / 48.0f; // Map -24dB to +24dB to 0-1
            float y = bounds.getBottom() - (gainRatio * bounds.getHeight());
            
            // Add very gentle spectral response for EQ curves (much less complex)
            float spectralOffset = 0.0f;
            if (x < spectralResponse.size()) {
                spectralOffset = spectralResponse[x] * bounds.getHeight() * 0.05f; // Much smaller scale
            }
            float spectralY = y - spectralOffset;
            
            if (!hpLpPathStarted)
            {
                hpLpPath.startNewSubPath(bounds.getX() + x, y);
                hpLpFillPath.startNewSubPath(bounds.getX() + x, y);
                spectralPath.startNewSubPath(bounds.getX() + x, spectralY);
                hpLpPathStarted = true;
                hpLpFillStarted = true;
                spectralPathStarted = true;
            }
            else
            {
                hpLpPath.lineTo(bounds.getX() + x, y);
                hpLpFillPath.lineTo(bounds.getX() + x, y);
                spectralPath.lineTo(bounds.getX() + x, spectralY);
            }
        }
        
        // Complete the fill path to create a closed shape
        hpLpFillPath.lineTo(bounds.getRight(), bounds.getBottom());
        hpLpFillPath.lineTo(bounds.getX(), bounds.getBottom());
        hpLpFillPath.closeSubPath();
        
        // Draw the gradient fill for HP/LP
        juce::Colour hpLpColor = isGreenMode ? juce::Colour(0xFF5AA95A) : juce::Colour(0xFF5AA9E6);
        juce::ColourGradient fillGrad(
            hpLpColor.withAlpha(0.6f), bounds.getX(), bounds.getY(),
            hpLpColor.withAlpha(0.02f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(fillGrad);
        g.fillPath(hpLpFillPath);
        
        // Draw the static HP/LP curve
        g.setColour(hpLpColor.withAlpha(0.7f));
        g.strokePath(hpLpPath, juce::PathStrokeType(2.0f));
        
        // Draw the simplified spectral response
        if (hasWaveformData) {
            g.setColour(hpLpColor.withAlpha(0.4f));
            g.strokePath(spectralPath, juce::PathStrokeType(1.5f));
        }
    }
    
    // Draw TILT curve with frequency control and gradient fill
    if (hasTilt) {
        juce::Path tiltPath;
        juce::Path tiltFillPath;
        bool tiltPathStarted = false;
        bool tiltFillStarted = false;
        
        for (int x = 0; x < numPoints; ++x)
        {
            // Calculate frequency for this x position (logarithmic scale)
            float freqRatio = static_cast<float>(x) / static_cast<float>(numPoints - 1);
            float freq = minFreq * std::pow(maxFreq / minFreq, freqRatio);
            
            // Calculate tilt response using the frequency control parameter
            float tiltGainDb = 0.0f;
            
            // Low shelf (around tilt frequency control)
            if (freq < 1000.0f) {
                float lowFreq = tiltFreqValue; // Use frequency control
                float lowGain = juce::jlimit(-12.0f, 12.0f, tiltValue);
                
                // Simplified low shelf response
                float normalizedFreq = freq / lowFreq;
                float lowResponse = 1.0f / std::sqrt(1.0f + std::pow(normalizedFreq, -2.0f));
                tiltGainDb += lowGain * lowResponse;
            }
            
            // High shelf (around 6kHz)
            if (freq > 1000.0f) {
                float highFreq = 6000.0f;
                float highGain = juce::jlimit(-12.0f, 12.0f, -tiltValue); // Opposite of low
                
                // Simplified high shelf response
                float normalizedFreq = freq / highFreq;
                float highResponse = 1.0f / std::sqrt(1.0f + std::pow(normalizedFreq, 2.0f));
                tiltGainDb += highGain * highResponse;
            }
            
            // Limit gain range for visualization
            tiltGainDb = juce::jlimit(-60.0f, 24.0f, tiltGainDb);
            
            // Convert gain to y position
            float gainRatio = (tiltGainDb + 24.0f) / 48.0f;
            float y = bounds.getBottom() - (gainRatio * bounds.getHeight());
            
            if (!tiltPathStarted)
            {
                tiltPath.startNewSubPath(bounds.getX() + x, y);
                tiltFillPath.startNewSubPath(bounds.getX() + x, y);
                tiltPathStarted = true;
                tiltFillStarted = true;
            }
            else
            {
                tiltPath.lineTo(bounds.getX() + x, y);
                tiltFillPath.lineTo(bounds.getX() + x, y);
            }
        }
        
        // Complete the fill path
        tiltFillPath.lineTo(bounds.getRight(), bounds.getBottom());
        tiltFillPath.lineTo(bounds.getX(), bounds.getBottom());
        tiltFillPath.closeSubPath();
        
        // Draw the gradient fill for TILT - match ball color action (warm/cool)
        juce::Colour tiltColor;
        if (isGreenMode) {
            // Green monochromatic palette
            if (tiltValue > 0.0f) {
                // Warm green (positive tilt)
                float intensity = juce::jmap(tiltValue, 0.0f, 12.0f, 0.0f, 1.0f);
                tiltColor = juce::Colour(0xFF5AA95A).interpolatedWith(juce::Colour(0xFF8CAA00), intensity);
            } else {
                // Cool green (negative tilt)
                float intensity = juce::jmap(std::abs(tiltValue), 0.0f, 12.0f, 0.0f, 1.0f);
                tiltColor = juce::Colour(0xFF5AA95A).interpolatedWith(juce::Colour(0xFF68B568), intensity);
            }
        } else {
            // Standard color palette - match ball colors
            if (tiltValue > 0.0f) {
                // Warm colors: yellow -> orange -> red
                float intensity = juce::jmap(tiltValue, 0.0f, 12.0f, 0.0f, 1.0f);
                float hue = juce::jmap(intensity, 0.0f, 1.0f, 0.15f, 0.0f);
                tiltColor = juce::Colour::fromHSV(hue, 0.7f, 0.7f + intensity * 0.15f, 1.0f);
            } else {
                // Cool colors: green -> cyan -> blue
                float intensity = juce::jmap(std::abs(tiltValue), 0.0f, 12.0f, 0.0f, 1.0f);
                float hue = juce::jmap(intensity, 0.0f, 1.0f, 0.3f, 0.8f);
                tiltColor = juce::Colour::fromHSV(hue, 0.7f, 0.7f + intensity * 0.15f, 1.0f);
            }
        }
        
        juce::ColourGradient fillGrad(
            tiltColor.withAlpha(0.6f), bounds.getX(), bounds.getY(),
            tiltColor.withAlpha(0.02f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(fillGrad);
        g.fillPath(tiltFillPath);
        
        // Draw the TILT curve
        g.setColour(tiltColor.withAlpha(0.8f));
        g.strokePath(tiltPath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    
    // Draw BASS curve with frequency control and gradient fill
    if (hasBass) {
        juce::Path bassPath;
        juce::Path bassFillPath;
        juce::Path spectralPath;
        bool bassPathStarted = false;
        bool bassFillStarted = false;
        bool spectralPathStarted = false;
        
        for (int x = 0; x < numPoints; ++x)
        {
            // Calculate frequency for this x position (logarithmic scale)
            float freqRatio = static_cast<float>(x) / static_cast<float>(numPoints - 1);
            float freq = minFreq * std::pow(maxFreq / minFreq, freqRatio);
            
            // Calculate BASS shelf response using frequency control
            float bassGainDb = 0.0f;
            
            // Bass shelf affects frequencies below the frequency control
            float bassFreq = bassFreqValue;
            if (freq < bassFreq * 2.0f) // Extend range for smoother transition
            {
                // Smooth shelf response that works for both positive and negative values
                float transition = (bassFreq - freq) / bassFreq;
                transition = juce::jlimit(0.0f, 1.0f, transition);
                transition = std::sin(transition * juce::MathConstants<float>::halfPi); // Smooth transition
                
                // Direct mapping of bass value to gain (can be negative for cut)
                bassGainDb = bassValue * transition;
            }
            
            // Limit gain range for visualization
            bassGainDb = juce::jlimit(-60.0f, 24.0f, bassGainDb);
            
            // Convert gain to y position
            float gainRatio = (bassGainDb + 24.0f) / 48.0f;
            float y = bounds.getBottom() - (gainRatio * bounds.getHeight());
            
            // Add gentle spectral response
            float spectralOffset = 0.0f;
            if (x < spectralResponse.size()) {
                spectralOffset = spectralResponse[x] * bounds.getHeight() * 0.03f; // Very small scale
            }
            float spectralY = y - spectralOffset;
            
            if (!bassPathStarted)
            {
                bassPath.startNewSubPath(bounds.getX() + x, y);
                bassFillPath.startNewSubPath(bounds.getX() + x, y);
                spectralPath.startNewSubPath(bounds.getX() + x, spectralY);
                bassPathStarted = true;
                bassFillStarted = true;
                spectralPathStarted = true;
            }
            else
            {
                bassPath.lineTo(bounds.getX() + x, y);
                bassFillPath.lineTo(bounds.getX() + x, y);
                spectralPath.lineTo(bounds.getX() + x, spectralY);
            }
        }
        
        // Complete the fill path
        bassFillPath.lineTo(bounds.getRight(), bounds.getBottom());
        bassFillPath.lineTo(bounds.getX(), bounds.getBottom());
        bassFillPath.closeSubPath();
        
        // Draw the gradient fill for BASS
        juce::Colour bassColor = isGreenMode ? juce::Colour(0xFF68B568) : juce::Colour(0xFFFF6B6B);
        juce::ColourGradient fillGrad(
            bassColor.withAlpha(0.6f), bounds.getX(), bounds.getY(),
            bassColor.withAlpha(0.02f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(fillGrad);
        g.fillPath(bassFillPath);
        
        // Draw the BASS curve
        g.setColour(bassColor.withAlpha(0.7f));
        g.strokePath(bassPath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Draw the simplified spectral response
        if (hasWaveformData) {
            g.setColour(bassColor.withAlpha(0.3f));
            g.strokePath(spectralPath, juce::PathStrokeType(1.0f));
        }
    }
    
    // Draw SCOOP curve with frequency control and gradient fill
    if (hasScoop) {
        juce::Path scoopPath;
        juce::Path scoopFillPath;
        juce::Path spectralPath;
        bool scoopPathStarted = false;
        bool scoopFillStarted = false;
        bool spectralPathStarted = false;
        
        for (int x = 0; x < numPoints; ++x)
        {
            // Calculate frequency for this x position (logarithmic scale)
            float freqRatio = static_cast<float>(x) / static_cast<float>(numPoints - 1);
            float freq = minFreq * std::pow(maxFreq / minFreq, freqRatio);
            
            // Calculate SCOOP bell curve response using frequency control
            float scoopGainDb = 0.0f;
            
            // Scoop affects mid frequencies around the frequency control - smooth range
            float centerFreq = scoopFreqValue;
            float bandwidth = centerFreq * 0.5f; // Q factor equivalent - proportional to center frequency
            
            // Smooth bell curve response across full frequency range
            float scoopFreqRatio = freq / centerFreq;
            float logFreqRatio = std::log(scoopFreqRatio);
            float response = 1.0f / (1.0f + std::pow(logFreqRatio / (bandwidth / centerFreq), 2.0f));
            
            // Apply scoop value (can be positive or negative)
            scoopGainDb = scoopValue * response;
            
            // Limit gain range for visualization
            scoopGainDb = juce::jlimit(-60.0f, 24.0f, scoopGainDb);
            
            // Convert gain to y position
            float gainRatio = (scoopGainDb + 24.0f) / 48.0f;
            float y = bounds.getBottom() - (gainRatio * bounds.getHeight());
            
            // Add gentle spectral response
            float spectralOffset = 0.0f;
            if (x < spectralResponse.size()) {
                spectralOffset = spectralResponse[x] * bounds.getHeight() * 0.03f; // Very small scale
            }
            float spectralY = y - spectralOffset;
            
            if (!scoopPathStarted)
            {
                scoopPath.startNewSubPath(bounds.getX() + x, y);
                scoopFillPath.startNewSubPath(bounds.getX() + x, y);
                spectralPath.startNewSubPath(bounds.getX() + x, spectralY);
                scoopPathStarted = true;
                scoopFillStarted = true;
                spectralPathStarted = true;
            }
            else
            {
                scoopPath.lineTo(bounds.getX() + x, y);
                scoopFillPath.lineTo(bounds.getX() + x, y);
                spectralPath.lineTo(bounds.getX() + x, spectralY);
            }
        }
        
        // Complete the fill path
        scoopFillPath.lineTo(bounds.getRight(), bounds.getBottom());
        scoopFillPath.lineTo(bounds.getX(), bounds.getBottom());
        scoopFillPath.closeSubPath();
        
        // Draw the gradient fill for SCOOP
        juce::Colour scoopColor;
        if (isGreenMode) {
            // Green monochromatic palette - use a green variant for scoop
            scoopColor = juce::Colour(0xFF8CAA00); // Green variant for scoop in green mode
        } else {
            // Standard purple color for scoop
            scoopColor = juce::Colour(0xFF9B59B6);
        }
        juce::ColourGradient fillGrad(
            scoopColor.withAlpha(0.6f), bounds.getX(), bounds.getY(),
            scoopColor.withAlpha(0.02f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(fillGrad);
        g.fillPath(scoopFillPath);
        
        // Draw the SCOOP curve
        g.setColour(scoopColor.withAlpha(0.7f));
        g.strokePath(scoopPath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Draw the simplified spectral response
        if (hasWaveformData) {
            g.setColour(scoopColor.withAlpha(0.3f));
            g.strokePath(spectralPath, juce::PathStrokeType(1.0f));
        }
    }
    
    // Draw AIR curve with frequency control and gradient fill
    if (hasAir) {
        juce::Path airPath;
        juce::Path airFillPath;
        juce::Path spectralPath;
        bool airPathStarted = false;
        bool airFillStarted = false;
        bool spectralPathStarted = false;
        
        for (int x = 0; x < numPoints; ++x)
        {
            // Calculate frequency for this x position (logarithmic scale)
            float freqRatio = static_cast<float>(x) / static_cast<float>(numPoints - 1);
            float freq = minFreq * std::pow(maxFreq / minFreq, freqRatio);
            
            // Calculate AIR band response using frequency control
            float airGainDb = 0.0f;
            
            // Air band starts affecting around the frequency control - smooth transition
            float airFreq = airFreqValue;
            if (freq > airFreq * 0.5f) // Start affecting earlier for smoother transition
            {
                // Smooth high shelf response
                float transition = (freq - airFreq * 0.5f) / (20000.0f - airFreq * 0.5f);
                transition = juce::jlimit(0.0f, 1.0f, transition);
                transition = std::sin(transition * juce::MathConstants<float>::halfPi); // Smooth transition
                
                // Direct mapping of air value to gain (0-6dB range)
                airGainDb = airValue * transition;
            }
            
            // Limit gain range for visualization
            airGainDb = juce::jlimit(-60.0f, 24.0f, airGainDb);
            
            // Convert gain to y position
            float gainRatio = (airGainDb + 24.0f) / 48.0f;
            float y = bounds.getBottom() - (gainRatio * bounds.getHeight());
            
            // Add gentle spectral response
            float spectralOffset = 0.0f;
            if (x < spectralResponse.size()) {
                spectralOffset = spectralResponse[x] * bounds.getHeight() * 0.02f; // Very small scale
            }
            float spectralY = y - spectralOffset;
            
            if (!airPathStarted)
            {
                airPath.startNewSubPath(bounds.getX() + x, y);
                airFillPath.startNewSubPath(bounds.getX() + x, y);
                spectralPath.startNewSubPath(bounds.getX() + x, spectralY);
                airPathStarted = true;
                airFillStarted = true;
                spectralPathStarted = true;
            }
            else
            {
                airPath.lineTo(bounds.getX() + x, y);
                airFillPath.lineTo(bounds.getX() + x, y);
                spectralPath.lineTo(bounds.getX() + x, spectralY);
            }
        }
        
        // Complete the fill path
        airFillPath.lineTo(bounds.getRight(), bounds.getBottom());
        airFillPath.lineTo(bounds.getX(), bounds.getBottom());
        airFillPath.closeSubPath();
        
        // Draw the gradient fill for AIR
        juce::Colour airColor = isGreenMode ? juce::Colour(0xFFE8F4E8) : juce::Colour(0xFFFFFFFF);
        juce::ColourGradient fillGrad(
            airColor.withAlpha(0.5f), bounds.getX(), bounds.getY(),
            airColor.withAlpha(0.01f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(fillGrad);
        g.fillPath(airFillPath);
        
        // Draw the AIR curve
        g.setColour(airColor.withAlpha(0.6f));
        g.strokePath(airPath, juce::PathStrokeType(1.5f)); // Thinner line for AIR
        
        // Draw the simplified spectral response
        if (hasWaveformData) {
            g.setColour(airColor.withAlpha(0.2f));
            g.strokePath(spectralPath, juce::PathStrokeType(0.8f));
        }
    }
    
    // Draw 0dB reference line
    float centerY = bounds.getCentreY();
    g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.3f));
    g.drawHorizontalLine(static_cast<int>(centerY), bounds.getX(), bounds.getRight());
    
    // Draw frequency markers if EQ is active
    if (hasHP || hasLP || hasAir || hasBass || hasScoop || hasTilt)
    {
        g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.5f));
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        
        // Mark key frequencies
        std::vector<std::pair<float, juce::String>> freqMarkers = {
            {100.0f, "100Hz"},
            {1000.0f, "1kHz"},
            {10000.0f, "10kHz"}
        };
        
        for (auto& marker : freqMarkers)
        {
            float freq = marker.first;
            float freqRatio = std::log(freq / minFreq) / std::log(maxFreq / minFreq);
            float x = bounds.getX() + freqRatio * bounds.getWidth();
            
            if (x >= bounds.getX() && x <= bounds.getRight())
            {
                g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getY() + 10);
                g.drawText(marker.second, static_cast<int>(x - 20), static_cast<int>(bounds.getY() + 12), 40, 12, juce::Justification::centred);
            }
        }
        
        // Add AIR-specific frequency marker
        if (hasAir) {
            float airFreq = 20000.0f;
            float freqRatio = std::log(airFreq / minFreq) / std::log(maxFreq / minFreq);
            float x = bounds.getX() + freqRatio * bounds.getWidth();
            
            if (x >= bounds.getX() && x <= bounds.getRight())
            {
                g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.6f)); // White with transparency for AIR
                g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getY() + 10);
                g.drawText("AIR", static_cast<int>(x - 20), static_cast<int>(bounds.getY() + 12), 40, 12, juce::Justification::centred);
            }
        }
    }
}

void XYPad::drawFrequencyRegions (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Define frequency ranges for different regions
    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;
    
    // Frequency regions with colors and labels
    struct FrequencyRegion {
        float startHz, endHz;
        juce::Colour color;
        juce::String label;
    };
    
    std::vector<FrequencyRegion> regions;
    
    if (isGreenMode) {
        // Green monochromatic palette for frequency regions
        regions = {
            {20.0f, 80.0f, juce::Colour(0x405AA95A), "Sub"},      // Sub bass
            {80.0f, 250.0f, juce::Colour(0x4068B568), "Bass"},    // Bass
            {250.0f, 500.0f, juce::Colour(0x4076C176), "Low Mid"}, // Low mid
            {500.0f, 2000.0f, juce::Colour(0x4084CD84), "Mid"},    // Mid
            {2000.0f, 4000.0f, juce::Colour(0x4092D992), "High Mid"}, // High mid
            {4000.0f, 8000.0f, juce::Colour(0x40A0E5A0), "Presence"}, // Presence
            {8000.0f, 16000.0f, juce::Colour(0x40AEF1AE), "Air"},  // Air
            {16000.0f, 20000.0f, juce::Colour(0x40BCFDBC), "Brilliance"} // Brilliance
        };
    } else {
        // Standard color palette for frequency regions
        regions = {
            {20.0f, 80.0f, juce::Colour(0x40FF6B6B), "Sub"},      // Sub bass
            {80.0f, 250.0f, juce::Colour(0x40FF8C8C), "Bass"},    // Bass
            {250.0f, 500.0f, juce::Colour(0x40FFB366), "Low Mid"}, // Low mid
            {500.0f, 2000.0f, juce::Colour(0x40FFFF99), "Mid"},    // Mid
            {2000.0f, 4000.0f, juce::Colour(0x40B3FF99), "High Mid"}, // High mid
            {4000.0f, 8000.0f, juce::Colour(0x4099CCFF), "Presence"}, // Presence
            {8000.0f, 16000.0f, juce::Colour(0x40CC99FF), "Air"},  // Air
            {16000.0f, 20000.0f, juce::Colour(0x40FF99FF), "Brilliance"} // Brilliance
        };
    }
    
    // Draw frequency regions
    for (const auto& region : regions)
    {
        // Calculate x positions for region boundaries
        float startRatio = std::log(region.startHz / minFreq) / std::log(maxFreq / minFreq);
        float endRatio = std::log(region.endHz / minFreq) / std::log(maxFreq / minFreq);
        
        float startX = bounds.getX() + startRatio * bounds.getWidth();
        float endX = bounds.getX() + endRatio * bounds.getWidth();
        
        // Draw region background
        g.setColour(region.color);
        g.fillRect(juce::Rectangle<float>(startX, bounds.getY(), endX - startX, bounds.getHeight()));
        
        // Draw region boundary lines
        g.setColour(region.color.withAlpha(0.6f));
        g.drawVerticalLine(static_cast<int>(startX), bounds.getY(), bounds.getBottom());
        g.drawVerticalLine(static_cast<int>(endX), bounds.getY(), bounds.getBottom());
    }
    
    // Draw frequency markers
    juce::Colour markerColor = isGreenMode ? juce::Colour(0xFFE8F4E8) : juce::Colour(0xFFFFFFFF);
    g.setColour(markerColor.withAlpha(0.7f));
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    
    std::vector<float> freqMarkers = {50.0f, 100.0f, 250.0f, 500.0f, 1000.0f, 2500.0f, 5000.0f, 10000.0f, 20000.0f};
    
    for (float freq : freqMarkers)
    {
        float freqRatio = std::log(freq / minFreq) / std::log(maxFreq / minFreq);
        float x = bounds.getX() + freqRatio * bounds.getWidth();
        
        if (x >= bounds.getX() && x <= bounds.getRight())
        {
            // Draw marker line
            g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getY() + 8);
            
            // Draw frequency label
            juce::String label;
            if (freq >= 1000.0f)
                label = juce::String(freq / 1000.0f, 1) + "k";
            else
                label = juce::String(static_cast<int>(freq));
            
            g.drawText(label, static_cast<int>(x - 15), static_cast<int>(bounds.getY() + 10), 30, 12, juce::Justification::centred);
        }
    }
}

void XYPad::analyzeSpectralResponse (std::vector<float>& response, float width)
{
    if (!hasWaveformData || waveformBufferSize == 0) return;
    
    const int numBins = static_cast<int>(width);
    response.resize(numBins, 0.0f);
    
    // Simple FFT-like analysis using sliding window
    const int windowSize = 64;
    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;
    
    for (int bin = 0; bin < numBins; ++bin)
    {
        // Calculate frequency for this bin (logarithmic scale)
        float freqRatio = static_cast<float>(bin) / static_cast<float>(numBins - 1);
        float targetFreq = minFreq * std::pow(maxFreq / minFreq, freqRatio);
        
        // Analyze signal energy around this frequency
        float energy = 0.0f;
        int sampleCount = 0;
        
        // Use a sliding window to analyze recent samples
        for (int i = 0; i < windowSize && i < waveformBufferSize; ++i)
        {
            int sampleIndex = (waveformWriteIndex - i + waveformBufferSize) % waveformBufferSize;
            
            // Combine left and right channels
            double sampleL = waveformL[sampleIndex];
            double sampleR = waveformR[sampleIndex];
            double sample = (sampleL + sampleR) * 0.5;
            
            // Simple frequency analysis using bandpass-like filtering
            // This is a simplified approach - in a real implementation you'd use FFT
            float timeOffset = static_cast<float>(i) / static_cast<float>(windowSize);
            float phase = 2.0f * juce::MathConstants<float>::pi * targetFreq * timeOffset;
            
            // Bandpass filter approximation
            float filterResponse = std::exp(-std::abs(sample) * targetFreq / 1000.0f);
            energy += std::abs(sample) * filterResponse;
            sampleCount++;
        }
        
        if (sampleCount > 0) {
            energy /= static_cast<float>(sampleCount);
            
            // Apply smoothing and scaling
            energy = std::sqrt(energy); // RMS-like calculation
            energy = juce::jlimit(0.0f, 1.0f, energy * 2.0f); // Scale and clamp
            
            // Add some randomness for more organic movement
            float noise = (static_cast<float>(rand()) / RAND_MAX) * 0.1f;
            energy += noise;
            energy = juce::jlimit(0.0f, 1.0f, energy);
            
            response[bin] = energy;
        }
    }
}

void MyPluginAudioProcessorEditor::styleSlider (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    // 6 o'clock (180°) → 6 o'clock (540°) full sweep, stopping at the ends
    constexpr float kStart = juce::MathConstants<float>::pi; // 180°
    constexpr float kEnd   = kStart + juce::MathConstants<float>::twoPi; // 540° (3π)
    s.setRotaryParameters(kStart, kEnd, true);
    // REMOVE: s.setSize(...) - sizing must happen in resized() only
}

void MyPluginAudioProcessorEditor::styleMainSlider (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    // 6 o'clock (180°) → 6 o'clock (540°) full sweep, stopping at the ends
    constexpr float kStart = juce::MathConstants<float>::pi; // 180°
    constexpr float kEnd   = kStart + juce::MathConstants<float>::twoPi; // 540° (3π)
    s.setRotaryParameters(kStart, kEnd, true);
    // REMOVE: s.setSize(...) - sizing must happen in resized() only
}

MyPluginAudioProcessorEditor::MyPluginAudioProcessorEditor (MyPluginAudioProcessor& p)
: AudioProcessorEditor (&p), proc (p)
{
    // Set initial size based on base dimensions
    setSize (baseWidth, baseHeight);
    lnf.theme.accent = juce::Colour (0xFF5AA9E6); // ocean default
    lnf.setupColours();
    setLookAndFeel (&lnf);
    

    
    // Setup options button (moved to bottom left corner of editor)
    addAndMakeVisible (optionsButton);
    optionsButton.setButtonText ("");
    optionsButton.onClick = [this] {
        juce::PopupMenu menu;
        
        // Add plugin info header
        menu.addSectionHeader ("Field - Spatial Audio Processor");
        menu.addSeparator();
        
        // Oversampling section
        menu.addSectionHeader ("🎛️ Oversampling");
        menu.addItem (1, "1x (Standard)", true, osSelect.getSelectedId() == 0);
        menu.addItem (2, "2x (High Quality)", true, osSelect.getSelectedId() == 1);
        menu.addItem (3, "4x (Ultra Quality)", true, osSelect.getSelectedId() == 2);
        menu.addItem (4, "8x (Maximum Quality)", true, osSelect.getSelectedId() == 3);
        menu.addItem (5, "16x (Extreme Quality)", true, osSelect.getSelectedId() == 4);
        menu.addSeparator();
        
        // Interface scaling removed: use drag handle snap (50%/100%) only
        
        // Close option with red X
        menu.addItem (99, "❌ Close", true, false);
        
        menu.showMenuAsync (juce::PopupMenu::Options(), [this] (int result) {
            if (result >= 1 && result <= 5) {
                osSelect.setSelectedId (result - 1);
            } else if (result == 99) {
                // Close menu - do nothing
            }
        });
    };
    
    // Setup bypass button (custom green when active)
    addAndMakeVisible (bypassButton);
    bypassButton.onClick = [this] {
        bool currentState = bypassButton.getToggleState();
        bypassButton.setToggleState (!currentState, juce::dontSendNotification);
        if (auto* bypassParam = proc.apvts.getParameter ("bypass")) {
            bypassParam->setValueNotifyingHost (currentState ? 0.0f : 1.0f);
        }
    };
    
    // Setup link button
    addAndMakeVisible (linkButton);
    linkButton.setButtonText ("");
    linkButton.setLookAndFeel (&lnf);
    linkButton.onClick = [this] {
        bool currentState = linkButton.getToggleState();
        linkButton.setToggleState (!currentState, juce::dontSendNotification);
        pad.setLinked (!currentState);
    };
    
    // Setup full screen button
    addAndMakeVisible (fullScreenButton);
    fullScreenButton.setButtonText ("");
    fullScreenButton.onClick = [this] {
        bool currentState = fullScreenButton.getToggleState();
        fullScreenButton.setToggleState (!currentState, juce::dontSendNotification);
        
        if (currentState) {
            // Exit full screen - restore original size
            setSize(savedBounds.getWidth(), savedBounds.getHeight());
        } else {
            // Enter full screen - save current bounds and maximize
            savedBounds = getBounds();
            auto& display = juce::Desktop::getInstance().getDisplays().getMainDisplay();
            setSize(display.totalArea.getWidth(), display.totalArea.getHeight());
        }
    };
    
    // Setup color mode button
    addAndMakeVisible (colorModeButton);
    colorModeButton.setButtonText ("");
    colorModeButton.setToggleState (isGreenMode, juce::dontSendNotification);
    colorModeButton.onClick = [this] {
        isGreenMode = !isGreenMode;
        colorModeButton.setToggleState (isGreenMode, juce::dontSendNotification);
        
        // Update the LookAndFeel colors
        lnf.setGreenMode(isGreenMode);
        
        // Update Space knob and XYPad green mode
        spaceKnob.setGreenMode(isGreenMode);
        spaceAlgorithmSwitch.setGreenMode(isGreenMode);
        pad.setGreenMode(isGreenMode);
        
        // Force repaint of all components
        repaint();
    };
    
    // Setup preset system
    addAndMakeVisible (presetCombo);
    addAndMakeVisible (savePresetButton);

    // Add header dividers (initially hidden; positioned in paint->resized flow)
    addAndMakeVisible(splitDivider);
    splitDivider.setVisible(true);

    // Split-pan container lives in the same grid cell as the stereo pan knob when split mode is active
    addAndMakeVisible (panSplitContainer);
    panSplitContainer.setVisible(false);
    panSplitContainer.setInterceptsMouseClicks(false, false);
    
    // Configure preset manager
    presetManager.setParameterGetter([this]() {
        std::map<juce::String, float> params;
        params["gain_db"] = gain.getValue();
        params["width"] = width.getValue();
        params["tilt"] = tilt.getValue();
        params["mono_hz"] = monoHz.getValue();
        params["hp_hz"] = hpHz.getValue();
        params["lp_hz"] = lpHz.getValue();
        params["sat_drive_db"] = satDrive.getValue();
        params["sat_mix"] = satMix.getValue();
        params["air_db"] = air.getValue();
        params["bass_db"] = bass.getValue();
        params["scoop"] = scoop.getValue(); // NEW: Scoop parameter
        params["pan"] = panKnob.getValue();
        params["depth"] = pad.getPoint01().second; // Get Y value (depth)
        params["ducking"] = duckingKnob.getValue();
        
        // NEW: Frequency control parameters
        params["tilt_freq"] = tiltFreqSlider.getValue();
        params["scoop_freq"] = scoopFreqSlider.getValue();
        params["bass_freq"] = bassFreqSlider.getValue();
        params["air_freq"] = airFreqSlider.getValue();
        return params;
    });
    
    presetManager.setParameterSetter([this](const juce::String& paramName, float value) {
        if (paramName == "gain_db") gain.setValue(value, juce::dontSendNotification);
        else if (paramName == "width") width.setValue(value, juce::dontSendNotification);
        else if (paramName == "tilt") tilt.setValue(value, juce::dontSendNotification);
        else if (paramName == "mono_hz") monoHz.setValue(value, juce::dontSendNotification);
        else if (paramName == "hp_hz") hpHz.setValue(value, juce::dontSendNotification);
        else if (paramName == "lp_hz") lpHz.setValue(value, juce::dontSendNotification);
        else if (paramName == "sat_drive_db") satDrive.setValue(value, juce::dontSendNotification);
        else if (paramName == "sat_mix") satMix.setValue(value, juce::dontSendNotification);
        else if (paramName == "air_db") air.setValue(value, juce::dontSendNotification);
        else if (paramName == "bass_db") bass.setValue(value, juce::dontSendNotification);
        else if (paramName == "scoop") scoop.setValue(value, juce::dontSendNotification);
        else if (paramName == "pan") {
            panKnob.setValue(value, juce::dontSendNotification);
            // Update XY pad X position to match pan value
            float x01 = juce::jmap(value, -1.0f, 1.0f, 0.0f, 1.0f);
            pad.setPoint01(x01, pad.getPoint01().second);
        }
        else if (paramName == "depth") {
            // Update XY pad Y position to match depth value
            pad.setPoint01(pad.getPoint01().first, value);
        }
        else if (paramName == "ducking") duckingKnob.setValue(value, juce::dontSendNotification);
        else if (paramName == "tilt_freq") tiltFreqSlider.setValue(value, juce::dontSendNotification);
        else if (paramName == "scoop_freq") scoopFreqSlider.setValue(value, juce::dontSendNotification);
        else if (paramName == "bass_freq") bassFreqSlider.setValue(value, juce::dontSendNotification);
        else if (paramName == "air_freq") airFreqSlider.setValue(value, juce::dontSendNotification);
        else if (paramName == "space_algo") {
            if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter("space_algo")))
            {
                const int idx = juce::roundToInt(p->convertFrom0to1(value));
                const int clamped = juce::jlimit(0, juce::jmax(0, p->choices.size() - 1), idx);
                spaceAlgorithmSwitch.setAlgorithmFromParameter(clamped);
            }
        }
        else if (paramName == "split_mode") splitToggle.setToggleState(value > 0.5f, juce::dontSendNotification);
    });
    
    // Setup preset combo box
    presetCombo.setPresetManager(&presetManager);
    
    presetCombo.onPresetSelected = [this](const juce::String& presetName) {
        presetManager.applyPreset(presetName);
        
        // Update the current A/B state name
        if (isStateA) {
            presetNameA = presetName;
        } else {
            presetNameB = presetName;
        }
        updatePresetDisplay();
    };
    
    // Text change handling is now integrated into the PresetComboBox class
    
    // Setup PresetManager parameter getter and setter
    presetManager.setParameterGetter([this]() {
        std::map<juce::String, float> params;
        params["gain_db"] = gain.getValue();
        params["width"] = width.getValue();
        params["tilt"] = tilt.getValue();
        params["mono_hz"] = monoHz.getValue();
        params["hp_hz"] = hpHz.getValue();
        params["lp_hz"] = lpHz.getValue();
        params["sat_drive_db"] = satDrive.getValue();
        params["sat_mix"] = satMix.getValue();
        params["air_db"] = air.getValue();
        params["bass_db"] = bass.getValue();
        params["scoop"] = scoop.getValue(); // NEW: Scoop parameter
        params["pan"] = panKnob.getValue();
        params["depth"] = pad.getPoint01().second;
        params["ducking"] = duckingKnob.getValue();
        
        // NEW: Frequency control parameters
        params["tilt_freq"] = tiltFreqSlider.getValue();
        params["scoop_freq"] = scoopFreqSlider.getValue();
        params["bass_freq"] = bassFreqSlider.getValue();
        params["air_freq"] = airFreqSlider.getValue();
        return params;
    });
    
    presetManager.setParameterSetter([this](const juce::String& paramName, float value) {
        if (paramName == "gain_db") gain.setValue(value, juce::dontSendNotification);
        else if (paramName == "width") width.setValue(value, juce::dontSendNotification);
        else if (paramName == "tilt") tilt.setValue(value, juce::dontSendNotification);
        else if (paramName == "mono_hz") monoHz.setValue(value, juce::dontSendNotification);
        else if (paramName == "hp_hz") hpHz.setValue(value, juce::dontSendNotification);
        else if (paramName == "lp_hz") lpHz.setValue(value, juce::dontSendNotification);
        else if (paramName == "sat_drive_db") satDrive.setValue(value, juce::dontSendNotification);
        else if (paramName == "sat_mix") satMix.setValue(value, juce::dontSendNotification);
        else if (paramName == "air_db") air.setValue(value, juce::dontSendNotification);
        else if (paramName == "bass_db") bass.setValue(value, juce::dontSendNotification);
        else if (paramName == "scoop") scoop.setValue(value, juce::dontSendNotification);
        else if (paramName == "pan") {
            panKnob.setValue(value, juce::dontSendNotification);
            // Update XY pad X position to match pan value
            float x01 = juce::jmap(value, -1.0f, 1.0f, 0.0f, 1.0f);
            pad.setPoint01(x01, pad.getPoint01().second);
        }
        else if (paramName == "depth") {
            // Update XY pad Y position to match depth value
            pad.setPoint01(pad.getPoint01().first, value);
        }
        else if (paramName == "ducking") duckingKnob.setValue(value, juce::dontSendNotification);
        else if (paramName == "tilt_freq") tiltFreqSlider.setValue(value, juce::dontSendNotification);
        else if (paramName == "scoop_freq") scoopFreqSlider.setValue(value, juce::dontSendNotification);
        else if (paramName == "bass_freq") bassFreqSlider.setValue(value, juce::dontSendNotification);
        else if (paramName == "air_freq") airFreqSlider.setValue(value, juce::dontSendNotification);
        else if (paramName == "space_algo") {
            if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter("space_algo")))
            {
                const int idx = juce::roundToInt(p->convertFrom0to1(value));
                const int clamped = juce::jlimit(0, juce::jmax(0, p->choices.size() - 1), idx);
                spaceAlgorithmSwitch.setAlgorithmFromParameter(clamped);
            }
        }
        else if (paramName == "split_mode") splitToggle.setToggleState(value > 0.5f, juce::dontSendNotification);
    });
    
    // Setup save button
    savePresetButton.onSavePreset = [this]() {
        // Show save preset dialog
        juce::AlertWindow saveDialog("Save Preset", "Save current settings as a preset", juce::AlertWindow::NoIcon);
        
        saveDialog.addTextEditor("name", "My Preset", "Preset Name:");
        saveDialog.addTextEditor("category", "User", "Category:");
        saveDialog.addTextEditor("subcategory", "Custom", "Subcategory:");
        saveDialog.addTextEditor("description", "Custom preset", "Description:");
        
        saveDialog.addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
        saveDialog.addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        
        saveDialog.enterModalState(true, juce::ModalCallbackFunction::create([this, &saveDialog](int result) {
            if (result == 1) {
                juce::String name = saveDialog.getTextEditorContents("name");
                juce::String category = saveDialog.getTextEditorContents("category");
                juce::String subcategory = saveDialog.getTextEditorContents("subcategory");
                juce::String description = saveDialog.getTextEditorContents("description");
                
                if (name.isNotEmpty()) {
                    presetManager.savePreset(name, category, subcategory, description);
                    presetCombo.refreshPresets();
                    
                    // Select the newly saved preset
                    for (int i = 0; i < presetCombo.getNumItems(); ++i) {
                        juce::String itemText = presetCombo.getItemText(i + 1);
                        if (itemText.startsWith("★ ")) {
                            itemText = itemText.substring(2);
                        }
                        if (itemText == name) {
                            presetCombo.setSelectedId(i + 1, juce::dontSendNotification);
                            if (presetCombo.onPresetSelected) {
                                presetCombo.onPresetSelected(name);
                            }
                            break;
                        }
                    }
                }
            }
        }), true);
    };
    
    // Setup A/B button
    addAndMakeVisible(abButtonA);
    addAndMakeVisible(abButtonB);
    addAndMakeVisible(copyButton);
    
    // Set initial state - A is active
    abButtonA.setToggleState(true, juce::dontSendNotification);
    abButtonB.setToggleState(false, juce::dontSendNotification);
    
    abButtonA.onClick = [this]() {
        if (!abButtonA.getToggleState()) {
            toggleABState();
        }
    };
    
    abButtonB.onClick = [this]() {
        if (!abButtonB.getToggleState()) {
            toggleABState();
        }
    };
    
    // Setup copy button
    copyButton.onClick = [this]() {
        // Show copy menu
        juce::PopupMenu copyMenu;
        copyMenu.addItem(1, "Copy A to B");
        copyMenu.addItem(2, "Copy B to A");
        
        copyMenu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            if (result == 1) {
                copyState(true);  // Copy from A
                pasteState(false); // Paste to B
            } else if (result == 2) {
                copyState(false); // Copy from B
                pasteState(true);  // Paste to A
            }
        });
    };
    
    // Setup arrow navigation
    addAndMakeVisible(prevPresetButton);
    addAndMakeVisible(nextPresetButton);
    
    prevPresetButton.onClick = [this]() {
        int currentIndex = presetCombo.getSelectedId() - 1;
        int totalPresets = presetCombo.getNumItems();
        if (totalPresets > 0) {
            int prevIndex = (currentIndex - 1 + totalPresets) % totalPresets;
            presetCombo.setSelectedId(prevIndex + 1, juce::dontSendNotification);
            if (presetCombo.onPresetSelected) {
                presetCombo.onPresetSelected(presetCombo.getText());
            }
        }
    };
    
    nextPresetButton.onClick = [this]() {
        int currentIndex = presetCombo.getSelectedId() - 1;
        int totalPresets = presetCombo.getNumItems();
        if (totalPresets > 0) {
            int nextIndex = (currentIndex + 1) % totalPresets;
            presetCombo.setSelectedId(nextIndex + 1, juce::dontSendNotification);
            if (presetCombo.onPresetSelected) {
                presetCombo.onPresetSelected(presetCombo.getText());
            }
        }
    };
    
    // Space algorithm functionality is now handled by the switch
    
    // Add parameter listeners to sync with host
    proc.apvts.addParameterListener("space_algo", this);
    proc.apvts.addParameterListener("pan", this);
    proc.apvts.addParameterListener("depth", this);
    
    // Setup split toggle switch
    addAndMakeVisible (splitToggle);
    splitToggle.onToggleChange = [this] (bool isSplit) {
        pad.setSplitMode (isSplit);
        linkButton.setVisible (isSplit); // Show link button only in split mode
        updateParameterLocks();
        
        // Update pan knob visibility and trigger layout update
        if (isSplit) {
            panKnob.setVisible(false);
            panValue.setVisible(false);
            panKnobLeft.setVisible(true);
            panKnobRight.setVisible(true);
            panValueLeft.setVisible(true);
            panValueRight.setVisible(true);
        } else {
            panKnob.setVisible(true);
            panValue.setVisible(true);
            panKnobLeft.setVisible(false);
            panKnobRight.setVisible(false);
            panValueLeft.setVisible(false);
            panValueRight.setVisible(false);
        }
        
        // Force layout update
        resized();
    };
    
    // Ensure we start in normal stereo mode (single ball)
    pad.setSplitMode (false);
    splitToggle.setToggleState (false);
    linkButton.setVisible (false); // Initially hidden
    
    // Snap-to-grid button
    addAndMakeVisible (snapButton);
    snapButton.setButtonText("");
    snapButton.setToggleState(false, juce::dontSendNotification); // default OFF per requirements
    snapButton.onClick = [this] {
        bool currentState = snapButton.getToggleState();
        snapButton.setToggleState(!currentState, juce::dontSendNotification);
        pad.setSnapEnabled(!currentState);
    };
    
    // Setup numerical indicators with recessed styling
    addAndMakeVisible (leftIndicator);
    addAndMakeVisible (rightIndicator);
    leftIndicator.setJustificationType (juce::Justification::centred);
    rightIndicator.setJustificationType (juce::Justification::centred);
    leftIndicator.setColour (juce::Label::textColourId, lnf.theme.text);
    rightIndicator.setColour (juce::Label::textColourId, lnf.theme.text);
    leftIndicator.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    rightIndicator.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    leftIndicator.setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    rightIndicator.setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);



    addAndMakeVisible (pad);
    xyShade = std::make_unique<ShadeOverlay>(lnf);
    addAndMakeVisible(*xyShade);
    xyShade->setBounds(pad.getBounds());
    if (auto* v = proc.apvts.state.getPropertyPointer("ui_xyShadeAmt"))
        xyShade->setAmount((float)*v, false);
    xyShade->onAmountChanged = [this](float a){ proc.apvts.state.setProperty("ui_xyShadeAmt", a, nullptr); };
    
    // Setup control containers
    addAndMakeVisible (mainControlsContainer);
    mainControlsContainer.setTitle ("MAIN CONTROLS");
    
    // Setup individual containers for Pan, Space, Volume and EQ with blue hover borders
    addAndMakeVisible (panKnobContainer);
    addAndMakeVisible (spaceKnobContainer);
    addAndMakeVisible (volumeContainer);
    addAndMakeVisible (eqContainer);
    panKnobContainer.setTitle ("PAN");
    spaceKnobContainer.setTitle ("SPACE");
    volumeContainer.setTitle ("VOLUME");
    eqContainer.setTitle ("EQ");
    panKnobContainer.setShowBorder(true);
    spaceKnobContainer.setShowBorder(true);
    volumeContainer.setShowBorder(true);
    eqContainer.setShowBorder(true);
    mainControlsContainer.setShowBorder(false); // Parent container without border
    
    // Waveform display removed - now integrated into XYPad background
    
    pad.onChange = [this](float x01, float y01)
    {
        // Single ball mode - use normal pan and disable split mode
        if (auto* splitParam = proc.apvts.getParameter ("split_mode")) { splitParam->beginChangeGesture(); splitParam->setValueNotifyingHost (0.0f); splitParam->endChangeGesture(); }
        if (auto* panParam = proc.apvts.getParameter ("pan"))          { panParam->beginChangeGesture();  panParam->setValueNotifyingHost (x01); panParam->endChangeGesture(); }
        if (auto* depParam = proc.apvts.getParameter ("depth"))        { depParam->beginChangeGesture();  depParam->setValueNotifyingHost (y01); depParam->endChangeGesture(); }
        
        // Update waveform display parameters
        pad.setMixValue(proc.apvts.getRawParameterValue("sat_mix")->load());
        pad.setDriveValue(proc.apvts.getRawParameterValue("sat_drive_db")->load());
        pad.setTiltValue(proc.apvts.getRawParameterValue("tilt")->load());
        pad.setHPValue(proc.apvts.getRawParameterValue("hp_hz")->load());
        pad.setLPValue(proc.apvts.getRawParameterValue("lp_hz")->load());
        pad.setAirValue(proc.apvts.getRawParameterValue("air_db")->load());
        pad.setWidthValue(proc.apvts.getRawParameterValue("width")->load());
        pad.setPanValue(proc.apvts.getRawParameterValue("pan")->load());
    };
    
    pad.onSplitChange = [this](float leftX01, float rightX01, float y01)
    {
        // Handle split mode parameter updates - Ableton-style split panning
        // Map the split positions to proper pan values (-1.0 to 1.0)
        float leftPan = juce::jmap(leftX01, 0.0f, 1.0f, -1.0f, 1.0f);
        float rightPan = juce::jmap(rightX01, 0.0f, 1.0f, -1.0f, 1.0f);
        
        // Set split mode active and update individual pan parameters
        if (auto* splitParam = proc.apvts.getParameter ("split_mode")) { splitParam->beginChangeGesture(); splitParam->setValueNotifyingHost (1.0f); splitParam->endChangeGesture(); }
        if (auto* panLParam = proc.apvts.getParameter ("pan_l"))       { panLParam->beginChangeGesture();  panLParam->setValueNotifyingHost (juce::jmap(leftPan, -1.0f, 1.0f, 0.0f, 1.0f)); panLParam->endChangeGesture(); }
        if (auto* panRParam = proc.apvts.getParameter ("pan_r"))       { panRParam->beginChangeGesture();  panRParam->setValueNotifyingHost (juce::jmap(rightPan, -1.0f, 1.0f, 0.0f, 1.0f)); panRParam->endChangeGesture(); }
        if (auto* depParam = proc.apvts.getParameter ("depth"))        { depParam->beginChangeGesture();   depParam->setValueNotifyingHost (y01); depParam->endChangeGesture(); }
        
        // Waveform display removed - integrated into XYPad background
    };
    
    pad.onBallChange = [this](int ballIndex, float x01, float y01)
    {
        // Handle individual ball control in split mode
        if (ballIndex == 1) {
            // Left ball - could map to different parameters
            if (auto* panParam = proc.apvts.getParameter ("pan")) {
                panParam->beginChangeGesture();
                panParam->setValueNotifyingHost (juce::jmap(x01, 0.0f, 1.0f, -1.0f, 1.0f));
                panParam->endChangeGesture();
            }
        } else if (ballIndex == 2) {
            // Right ball - could map to different parameters
            if (auto* depParam = proc.apvts.getParameter ("depth")) {
                depParam->beginChangeGesture();
                depParam->setValueNotifyingHost (y01);
                depParam->endChangeGesture();
            }
        }
    };

    // Style regular knobs
    // Set knob names for internal labels
    width.setName("WIDTH");
    tilt.setName("TILT");
    monoHz.setName("MONO");
    hpHz.setName("HP");
    lpHz.setName("LP");
    satDrive.setName("DRIVE");
    satMix.setName("MIX");
    air.setName("AIR");
    bass.setName("BASS");
    scoop.setName("SCOOP"); // NEW: Scoop knob name
    panKnob.setName("PAN");
    spaceKnob.setName("SPACE");
    duckingKnob.setName("DUCK");
    
    // Style and add regular sliders
    for (juce::Slider* s : { &width,&tilt,&monoHz,&hpHz,&lpHz,&satDrive,&satMix,&air,&bass,&scoop }) { styleSlider(*s); addAndMakeVisible(*s); }
    
    // NEW: Style and add frequency control sliders (smaller, horizontal)
    for (juce::Slider* s : { &tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider }) { 
        s->setSliderStyle(juce::Slider::LinearHorizontal);
        s->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s->setMouseDragSensitivity(100); // More responsive to mouse movement
        s->setVelocityBasedMode(false); // Disable velocity mode for more direct control
        s->setRepaintsOnMouseActivity(true); // Repaint on mouse activity for smooth feedback
        s->setSliderSnapsToMousePosition(true); // Snap to mouse position for better control
        s->setPopupDisplayEnabled(false, false, nullptr); // Disable popup to avoid interference
        s->setWantsKeyboardFocus(false); // Disable keyboard focus to avoid interference
        s->setMouseClickGrabsKeyboardFocus(false); // Don't grab keyboard focus on click
        s->setInterceptsMouseClicks(true, true); // Ensure mouse clicks are intercepted
        s->setDoubleClickReturnValue(true, 0.0f); // Allow double-click to reset
        // Sizing happens in resized() only
        addAndMakeVisible(*s); 
    }
    
    // Style and add gain slider separately (it's a custom GainSlider)
    styleSlider(gain); addAndMakeVisible(gain);
    
    // Style main knobs (larger)
    styleMainSlider(panKnob); addAndMakeVisible(panKnob);
    panKnob.setOverlayEnabled(false); // stereo: normal slider only
    styleMainSlider(panKnobLeft); addAndMakeVisible(panKnobLeft);
    panKnobLeft.setOverlayEnabled(true);
    styleMainSlider(panKnobRight); addAndMakeVisible(panKnobRight);
    panKnobRight.setOverlayEnabled(true);
    
    // Set L/R labels for split pan knobs and configure ranges
    panKnobLeft.setLabel("L");
    panKnobRight.setLabel("R");
    
    // Set ranges for pan knobs (50L to 50R)
    panKnob.setRange(-1.0, 1.0, 0.01);
    panKnobLeft.setRange(-1.0, 1.0, 0.01);
    panKnobRight.setRange(-1.0, 1.0, 0.01);
    
    // Set initial visibility - start in stereo mode (single pan knob visible)
    panKnob.setVisible(true);
    panKnobLeft.setVisible(false);
    panKnobRight.setVisible(false);
    
    // Add value change listeners for linked pan knob behavior
    panKnobLeft.addListener(this);
    panKnobRight.addListener(this);
    
    styleMainSlider(spaceKnob); addAndMakeVisible(spaceKnob);
    
    // Setup space algorithm switch
    addAndMakeVisible(spaceAlgorithmSwitch);
    spaceAlgorithmSwitch.setGreenMode(isGreenMode);
    
    // Initialize switch with current parameter value
    if (auto* algoParam = proc.apvts.getRawParameterValue("space_algo")) {
        int idx = static_cast<int>(algoParam->load());
        idx = juce::jlimit(0, 2, idx);
        spaceAlgorithmSwitch.setAlgorithmFromParameter(idx);
    }
    
    spaceAlgorithmSwitch.onAlgorithmChange = [this](int algorithm) {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter("space_algo")))
        {
            const int maxIdx = juce::jmax(0, p->choices.size() - 1);
            const int idx    = juce::jlimit(0, maxIdx, algorithm);
            const float normalised = p->convertTo0to1((float) idx);

            p->beginChangeGesture();
            p->setValueNotifyingHost(normalised);
            p->endChangeGesture();
        }
    };
    
    // Style and add ducking knob (smaller custom design)
    addAndMakeVisible(duckingKnob);

    // Setup text labels for knobs
    // Text labels removed - now using internal knob labels
    
    // Row labels removed - using internal knob labels now
    
    // Setup numerical value displays with recessed styling
    for (juce::Label* l : { &gainValue,&widthValue,&tiltValue,&monoValue,&hpValue,&lpValue,&satDriveValue,&satMixValue,&airValue,&bassValue,&scoopValue,&panValue,&panValueLeft,&panValueRight,&spaceValue,&duckingValue }) {
        addAndMakeVisible(*l);
        l->setJustificationType (juce::Justification::centred);
        l->setColour (juce::Label::textColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    }
    
    // NEW: Setup frequency control value labels
    for (juce::Label* l : { &tiltFreqValue,&scoopFreqValue,&bassFreqValue,&airFreqValue }) {
        addAndMakeVisible(*l);
        l->setJustificationType (juce::Justification::centred);
        l->setColour (juce::Label::textColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    }
    
    // Set initial visibility for pan value labels - start in stereo mode
    panValue.setVisible(true);
    panValueLeft.setVisible(false);
    panValueRight.setVisible(false);
    
    // Setup lock buttons
    // Locks removed
    
    // Setup lock button onClick handlers
    // Locks removed



    // Setup oversampling (hidden, controlled via options menu)
    osSelect.addItemList (juce::StringArray { "Off", "2x", "4x" }, 1);
    osSelect.setSelectedId (1); // Default to Off

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    attachments.push_back (std::make_unique<SA> (proc.apvts, "gain_db", gain));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "width",   width));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "tilt",    tilt));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "mono_hz", monoHz));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "hp_hz",   hpHz));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "lp_hz",   lpHz));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "sat_drive_db", satDrive));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "sat_mix", satMix));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "air_db", air));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "bass_db", bass));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "scoop", scoop)); // NEW: Scoop attachment
    attachments.push_back (std::make_unique<SA> (proc.apvts, "ducking", duckingKnob));
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "bypass", bypassButton));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "pan", panKnob));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "pan_l", panKnobLeft));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "pan_r", panKnobRight));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "depth", spaceKnob));
    
    // NEW: Frequency control attachments
    attachments.push_back (std::make_unique<SA> (proc.apvts, "tilt_freq", tiltFreqSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "scoop_freq", scoopFreqSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "bass_freq", bassFreqSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "air_freq", airFreqSlider));
    
    // Missing parameter attachments for persistence
    attachments.push_back (std::make_unique<SA> (proc.apvts, "bass_db", bass));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "scoop", scoop));
    
    // Handle split mode parameter manually since ToggleSwitch doesn't inherit from Button
    proc.apvts.addParameterListener("split_mode", this);
    
    // Space knob attachment (for the depth value)
    attachments.push_back (std::make_unique<SA> (proc.apvts, "depth", spaceKnob));
    
    comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "os_mode", osSelect));

    startTimerHz (30);
    
    // Connect audio sample callback for XYPad background waveform
    proc.onAudioSample = [this](float sampleL, float sampleR) {
        pad.pushWaveformSample (sampleL, sampleR);
    };
    
    // Sync XY pad with current parameter values
    syncXYPadWithParameters();
    
    // Setup tooltips
    setupTooltips();
    
    // Search functionality now integrated into preset combo box
    
    // Apply a plugin-wide cursor policy: PointingHand for clickable/draggable controls
    applyGlobalCursorPolicy();
}
// Walk the component tree and set pointing-hand cursor for interactive items
void MyPluginAudioProcessorEditor::applyGlobalCursorPolicy()
{
    auto setCursorRecursive = [] (juce::Component& c, auto& setCursorRef) -> void
    {
        // Identify interactive components by type
        const bool isInteractive = dynamic_cast<juce::Button*>(&c) != nullptr
                                 || dynamic_cast<juce::Slider*>(&c) != nullptr
                                 || dynamic_cast<juce::ComboBox*>(&c) != nullptr
                                 || dynamic_cast<ToggleSwitch*>(&c) != nullptr
                                 || dynamic_cast<XYPad*>(&c) != nullptr;

        if (isInteractive)
            c.setMouseCursor(juce::MouseCursor::PointingHandCursor);

        for (int i = 0; i < c.getNumChildComponents(); ++i)
            setCursorRef(*c.getChildComponent(i), setCursorRef);
    };

    setCursorRecursive(*this, setCursorRecursive);
}

// WaveformDisplay implementation removed - functionality integrated into XYPad background

MyPluginAudioProcessorEditor::~MyPluginAudioProcessorEditor()
{
    // Remove listeners
    panKnobLeft.removeListener(this);
    panKnobRight.removeListener(this);
    
    // Remove parameter listeners
    proc.apvts.removeParameterListener("space_algo", this);
    proc.apvts.removeParameterListener("split_mode", this);
    proc.apvts.removeParameterListener("pan", this);
    proc.apvts.removeParameterListener("depth", this);
    
    // Save current state to A if we're in B state, so A always contains the final state
    if (!isStateA) {
        saveCurrentState(); // This saves to B, but we want A to have the final state
        // Copy B to A so A has the final state
        stateA = stateB;
    }
    
    // B state will revert to default when plugin is reopened
    // A state will persist as the plugin's current state
    
    setLookAndFeel (nullptr);
}

void MyPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Plugin background: top-to-bottom gradient (dark charcoal → lighter)
    auto full = getLocalBounds();
    juce::Colour topColour    = juce::Colour (0xFF2A2C30);
    juce::Colour midColour    = juce::Colour (0xFF4A4D55);
    juce::Colour bottomColour = juce::Colour (0xFF2A2C30); // return to dark at the very end
    juce::ColourGradient bg (topColour, (float) full.getCentreX(), (float) full.getY(),
                             bottomColour, (float) full.getCentreX(), (float) full.getBottom(), false);
    bg.addColour (0.85, midColour); // long mid ramp, snap back to dark near the end
    g.setGradientFill (bg);
    g.fillAll();
    
    // Draw branding header with Main Controls Container styling
    auto headerArea = getLocalBounds().removeFromTop (static_cast<int>(100 * scaleFactor)); // Increased to 100px to fit controls
    auto r = headerArea.toFloat();
    float radius = 8.0f; // Same radius as ControlContainer

    // Header background: no separate fill so the plugin gradient shows through
    
    // Removed header shadows and inner rim for a completely flat header background
    
    // Logo and tagline at top-left with minimal top spacing
    const int logoFontPx   = static_cast<int>(26 * scaleFactor); // slightly larger
    const int taglineFontPx= static_cast<int>(13 * scaleFactor); // slightly smaller for tight stack
    const int topPad       = static_cast<int>(4 * scaleFactor);  // reduced top spacing
    const int gap          = static_cast<int>(2 * scaleFactor);  // tighter gap between logo and tagline

    // Draw logo tightly at the top-left
    g.setColour (lnf.theme.text);
    juce::Font logoFont (juce::FontOptions ((float) logoFontPx).withStyle ("Bold"));
    g.setFont (logoFont);
    const int leftInset = Layout::dp(20, scaleFactor);
    auto logoArea = juce::Rectangle<int>(headerArea.getX() + leftInset, headerArea.getY() + topPad,
                                         headerArea.getWidth(), logoFontPx + 2);
    const juce::String logoText = "FIELD";
    g.drawText (logoText, logoArea, juce::Justification::centredLeft);

    // Version tag to the right of the logo (semantic version, small font)
    {
        const int logoTextW = (int) logoFont.getStringWidthFloat (logoText);
        const int versionFontPx = juce::jmax (9, (int) std::round (8 * scaleFactor));
        juce::Font versionFont (juce::FontOptions ((float) versionFontPx));
        g.setFont (versionFont);
        g.setColour (lnf.theme.textMuted);
        const juce::String versionText = " v" + juce::String (JUCE_STRINGIFY(JucePlugin_VersionString));
        const int vx = logoArea.getX() + logoTextW + Layout::dp (8, scaleFactor);
        const int vy = logoArea.getY() + (logoFontPx - versionFontPx) / 2 + 1;
        const int vw = (int) versionFont.getStringWidthFloat (versionText) + Layout::dp (8, scaleFactor);
        auto versionArea = juce::Rectangle<int> (vx, vy, vw, versionFontPx + 2);
        g.drawText (versionText, versionArea, juce::Justification::centredLeft);
    }

    // Draw tagline directly under the logo with small gap
    g.setColour (lnf.theme.textMuted);
    g.setFont (juce::Font (juce::FontOptions ((float) taglineFontPx).withStyle ("Bold")));
    auto taglineY = logoArea.getBottom() + gap;
    auto taglineArea = juce::Rectangle<int>(headerArea.getX() + leftInset, taglineY, headerArea.getWidth(), taglineFontPx + 2);
    g.drawText ("Spatial Audio Processor", taglineArea, juce::Justification::centredLeft);
    

    

    
    // Draw recessed location indicators
    if (leftIndicator.isVisible()) {
        drawRecessedLabel (g, leftIndicator.getBounds(), leftIndicator.getText(), true);
    }
    if (rightIndicator.isVisible()) {
        drawRecessedLabel (g, rightIndicator.getBounds(), rightIndicator.getText(), true);
    }
    
    // Draw integrated knob labels with values (active state based on lock status)
    bool isSplit = pad.getSplitMode();
    
    // Helper to format slider values inline (ensures visibility regardless of Label state)
    auto fmt = [] (juce::Slider& s, int decimals = 0, const juce::String& unit = juce::String()) {
        if (decimals <= 0)
            return juce::String (static_cast<int>(std::round(s.getValue()))) + unit;
        return juce::String(s.getValue(), decimals) + unit;
    };

    // (moved to paintOverChildren so values render above child components)
    
    // Draw separator lines between knob groups
    g.setColour (juce::Colour (0xFF6A6D75).withAlpha (0.8f)); // More visible separator color
    auto mainControlsBounds = mainControlsContainer.getBounds();
    
    // Find separator positions based on knob layout (Pan, Space, and Ducking moved to XY Pad area)
    if (mainControlsBounds.getWidth() > 0) {
        // Single separator: between Volume and EQ knobs
        auto sepX = mainControlsBounds.getX() + static_cast<int>(mainControlsBounds.getWidth() * 0.5f);
        g.drawLine (sepX, mainControlsBounds.getY() + 10, sepX, mainControlsBounds.getBottom() - 10, 2.0f);
    }
    

    
    // Draw resize handle in bottom right corner
    auto bounds = getLocalBounds();
    auto resizeArea = bounds.removeFromRight (20).removeFromBottom (20);
    
    // Draw diagonal lines for resize handle
    g.setColour (juce::Colour (0xFF6A6D75));
    for (int i = 0; i < 3; ++i) {
        int offset = i * 4;
        g.drawLine (resizeArea.getX() + resizeArea.getWidth() - 8 - offset, 
                   resizeArea.getY() + resizeArea.getHeight() - 4 - offset,
                   resizeArea.getX() + resizeArea.getWidth() - 4 - offset, 
                   resizeArea.getY() + resizeArea.getHeight() - 8 - offset, 1.0f);
    }
}

void MyPluginAudioProcessorEditor::resized()
{
    const float s = juce::jmax(0.6f, scaleFactor);
    auto r = getLocalBounds().reduced(Layout::dp(Layout::PAD, s)).withTrimmedBottom(50);

    // 1) Wood bar area with logo, tagline, and all controls - reduced height (half)
    auto woodBar = r.removeFromTop(Layout::dp(50, s));
    
    // Calculate positions for logo/tagline and controls within the wood bar
    auto logoHeight = static_cast<int>(24 * s);
    auto taglineHeight = static_cast<int>(14 * s);
    auto taglineY = woodBar.getY() + logoHeight + static_cast<int>(8 * s); // Reduced spacing

    // Position all controls flush with the top of the wood bar (no extra top spacing)
    auto controlsArea = woodBar;
    
    // Grid layout for controls within the wood bar
    juce::Grid headerGrid;
    headerGrid.rowGap = juce::Grid::Px(Layout::dp(4, s)); // Tighter gaps
    headerGrid.columnGap = juce::Grid::Px(Layout::dp(6, s)); // Tighter gaps
    headerGrid.alignContent = juce::Grid::AlignContent::center;
    headerGrid.justifyContent = juce::Grid::JustifyContent::center;
    headerGrid.alignItems = juce::Grid::AlignItems::center;
    headerGrid.justifyItems = juce::Grid::JustifyItems::center;
    headerGrid.templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };

    // columns:
    // [spacer] [bypass] [presetComboWide] [prev] [next] [save] [A] [B] [copy] [split] [spacer] [link] [spacer] [snap] [color] [fullscreen]
    headerGrid.templateColumns = {
        juce::Grid::TrackInfo(juce::Grid::Fr(1)),                 // left spacer
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))), // bypass (now large)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(400, s))),// preset combo (wider)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))), // prev (now large)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))), // next (now large)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))), // save (now large)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))), // A
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))), // B
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))), // copy
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(20, s))), // right spacer (reduced by half)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(120, s))),// split toggle (narrower)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))), // link (now large)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))), // snap (now large)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))), // color (now large)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s)))  // fullscreen (now large)
    };

    // Make sure mini widgets have explicit heights - smaller to fit in wood bar
    const int h = Layout::dp(24, s); // Reduced height
    bypassButton.setSize(Layout::dp(40, s), h);
    savePresetButton.setSize(Layout::dp(40, s), h);
    abButtonA.setSize(Layout::dp(40, s), h);
    abButtonB.setSize(Layout::dp(40, s), h);
    copyButton.setSize(Layout::dp(40, s), h);
    prevPresetButton.setSize(Layout::dp(40, s), h);
    nextPresetButton.setSize(Layout::dp(40, s), h);
    splitToggle.setSize(Layout::dp(120, s), Layout::dp(28, s)); // Toggle is now narrower
    linkButton.setSize(Layout::dp(40, s), h);
    snapButton.setSize(Layout::dp(40, s), h);
    colorModeButton.setSize(Layout::dp(40, s), h);
    fullScreenButton.setSize(Layout::dp(40, s), h);
    optionsButton.setSize(Layout::dp(40, s), h); // For manual positioning

    headerGrid.items = {
        juce::GridItem(), // left spacer
        juce::GridItem(bypassButton).withAlignSelf(juce::GridItem::AlignSelf::center),
        juce::GridItem(presetCombo).withAlignSelf(juce::GridItem::AlignSelf::center),
        juce::GridItem(prevPresetButton).withAlignSelf(juce::GridItem::AlignSelf::center),
        juce::GridItem(nextPresetButton).withAlignSelf(juce::GridItem::AlignSelf::center),
        juce::GridItem(savePresetButton).withAlignSelf(juce::GridItem::AlignSelf::center),
        juce::GridItem(abButtonA).withAlignSelf(juce::GridItem::AlignSelf::center),
        juce::GridItem(abButtonB).withAlignSelf(juce::GridItem::AlignSelf::center),
        juce::GridItem(copyButton).withAlignSelf(juce::GridItem::AlignSelf::center),
        juce::GridItem(), // right spacer
        juce::GridItem(splitToggle).withAlignSelf(juce::GridItem::AlignSelf::center),
        juce::GridItem(linkButton).withAlignSelf(juce::GridItem::AlignSelf::center),
        juce::GridItem(snapButton).withAlignSelf(juce::GridItem::AlignSelf::center),
        juce::GridItem(colorModeButton).withAlignSelf(juce::GridItem::AlignSelf::center),
        juce::GridItem(fullScreenButton).withAlignSelf(juce::GridItem::AlignSelf::center),
    };

    auto headerLayoutArea = controlsArea.reduced(Layout::dp(Layout::GAP, s), Layout::dp(6, s))
                                      .withTrimmedBottom(Layout::dp(8, s))
                                      .withTrimmedTop(Layout::dp(2, s));
    headerGrid.performLayout(headerLayoutArea);

    // Place options button at bottom-left corner of the window
    {
        auto bounds = getLocalBounds();
        const int padPx = Layout::dp(8, s);
        const int btnW = optionsButton.getWidth();
        const int btnH = optionsButton.getHeight();
        optionsButton.setBounds(bounds.getX() + padPx, bounds.getBottom() - btnH - padPx, btnW, btnH);
    }

    // Position a single bold vertical divider to the left of the split toggle
    {
        auto b = splitToggle.getBounds();
        const int lineH = Layout::dp(24, s); // twice as tall
        const int gapX = Layout::dp(8, s);   // bring divider farther from toggle
        auto centerY = b.getCentreY();
        splitDivider.setBounds(b.getX() - gapX - 1, centerY - lineH/2, 4, lineH); // twice as wide
        splitDivider.toFront(false);
    }

    // 2) Main content: XYPad fills area (right-side items will be moved to bottom row)
    {
        auto main = r.removeFromTop(juce::jmax(Layout::dp(400, s), r.getHeight() * 2 / 3));
        pad.setBounds(main.reduced(Layout::dp(Layout::GAP, s)));
        if (xyShade)
            xyShade->setBounds(pad.getBounds());
    }

    r.removeFromTop(Layout::dp(Layout::GAP, s));

    // 3) Bottom knobs: Volume row then EQ row (with micro-sliders above their parent knobs)
    {
        auto bottom = r;

        // Decide columns based on width
        const int knobTop    = Layout::dp(Layout::KNOB, s);
        const int knobBottom = Layout::dp(Layout::KNOB, s) + Layout::dp(20, s); // larger EQ row knobs
        const int gap  = Layout::dp(Layout::GAP, s);
        const int eqGap = juce::jmax(1, gap / 3); // tighter gaps for bottom row

        // Volume row (Pan/PanL+PanR, Space, Switch, Gain, Drive, Mix, Width, Duck)
        juce::Grid vol;
        vol.rowGap = juce::Grid::Px(gap); 
        vol.columnGap = juce::Grid::Px(gap);
        vol.templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
        const bool isSplitPan = pad.getSplitMode();
        vol.templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                                juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                                juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                                juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                                juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                                juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                                juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                                juce::Grid::TrackInfo(juce::Grid::Fr(1)) };

        // EQ row (Bass + micro, Air + micro, Tilt + micro, Scoop + micro, HP, LP, Mono)
        juce::Grid eq;
        eq.rowGap = juce::Grid::Px(eqGap); 
        eq.columnGap = juce::Grid::Px(eqGap);
        eq.templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
        eq.templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), // bass
                               juce::Grid::TrackInfo(juce::Grid::Fr(1)), // bass micro
                               juce::Grid::TrackInfo(juce::Grid::Fr(1)), // air
                               juce::Grid::TrackInfo(juce::Grid::Fr(1)), // air micro
                               juce::Grid::TrackInfo(juce::Grid::Fr(1)), // tilt
                               juce::Grid::TrackInfo(juce::Grid::Fr(1)), // tilt micro
                               juce::Grid::TrackInfo(juce::Grid::Fr(1)), // scoop
                               juce::Grid::TrackInfo(juce::Grid::Fr(1)), // scoop micro
                               juce::Grid::TrackInfo(juce::Grid::Fr(1)), // hp
                               juce::Grid::TrackInfo(juce::Grid::Fr(1)), // lp
                               juce::Grid::TrackInfo(juce::Grid::Fr(1)) };// mono

        auto volArea   = bottom.removeFromTop(knobTop + Layout::dp(70, s)); // room for labels inside your paint
        auto spacerArea= bottom.removeFromTop(Layout::dp(28, s));        // padding band between rows (increased)
        auto eqArea    = bottom;

        // Set sizes once; Grid will position them
        auto setKnobSizeTop = [&](juce::Component& c) { c.setBounds(0,0, knobTop, knobTop); };
        auto setKnobSizeBottom = [&](juce::Component& c) { c.setBounds(0,0, knobBottom, knobBottom); };

        // Top (volume) row sizes
        panKnob.setBounds(0,0, Layout::dp(150, s), Layout::dp(150, s));
        setKnobSizeTop(spaceKnob);
        spaceAlgorithmSwitch.setBounds(0,0, Layout::dp(56, s), knobTop);
        setKnobSizeTop(gain); setKnobSizeTop(satDrive); setKnobSizeTop(satMix); setKnobSizeTop(width); setKnobSizeTop(duckingKnob);

        // EQ sizes
        setKnobSizeBottom(bass); setKnobSizeBottom(air); setKnobSizeBottom(tilt); setKnobSizeBottom(scoop);
        setKnobSizeBottom(hpHz); setKnobSizeBottom(lpHz); setKnobSizeBottom(monoHz);

        if (isSplitPan)
            vol.items = {
                juce::GridItem(panSplitContainer).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(spaceKnob).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(spaceAlgorithmSwitch).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(duckingKnob).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(gain).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(satDrive).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(width).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(satMix).withAlignSelf(juce::GridItem::AlignSelf::center)
            };
        else
            vol.items = {
                juce::GridItem(panKnob).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(spaceKnob).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(spaceAlgorithmSwitch).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(duckingKnob).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(gain).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(satDrive).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(width).withAlignSelf(juce::GridItem::AlignSelf::center),
                juce::GridItem(satMix).withAlignSelf(juce::GridItem::AlignSelf::center)
            };

        eq.items = {
            juce::GridItem(bass).withAlignSelf(juce::GridItem::AlignSelf::center),
            juce::GridItem(bassFreqSlider).withAlignSelf(juce::GridItem::AlignSelf::center)
                                          .withWidth(Layout::dp(Layout::MICRO_W, s))
                                          .withHeight(Layout::dp(Layout::MICRO_H, s)),
            juce::GridItem(air).withAlignSelf(juce::GridItem::AlignSelf::center),
            juce::GridItem(airFreqSlider).withAlignSelf(juce::GridItem::AlignSelf::center)
                                         .withWidth(Layout::dp(Layout::MICRO_W, s))
                                         .withHeight(Layout::dp(Layout::MICRO_H, s)),
            juce::GridItem(tilt).withAlignSelf(juce::GridItem::AlignSelf::center),
            juce::GridItem(tiltFreqSlider).withAlignSelf(juce::GridItem::AlignSelf::center)
                                          .withWidth(Layout::dp(Layout::MICRO_W, s))
                                          .withHeight(Layout::dp(Layout::MICRO_H, s)),
            juce::GridItem(scoop).withAlignSelf(juce::GridItem::AlignSelf::center),
            juce::GridItem(scoopFreqSlider).withAlignSelf(juce::GridItem::AlignSelf::center)
                                            .withWidth(Layout::dp(Layout::MICRO_W, s))
                                            .withHeight(Layout::dp(Layout::MICRO_H, s)),
            juce::GridItem(hpHz).withAlignSelf(juce::GridItem::AlignSelf::center),
            juce::GridItem(lpHz).withAlignSelf(juce::GridItem::AlignSelf::center),
            juce::GridItem(monoHz).withAlignSelf(juce::GridItem::AlignSelf::center)
        };

        vol.performLayout(volArea.reduced(gap));

        // Place split pan knobs inside the container without changing grid footprint
        if (isSplitPan)
        {
            panSplitContainer.setVisible(true);
            panKnob.setVisible(false);
            auto c = panSplitContainer.getBounds();
            const int innerGap = juce::jmax(1, gap / 2);
            auto leftArea  = c.removeFromLeft(c.getWidth() / 2).reduced(innerGap / 2);
            auto rightArea = c.reduced(innerGap / 2);
            // Make squares centered in their halves
            int size = juce::jmin(leftArea.getWidth(), leftArea.getHeight());
            juce::Rectangle<int> leftKnob  = juce::Rectangle<int>(size, size).withCentre(leftArea.getCentre());
            int sizeR = juce::jmin(rightArea.getWidth(), rightArea.getHeight());
            juce::Rectangle<int> rightKnob = juce::Rectangle<int>(sizeR, sizeR).withCentre(rightArea.getCentre());
            panKnobLeft.setBounds(leftKnob);
            panKnobRight.setBounds(rightKnob);
            panKnobLeft.setVisible(true);
            panKnobRight.setVisible(true);
        }
        else
        {
            panSplitContainer.setVisible(false);
            panKnobLeft.setVisible(false);
            panKnobRight.setVisible(false);
            panKnob.setVisible(true);
        }
        eq.performLayout(eqArea.reduced(eqGap));

        // Request repaint of divider region; actual drawing occurs in paintOverChildren
        repaint (spacerArea);

        // Cache divider positions for paintOverChildren
        dividerVolBounds = volArea.reduced(gap);

        // micro slider value labels: place above each micro slider based on its bounds
        auto placeMicroLabel = [&](juce::Slider& micro, juce::Label& val) {
            const auto mb = micro.getBounds();
            const int mw = Layout::dp(Layout::MICRO_W, s);
            const int labelW = Layout::dp(40, s);
            const int labelH = Layout::dp(16, s);
            const int labelX = mb.getX() + (mw - labelW) / 2;
            const int labelY = mb.getY() - labelH - Layout::dp(2, s);
            val.setBounds(labelX, labelY, labelW, labelH);
        };

        // only these four have micros:
        placeMicroLabel(bassFreqSlider , bassFreqValue );
        placeMicroLabel(airFreqSlider  , airFreqValue  );
        placeMicroLabel(tiltFreqSlider , tiltFreqValue );
        placeMicroLabel(scoopFreqSlider, scoopFreqValue);
    }

    // show/hide pan value labels consistent with mode
    const bool split = pad.getSplitMode();
    panValue.setVisible(!split);
    panValueLeft.setVisible(split);
    panValueRight.setVisible(split);

    // containers are decorative only in your current paint flow
    mainControlsContainer.setBounds(juce::Rectangle<int>()); // keep hidden
    panKnobContainer.setVisible(false);
    spaceKnobContainer.setVisible(false);
    volumeContainer.setVisible(false);
    eqContainer.setVisible(false);
}

void MyPluginAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    auto bounds = getLocalBounds();
    auto resizeArea = bounds.removeFromRight (20).removeFromBottom (20);
    
    if (resizeArea.contains (e.position.toInt())) {
        isResizing = true;
        resizeStart = e.position.toInt();
        originalBounds = getBounds();
        setMouseCursor (juce::MouseCursor::BottomRightCornerResizeCursor);
    }
}

void MyPluginAudioProcessorEditor::mouseDrag (const juce::MouseEvent& e)
{
    if (isResizing) {
        auto delta = e.position.toInt() - resizeStart;

        // Compute a continuous scale from horizontal drag, clamped to supported range
        float newScale = (originalBounds.getWidth() + static_cast<float>(delta.x)) / static_cast<float>(baseWidth);
        newScale = juce::jlimit(0.5f, 2.0f, newScale);

        // Apply proportional size while preserving aspect ratio
        scaleFactor = newScale;
        const int newWidth  = static_cast<int>(baseWidth  * scaleFactor);
        const int newHeight = static_cast<int>(baseHeight * scaleFactor);
        setSize(newWidth, newHeight);
    }
}

void MyPluginAudioProcessorEditor::mouseUp (const juce::MouseEvent&)
{
    if (isResizing) {
        isResizing = false;
        setMouseCursor (juce::MouseCursor::NormalCursor);
    }
}

void MyPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    // Handle pan knob behavior in split mode
    if (pad.getSplitMode()) {
        if (pad.getLinked()) {
            // Locked mode - sync both knobs and balls
            if (slider == &panKnobLeft) {
                // Left knob changed - sync right knob to same value
                float leftValue = panKnobLeft.getValue();
                panKnobRight.setValue(leftValue, juce::dontSendNotification);
                
                // Update the XY pad balls to match
                float x01 = juce::jmap(leftValue, -1.0f, 1.0f, 0.0f, 1.0f);
                pad.setBallPosition(1, x01, pad.getPoint01().second);
                pad.setBallPosition(2, x01, pad.getPoint01().second);
            } else if (slider == &panKnobRight) {
                // Right knob changed - sync left knob to same value
                float rightValue = panKnobRight.getValue();
                panKnobLeft.setValue(rightValue, juce::dontSendNotification);
                
                // Update the XY pad balls to match
                float x01 = juce::jmap(rightValue, -1.0f, 1.0f, 0.0f, 1.0f);
                pad.setBallPosition(1, x01, pad.getPoint01().second);
                pad.setBallPosition(2, x01, pad.getPoint01().second);
            }
        } else {
            // Unlocked mode - independent control
            if (slider == &panKnobLeft) {
                // Left knob controls left ball only
                float leftValue = panKnobLeft.getValue();
                float x01 = juce::jmap(leftValue, -1.0f, 1.0f, 0.0f, 1.0f);
                pad.setBallPosition(1, x01, pad.getPoint01().second);
            } else if (slider == &panKnobRight) {
                // Right knob controls right ball only
                float rightValue = panKnobRight.getValue();
                float x01 = juce::jmap(rightValue, -1.0f, 1.0f, 0.0f, 1.0f);
                pad.setBallPosition(2, x01, pad.getPoint01().second);
            }
        }
    }
}

void MyPluginAudioProcessorEditor::timerCallback()
{
    // Handle header hover fade
    if (!headerHovered && headerHoverActive) {
        headerHoverActive = false;
        stopTimer();
        repaint();
        return;
    }
    

    
    // Get current parameter values from the processor
    auto* panParam   = proc.apvts.getRawParameterValue ("pan");
    auto* depthParam = proc.apvts.getRawParameterValue ("depth");
    if (! panParam || ! depthParam) return;

    float panValueRaw  = panParam->load();
    float x01  = juce::jmap (panValueRaw, -1.0f, 1.0f, 0.0f, 1.0f);
    float y01  = depthParam->load();

    // If snap is enabled, quantize both X and Y and write back to parameters so knobs respect snapping
    if (pad.getSnapEnabled())
    {
        float snappedX01 = std::round(x01 * 20.0f) / 20.0f; // 5-unit stereo steps
        float snappedY01 = std::round(y01 * 10.0f) / 10.0f; // 10% depth steps

        if (std::abs(snappedX01 - x01) > 0.0005f)
        {
            if (auto* panParamEditable = proc.apvts.getParameter ("pan"))
                panParamEditable->setValueNotifyingHost (snappedX01); // APVTS expects normalized 0..1
            x01 = snappedX01;
            panValueRaw = juce::jmap (snappedX01, 0.0f, 1.0f, -1.0f, 1.0f);
        }

        if (std::abs(snappedY01 - y01) > 0.0005f)
        {
            if (auto* depthParamEditable = proc.apvts.getParameter ("depth"))
                depthParamEditable->setValueNotifyingHost (snappedY01);
            y01 = snappedY01;
        }
    }

    // Sync XY pad with parameter changes (including from host automation)
    auto [curX, curY] = pad.getPoint01();
    if (std::abs (curX - x01) > 0.01f || std::abs (curY - y01) > 0.01f)
    {
        pad.setPoint01 (x01, y01);
    }
    
    // Update visual feedback from all parameters
    pad.setGainValue (gain.getValue());
    pad.setWidthValue (width.getValue());
    pad.setTiltValue (tilt.getValue());
    pad.setMixValue (satMix.getValue());
    pad.setDriveValue (satDrive.getValue());
    pad.setAirValue (air.getValue());
    pad.setBassValue (bass.getValue());
    pad.setScoopValue (scoop.getValue()); // NEW: Scoop value
    pad.setHPValue (hpHz.getValue());
    pad.setLPValue (lpHz.getValue());
    pad.setMonoValue (monoHz.getValue());
    pad.setSpaceValue (spaceKnob.getValue());
    pad.setSpaceAlgorithm (spaceAlgorithmSwitch.getAlgorithm());
    
    // NEW: Pass frequency control values to XYPad for visualization
    pad.setTiltFreqValue (tiltFreqSlider.getValue());
    pad.setScoopFreqValue (scoopFreqSlider.getValue());
    pad.setBassFreqValue (bassFreqSlider.getValue());
    pad.setAirFreqValue (airFreqSlider.getValue());
    
    // Update numerical indicators based on split mode (50L to 0 to 50R)
    if (pad.getSplitMode()) {
        // Split mode: show two indicators
        leftIndicator.setVisible (true);
        rightIndicator.setVisible (true);
        auto [leftX, rightX] = pad.getSplitPoints();
        
        // Convert X coordinates to 50L-0-50R range
        int leftVal = static_cast<int>(juce::jmap(leftX, 0.0f, 1.0f, -50.0f, 50.0f));
        int rightVal = static_cast<int>(juce::jmap(rightX, 0.0f, 1.0f, -50.0f, 50.0f));
        
        leftIndicator.setText (juce::String (leftVal) + "L", juce::dontSendNotification);
        rightIndicator.setText (juce::String (rightVal) + "R", juce::dontSendNotification);
        
        // Update Pan knob split percentage visualization
        float leftPercent = juce::jmap(leftX, 0.0f, 1.0f, 0.0f, 100.0f);
        float rightPercent = juce::jmap(rightX, 0.0f, 1.0f, 0.0f, 100.0f);
        panKnob.setSplitPercentage(leftPercent, rightPercent);
    } else {
        // Stereo mode: show single indicator
        leftIndicator.setVisible (true);
        rightIndicator.setVisible (false);
        
        // Convert X coordinate to 50L-0-50R range
        int panVal = static_cast<int>(juce::jmap(x01, 0.0f, 1.0f, -50.0f, 50.0f));
        leftIndicator.setText (juce::String (panVal), juce::dontSendNotification);
        
        // Clear Pan knob split percentage visualization
        panKnob.setSplitPercentage(-1.0f, -1.0f);
    }
    
    // Sync toggle state
    splitToggle.setToggleState (pad.getSplitMode(), juce::dontSendNotification);
    
    // Sync Space algorithm switch state
    if (auto* algoParam = proc.apvts.getRawParameterValue("space_algo")) {
        int algoValue = static_cast<int>(algoParam->load());
        if (algoValue != spaceAlgorithmSwitch.getAlgorithm()) {
            spaceAlgorithmSwitch.setAlgorithmFromParameter(algoValue);
        }
    }
    
    // Sync split mode state
    if (auto* splitParam = proc.apvts.getRawParameterValue("split_mode")) {
        bool splitValue = splitParam->load() > 0.5f;
        if (splitValue != splitToggle.getToggleState()) {
            splitToggle.setToggleState(splitValue, juce::dontSendNotification);
        }
    }
    
    // Update knob value displays
    gainValue.setText (juce::String (static_cast<int>(gain.getValue())) + "dB", juce::dontSendNotification);
    {
        const float widthPct = width.getValue() * 100.0f; // 0..200
        widthValue.setText (juce::String (widthPct, 1) + "%", juce::dontSendNotification);
    }
    tiltValue.setText (juce::String (static_cast<int>(tilt.getValue())) + "°", juce::dontSendNotification);
    monoValue.setText (juce::String (static_cast<int>(monoHz.getValue())) + "Hz", juce::dontSendNotification);
    hpValue.setText (juce::String (static_cast<int>(hpHz.getValue())) + "Hz", juce::dontSendNotification);
    lpValue.setText (juce::String (static_cast<int>(lpHz.getValue())) + "Hz", juce::dontSendNotification);
    satDriveValue.setText (juce::String (static_cast<int>(satDrive.getValue())) + "dB", juce::dontSendNotification);
    satMixValue.setText (juce::String (static_cast<int>(satMix.getValue() * 100.0f)) + "%", juce::dontSendNotification);
    airValue.setText (juce::String (static_cast<int>(air.getValue())) + "dB", juce::dontSendNotification);
    bassValue.setText (juce::String (static_cast<int>(bass.getValue())) + "dB", juce::dontSendNotification);
    scoopValue.setText (juce::String (static_cast<int>(scoop.getValue())) + "dB", juce::dontSendNotification); // NEW: Scoop value
    
    // Dynamic scoop knob label: "SCOOP" when negative, "BOOST" when positive
    float scoopValueFloat = scoop.getValue();
    if (scoopValueFloat > 0.0f) {
        scoop.setName("BOOST");
    } else {
        scoop.setName("SCOOP");
    }
    
    // NEW: Frequency control value labels
    tiltFreqValue.setText (juce::String (static_cast<int>(tiltFreqSlider.getValue())) + "Hz", juce::dontSendNotification);
    scoopFreqValue.setText (juce::String (static_cast<int>(scoopFreqSlider.getValue())) + "Hz", juce::dontSendNotification);
    bassFreqValue.setText (juce::String (static_cast<int>(bassFreqSlider.getValue())) + "Hz", juce::dontSendNotification);
    airFreqValue.setText (juce::String (static_cast<int>(airFreqSlider.getValue())) + "Hz", juce::dontSendNotification);
    
    // Update pan values based on mode
    if (pad.getSplitMode()) {
        // Get ball positions and update pan knobs
        auto [leftX, rightX] = pad.getSplitPoints();
        
        // Convert ball positions to pan values (-1 to 1)
        float leftPan = juce::jmap(leftX, 0.0f, 1.0f, -1.0f, 1.0f);
        float rightPan = juce::jmap(rightX, 0.0f, 1.0f, -1.0f, 1.0f);
        
        // Update pan knob values without triggering parameter change
        panKnobLeft.setValue(leftPan, juce::dontSendNotification);
        panKnobRight.setValue(rightPan, juce::dontSendNotification);
        
        // Convert to 50L..50R range for display
        int leftVal = static_cast<int>(juce::jmap(leftPan, -1.0f, 1.0f, -50.0f, 50.0f));
        int rightVal = static_cast<int>(juce::jmap(rightPan, -1.0f, 1.0f, -50.0f, 50.0f));
        panValueLeft.setText (juce::String (leftVal) + (leftVal < 0 ? "L" : "R"), juce::dontSendNotification);
        panValueRight.setText (juce::String (rightVal) + (rightVal < 0 ? "L" : "R"), juce::dontSendNotification);
    } else {
        // Convert -1..1 to 50L..50R range
        int panVal = static_cast<int>(juce::jmap(static_cast<float>(panKnob.getValue()), -1.0f, 1.0f, -50.0f, 50.0f));
        panValue.setText (juce::String (panVal) + (panVal < 0 ? "L" : (panVal > 0 ? "R" : "")), juce::dontSendNotification);
    }
    
    spaceValue.setText (juce::String (static_cast<int>(spaceKnob.getValue() * 100.0f)) + "%", juce::dontSendNotification);
    duckingValue.setText (juce::String (static_cast<int>(duckingKnob.getValue() * 100.0f)) + "%", juce::dontSendNotification);
}

// pushAudioSample function removed - waveform display integrated into XYPad background

void MyPluginAudioProcessorEditor::syncXYPadWithParameters()
{
    // Get current parameter values from the processor and sync the XY pad
    auto* panParam = proc.apvts.getRawParameterValue("pan");
    auto* depthParam = proc.apvts.getRawParameterValue("depth");
    
    if (panParam && depthParam)
    {
        float currentPan = panParam->load();
        float currentDepth = depthParam->load();
        
        // Convert pan (-1 to 1) to X coordinate (0 to 1)
        float x01 = (currentPan + 1.0f) * 0.5f;
        
        // Convert depth (0 to 1) to Y coordinate (0 to 1)
        float y01 = currentDepth;
        
        // Update the XY pad position
        pad.setPoint01(x01, y01);
    }
}

void MyPluginAudioProcessorEditor::setupTooltips()
{
    // Main controls
    panKnob.setTooltip("Pan: Controls left/right stereo positioning");
    spaceKnob.setTooltip("Space: Controls near/far depth positioning");
    
    // Volume section
    gain.setTooltip("Gain +: Output level adjustment (-24dB to +24dB)");
    satDrive.setTooltip("Drive: Saturation amount for harmonic enhancement");
    satMix.setTooltip("Mix: Blend between dry and saturated signal");
    
    // EQ section
    tilt.setTooltip("Tilt: Tone control - positive for warmth, negative for brightness");
    hpHz.setTooltip("High Pass: Removes frequencies below this point");
    lpHz.setTooltip("Low Pass: Removes frequencies above this point");
    
    // Placement section
    width.setTooltip("Width: Stereo width control - narrow to wide");
    monoHz.setTooltip("Mono: Frequencies below this point become mono (like Ableton's Utility)");
    
    // Note: Custom components (ToggleSwitch, XYPad, WaveformDisplay) don't support tooltips
    // Tooltips are available on standard JUCE sliders and controls above
}

void MyPluginAudioProcessorEditor::updateParameterLocks()
{
    // All controls remain enabled regardless of split state
    // This function is kept for potential future use
}



void MyPluginAudioProcessorEditor::drawRecessedLabel (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text, bool isActive)
{
    // Draw recessed background with rounded corners
    g.setColour (lnf.theme.panel);
    g.fillRoundedRectangle (bounds.toFloat(), 6.0f);
    
    // Draw outer border
    g.setColour (lnf.theme.sh.withAlpha (0.6f));
    g.drawRoundedRectangle (bounds.toFloat(), 6.0f, 1.0f);
    
    // Draw inner shadow for recessed effect
    g.setColour (lnf.theme.hl.withAlpha (0.3f));
    g.drawRoundedRectangle (bounds.toFloat().reduced (1.0f), 5.0f, 1.0f);
    
    // Draw text with larger font
    g.setColour (isActive ? lnf.theme.text : lnf.theme.textMuted);
    g.setFont (juce::Font (juce::FontOptions (13.0f * scaleFactor).withStyle ("Bold")));
    g.drawText (text, bounds, juce::Justification::centred);
}

void MyPluginAudioProcessorEditor::drawKnobWithIntegratedValue (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& knobName, const juce::String& value, bool isActive)
{
    // Draw recessed background with rounded corners
    g.setColour (lnf.theme.panel);
    g.fillRoundedRectangle (bounds.toFloat(), 6.0f);
    
    // Draw outer border
    g.setColour (lnf.theme.sh.withAlpha (0.6f));
    g.drawRoundedRectangle (bounds.toFloat(), 6.0f, 1.0f);
    
    // Draw inner shadow for recessed effect
    g.setColour (lnf.theme.hl.withAlpha (0.3f));
    g.drawRoundedRectangle (bounds.toFloat().reduced (1.0f), 5.0f, 1.0f);
    
    // Create text areas with adequate padding for two-line text
    auto textArea = bounds.reduced(6); // Slightly less padding to give more room for text
    int totalHeight = textArea.getHeight();
    int nameHeight = totalHeight / 2; // 50% for name
    int valueHeight = totalHeight / 2; // 50% for value
    
    // Top area for knob name
    auto nameBounds = juce::Rectangle<int>(textArea.getX(), textArea.getY(), textArea.getWidth(), nameHeight);
    
    // Bottom area for value (no gap)
    auto valueBounds = juce::Rectangle<int>(textArea.getX(), textArea.getY() + nameHeight, textArea.getWidth(), valueHeight);
    
    // Draw knob name (top)
    g.setColour (isActive ? lnf.theme.text : lnf.theme.textMuted);
    g.setFont (juce::Font (juce::FontOptions (11.0f * scaleFactor).withStyle ("Bold")));
    g.drawText (knobName, nameBounds, juce::Justification::centred);
    
    // Draw value (bottom) - use bright white color to ensure visibility with larger font
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions (12.0f * scaleFactor).withStyle ("Bold")));
    g.drawText (value, valueBounds, juce::Justification::centred);
}

void MyPluginAudioProcessorEditor::setScaleFactor (float newScale)
{
    scaleFactor = juce::jlimit (0.5f, 2.0f, newScale);
    
    // Call base class implementation first
    AudioProcessorEditor::setScaleFactor (newScale);
    
    int newWidth = static_cast<int> (baseWidth * scaleFactor);
    int newHeight = static_cast<int> (baseHeight * scaleFactor);
    
    setSize (newWidth, newHeight);
    repaint();
}



// Old applyPreset function removed - now using PresetManager system

void MyPluginAudioProcessorEditor::saveCurrentState()
{
    // Get current parameters
    std::map<juce::String, float> currentParams;
    currentParams["gain_db"] = gain.getValue();
    currentParams["width"] = width.getValue();
    currentParams["tilt"] = tilt.getValue();
    currentParams["mono_hz"] = monoHz.getValue();
    currentParams["hp_hz"] = hpHz.getValue();
    currentParams["lp_hz"] = lpHz.getValue();
    currentParams["sat_drive_db"] = satDrive.getValue();
    currentParams["sat_mix"] = satMix.getValue();
    currentParams["air_db"] = air.getValue();
    currentParams["bass_db"] = bass.getValue();
    currentParams["scoop"] = scoop.getValue(); // NEW: Scoop parameter
    currentParams["pan"] = panKnob.getValue();
    currentParams["depth"] = pad.getPoint01().second;
    currentParams["ducking"] = duckingKnob.getValue();
    
    // NEW: Frequency control parameters
    currentParams["tilt_freq"] = tiltFreqSlider.getValue();
    currentParams["scoop_freq"] = scoopFreqSlider.getValue();
    currentParams["bass_freq"] = bassFreqSlider.getValue();
    currentParams["air_freq"] = airFreqSlider.getValue();
    
    // Save to appropriate state
    if (isStateA) {
        stateA = currentParams;
        presetNameA = presetCombo.getText(); // Save current preset name
    } else {
        stateB = currentParams;
        presetNameB = presetCombo.getText(); // Save current preset name
    }
}

void MyPluginAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    // Removed horizontal divider between rows

    // Draw thin vertical dividers between Pan|Space, Switch|Duck, and Width|Mix in the top bottom-row grid
    if (!dividerVolBounds.isEmpty()) {
        const int colW = dividerVolBounds.getWidth() / 8;
        const int x1 = dividerVolBounds.getX() + colW;      // after Pan
        const int x2 = dividerVolBounds.getX() + colW * 4;  // after Space, Switch & Duck
        const int x3 = dividerVolBounds.getX() + colW * 7;  // after Width (before Mix)
        g.setColour(juce::Colour(0x40FFFFFF));
        g.drawLine((float)x1, (float)dividerVolBounds.getY()+4, (float)x1, (float)dividerVolBounds.getBottom()-4, 1.0f);
        g.drawLine((float)x2, (float)dividerVolBounds.getY()+4, (float)x2, (float)dividerVolBounds.getBottom()-4, 1.0f);
        g.drawLine((float)x3, (float)dividerVolBounds.getY()+4, (float)x3, (float)dividerVolBounds.getBottom()-4, 1.0f);
    }

    // Draw individual vertical dividers between each EQ bottom-row knob group (knob + its mini slider/label)
    {
        const float s = juce::jmax(0.6f, scaleFactor);
        struct Group {
            juce::Rectangle<int> rect;
        };
        auto groupRectOf = [] (juce::Component& knob, juce::Component* micro, juce::Component* label) {
            juce::Rectangle<int> r = knob.getBounds();
            if (micro != nullptr) r = r.getUnion(micro->getBounds());
            if (label != nullptr) r = r.getUnion(label->getBounds());
            return r;
        };

        juce::Array<Group> groups;
        groups.add({ groupRectOf(bass , &bassFreqSlider , &bassFreqValue ) });
        groups.add({ groupRectOf(air  , &airFreqSlider  , &airFreqValue  ) });
        groups.add({ groupRectOf(tilt , &tiltFreqSlider , &tiltFreqValue ) });
        groups.add({ groupRectOf(scoop, &scoopFreqSlider, &scoopFreqValue) });
        groups.add({ groupRectOf(hpHz , nullptr         , nullptr        ) });
        groups.add({ groupRectOf(lpHz , nullptr         , nullptr        ) });
        groups.add({ groupRectOf(monoHz, nullptr        , nullptr        ) });

        if (groups.size() > 1)
        {
            int eqTop = std::numeric_limits<int>::max();
            int eqBottom = std::numeric_limits<int>::min();
            for (const auto& gr : groups) {
                eqTop = juce::jmin(eqTop, gr.rect.getY());
                eqBottom = juce::jmax(eqBottom, gr.rect.getBottom());
            }
            const int topY = eqTop + 2;
            const int bottomY = eqBottom - 2;
            g.setColour(juce::Colour(0x40FFFFFF));
            for (int i = 0; i < groups.size() - 1; ++i) {
                // Skip dividers between HP|LP (i == 4) and LP|Mono (i == 5)
                if (i == 4 || i == 5) continue;
                int x = groups.getReference(i).rect.getRight() + Layout::dp(6, s);
                g.drawLine((float)x, (float)topY, (float)x, (float)bottomY, 1.0f);
            }
        }
    }

    // Overlay knob names and values above child components without occluding knobs
    const bool isSplit = pad.getSplitMode();
    auto fmt = [] (juce::Slider& s, int decimals = 0, const juce::String& unit = juce::String()) {
        return juce::String(s.getValue(), decimals) + unit;
    };
    auto drawLabels = [this, &g] (juce::Rectangle<int> bounds, const juce::String& name, const juce::String& value, bool isActive)
    {
        // Place label directly below the knob area (no extra bottom spacing)
        const float s = juce::jmax(0.6f, scaleFactor);
        const int labelY = bounds.getBottom();
        // Prepare font and compute text bounds with padding
        g.setFont (juce::Font (juce::FontOptions (14.0f * scaleFactor).withStyle ("Bold")));
        const auto font = g.getCurrentFont();
        const int textW = font.getStringWidth (value);
        const int textH = juce::roundToInt (font.getHeight());
        const int padPx = 5;
        auto panel = juce::Rectangle<int> (bounds.getCentreX() - (textW / 2) - padPx,
                                           labelY,
                                           textW + padPx * 2,
                                           textH + padPx * 2);

        // Recessed panel behind value (border + inner shadow), tight to text
        g.setColour (lnf.theme.panel.darker (0.5f));
        g.fillRoundedRectangle (panel.toFloat(), 6.0f);
        g.setColour (lnf.theme.sh.withAlpha (0.6f));
        g.drawRoundedRectangle (panel.toFloat(), 6.0f, 1.0f);
        g.setColour (lnf.theme.hl.withAlpha (0.3f));
        g.drawRoundedRectangle (panel.toFloat().reduced (1.0f), 5.0f, 1.0f);

        // Draw only the value, centered, using LookAndFeel text color
        g.setColour (isActive ? lnf.theme.text : lnf.theme.textMuted);
        g.drawText (value, panel, juce::Justification::centred);
    };

    // Volume
    drawLabels (gain.getBounds(),     juce::String(),  fmt(gain, 1, " dB"),      !isSplit);
    drawLabels (satDrive.getBounds(), juce::String(),  fmt(satDrive, 1, ""),     !isSplit);
    {
        const int mixPct = static_cast<int>(std::round(satMix.getValue() * 100.0f));
        drawLabels (satMix.getBounds(),   juce::String(),  juce::String(mixPct) + "%",    !isSplit);
    }
    {
        const float widthPct = width.getValue() * 100.0f;
        drawLabels (width.getBounds(), juce::String(), juce::String(widthPct, 1) + "%", !isSplit);
    }

    // EQ
    drawLabels (bass.getBounds(),  juce::String(),  fmt(bass, 1, " dB"),  !isSplit);
    drawLabels (air.getBounds(),   juce::String(),  fmt(air, 1, " dB"),   !isSplit);
    drawLabels (tilt.getBounds(),  juce::String(),  fmt(tilt, 1, " dB"),  !isSplit);
    drawLabels (scoop.getBounds(), juce::String(),  fmt(scoop, 1, " dB"), !isSplit);
    drawLabels (hpHz.getBounds(),  juce::String(),  fmt(hpHz, 0, " Hz"),  !isSplit);
    drawLabels (lpHz.getBounds(),  juce::String(),  fmt(lpHz, 0, " Hz"),  !isSplit);
    drawLabels (monoHz.getBounds(),juce::String(),  fmt(monoHz, 0, " Hz"), !isSplit);

    // XY Pad section
    if (pad.getSplitMode()) {
        auto formatPan = [] (juce::Slider& s, bool invertSuffix) {
            int v = static_cast<int>(std::round(juce::jmap(static_cast<float>(s.getValue()), -1.0f, 1.0f, -50.0f, 50.0f)));
            if (v == 0) return juce::String ("0");
            const int mag = std::abs(v);
            if (!invertSuffix)
                return juce::String(mag) + (v < 0 ? "L" : "R");
            else
                return juce::String(mag) + (v < 0 ? "R" : "L");
        };
        drawLabels (panKnobLeft.getBounds(),  juce::String(), formatPan(panKnobLeft, false),  !isSplit);
        drawLabels (panKnobRight.getBounds(), juce::String(), formatPan(panKnobRight, true),  !isSplit);
    } else {
        auto formatPanStereo = [] (juce::Slider& s) {
            int v = static_cast<int>(std::round(juce::jmap(static_cast<float>(s.getValue()), -1.0f, 1.0f, -50.0f, 50.0f)));
            if (v == 0) return juce::String ("0");
            const int mag = std::abs(v);
            return juce::String(mag) + (v < 0 ? "L" : "R");
        };
        drawLabels (panKnob.getBounds(),      juce::String(), formatPanStereo(panKnob),      !isSplit);
    }
    drawLabels (spaceKnob.getBounds(),   juce::String(), fmt(spaceKnob, 2, ""),  !isSplit);
    drawLabels (duckingKnob.getBounds(), juce::String(),  fmt(duckingKnob, 1, ""),!isSplit);
}

void MyPluginAudioProcessorEditor::loadState(bool loadStateA)
{
    auto& stateToLoad = loadStateA ? stateA : stateB;
    
    // Load all parameter values
    if (stateToLoad.find("gain_db") != stateToLoad.end()) gain.setValue(stateToLoad["gain_db"], juce::dontSendNotification);
    if (stateToLoad.find("width") != stateToLoad.end()) width.setValue(stateToLoad["width"], juce::dontSendNotification);
    if (stateToLoad.find("tilt") != stateToLoad.end()) tilt.setValue(stateToLoad["tilt"], juce::dontSendNotification);
    if (stateToLoad.find("mono_hz") != stateToLoad.end()) monoHz.setValue(stateToLoad["mono_hz"], juce::dontSendNotification);
    if (stateToLoad.find("hp_hz") != stateToLoad.end()) hpHz.setValue(stateToLoad["hp_hz"], juce::dontSendNotification);
    if (stateToLoad.find("lp_hz") != stateToLoad.end()) lpHz.setValue(stateToLoad["lp_hz"], juce::dontSendNotification);
    if (stateToLoad.find("sat_drive_db") != stateToLoad.end()) satDrive.setValue(stateToLoad["sat_drive_db"], juce::dontSendNotification);
    if (stateToLoad.find("sat_mix") != stateToLoad.end()) satMix.setValue(stateToLoad["sat_mix"], juce::dontSendNotification);
    if (stateToLoad.find("air_db") != stateToLoad.end()) air.setValue(stateToLoad["air_db"], juce::dontSendNotification);
    if (stateToLoad.find("bass_db") != stateToLoad.end()) bass.setValue(stateToLoad["bass_db"], juce::dontSendNotification);
    if (stateToLoad.find("scoop") != stateToLoad.end()) scoop.setValue(stateToLoad["scoop"], juce::dontSendNotification); // NEW: Scoop parameter
    if (stateToLoad.find("pan") != stateToLoad.end()) {
        float panValue = stateToLoad["pan"];
        panKnob.setValue(panValue, juce::dontSendNotification);
        // Update XY pad X position to match pan value
        float x01 = juce::jmap(panValue, -1.0f, 1.0f, 0.0f, 1.0f);
        pad.setPoint01(x01, pad.getPoint01().second);
    }
    if (stateToLoad.find("depth") != stateToLoad.end()) {
        float depthValue = stateToLoad["depth"];
        // Update XY pad Y position to match depth value
        pad.setPoint01(pad.getPoint01().first, depthValue);
    }
    if (stateToLoad.find("ducking") != stateToLoad.end()) duckingKnob.setValue(stateToLoad["ducking"], juce::dontSendNotification);
    
    // NEW: Frequency control parameters
    if (stateToLoad.find("tilt_freq") != stateToLoad.end()) tiltFreqSlider.setValue(stateToLoad["tilt_freq"], juce::dontSendNotification);
    if (stateToLoad.find("scoop_freq") != stateToLoad.end()) scoopFreqSlider.setValue(stateToLoad["scoop_freq"], juce::dontSendNotification);
    if (stateToLoad.find("bass_freq") != stateToLoad.end()) bassFreqSlider.setValue(stateToLoad["bass_freq"], juce::dontSendNotification);
    if (stateToLoad.find("air_freq") != stateToLoad.end()) airFreqSlider.setValue(stateToLoad["air_freq"], juce::dontSendNotification);
    
    // Update preset display to show the loaded state's preset name
    updatePresetDisplay();
}

void MyPluginAudioProcessorEditor::toggleABState()
{
    // Save current state before switching
    saveCurrentState();
    
    // Toggle state
    isStateA = !isStateA;
    abButtonA.setToggleState(isStateA, juce::dontSendNotification);
    abButtonB.setToggleState(!isStateA, juce::dontSendNotification);
    
    // Load the other state
    loadState(isStateA);
}

void MyPluginAudioProcessorEditor::copyState(bool copyFromA)
{
    auto& sourceState = copyFromA ? stateA : stateB;
    clipboardState = sourceState; // Copy to clipboard
}

void MyPluginAudioProcessorEditor::pasteState(bool pasteToA)
{
    auto& targetState = pasteToA ? stateA : stateB;
    targetState = clipboardState; // Paste from clipboard
    
    // Also copy the preset name
    if (pasteToA) {
        presetNameA = presetNameB; // Copy B's preset name to A
    } else {
        presetNameB = presetNameA; // Copy A's preset name to B
    }
    
    // If we're pasting to the currently active state, apply it immediately
    if ((pasteToA && isStateA) || (!pasteToA && !isStateA)) {
        loadState(isStateA);
    }
}

void MyPluginAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Handle parameter changes from the host
    if (parameterID == "space_algo") {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter("space_algo")))
        {
            const int idx = juce::roundToInt(p->convertFrom0to1(newValue));
            const int clamped = juce::jlimit(0, juce::jmax(0, p->choices.size() - 1), idx);
            spaceAlgorithmSwitch.setAlgorithmFromParameter(clamped);
        }
        return;
    }
    else if (parameterID == "split_mode") {
        splitToggle.setToggleState(newValue > 0.5f, juce::dontSendNotification);
        // Update the pad split mode to match
        pad.setSplitMode(newValue > 0.5f);
    }
    else if (parameterID == "pan") {
        // Update XY pad X position to match pan value
        float x01 = juce::jmap(newValue, -1.0f, 1.0f, 0.0f, 1.0f);
        pad.setPoint01(x01, pad.getPoint01().second);
    }
    else if (parameterID == "depth") {
        // Update XY pad Y position to match depth value
        pad.setPoint01(pad.getPoint01().first, newValue);
    }
}

void MyPluginAudioProcessorEditor::updatePresetDisplay()
{
    // Update the dropdown to show the current state's preset name
    juce::String currentPresetName = isStateA ? presetNameA : presetNameB;
    
    // Find the preset in the combo box and select it
    for (int i = 0; i < presetCombo.getNumItems(); ++i) {
        juce::String itemText = presetCombo.getItemText(i + 1);
        // Remove favorite star if present for comparison
        if (itemText.startsWith("★ ")) {
            itemText = itemText.substring(2);
        }
        
        if (itemText == currentPresetName) {
            presetCombo.setSelectedId(i + 1, juce::dontSendNotification);
            // Update the text display to show the correct preset name
            presetCombo.setText(itemText, juce::dontSendNotification);
            break;
        }
    }
    
    // If no matching preset found, set to default
    if (currentPresetName.isEmpty()) {
        presetCombo.setSelectedId(1, juce::dontSendNotification);
        if (presetCombo.getNumItems() > 0) {
            presetCombo.setText(presetCombo.getItemText(1), juce::dontSendNotification);
        }
    }
}

 