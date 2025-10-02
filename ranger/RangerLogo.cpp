#include "RangerLogo.h"

RangerLogo::RangerLogo()
{
    setSize(300, 80);
}

RangerLogo::~RangerLogo()
{
}

void RangerLogo::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw Field-style background panel
    drawFieldStylePanel(g, bounds, 12.0f);
    
    // Draw the logo
    drawFieldStyleLogo(g, bounds);
    
    // Draw metallic effect
    drawMetallicEffect(g, bounds);
    
    // Draw tagline
    drawTagline(g, bounds);
}

void RangerLogo::resized()
{
    // Logo automatically sizes itself
}

void RangerLogo::setTheme(const SimpleFieldTheme& theme)
{
    currentTheme = theme;
    repaint();
}

void RangerLogo::drawFieldStylePanel(juce::Graphics& g, juce::Rectangle<float> r, float radius)
{
    auto inner = r.reduced(3.0f);

    // Main panel background
    g.setColour(currentTheme.panel);
    g.fillRoundedRectangle(inner, radius);

    // Enhanced shadows for more depth
    juce::DropShadow deepShadow(currentTheme.shadowDark.withAlpha(0.6f), 12, {-2, -2});
    juce::DropShadow lightShadow(currentTheme.shadowLight.withAlpha(0.4f), 6, {-1, -1});

    const auto shadowRect = inner.getSmallestIntegerContainer();
    deepShadow.drawForRectangle(g, shadowRect);
    lightShadow.drawForRectangle(g, shadowRect);

    // Subtle inner rim for inset effect
    g.setColour(currentTheme.sh.withAlpha(0.2f));
    g.drawRoundedRectangle(inner.reduced(1.0f), juce::jmax(0.0f, radius - 1.0f), 1.0f);
}

void RangerLogo::drawFieldStyleLogo(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Main logo text
    g.setColour(currentTheme.text);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    
    auto logoBounds = bounds.removeFromTop(bounds.getHeight() * 0.6f);
    logoBounds = logoBounds.reduced(20, 10);
    
    g.drawText(logoText, logoBounds, juce::Justification::centred);
    
    // Accent line
    g.setColour(currentTheme.accent);
    g.fillRect(static_cast<int>(logoBounds.getX()), static_cast<int>(logoBounds.getBottom() - 2), 
               static_cast<int>(logoBounds.getWidth()), 2);
}

void RangerLogo::drawMetallicEffect(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Subtle metallic gradient overlay
    juce::ColourGradient gradient;
    gradient.addColour(0.0, currentTheme.accent.withAlpha(0.1f));
    gradient.addColour(0.5, juce::Colours::transparentBlack);
    gradient.addColour(1.0, currentTheme.accent.withAlpha(0.05f));
    
    gradient.point1 = bounds.getTopLeft();
    gradient.point2 = bounds.getBottomRight();
    
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds.reduced(2), 10);
}

void RangerLogo::drawTagline(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Tagline text
    g.setColour(currentTheme.textMuted);
    g.setFont(juce::Font(12.0f));
    
    auto taglineBounds = bounds.removeFromBottom(bounds.getHeight() * 0.4f);
    taglineBounds = taglineBounds.reduced(20, 5);
    
    g.drawText(tagline, taglineBounds, juce::Justification::centred);
}
