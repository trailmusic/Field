#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Core/FieldLookAndFeel.h"

class SwitchCell : public juce::Component
{
public:
    explicit SwitchCell(juce::Component& contentToHost) : content(contentToHost)
    {
        setOpaque(false);
        caption.setJustificationType (juce::Justification::centred);
        caption.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (caption);
        // If content is a ToggleButton, clear text to prefer icon-only LNF drawing
        if (auto* tb = dynamic_cast<juce::ToggleButton*>(&content)) tb->setButtonText("");
    }
    void setMetrics (int /*knobPx*/, int /*valuePx*/, int /*gapPx*/) { resized(); }
    void setShowBorder (bool show) { showBorder = show; repaint(); }
    void setDelayTheme (bool on) { isDelayTheme = on; repaint(); }
    void setCaption (const juce::String& text)
    {
        captionText = text;
        caption.setText (captionText, juce::dontSendNotification);
        repaint();
    }
    void resized() override
    {
        if (content.getParentComponent() != this)
            addAndMakeVisible (content);
        auto b = getLocalBounds().reduced (6); // inset to reveal panel border fully
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        // Caption height
        const int capH = captionText.isNotEmpty() ? 14 : 0;
        if (captionText.isNotEmpty())
        {
            caption.setBounds (b.removeFromTop (capH));
            if (lf) caption.setColour (juce::Label::textColourId, lf->theme.textMuted);
        }
        content.setBounds (b);
    }
    void paint (juce::Graphics& g) override
    {
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            const bool metallic = (bool) getProperties().getWithDefault ("metallic", false);
            const bool motionGreen = (bool) getProperties().getWithDefault ("motionPurpleBorder", (bool) getProperties().getWithDefault ("motionGreenBorder", false));
            const bool reverbMaroon = (bool) getProperties().getWithDefault ("reverbMaroonBorder", false);
            
            if (isDelayTheme)
            {
                auto r = getLocalBounds().toFloat().reduced (3.0f);
                auto panel  = lf->theme.panel.brighter (0.10f);
                auto border = lf->theme.text; // use default font grey for border
                g.setColour (panel);  g.fillRoundedRectangle (r, 8.0f);
                if (showBorder) { g.setColour (border); g.drawRoundedRectangle (r, 8.0f, 1.5f); }
            }
            else if (motionGreen)
            {
                // Custom paint for Motion cells with deep blue/purple border
                auto r = getLocalBounds().toFloat();
                const float rad = 8.0f;
                const bool metallicOn = (bool) getProperties().getWithDefault ("metallic", false);

                auto rr = r.reduced (3.0f);
                if (metallicOn)
                {
                    // Bluish-purple metallic gradient from theme
                    juce::Colour top = lf->theme.motionPanelTop;
                    juce::Colour bot = lf->theme.motionPanelBot;
                    juce::ColourGradient grad (top, rr.getX(), rr.getY(), bot, rr.getX(), rr.getBottom(), false);
                    g.setGradientFill (grad);
                    g.fillRoundedRectangle (rr, rad);

                    // Subtle vignette for depth
                    juce::ColourGradient vg (juce::Colours::transparentBlack, rr.getCentreX(), rr.getCentreY(),
                                             juce::Colours::black.withAlpha (0.22f), rr.getCentreX(), rr.getCentreY() - rr.getHeight() * 0.6f, true);
                    g.setGradientFill (vg);
                    g.fillRoundedRectangle (rr, rad);
                }
                else
                {
                    g.setColour (lf->theme.panel);
                    g.fillRoundedRectangle (rr, rad);
                }

                juce::DropShadow ds1 (lf->theme.shadowDark.withAlpha (0.35f), 12, { -1, -1 });
                juce::DropShadow ds2 (lf->theme.shadowLight.withAlpha (0.25f),  6, { -1, -1 });
                ds1.drawForRectangle (g, rr.getSmallestIntegerContainer());
                ds2.drawForRectangle (g, rr.getSmallestIntegerContainer());
                
                g.setColour (lf->theme.sh.withAlpha (0.18f));
                g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 0.8f);
                
                if (showBorder)
                {
                    auto border = r.reduced (2.0f);
                    g.setColour (lf->theme.motionBorder); // purple border from theme
                    if (isMouseOverOrDragging() || hoverActive)
                    {
                        for (int i = 1; i <= 6; ++i)
                        {
                            const float t = (float) i / 6.0f;
                            const float expand = 2.0f + t * 8.0f;
                            g.setColour (lf->theme.motionBorder.withAlpha ((1.0f - t) * 0.22f));
                            g.drawRoundedRectangle (border.expanded (expand), rad + expand * 0.35f, 2.0f);
                        }
                    }
                    g.setColour (lf->theme.motionBorder);
                    g.drawRoundedRectangle (border, rad, 1.5f);
                }
            }
            else if (metallic)
            {
                auto r = getLocalBounds().toFloat().reduced (3.0f);
                const float rad = 8.0f;
                // Darker metallic panel (Ocean-harmonized neutral steel)
                juce::Colour top = juce::Colour (0xFF9CA4AD);
                juce::Colour bot = juce::Colour (0xFF6E747C);
                juce::ColourGradient grad (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false);
                g.setGradientFill (grad);
                g.fillRoundedRectangle (r, rad);

                // Brushing lines
                g.setColour (juce::Colours::white.withAlpha (0.045f));
                const int step = 1;
                for (int y = (int) r.getY() + step; y < r.getBottom(); y += step)
                    g.fillRect (juce::Rectangle<int> ((int) r.getX() + 4, y, (int) r.getWidth() - 8, 1));

                // Static metallic texture (no randomization for performance)
                {
                    g.setColour (juce::Colours::black.withAlpha (0.040f));
                    const int noiseRows = juce::jmax (1, (int) r.getHeight() / 4);
                    for (int i = 0; i < noiseRows; ++i)
                    {
                        const int y = (int) r.getY() + 2 + i * 4;
                        const int w = juce::jmax (8, (int) r.getWidth() - 12);
                        const int x = (int) r.getX() + 6;
                        g.fillRect (juce::Rectangle<int> (x, y, w, 1));
                    }
                }

                // Static diagonal micro-scratches (no randomization for performance)
                {
                    const int scratches = juce::jmax (6, (int) r.getWidth() / 22);
                    g.setColour (juce::Colours::white.withAlpha (0.035f));
                    for (int i = 0; i < scratches; ++i)
                    {
                        float sx = r.getX() + 6 + std::fmod (i * 3.7f, r.getWidth() - 12);
                        float sy = r.getY() + 6 + std::fmod (i * 2.3f, r.getHeight() - 12);
                        float len = 14.0f;
                        float dx = len * 0.86f;
                        float dy = len * 0.50f;
                        g.drawLine (sx, sy, sx + dx, sy + dy, 1.0f);
                    }
                    g.setColour (juce::Colours::black.withAlpha (0.025f));
                    for (int i = 0; i < scratches; ++i)
                    {
                        float sx = r.getX() + 6 + std::fmod (i * 4.1f, r.getWidth() - 12);
                        float sy = r.getY() + 6 + std::fmod (i * 3.1f, r.getHeight() - 12);
                        float len = 11.0f;
                        float dx = len * -0.80f;
                        float dy = len * 0.58f;
                        g.drawLine (sx, sy, sx + dx, sy + dy, 1.0f);
                    }
                }

                // Vignette
                {
                    juce::ColourGradient vg (juce::Colours::transparentBlack, r.getCentreX(), r.getCentreY(),
                                             juce::Colours::black.withAlpha (0.16f), r.getCentreX(), r.getCentreY() - r.getHeight() * 0.6f, true);
                    g.setGradientFill (vg);
                    g.fillRoundedRectangle (r, rad);
                }

                // Subtle rim
                g.setColour (lf->theme.sh.withAlpha (0.14f));
                g.drawRoundedRectangle (r.reduced (1.0f), rad - 1.0f, 0.8f);

                if (showBorder)
                {
                    auto border = r.reduced (2.0f);
                    g.setColour (juce::Colour (0xFF5A5F66));
                    if (isMouseOverOrDragging() || hoverActive)
                    {
                        for (int i = 1; i <= 6; ++i)
                        {
                            const float t = (float) i / 6.0f;
                            const float expand = 2.0f + t * 8.0f;
                            g.setColour (juce::Colour (0xFF5A5F66).withAlpha ((1.0f - t) * 0.22f));
                            g.drawRoundedRectangle (border.expanded (expand), rad + expand * 0.35f, 2.0f);
                        }
                    }
                    g.setColour (juce::Colour (0xFF51565D));
                    g.drawRoundedRectangle (border, rad, 1.5f);
                }
            }
            else if (reverbMaroon)
            {
                // Custom paint for Reverb cells with vintage orange/red maroon border
                auto r = getLocalBounds().toFloat();
                const float rad = 8.0f;

                g.setColour (lf->theme.panel);
                g.fillRoundedRectangle (r.reduced (3.0f), rad);

                juce::DropShadow ds1 (lf->theme.shadowDark.withAlpha (0.35f), 12, { -1, -1 });
                juce::DropShadow ds2 (lf->theme.shadowLight.withAlpha (0.25f),  6, { -1, -1 });
                ds1.drawForRectangle (g, r.reduced (3.0f).getSmallestIntegerContainer());
                ds2.drawForRectangle (g, r.reduced (3.0f).getSmallestIntegerContainer());

                g.setColour (lf->theme.sh.withAlpha (0.18f));
                g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 0.8f);

                if (showBorder)
                {
                    auto border = r.reduced (2.0f);
                    const juce::Colour maroon = juce::Colour (0xFF8E3A2F);
                    if (isMouseOverOrDragging() || hoverActive)
                    {
                        for (int i = 1; i <= 6; ++i)
                        {
                            const float t = (float) i / 6.0f;
                            const float expand = 2.0f + t * 8.0f;
                            g.setColour (maroon.withAlpha ((1.0f - t) * 0.22f));
                            g.drawRoundedRectangle (border.expanded (expand), rad + expand * 0.35f, 2.0f);
                        }
                    }
                    g.setColour (maroon);
                    g.drawRoundedRectangle (border, rad, 1.5f);
                }
            }
            else
            {
                lf->paintCellPanel (g, *this, showBorder, isMouseOverOrDragging() || hoverActive);
            }
        }
    }
    void visibilityChanged() override
    {
        if (isVisible())
        {
            if (content.getParentComponent() != this)
                addAndMakeVisible (content);
            resized();
            content.setVisible (true);
            repaint();
        }
    }
    void mouseEnter (const juce::MouseEvent&) override { hoverActive = true;  repaint(); }
    void mouseExit  (const juce::MouseEvent&) override { hoverActive = false; repaint(); }
private:
    juce::Component& content;
    juce::Label caption;
    juce::String captionText;
    bool showBorder { true };
    bool hoverActive { false };
    bool isDelayTheme { false };
};
