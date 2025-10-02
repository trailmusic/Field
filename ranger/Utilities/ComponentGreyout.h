#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * ComponentGreyout - Site-wide utility for greying out components
 * 
 * This utility provides a consistent way to grey out components when they're inactive.
 * It handles visual styling, component state, and can be applied to any component hierarchy.
 */
class ComponentGreyout
{
public:
    /**
     * Apply greyout styling to a component and all its children
     * @param component The component to grey out
     * @param greyedOut Whether to apply greyout styling
     * @param alpha The alpha value for the greyout overlay (0.0 = transparent, 1.0 = opaque)
     */
    static void setGreyedOut(juce::Component& component, bool greyedOut, float alpha = 0.4f);
    
    /**
     * Apply greyout styling to a specific component
     * @param component The component to grey out
     * @param greyedOut Whether to apply greyout styling
     * @param alpha The alpha value for the greyout overlay
     */
    static void setComponentGreyedOut(juce::Component& component, bool greyedOut, float alpha = 0.4f);
    
    /**
     * Paint a greyout overlay on a component
     * @param g Graphics context
     * @param bounds Component bounds
     * @param alpha Alpha value for the overlay
     * @param cornerRadius Corner radius for rounded rectangle
     */
    static void paintGreyoutOverlay(juce::Graphics& g, juce::Rectangle<float> bounds, 
                                   float alpha = 0.4f, float cornerRadius = 8.0f);
    
    /**
     * Get the appropriate alpha value for disabled components
     * @param baseAlpha Base alpha value (default 0.4f)
     * @return Alpha value for disabled state
     */
    static float getDisabledAlpha(float baseAlpha = 0.4f);
    
    /**
     * Get the appropriate alpha value for enabled components
     * @return Alpha value for enabled state (1.0f)
     */
    static float getEnabledAlpha() { return 1.0f; }
};
