#pragma once

#include <JuceHeader.h>

class MyPluginAudioProcessorEditor;

class EventManager
{
public:
    EventManager(MyPluginAudioProcessorEditor& editor);
    ~EventManager() = default;
    
    // Mouse events
    void handleMouseDown(const juce::MouseEvent& e);
    void handleMouseDrag(const juce::MouseEvent& e);
    void handleMouseUp(const juce::MouseEvent& e);
    void handleMouseMove(const juce::MouseEvent& e);
    
    // Button events
    void handleButtonClicked(juce::Button* button);
    void handleButtonStateChanged(juce::Button* button);
    
    // Slider events
    void handleSliderValueChanged(juce::Slider* slider);
    void handleSliderDragStarted(juce::Slider* slider);
    void handleSliderDragEnded(juce::Slider* slider);
    
    // ComboBox events
    void handleComboBoxChanged(juce::ComboBox* comboBox);
    
    // Keyboard events
    void handleKeyPressed(const juce::KeyPress& key);
    void handleKeyReleased(const juce::KeyPress& key);
    
    // Parameter events
    void handleParameterChanged(const juce::String& parameterID, float newValue);
    
    // Timer events
    void handleTimerCallback();
    
    // Resize events
    void handleResizeStarted();
    void handleResizeEnded();
    
    // Tooltip events
    void handleTooltipShow(juce::Component* target, const juce::String& text);
    void handleTooltipHide();
    void setupTooltipBubble();
    
    // Preset events
    void handlePresetLoad(const juce::String& presetName);
    void handlePresetSave(const juce::String& presetName);
    
    // Tab events
    void handleTabChanged(int newTabIndex);
    
    // XY Pad events
    void handleXYPadDrag(const juce::Point<float>& position);
    void handleXYPadClick(const juce::Point<float>& position);
    
private:
    MyPluginAudioProcessorEditor& editor;
    
    // Event state
    bool isDragging = false;
    juce::Point<int> dragStartPosition;
    juce::Component* dragTarget = nullptr;
    
    // Tooltip state
    juce::Component* currentTooltipTarget = nullptr;
    juce::String currentTooltipText;
    bool tooltipVisible = false;
    
    // Resize state
    bool isResizing = false;
    juce::Rectangle<int> resizeStartBounds;
    
    // Helper methods
    void updateTooltipPosition();
    void hideTooltip();
    void showTooltip(juce::Component* target, const juce::String& text);
    
    // Parameter binding helpers
    void bindParameterToControl(juce::Component* control, const juce::String& parameterID);
    void unbindParameterFromControl(juce::Component* control);
    
    // Event routing
    void routeMouseEvent(const juce::MouseEvent& e, const std::function<void()>& handler);
    void routeButtonEvent(juce::Button* button, const std::function<void()>& handler);
    void routeSliderEvent(juce::Slider* slider, const std::function<void()>& handler);
    void routeComboBoxEvent(juce::ComboBox* comboBox, const std::function<void()>& handler);
};
