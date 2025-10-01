#include "FieldMetallic.h"

// Metallic rendering system implementation
// Separated from FieldLookAndFeel for better organization

void MetallicRenderer::paintMetal(juce::Graphics& g, const juce::Rectangle<float>& r,
                                 const FieldTheme::MetalStops& metal, float corner)
{
    // Create gradient from top to bottom
    juce::ColourGradient gradient(metal.top, r.getX(), r.getY(),
                                 metal.bottom, r.getX(), r.getBottom(), false);
    
    // Apply tint if specified
    if (metal.tintAlpha > 0.0f)
    {
        gradient.addColour(0.5f, metal.tint.withAlpha(metal.tintAlpha));
    }
    
    g.setGradientFill(gradient);
    g.fillRect(r);  // Fill entire area first
    g.fillRoundedRectangle(r, corner);  // Then draw rounded rectangle
    
    // Add subtle border
    g.setColour(metal.bottom.darker(0.2f));
    g.drawRoundedRectangle(r, corner, 1.0f);
}

void MetallicRenderer::paintPhaseMetal(juce::Graphics& g, const juce::Rectangle<float>& r,
                                      const PhaseMetal& metal, float corner, float dpi)
{
    // Base gradient
    juce::ColourGradient gradient(metal.top, r.getX(), r.getY(),
                                 metal.bottom, r.getX(), r.getBottom(), false);
    g.setGradientFill(gradient);
    g.fillRect(r);  // Fill entire area first
    g.fillRoundedRectangle(r, corner);  // Then draw rounded rectangle
    
    // Airy tint overlay
    if (metal.airyAlpha > 0.0f)
    {
        g.setColour(metal.airyTint.withAlpha(metal.airyAlpha));
        g.fillRoundedRectangle(r, corner);
    }
    
    // Bottom multiply effect
    if (metal.bottomMulAlpha > 0.0f)
    {
        g.setColour(metal.bottomMul.withAlpha(metal.bottomMulAlpha));
        g.fillRoundedRectangle(r, corner);
    }
    
    // Sheen effect
    if (metal.sheenAlpha > 0.0f)
    {
        juce::ColourGradient sheen(metal.top.withAlpha(metal.sheenAlpha), r.getX(), r.getY(),
                                  juce::Colours::transparentBlack, r.getX(), r.getCentreY(), false);
        g.setGradientFill(sheen);
        g.fillRoundedRectangle(r, corner);
    }
    
    // Border
    g.setColour(metal.bottom.darker(0.3f));
    g.drawRoundedRectangle(r, corner, 1.5f);
}
