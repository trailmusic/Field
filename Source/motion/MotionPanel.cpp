#include "MotionPanel.h"
#include "../ui/Layout.h"
using namespace UI;
namespace motion {

MotionPanel::MotionPanel(juce::AudioProcessorValueTreeState& s, juce::UndoManager*)
: state(s)
{
    // Start timer for real-time visual updates
    startTimerHz(60);
    
    // Initialize path points for smooth animation
    pathPoints.ensureStorageAllocated(64);
}

void MotionPanel::setVisualState(const VisualState& newState)
{
    visualState = newState;
    repaint(); // Trigger immediate repaint when visual state changes
}

void MotionPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff121317));
    
    if (orbBounds.isEmpty()) return;
    
    auto orb = orbBounds.toFloat();
    
    // Update legacy fields based on active panner for backward compatibility with existing draw methods
    const PannerViz& activePanner = (visualState.active == ActiveSel::P2) ? visualState.p2 : visualState.p1;
    
    legacyState.pathType = activePanner.pathType;
    legacyState.rate = activePanner.rateHz;
    legacyState.depth = activePanner.depth;
    legacyState.spread = activePanner.spread;
    legacyState.elevationBias = activePanner.elevBias;
    legacyState.frontBias = activePanner.frontBias;
    legacyState.swing = activePanner.swing;
    legacyState.inertia = activePanner.inertia;
    legacyState.anchor = activePanner.anchor;
    legacyState.quantizeDiv = activePanner.quantizeDiv;
    legacyState.mode = activePanner.mode;
    legacyState.retrig = activePanner.retrig;
    legacyState.holdMs = activePanner.holdMs;
    legacyState.motionSend = activePanner.motionSend;
    
    // Draw enable indicator first
    drawEnableIndicator(g, orb);
    
    // If motion is disabled, don't draw other elements
    if (!visualState.enable) {
        // Draw title
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.setFont(14.0f);
        g.drawFittedText("Field • Motion", orbBounds.removeFromTop(24), juce::Justification::centredTop, 1);
        return;
    }
    
    // Draw background elements
    drawOrbBackground(g, orb);
    drawElevationRings(g, orb);
    drawBassFloorRing(g, orb);
    drawAnchorCircle(g, orb);
    
    // Draw occlusion effect
    drawOcclusionEffect(g, orb);
    
    // Draw path preview
    drawPathPreview(g, orb);
    
    // Draw quantize grid if in sync mode
    if (legacyState.mode == 1) { // Sync mode
        drawQuantizeGrid(g, orb);
        // Draw swing grid if swing is active
        drawSwingGrid(g, orb);
    }
    
    // Draw inertia trail
    drawInertiaTrail(g, orb);
    
    // Draw panner dots
    drawPannerDots(g, orb);
    
    // Draw status indicators
    drawStatusIndicators(g, orb);
    
    // Draw title
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(14.0f);
    g.drawFittedText("Field • Motion", orbBounds.removeFromTop(24), juce::Justification::centredTop, 1);
    
    // Draw diagnostics (small monospace in corner)
    g.setColour(juce::Colours::yellow.withAlpha(0.7f));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain));
    juce::String diag = "seq:" + juce::String(visualState.seq) + 
                       " active:" + juce::String((int)visualState.active) +
                       (visualState.link ? " LINK" : "");
    g.drawFittedText(diag, orbBounds.removeFromTop(12), juce::Justification::centredTop, 1);
}

void MotionPanel::mouseDown (const juce::MouseEvent& e)
{
    if (orbBounds.isEmpty()) return;
    auto orb = orbBounds.toFloat();

    auto pt = e.position;

    auto hitCircle = [](juce::Point<float> p, juce::Point<float> c, float r)->bool {
        auto dx = p.x - c.x; auto dy = p.y - c.y; return (dx*dx + dy*dy) <= r*r;
    };

    // Compute positions for hit-testing
    auto p1Pos = polarToCartesian(visualState.p1.azimuth, visualState.p1.radius, orb);
    auto p2Pos = polarToCartesian(visualState.p2.azimuth, visualState.p2.radius, orb);
    auto linkPos = juce::Point<float> ( (p1Pos.x + p2Pos.x) * 0.5f, (p1Pos.y + p2Pos.y) * 0.5f );

    const float dotR   = 10.0f; // generous hit radius
    const float linkR  = 14.0f;

    using PS = motion::PannerSelect;

    if (hitCircle (pt, p1Pos, dotR))
    {
        if (auto* p = state.getParameter (motion::id::panner_select))
            p->setValueNotifyingHost ((float) (int) PS::P1 / 2.0f);
        return;
    }
    if (hitCircle (pt, p2Pos, dotR))
    {
        if (auto* p = state.getParameter (motion::id::panner_select))
            p->setValueNotifyingHost ((float) (int) PS::P2 / 2.0f);
        return;
    }
    if (hitCircle (pt, linkPos, linkR))
    {
        if (auto* p = state.getParameter (motion::id::panner_select))
            p->setValueNotifyingHost ((float) (int) PS::Link / 2.0f);
        return;
    }
}

void MotionPanel::drawOrbBackground(juce::Graphics& g, const juce::Rectangle<float>& orb)
{
    // Main orb background
    g.setColour(juce::Colours::white.withAlpha(0.07f));
    g.fillEllipse(orb);
    
    // Concentric rings
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    for (int i = 1; i < 6; ++i) {
        float t = i / 6.0f;
        auto r = orb.reduced(orb.getWidth() * t * 0.12f);
        g.drawEllipse(r, 1.0f);
    }
}

void MotionPanel::drawPathPreview(juce::Graphics& g, const juce::Rectangle<float>& orb)
{
    if (legacyState.pathType < 0 || legacyState.pathType > 7) return;
    
    g.setColour(getPathColor(legacyState.pathType).withAlpha(0.3f));
    
    // Generate path points based on current parameters
    pathPoints.clear();
    float dt = 0.02f;
    float t = 0.0f;
    
    for (int i = 0; i < 64; ++i) {
        float azimuth = 0.0f, radius = 0.0f;
        
        // Generate path based on type
        switch (legacyState.pathType) {
            case 0: // Circle
                azimuth = std::sin(2.0f * juce::MathConstants<float>::pi * legacyState.rate * t);
                radius = legacyState.depth;
                break;
            case 1: // Figure-8
                azimuth = std::sin(2.0f * juce::MathConstants<float>::pi * legacyState.rate * t);
                radius = legacyState.depth * (0.8f + 0.2f * std::sin(4.0f * juce::MathConstants<float>::pi * legacyState.rate * t));
                break;
            case 2: // Bounce
                azimuth = 2.0f * std::abs(std::fmod(legacyState.rate * t, 1.0f) - 0.5f) - 1.0f;
                radius = legacyState.depth;
                break;
            case 3: // Arc
                azimuth = 0.6f * (2.0f * std::abs(std::fmod(legacyState.rate * t, 1.0f) - 0.5f) - 1.0f);
                radius = legacyState.depth;
                break;
            case 4: // Spiral
                azimuth = std::sin(2.0f * juce::MathConstants<float>::pi * legacyState.rate * t);
                radius = legacyState.depth * (0.6f + 0.4f * std::sin(0.2f * 2.0f * juce::MathConstants<float>::pi * legacyState.rate * t));
                break;
            case 5: // Polygon
                {
                    float u = std::fmod(legacyState.rate * t, 1.0f);
                    int idx = int(u * 5);
                    float th = (idx / 5.0f) * 2.0f * juce::MathConstants<float>::pi;
                    azimuth = std::sin(th);
                    radius = legacyState.depth;
                }
                break;
            case 6: // Random Walk
                azimuth = std::sin(2.0f * juce::MathConstants<float>::pi * legacyState.rate * t + std::sin(t * 0.5f));
                radius = legacyState.depth;
                break;
            case 7: // User Shape
                azimuth = std::sin(2.0f * juce::MathConstants<float>::pi * legacyState.rate * t);
                radius = legacyState.depth;
                break;
        }
        
        auto point = polarToCartesian(azimuth, radius, orb);
        pathPoints.add(point);
        t += dt;
    }
    
    // Draw path
    if (pathPoints.size() > 1) {
        juce::Path path;
        path.startNewSubPath(pathPoints[0]);
        for (int i = 1; i < pathPoints.size(); ++i) {
            path.lineTo(pathPoints[i]);
        }
        g.strokePath(path, juce::PathStrokeType(2.0f));
    }
}

void MotionPanel::drawPannerDots(juce::Graphics& g, const juce::Rectangle<float>& orb)
{
    // P1 dot
    auto p1Pos = polarToCartesian(visualState.p1.azimuth, visualState.p1.radius, orb);
    g.setColour(juce::Colours::aqua.withAlpha(0.9f));
    g.fillEllipse(p1Pos.x - 4, p1Pos.y - 4, 8, 8);
    
    // P2 dot (if not in Link mode)
    if (visualState.active != ActiveSel::Link) {
        auto p2Pos = polarToCartesian(visualState.p2.azimuth, visualState.p2.radius, orb);
        g.setColour(juce::Colours::orange.withAlpha(0.9f));
        g.fillEllipse(p2Pos.x - 4, p2Pos.y - 4, 8, 8);
    }
    
    // Link halo when in Link mode
    if (visualState.active == ActiveSel::Link) {
        auto centerPos = polarToCartesian(0.5f * (visualState.p1.azimuth + visualState.p2.azimuth), 
                                         0.5f * (visualState.p1.radius + visualState.p2.radius), orb);
        g.setColour(juce::Colours::aqua.withAlpha(0.3f));
        g.drawEllipse(centerPos.x - 12, centerPos.y - 12, 24, 24, 2.0f);
    }
}

void MotionPanel::drawElevationRings(juce::Graphics& g, const juce::Rectangle<float>& orb)
{
    if (std::abs(legacyState.elevationBias) < 0.01f) return;
    
    float elevation = legacyState.elevationBias;
    juce::Colour ringColor = getElevationColor(elevation);
    
    // Draw elevation rings with intensity based on elevation
    float intensity = std::abs(elevation);
    for (int i = 1; i <= 3; ++i) {
        float t = i / 3.0f;
        auto ring = orb.reduced(orb.getWidth() * t * 0.08f);
        g.setColour(ringColor.withAlpha(0.1f * intensity * t));
        g.drawEllipse(ring, 1.0f);
    }
}

void MotionPanel::drawBassFloorRing(juce::Graphics& g, const juce::Rectangle<float>& orb)
{
    // Draw small inner ring showing bass floor zone
    float bassFloorRadius = 0.15f; // Relative to orb size
    auto bassRing = orb.reduced(orb.getWidth() * bassFloorRadius);
    
    g.setColour(juce::Colours::yellow.withAlpha(0.4f));
    g.drawEllipse(bassRing, 1.5f);
    
    // Add label
    g.setColour(juce::Colours::yellow.withAlpha(0.7f));
    g.setFont(10.0f);
    g.drawText("SUBS", bassRing, juce::Justification::centred);
}

void MotionPanel::drawAnchorCircle(juce::Graphics& g, const juce::Rectangle<float>& orb)
{
    if (!legacyState.anchor) return;
    
    float anchorRadius = 0.15f;
    auto anchorCircle = orb.reduced(orb.getWidth() * anchorRadius);
    
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawEllipse(anchorCircle, 1.0f);
    
    // Dashed line style
    juce::Path anchorPath;
    anchorPath.addEllipse(anchorCircle);
    g.strokePath(anchorPath, juce::PathStrokeType(1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void MotionPanel::drawStatusIndicators(juce::Graphics& g, const juce::Rectangle<float>&)
{
    auto statusArea = orbBounds.removeFromBottom(60);
    
    // Headphone Safe indicator
    if (visualState.headphoneSafe) {
        g.setColour(juce::Colours::green.withAlpha(0.8f));
        g.setFont(10.0f);
        g.drawText("HP SAFE", statusArea.removeFromTop(15), juce::Justification::centredLeft);
    }
    
    // Motion Send meter
    if (legacyState.motionSend > 0.001f) {
        auto meterArea = statusArea.removeFromTop(15);
        g.setColour(juce::Colours::blue.withAlpha(0.6f));
        g.fillRect(meterArea.removeFromLeft(meterArea.getWidth() * legacyState.motionSend));
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.drawRect(meterArea, 1.0f);
    }
    
    // Retrig indicator
    if (legacyState.retrig) {
        g.setColour(juce::Colours::red.withAlpha(0.8f));
        g.setFont(10.0f);
        g.drawText("RETRIG", statusArea.removeFromTop(15), juce::Justification::centredLeft);
    }
}

void MotionPanel::drawQuantizeGrid(juce::Graphics& g, const juce::Rectangle<float>& orb)
{
    if (legacyState.quantizeDiv == 0) return;
    
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    
    // Draw beat markers based on quantize division
    int divisions = 4; // Default to quarter notes
    switch (legacyState.quantizeDiv) {
        case 1: divisions = 1; break;   // 1/1
        case 2: divisions = 2; break;   // 1/2
        case 3: divisions = 4; break;   // 1/4
        case 4: divisions = 8; break;   // 1/8
        case 5: divisions = 16; break;  // 1/16
        case 6: divisions = 32; break;  // 1/32
        default: divisions = 4; break;
    }
    
    for (int i = 0; i < divisions; ++i) {
        float angle = (i / (float)divisions) * 2.0f * juce::MathConstants<float>::pi;
        float x = orb.getCentreX() + std::cos(angle) * orb.getWidth() * 0.4f;
        float y = orb.getCentreY() + std::sin(angle) * orb.getHeight() * 0.4f;
        g.fillEllipse(x - 2, y - 2, 4, 4);
    }
}

juce::Point<float> MotionPanel::polarToCartesian(float azimuth, float radius, const juce::Rectangle<float>& orb)
{
    float x = orb.getCentreX() + azimuth * radius * orb.getWidth() * 0.4f;
    float y = orb.getCentreY() + radius * orb.getHeight() * 0.4f;
    return {x, y};
}

juce::Colour MotionPanel::getElevationColor(float elevation)
{
    if (elevation > 0) {
        return juce::Colours::lightblue.withAlpha(0.6f); // Up
    } else {
        return juce::Colours::darkblue.withAlpha(0.6f);  // Down
    }
}

juce::Colour MotionPanel::getPathColor(int pathType)
{
    switch (pathType) {
        case 0: return juce::Colours::aqua;      // Circle
        case 1: return juce::Colours::orange;    // Figure-8
        case 2: return juce::Colours::yellow;    // Bounce
        case 3: return juce::Colours::green;     // Arc
        case 4: return juce::Colours::purple;    // Spiral
        case 5: return juce::Colours::red;       // Polygon
        case 6: return juce::Colours::pink;      // Random Walk
        case 7: return juce::Colours::white;     // User Shape
        default: return juce::Colours::aqua;
    }
}

void MotionPanel::resized()
{
    auto area = getLocalBounds().reduced(pad);
    int orbSize = juce::jmin(area.getWidth(), area.getHeight() - pad);
    orbBounds = area.withSizeKeepingCentre(orbSize, orbSize);
}

void MotionPanel::drawInertiaTrail(juce::Graphics& g, const juce::Rectangle<float>& orb)
{
    if (legacyState.inertia < 0.001f) return;
    
    // Draw fading trail behind the main dot
    float trailLength = juce::jmap(legacyState.inertia, 0.0f, 500.0f, 0.0f, 0.3f);
    if (trailLength > 0.01f) {
        auto p1Pos = polarToCartesian(visualState.p1.azimuth, visualState.p1.radius, orb);
        auto p2Pos = polarToCartesian(visualState.p2.azimuth, visualState.p2.radius, orb);
        
        // Draw trail for P1
        g.setColour(juce::Colours::cyan.withAlpha(0.3f * trailLength));
        g.fillEllipse(p1Pos.x - 3, p1Pos.y - 3, 6, 6);
        
        // Draw trail for P2 if not linked
        if (visualState.active != ActiveSel::Link) {
            g.setColour(juce::Colours::magenta.withAlpha(0.3f * trailLength));
            g.fillEllipse(p2Pos.x - 3, p2Pos.y - 3, 6, 6);
        }
    }
}

void MotionPanel::drawSwingGrid(juce::Graphics& g, const juce::Rectangle<float>& orb)
{
    if (legacyState.swing < 0.001f || legacyState.mode != 1) return; // Only in Sync mode
    
    // Draw swing timing grid
    float swingAmount = legacyState.swing;
    g.setColour(juce::Colours::yellow.withAlpha(0.4f * swingAmount));
    
    // Draw off-beat markers shifted by swing amount
    float centerX = orb.getCentreX();
    float centerY = orb.getCentreY();
    float radius = orb.getWidth() * 0.4f;
    
    for (int i = 0; i < 8; ++i) {
        float angle = i * juce::MathConstants<float>::pi / 4.0f;
        float swingOffset = (i % 2 == 1) ? swingAmount * 0.1f : 0.0f; // Off-beats get swing
        angle += swingOffset;
        
        float x = centerX + radius * std::cos(angle);
        float y = centerY + radius * std::sin(angle);
        
        g.fillEllipse(x - 1, y - 1, 2, 2);
    }
}

void MotionPanel::drawOcclusionEffect(juce::Graphics& g, const juce::Rectangle<float>& orb)
{
    if (visualState.occlusion < 0.001f) return;
    
    // Draw rear hemisphere darkening
    float occlusionAmount = visualState.occlusion;
    g.setColour(juce::Colours::black.withAlpha(0.3f * occlusionAmount));
    
    // Draw semi-circle for rear hemisphere
    auto rearArc = orb.reduced(orb.getWidth() * 0.1f);
    juce::Path rearPath;
    rearPath.addCentredArc(rearArc.getCentreX(), rearArc.getCentreY(),
                          rearArc.getWidth() * 0.5f, rearArc.getHeight() * 0.5f,
                          0.0f, juce::MathConstants<float>::halfPi, 
                          juce::MathConstants<float>::pi + juce::MathConstants<float>::halfPi, true);
    g.fillPath(rearPath);
    
    // Draw LPF cutoff indicator
    float cutoffFreq = juce::jmap(occlusionAmount, 0.0f, 1.0f, 12000.0f, 2500.0f);
    float cutoffRadius = juce::jmap(cutoffFreq, 2500.0f, 12000.0f, 0.8f, 0.2f);
    
    g.setColour(juce::Colours::orange.withAlpha(0.6f * occlusionAmount));
    g.drawEllipse(orb.reduced(orb.getWidth() * cutoffRadius), 2.0f);
}

void MotionPanel::drawEnableIndicator(juce::Graphics& g, const juce::Rectangle<float>& orb)
{
    if (!visualState.enable) {
        // Draw disabled state - desaturated and dimmed
        g.setColour(juce::Colours::grey.withAlpha(0.3f));
        g.fillEllipse(orb);
        
        // Draw "OFF" text
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(12.0f);
        g.drawText("MOTION OFF", orb, juce::Justification::centred);
    } else {
        // Draw small power indicator
        auto powerRect = orb.withSizeKeepingCentre(20, 20).translated(orb.getWidth() * 0.35f, -orb.getHeight() * 0.35f);
        g.setColour(juce::Colours::green.withAlpha(0.8f));
        g.fillEllipse(powerRect);
        
        g.setColour(juce::Colours::white);
        g.setFont(10.0f);
        g.drawText("ON", powerRect, juce::Justification::centred);
    }
}

}