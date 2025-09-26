#pragma once
#include <JuceHeader.h>
#include "../Core/PluginProcessor.h"

class PhaseTab : public juce::Component
{
public:
    PhaseTab (MyPluginAudioProcessor& p, juce::LookAndFeel* lnf)
        : proc (p)
    {
        setLookAndFeel (lnf);
        
        // Phase Mode controls
        phaseModeButton.setButtonText ("Phase Mode");
        phaseModeButton.setToggleState (true, juce::dontSendNotification);
        addAndMakeVisible (phaseModeButton);
        
        // Phase Recording controls
        phaseRecButton.setButtonText ("Phase Rec");
        phaseRecButton.setToggleState (false, juce::dontSendNotification);
        addAndMakeVisible (phaseRecButton);
        
        // Phase Amount slider
        phaseAmountSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        phaseAmountSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
        phaseAmountSlider.setRange (0.0, 1.0, 0.01);
        phaseAmountSlider.setValue (0.5);
        addAndMakeVisible (phaseAmountSlider);
        
        // Quality button
        qualityButton.setButtonText ("Quality");
        qualityButton.setToggleState (true, juce::dontSendNotification);
        addAndMakeVisible (qualityButton);
        
        // Attach parameters
        phaseModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, IDs::phaseMode, phaseModeButton);
        phaseRecAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, IDs::centerPhaseRecOn, phaseRecButton);
        phaseAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, IDs::centerPhaseAmt01, phaseAmountSlider);
        qualityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, IDs::quality, qualityButton);
    }
    
    ~PhaseTab() override
    {
        setLookAndFeel (nullptr);
    }
    
    void resized() override
    {
        auto r = getLocalBounds().reduced (20);
        
        // Phase Mode button (top-left)
        phaseModeButton.setBounds (r.removeFromTop (40).removeFromLeft (120));
        
        // Quality button (top-right)
        qualityButton.setBounds (r.removeFromTop (40).removeFromRight (120));
        
        // Phase Rec button (middle-left)
        r = getLocalBounds().reduced (20);
        r.removeFromTop (60);
        phaseRecButton.setBounds (r.removeFromTop (40).removeFromLeft (120));
        
        // Phase Amount slider (center)
        r = getLocalBounds().reduced (20);
        r.removeFromTop (120);
        auto centerArea = r.removeFromTop (120);
        phaseAmountSlider.setBounds (centerArea.removeFromTop (80).reduced (20));
    }
    
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Use theme colors like other components
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            g.setColour(lf->theme.panel);
            g.fillRoundedRectangle(bounds, 6.0f);
            
            // Standard accent border treatment
            g.setColour(lf->theme.accent.withAlpha(0.3f));
            g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
        }
        else
        {
            // Fallback for non-theme look and feel
            g.setColour(juce::Colours::darkgrey);
            g.fillRoundedRectangle(bounds, 6.0f);
        }
        
        // Draw title
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (24.0f, juce::Font::bold));
        g.drawText ("Phase Controls", getLocalBounds().removeFromTop (50), juce::Justification::centred);
    }

private:
    MyPluginAudioProcessor& proc;
    
    juce::ToggleButton phaseModeButton;
    juce::ToggleButton phaseRecButton;
    juce::Slider phaseAmountSlider;
    juce::ToggleButton qualityButton;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseRecAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> phaseAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> qualityAttachment;
};
