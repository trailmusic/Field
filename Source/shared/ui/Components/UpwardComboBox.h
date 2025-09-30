#pragma once
#include <JuceHeader.h>

// Custom ComboBox that opens its popup menu upward
class UpwardComboBox : public juce::ComboBox
{
public:
    UpwardComboBox() = default;
    ~UpwardComboBox() = default;

    void showPopup() override
    {
        juce::PopupMenu::Options options;
        options = options.withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::upwards);
        
        // Create and show the popup menu with upward direction
        juce::PopupMenu menu;
        for (int i = 1; i <= getNumItems(); ++i)
        {
            menu.addItem(i, getItemText(i));
        }
        
        menu.showMenuAsync(options.withTargetComponent(this));
    }
};
