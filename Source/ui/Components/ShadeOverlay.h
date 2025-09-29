#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include "../../Core/FieldLookAndFeel.h"
#include "../Managers/PaneManager.h"

class ShadeOverlay : public juce::Component, private juce::Timer
{
public:
    ~ShadeOverlay() override { stopTimer(); }
    
    explicit ShadeOverlay (FieldLNF& lnfRef) : lnf(lnfRef)
    {
        setAlwaysOnTop(true);
        setInterceptsMouseClicks(true, true);
        amount.reset(0.0, 0.12);
        amount.setCurrentAndTargetValue(0.0f);
        startTimerHz(30);
    }
    
    // Set the PaneManager reference for getting active graphics container bounds
    void setPaneManager(PaneManager* pm) { paneManager = pm; }

    void setAmount (float a, bool animate = true)
    {
        a = juce::jlimit(0.f, 1.f, a);
        animate ? amount.setTargetValue(a) : amount.setCurrentAndTargetValue(a);
        if (onAmountChanged) onAmountChanged(getAmount());
        repaint();
    }
    float getAmount() const { return amount.getCurrentValue(); }
    void toggle(bool animate = true) { setAmount(getAmount() > 0.5f ? 0.f : 1.f, animate); }

    std::function<void(float)> onAmountChanged;

    bool hitTest (int x, int y) override
    {
        auto edge = juce::jlimit (0.0f, (float) getHeight(), shadeEdgeY());
        if (y <= edge) return true; // covered area blocks
        return getHandle().contains ((float) x, (float) y);
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat();
        
        // Get the shade area respecting the active graphics container bounds
        juce::Rectangle<float> shadeArea = r;
        if (paneManager)
        {
            auto activeContainerBounds = paneManager->getActiveGraphicsContainerBounds();
            if (!activeContainerBounds.isEmpty())
            {
                // Convert the graphics container bounds to ShadeOverlay local coordinates
                auto localContainerBounds = activeContainerBounds - r.getPosition().toInt();
                shadeArea = localContainerBounds.toFloat();
            }
        }
        
        const float coveredH = shadeArea.getHeight() * getAmount();
        const auto cover = shadeArea.withHeight(coveredH);

        if (coveredH > 0.001f)
        {
            g.setColour(lnf.theme.panel.withAlpha(0.92f));
            g.fillRect(cover);

            g.setColour(lnf.theme.sh.withAlpha(0.07f));
            for (int yy = 0; yy < (int)coveredH; yy += 3)
                g.drawHorizontalLine(yy, cover.getX(), cover.getRight());

            g.setColour(lnf.theme.sh.withAlpha(0.85f));
            g.fillRect(juce::Rectangle<float>(cover.getX(), cover.getBottom()-1.0f, cover.getWidth(), 1.0f));

            juce::DropShadow(juce::Colours::black.withAlpha(0.5f), 12, {0,2})
                .drawForRectangle(g, cover.getSmallestIntegerContainer());

            drawFieldLogo(g, cover);
        }

        drawHandle(g, getHandle());
    }

    void visibilityChanged() override { if (isVisible()) startTimerHz(30); else stopTimer(); }

    void mouseDown (const juce::MouseEvent& e) override { dragStartY = e.y; startAmt = amount.getTargetValue(); }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        const float dy = (float)(e.y - dragStartY);
        setAmount(juce::jlimit(0.f, 1.f, startAmt + dy / (float)getHeight()));
    }
    void mouseDoubleClick (const juce::MouseEvent&) override { toggle(); }
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wd) override
    {
        setAmount(juce::jlimit(0.f, 1.f, getAmount() - wd.deltaY * 0.5f));
    }
    void mouseMove (const juce::MouseEvent& e) override
    {
        const bool over = getHandle().contains (e.position.toFloat());
        if (over != hoverHandle)
        {
            hoverHandle = over;
            repaint();
        }
        setMouseCursor (over ? juce::MouseCursor::UpDownResizeCursor : juce::MouseCursor::NormalCursor);
    }
    void mouseExit (const juce::MouseEvent&) override
    {
        if (hoverHandle)
        {
            hoverHandle = false;
            repaint();
        }
        setMouseCursor (juce::MouseCursor::NormalCursor);
    }

private:
    FieldLNF& lnf;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amount;
    int   dragStartY = 0;
    float startAmt   = 0.f;
    bool  hoverHandle = false;
    PaneManager* paneManager = nullptr;

    float shadeEdgeY () const 
    { 
        // Get the shade edge position respecting the active graphics container bounds
        auto r = getLocalBounds().toFloat();
        
        // If we have a PaneManager reference, limit the shade to the container bounds
        if (paneManager)
        {
            auto activeContainerBounds = paneManager->getActiveGraphicsContainerBounds();
            if (!activeContainerBounds.isEmpty())
            {
                // Convert the graphics container bounds to ShadeOverlay local coordinates
                auto localContainerBounds = activeContainerBounds - r.getPosition().toInt();
                auto containerHeight = (float)localContainerBounds.getHeight();
                
                // Calculate shade edge within the container bounds
                return localContainerBounds.getY() + (containerHeight * getAmount());
            }
        }
        
        // Fallback to full area
        return r.getHeight() * getAmount();
    }

    juce::Rectangle<float> getHandle () const
    {
        auto r = getLocalBounds().toFloat();
        float tabW = juce::jmin(120.0f, r.getWidth() * 0.6f);
        float tabH = 20.0f;  // Reduced to 20.0f as per user request
        
        // Position handle so its TOP edge is pinned to the top of the active graphics container
        float y = 0.0f;  // Default to top of ShadeOverlay bounds
        float x = r.getCentreX() - tabW * 0.5f;  // Center horizontally
        
        // If we have a PaneManager reference, get the active graphics container bounds
        if (paneManager)
        {
            auto activeContainerBounds = paneManager->getActiveGraphicsContainerBounds();
            if (!activeContainerBounds.isEmpty())
            {
                // Convert the graphics container bounds to ShadeOverlay local coordinates
                // The graphics container is positioned within the ShadeOverlay bounds
                auto localContainerBounds = activeContainerBounds - r.getPosition().toInt();
                
                // For Dynamic EQ, Imager, and Machine, the tabs themselves are the graphics containers
                // The ShadeOverlay is positioned below the tab headers, so we need to position
                // the handle at the top of the ShadeOverlay bounds (y = 0) for these tabs
                // Check if the active graphics container is the same as the active pane component
                if (activeContainerBounds == paneManager->getBounds())
                {
                    y = 0.0f; // Position at top of ShadeOverlay bounds
                    x = r.getCentreX() - tabW * 0.5f; // Center horizontally
                }
                else
                {
                    // For other tabs, position at the top of the graphics container
                    y = (float)localContainerBounds.getY();
                    x = (float)localContainerBounds.getCentreX() - tabW * 0.5f;
                }
            }
        }
        
        return { x, y, tabW, tabH };
    }

    void drawHandle (juce::Graphics& g, juce::Rectangle<float> tab) const
    {
        // Base handle background
        g.setColour (lnf.theme.meters.trackBase.withAlpha (0.85f));
        g.fillRoundedRectangle (tab, 8.0f);
        
        // Hover effects with proper accent colors and outer glow
        if (hoverHandle)
        {
            // Use the theme accent color directly (now darker)
            auto accentColor = lnf.theme.accent;
            
            // True outer glow effect - draw multiple shadow layers for proper outer glow
            juce::DropShadow outerGlow1 (accentColor.withAlpha (0.4f), 20, {0, 0});
            outerGlow1.drawForRectangle (g, tab.getSmallestIntegerContainer());
            
            juce::DropShadow outerGlow2 (accentColor.withAlpha (0.2f), 12, {0, 0});
            outerGlow2.drawForRectangle (g, tab.getSmallestIntegerContainer());
            
            // Accent border using theme accent color
            g.setColour (accentColor.withAlpha (0.9f));
            g.drawRoundedRectangle (tab, 8.0f, 1.5f);
        }
        else
        {
            // Normal border using theme highlight
            g.setColour (lnf.theme.hl.withAlpha (0.6f));
            g.drawRoundedRectangle (tab, 8.0f, 1.0f);
        }

        // Dashed grip bars with hover accent
        const int numBars = 4;
        const float barW = 10.0f, barH = 6.0f, gap = 14.0f;
        const float totalW = numBars * barW + (numBars - 1) * gap;
        float startX = tab.getCentreX() - totalW * 0.5f;
        float y = tab.getCentreY() - barH * 0.5f;

        // Grip bars with theme accent color
        g.setColour (lnf.theme.accent.withAlpha (hoverHandle ? 0.9f : 0.7f));
        for (int i = 0; i < numBars; ++i)
        {
            juce::Rectangle<float> r (startX + i * (barW + gap), y, barW, barH);
            g.fillRoundedRectangle(r, 2.0f);
        }
    }

    void timerCallback() override
    {
        if (amount.isSmoothing()) repaint();
    }

    void drawFieldLogo (juce::Graphics& g, juce::Rectangle<float> area) const
    {
        // Calculate logo size based on covered area - increased to 80%
        const float logoHeight = juce::jmin(area.getHeight() * 0.8f, 200.0f);
        const float logoWidth = logoHeight * 2.5f; // FIELD is wider than tall
        
        // Center the logo in the covered area
        const float logoX = area.getCentreX() - logoWidth * 0.5f;
        const float logoY = area.getCentreY() - logoHeight * 0.5f;
        const auto logoRect = juce::Rectangle<float>(logoX, logoY, logoWidth, logoHeight);
        
        // Create large bold font matching the main logo
        juce::Font logoFont(juce::FontOptions(logoHeight * 0.8f).withStyle("Bold"));
        g.setFont(logoFont);
        
        // Enhanced shadow system with stronger effects (matching header)
        const int shadowLayers = 12; // Increased from 8 to 12
        for (int i = shadowLayers; i > 0; --i)
        {
            const float shadowOffset = (float)i * 3.5f; // Increased offset for more dramatic effect
            const float shadowAlpha = (1.0f - (float)i / shadowLayers) * 0.7f; // Increased alpha for stronger effect (matching header)
            
            // Multiple glow shadows with different colors and intensities
            // Outer accent glow (stronger)
            g.setColour(lnf.theme.accent.withAlpha(shadowAlpha * 0.8f));
            g.drawText("FIELD", logoRect.translated(shadowOffset, shadowOffset), 
                      juce::Justification::centred);
            
            // Secondary glow with brighter accent (stronger)
            g.setColour(lnf.theme.accent.brighter(0.4f).withAlpha(shadowAlpha * 0.6f));
            g.drawText("FIELD", logoRect.translated(shadowOffset * 0.8f, shadowOffset * 0.8f), 
                      juce::Justification::centred);
            
            // Dark shadow for depth with increased intensity (stronger)
            g.setColour(juce::Colours::black.withAlpha(shadowAlpha * 0.9f));
            g.drawText("FIELD", logoRect.translated(shadowOffset * 0.5f, shadowOffset * 0.5f), 
                      juce::Justification::centred);
            
            // Additional depth shadow (stronger)
            g.setColour(juce::Colours::darkgrey.withAlpha(shadowAlpha * 0.5f));
            g.drawText("FIELD", logoRect.translated(shadowOffset * 0.6f, shadowOffset * 0.6f), 
                      juce::Justification::centred);
        }
        
        // Enhanced gradient effect with stronger effects (matching header)
        juce::ColourGradient logoGradient(
            lnf.theme.accent.brighter(0.6f), logoRect.getX(), logoRect.getY(),
            lnf.theme.accent.darker(0.3f), logoRect.getX(), logoRect.getBottom(), false);
        logoGradient.addColour(0.25f, lnf.theme.accent.brighter(0.3f));
        logoGradient.addColour(0.5f, lnf.theme.accent);
        logoGradient.addColour(0.75f, lnf.theme.accent.darker(0.1f));
        
        g.setGradientFill(logoGradient);
        g.drawText("FIELD", logoRect, juce::Justification::centred);
        
        // Enhanced highlight system with stronger effects (matching header)
        // Primary highlight (stronger)
        g.setColour(lnf.theme.accent.brighter(0.7f).withAlpha(0.9f));
        g.drawText("FIELD", logoRect, juce::Justification::centred);
        
        // Secondary highlight for extra shine (stronger)
        g.setColour(lnf.theme.accent.brighter(0.9f).withAlpha(0.5f));
        g.drawText("FIELD", logoRect, juce::Justification::centred);
        
        // Final white highlight for maximum shine (stronger)
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.drawText("FIELD", logoRect, juce::Justification::centred);
    }
};
