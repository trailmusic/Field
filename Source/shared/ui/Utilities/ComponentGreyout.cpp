#include "ComponentGreyout.h"
#include "shared/Core/FieldLookAndFeel.h"

void ComponentGreyout::setGreyedOut(juce::Component& component, bool greyedOut, float alpha)
{
    // Set the component's enabled state
    component.setEnabled(!greyedOut);
    
    // Store greyout state as a property for custom rendering
    component.getProperties().set("greyedOut", greyedOut);
    component.getProperties().set("greyoutAlpha", alpha);
    
    // Apply greyout to all child components recursively
    for (int i = 0; i < component.getNumChildComponents(); ++i)
    {
        if (auto* child = component.getChildComponent(i))
        {
            setGreyedOut(*child, greyedOut, alpha);
        }
    }
    
    // Special handling for specific component types
    if (auto* slider = dynamic_cast<juce::Slider*>(&component))
    {
        // Sliders need special greyout handling
        slider->setEnabled(!greyedOut);
        slider->getProperties().set("greyedOut", greyedOut);
        slider->getProperties().set("greyoutAlpha", alpha);
    }
    else if (auto* label = dynamic_cast<juce::Label*>(&component))
    {
        // Labels need special greyout handling
        label->setEnabled(!greyedOut);
        label->getProperties().set("greyedOut", greyedOut);
        label->getProperties().set("greyoutAlpha", alpha);
    }
    else if (auto* comboBox = dynamic_cast<juce::ComboBox*>(&component))
    {
        // ComboBoxes need special greyout handling
        comboBox->setEnabled(!greyedOut);
        comboBox->getProperties().set("greyedOut", greyedOut);
        comboBox->getProperties().set("greyoutAlpha", alpha);
    }
    
    // Force repaint to update visual state
    component.repaint();
}

void ComponentGreyout::setComponentGreyedOut(juce::Component& component, bool greyedOut, float alpha)
{
    // Set the component's enabled state
    component.setEnabled(!greyedOut);
    
    // Store greyout state as a property for custom rendering
    component.getProperties().set("greyedOut", greyedOut);
    component.getProperties().set("greyoutAlpha", alpha);
    
    // Force repaint to update visual state
    component.repaint();
}

void ComponentGreyout::paintGreyoutOverlay(juce::Graphics& g, juce::Rectangle<float> bounds, 
                                          float alpha, float cornerRadius)
{
    // Create a semi-transparent overlay
    g.setColour(juce::Colour(0x40000000).withAlpha(alpha));
    g.fillRoundedRectangle(bounds, cornerRadius);
}

float ComponentGreyout::getDisabledAlpha(float baseAlpha)
{
    return baseAlpha;
}
