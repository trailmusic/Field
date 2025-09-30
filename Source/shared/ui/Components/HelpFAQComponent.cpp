#include "HelpFAQComponent.h"

HelpFAQComponent::HelpFAQComponent(FieldLNF& lnf) : lnf(lnf)
{
    addAndMakeVisible(text);
    text.setReadOnly(true);
    text.setMultiLine(true);
    text.setScrollbarsShown(true);
    text.setCaretVisible(false);
    text.setFont(juce::Font(juce::FontOptions(14.0f)));
    text.setText(
        "FIELD — FAQ\n\n"
        "Q: How do I change color modes?\n"
        "A: Click the palette button in the header to cycle Ocean → Green → Pink → Yellow → Grey.\n\n"
        "Q: Where are colors defined?\n"
        "A: All colors live in FieldLookAndFeel (FieldLNF::theme). Components never hardcode colors.\n\n"
        "Q: Why don't knobs move when I resize?\n"
        "A: Sizing happens in resized() only; layout is responsive via Layout::dp().\n\n"
        "Q: How do I reset a control?\n"
        "A: Double-click most knobs/sliders to reset to default.\n\n"
        "Q: Where are presets saved?\n"
        "A: In your user data folder under the plugin's presets directory.\n\n"
    );
}

void HelpFAQComponent::resized()
{
    text.setBounds(getLocalBounds().reduced(12));
}

void HelpFAQComponent::paint(juce::Graphics& g)
{
    g.fillAll(lnf.theme.panel);
    g.setColour(lnf.theme.sh);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(2.0f), 6.0f, 1.0f);
}
