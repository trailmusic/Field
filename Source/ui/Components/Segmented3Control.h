#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Core/FieldLookAndFeel.h"

class Segmented3Control : public juce::Component
{
public:
    Segmented3Control (juce::AudioProcessorValueTreeState& state, const juce::String& paramID,
                       const juce::StringArray& labels)
        : apvts (state), id (paramID)
    {
        for (int i = 0; i < 3; ++i)
        {
            buttons[i].setClickingTogglesState (false);
            buttons[i].onClick = [this, i]{ setIndexFromUI (i); };
            if (i < labels.size()) buttons[i].setButtonText (labels[i]);
            // assign painter: i=0 Straight, 1 Dotted, 2 Triplet
            buttons[i].painter = [this, i](juce::Graphics& g, juce::Rectangle<float> area, bool over, bool on)
            {
                auto* lf2 = dynamic_cast<FieldLNF*>(&getLookAndFeel());
                juce::Colour accent = lf2 ? lf2->theme.accent : juce::Colour (0xFF2196F3);
                juce::Colour text   = lf2 ? lf2->theme.text   : juce::Colours::white;
                juce::Colour iconCol = on ? accent : (over ? text.withAlpha (0.80f) : text.withAlpha (0.65f));
                // Shadowed, slightly heavier icon
                auto inner = area.reduced (2.0f);
                g.setColour (juce::Colours::black.withAlpha (0.18f));
                drawFeelIcon (g, inner.translated (0.6f, 0.9f), i, iconCol);
                g.setColour (iconCol);
                drawFeelIcon (g, inner, i, iconCol);
            };
            addAndMakeVisible (buttons[i]);
        }
        applyTheme();
        if (auto* p = apvts.getParameter (id))
        {
            attachment = std::make_unique<juce::ParameterAttachment>(*dynamic_cast<juce::RangedAudioParameter*>(p),
                [this](float newVal)
                {
                    const int idx = (int) juce::roundToInt (newVal);
                    updateButtons (idx);
                }, nullptr);
            updateButtons ((int) juce::roundToInt (p->getValue()));
        }
    }
    void resized() override
    {
        auto b = getLocalBounds().reduced (4);
        int w = b.getWidth() / 3;
        for (int i = 0; i < 3; ++i)
        {
            auto cell = b.removeFromLeft (w);
            auto inner = cell.reduced (2);
            buttons[i].setBounds (inner);
            buttons[i].setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
        }
    }
    void lookAndFeelChanged() override { applyTheme(); }
    void setLabels (const juce::StringArray& labels)
    {
        for (int i = 0; i < 3 && i < labels.size(); ++i) buttons[i].setButtonText (labels[i]);
    }
private:
    static void drawFeelIcon (juce::Graphics& g, juce::Rectangle<float> r, int feelIndex, juce::Colour c)
    {
        g.setColour (c);

        auto cx = r.getCentreX();
        auto baseY = r.getCentreY() + r.getHeight() * 0.10f;

        const float headW = juce::jmin (10.0f, r.getWidth() * 0.8f);
        const float headH = headW * 0.68f;
        const float stemH = juce::jmin (22.0f, r.getHeight() * 0.42f);
        const float stemX = cx + headW * 0.45f;
        const float headY = baseY - headH * 0.5f;

        juce::Path head;
        head.addEllipse (cx - headW * 0.5f, headY, headW, headH);
        auto tilt = juce::AffineTransform::rotation (juce::degreesToRadians (-12.0f), cx, headY + headH * 0.5f);
        head.applyTransform (tilt);
        g.fillPath (head);

        juce::Path stem;
        stem.startNewSubPath (stemX, headY + headH * 0.15f);
        stem.lineTo (stemX, headY - stemH);
        g.strokePath (stem, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (feelIndex == 1)
        {
            float dotR = 2.0f;
            float dotX = stemX + 3.0f;
            float dotY = headY + headH * 0.50f;
            if (r.getWidth() < 16.0f) { dotX = stemX; dotY = headY + headH * 0.15f - 4.0f; }
            g.fillEllipse (dotX - dotR, dotY - dotR, dotR * 2, dotR * 2);
        }
        else if (feelIndex == 2)
        {
            g.setFont (juce::Font (10.0f, juce::Font::bold));
            float tx = stemX + 2.0f;
            float ty = headY - 6.0f;
            g.drawSingleLineText ("3", (int) tx, (int) ty);
        }
    }
    void applyTheme()
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        juce::Colour accent = lf ? lf->theme.accent : juce::Colour (0xFF2196F3);
        juce::Colour text   = lf ? lf->theme.text   : juce::Colours::white;
        juce::Colour muted  = lf ? lf->theme.textMuted : juce::Colour (0xFFB8BDC7);
        for (int i = 0; i < 3; ++i)
        {
            auto& b = buttons[i];
            b.setColour (juce::TextButton::buttonOnColourId, accent);
            b.setColour (juce::TextButton::textColourOnId,   text);
            b.setColour (juce::TextButton::textColourOffId,  muted);
        }
    }
    void setIndexFromUI (int i)
    {
        if (! attachment) return;
        attachment->setValueAsCompleteGesture ((float) i);
        updateButtons (i);
    }
    void updateButtons (int idx)
    {
        for (int i = 0; i < 3; ++i)
            buttons[i].setToggleState (i == idx, juce::dontSendNotification);
        repaint();
    }
    juce::AudioProcessorValueTreeState& apvts;
    juce::String id;
    // Custom button subclass to draw icon
    struct IconButton : public juce::TextButton
    {
        std::function<void(juce::Graphics&, juce::Rectangle<float>, bool, bool)> painter;
        void paintButton (juce::Graphics& g, bool over, bool down) override
        {
            // Check for metallic properties first - if found, delegate to FieldLNF
            auto metallicKind = metallicFromProps(getProperties());
            if (metallicKind != MetallicKind::None)
            {
                // Delegate to FieldLNF for metallic buttons
                if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                {
                    lf->drawButtonBackground(g, *this, juce::Colour(), over, down);
                    return;
                }
            }
            
            // Fall back to custom rendering for non-metallic buttons
            auto r = getLocalBounds().toFloat();
            auto rr = r.reduced (1.0f);
            float cr = 4.0f;
            auto* lf2 = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            auto panel  = lf2 ? lf2->theme.panel  : juce::Colour (0xFF3A3D45);
            auto border = lf2 ? lf2->theme.sh     : juce::Colour (0xFF2A2A2A);
            juce::Colour accent = lf2 ? lf2->theme.accent : juce::Colour (0xFF2196F3);
            juce::Colour text   = lf2 ? lf2->theme.text   : juce::Colours::white;

            g.setColour (panel);
            g.fillRoundedRectangle (rr, cr);
            g.setColour (border);
            g.drawRoundedRectangle (rr, cr, 1.0f);

            if (painter)
            {
                bool on = getToggleState();
                juce::Colour iconCol = on ? accent : (over ? text.withAlpha (0.85f) : text.withAlpha (0.75f));
                auto inner = rr.reduced (3.0f);
                // Shadow pass
                g.setColour (juce::Colours::black.withAlpha (0.18f));
                painter (g, inner.translated (0.6f, 0.9f), over, on);
                // Main pass
                g.setColour (iconCol);
                painter (g, inner, over, on);
            }
        }
    };
    IconButton buttons[3];
    std::unique_ptr<juce::ParameterAttachment> attachment;
};
