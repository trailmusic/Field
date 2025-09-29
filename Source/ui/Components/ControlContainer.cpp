#include "ControlContainer.h"
#include "../../Core/FieldLookAndFeel.h"

ControlContainer::ControlContainer() { setWantsKeyboardFocus (false); }

void ControlContainer::setTitle (const juce::String& t) { containerTitle = t; repaint(); }

void ControlContainer::paint (juce::Graphics& g)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto panel = lf ? lf->theme.meters.panelDark : juce::Colour (0xFF2A2C30);
    const auto text  = lf ? lf->theme.text  : juce::Colour (0xFFF0F2F5);
    const auto accent= lf ? lf->theme.accent: juce::Colour (0xFF5AA9E6);

    auto r = getLocalBounds().toFloat();
    const float rad = 8.0f;

    if (showBorder)
    {
        // Use custom background color if set, otherwise use theme panel color
        juce::Colour bgColor = useCustomBackgroundColour ? backgroundColour : panel;
        g.setColour (bgColor);
        g.fillRoundedRectangle (r.reduced (3.0f), rad);

        // depth
        juce::DropShadow ds1 ((lf ? lf->theme.shadowDark  : juce::Colour (0xFF1A1C20)).withAlpha (0.6f), 20, { -2, -2 });
        juce::DropShadow ds2 ((lf ? lf->theme.shadowLight : juce::Colour (0xFF60646C)).withAlpha (0.4f),  8, { -1, -1 });
        auto ri = r.reduced (3.0f).getSmallestIntegerContainer();
        ds1.drawForRectangle (g, ri);
        ds2.drawForRectangle (g, ri);

        // inner rim
        g.setColour ((lf ? lf->theme.sh : juce::Colour (0xFF2A2C30)).withAlpha (0.3f));
        g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 1.0f);
    }

    if (showBorder)
    {
        auto border = r.reduced (3.0f);
        // hover halo
        const bool over = isMouseOverOrDragging();
        if (over || hoverActive)
        {
            g.setColour (accent.withAlpha (0.5f));
            g.drawRoundedRectangle (border.expanded (2.0f), rad, 2.0f);
        }

        // Use custom border color if set, otherwise use theme accent
        juce::Colour borderColor = useCustomBorderColour ? borderColour : accent;
        g.setColour (borderColor.withAlpha (0.4f));
        g.drawRoundedRectangle (border, rad, 1.0f);
    }

    // title
    if (containerTitle.isNotEmpty())
    {
        g.setColour (text.withAlpha (0.8f));
        g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
        auto titleArea = r.reduced (12.0f, 6.0f).removeFromTop (16.0f);
        g.drawFittedText (containerTitle, titleArea.toNearestInt(), juce::Justification::centredLeft, 1);
    }
}
