#include "ButtonManager.h"

ButtonManager::ButtonManager(MyPluginAudioProcessorEditor& editor)
    : editor(editor)
{
    // Add container to editor
    editor.addAndMakeVisible(buttonsContainer);
    
    initializeButtons();
    setupButtonCallbacks();
}

void ButtonManager::initializeButtons()
{
    // Add buttons to container
    buttonsContainer.addAndMakeVisible(phaseModeButton);
    buttonsContainer.addAndMakeVisible(qualityButton);
    
    // Set look and feel for buttons
    phaseModeButton.setLookAndFeel(&editor.lnf);
    qualityButton.setLookAndFeel(&editor.lnf);
}

void ButtonManager::setupButtonCallbacks()
{
    // Phase mode button callback
    phaseModeButton.onClick = [this]
    {
        showPhaseModeMenu();
    };
    
    // Quality button callback
    qualityButton.onClick = [this]
    {
        showQualityModeMenu();
    };
}

void ButtonManager::showPhaseModeMenu()
{
    // Phase mode menu - Zero, Natural, Hybrid, Full Linear
    juce::PopupMenu phaseMenu;
    phaseMenu.addItem("Zero", [this] { setPhaseMode(0); });
    phaseMenu.addItem("Natural", [this] { setPhaseMode(1); });
    phaseMenu.addItem("Hybrid", [this] { setPhaseMode(2); });
    phaseMenu.addItem("Full Linear", [this] { setPhaseMode(3); });
    
    phaseMenu.showMenuAsync(juce::PopupMenu::Options()
        .withTargetComponent(&phaseModeButton)
        .withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::upwards));
}

void ButtonManager::showQualityModeMenu()
{
    // Quality menu - Eco, Standard, High
    juce::PopupMenu qualityMenu;
    qualityMenu.addItem("Eco", [this] { setQualityMode(0); });
    qualityMenu.addItem("Standard", [this] { setQualityMode(1); });
    qualityMenu.addItem("High", [this] { setQualityMode(2); });
    
    qualityMenu.showMenuAsync(juce::PopupMenu::Options()
        .withTargetComponent(&qualityButton)
        .withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::upwards));
}

void ButtonManager::setPhaseMode(int mode)
{
    // Set phase mode parameter (0=Zero, 1=Natural, 2=Hybrid, 3=Full Linear)
    if (auto* param = editor.proc.apvts.getParameter("phase_mode"))
    {
        if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        {
            float normalizedValue = (float)mode / 3.0f; // Convert to 0.0-1.0 range (4 modes: 0,1,2,3)
            choiceParam->setValueNotifyingHost(normalizedValue);
        }
    }
    
    // Update button appearance to show current mode
    phaseModeButton.setToggleState(mode > 0, juce::dontSendNotification);
    phaseModeButton.repaint();
}

void ButtonManager::setQualityMode(int mode)
{
    // Set quality mode parameter (0=Eco, 1=Standard, 2=High)
    if (auto* param = editor.proc.apvts.getParameter("quality"))
    {
        if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        {
            choiceParam->setValueNotifyingHost(mode / 2.0f); // Convert to 0.0-1.0 range
        }
    }
    
    // Update button appearance to show current mode
    qualityButton.setToggleState(mode > 0, juce::dontSendNotification);
    qualityButton.repaint();
}
