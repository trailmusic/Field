#include "PaintManager.h"
#include "../../Core/PluginEditor.h"

PaintManager::PaintManager(MyPluginAudioProcessorEditor& editor)
    : editor(editor)
{
}

void PaintManager::paint(juce::Graphics& g)
{
    // Paint method - removed logging to prevent file I/O on every paint
    paintBackground(g);
    paintHeader(g);
    paintResizeHandle(g);
}

void PaintManager::paintBackground(juce::Graphics& g)
{
    // background gradient (toned down and tinted with accent color)
    auto full = editor.getLocalBounds();
    juce::Colour top    = editor.lnf.theme.sh.interpolatedWith(editor.lnf.theme.accent, 0.08f); // Subtle accent tint
    juce::Colour mid    = editor.lnf.theme.hl.darker(0.15f).interpolatedWith(editor.lnf.theme.accent, 0.12f); // Accent tint on middle
    juce::Colour bottom = editor.lnf.theme.sh.interpolatedWith(editor.lnf.theme.accent, 0.08f); // Subtle accent tint
    juce::ColourGradient bg (top, (float) full.getCentreX(), (float) full.getY(),
                             bottom, (float) full.getCentreX(), (float) full.getBottom(), false);
    bg.addColour (0.85, mid);
    g.setGradientFill (bg);
    g.fillAll();
}

void PaintManager::paintHeader(juce::Graphics& g)
{
    // logo + tagline
    auto header = editor.getLocalBounds().removeFromTop ((int) (100 * editor.scaleFactor));
    const int leftInset = (int) (20 * editor.scaleFactor);
    auto logoArea = juce::Rectangle<int> (header.getX() + leftInset, header.getY() + (int) (4 * editor.scaleFactor),
                                          header.getWidth(), (int) (30 * editor.scaleFactor + 2));
    
    // Enhanced FIELD logo with shadow and glow effects
    drawHeaderFieldLogo(g, logoArea.toFloat());

    // version - position after the actual logo width
    const juce::String ver = " v" + juce::String (JUCE_STRINGIFY (JucePlugin_VersionString));
    juce::Font vfont (juce::FontOptions (juce::jmax (9, (int) std::round (8 * editor.scaleFactor))));
    g.setFont (vfont);
    g.setColour (editor.lnf.theme.textMuted);
    
    // Calculate actual logo width and position version after it
    const float actualLogoWidth = juce::jmin(logoArea.getHeight() * 0.8f, 30.0f) * 2.5f;
    const int vx = logoArea.getX() + (int) actualLogoWidth + (int) (8 * editor.scaleFactor);
    const int vy = logoArea.getY() + (logoArea.getHeight() - vfont.getHeight()) * 0.5f + 1;
    g.drawText (ver, juce::Rectangle<int> (vx, vy, 120, (int) vfont.getHeight() + 2), juce::Justification::centredLeft);

    // tagline
    g.setColour (editor.lnf.theme.textMuted);
    g.setFont (juce::Font (juce::FontOptions (13.0f * editor.scaleFactor).withStyle ("Bold")));
    g.drawText ("Spatial Audio Processor",
                juce::Rectangle<int> (logoArea.getX(), logoArea.getBottom() + (int) (2 * editor.scaleFactor),
                                     header.getWidth(), (int) (14 * editor.scaleFactor + 2)),
                juce::Justification::centredLeft);
}

void PaintManager::paintResizeHandle(juce::Graphics& g)
{
    // resize handle
    auto bounds = editor.getLocalBounds();
    auto resizeArea = bounds.removeFromRight (20).removeFromBottom (20);
    g.setColour (editor.lnf.theme.textMuted);
    for (int i = 0; i < 3; ++i)
    {
        int off = i * 4;
        g.drawLine (resizeArea.getRight() - 8 - off, resizeArea.getBottom() - 4 - off,
                    resizeArea.getRight() - 4 - off, resizeArea.getBottom() - 8 - off, 1.0f);
    }
}

void PaintManager::drawHeaderFieldLogo (juce::Graphics& g, juce::Rectangle<float> area)
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
        g.setColour(editor.lnf.theme.accent.withAlpha(shadowAlpha * 0.8f));
        g.drawText("FIELD", logoRect.translated(shadowOffset, shadowOffset), 
                  juce::Justification::centredLeft);
        
        // Dark shadow for depth
        g.setColour(juce::Colours::black.withAlpha(shadowAlpha * 0.9f));
        g.drawText("FIELD", logoRect.translated(shadowOffset * 0.5f, shadowOffset * 0.5f), 
                  juce::Justification::centredLeft);
    }
    
    // Enhanced gradient effect for header (stronger)
    juce::ColourGradient logoGradient(
        editor.lnf.theme.accent.brighter(0.6f), logoRect.getX(), logoRect.getY(),
        editor.lnf.theme.accent.darker(0.3f), logoRect.getX(), logoRect.getBottom(), false);
    logoGradient.addColour(0.5f, editor.lnf.theme.accent);
    
    g.setGradientFill(logoGradient);
    g.drawText("FIELD", logoRect, juce::Justification::centredLeft);
    
    // Enhanced highlight for header (stronger)
    g.setColour(editor.lnf.theme.accent.brighter(0.7f).withAlpha(0.9f));
    g.drawText("FIELD", logoRect, juce::Justification::centredLeft);
    
    // Final white highlight for shine (stronger)
    g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.drawText("FIELD", logoRect, juce::Justification::centredLeft);
}
