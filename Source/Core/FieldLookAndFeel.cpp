#include "FieldLookAndFeel.h"
#include "IconSystem.h"

// FieldLookAndFeel implementation - streamlined and organized
// Delegates to separated systems (FieldTheme, FieldMetallic, FieldRendering)

// Most rendering methods are now implemented in FieldRendering.cpp
// This file contains only the core LNF functionality and any remaining methods

// Additional LNF-specific methods that weren't moved to FieldRendering
void FieldLNF::drawNeoPanel(juce::Graphics& g, juce::Rectangle<float> r, float radius) const
{
    FieldRendering::drawNeoPanel(g, r, radius, theme);
}

void FieldLNF::paintCellPanel(juce::Graphics& g, juce::Component& c, bool showBorder, bool hover) const
{
    FieldRendering::paintCellPanel(g, c, showBorder, hover, theme);
}

void FieldLNF::drawRotationPad(juce::Graphics& g, juce::Rectangle<float> bounds, float rotationDeg, float asymmetry,
                               juce::Colour accent, juce::Colour text, juce::Colour panel) const
{
    FieldRendering::drawRotationPad(g, bounds, rotationDeg, asymmetry, accent, text, panel);
}

void FieldLNF::drawGainSlider(juce::Graphics& g, int x, int y, int w, int h, float sliderPosProportional,
                              float rotaryStartAngle, float rotaryEndAngle, float gainDb)
{
    FieldRendering::drawGainSlider(g, x, y, w, h, sliderPosProportional, rotaryStartAngle, rotaryEndAngle, gainDb, theme);
}

void FieldLNF::drawKnobLabel(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text)
{
    FieldRendering::drawKnobLabel(g, bounds, text, theme);
}

void FieldLNF::drawTabPill(juce::Graphics& g, juce::Rectangle<float> r, bool active) const
{
    FieldRendering::drawTabPill(g, r, active, theme);
}

// Metallic rendering delegates
void FieldLNF::paintMetal(juce::Graphics& g, const juce::Rectangle<float>& r,
                          const FieldTheme::MetalStops& metal, float corner)
{
    MetallicRenderer::paintMetal(g, r, metal, corner);
}

void FieldLNF::paintPhaseMetal(juce::Graphics& g, const juce::Rectangle<float>& r,
                               const MetallicRenderer::PhaseMetal& metal, float corner, float dpi)
{
    MetallicRenderer::paintPhaseMetal(g, r, metal, corner, dpi);
}
