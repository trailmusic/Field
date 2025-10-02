#pragma once

#include <JuceHeader.h>
#include "../Components/KnobCell.h"
#include "../Controls/ControlGridMetrics.h"

class MyPluginAudioProcessorEditor;

class LayoutManager
{
public:
    LayoutManager(MyPluginAudioProcessorEditor& editor);
    ~LayoutManager() = default;
    
    // Make editor accessible for layout methods
    MyPluginAudioProcessorEditor& getEditor() { return editor; }
    
    // Main layout method
    void performLayout();
    
    // Layout sections
    void layoutHeader();
    void layoutMainControls();
    void layoutCenterGroup();
    void layoutPhaseControls();
    void layoutDelayControls();
    void layoutReverbControls();
    void layoutMotionControls();
    void layoutImagerControls();
    void layoutMachineControls();
    void layoutXYPad();
    void layoutSpectrumAnalyzer();
    
    // Utility methods
    void setResizeConstraints();
    void updateControlGridMetrics();
    
    // Cell management
    void buildCells();
    
    // Grid layout helpers
    juce::Rectangle<int> getControlBounds(int row, int col, int width = 1, int height = 1) const;
    juce::Rectangle<int> getHeaderBounds() const;
    juce::Rectangle<int> getMainAreaBounds() const;
    juce::Rectangle<int> getTabAreaBounds() const;
    
private:
    MyPluginAudioProcessorEditor& editor;
    ControlGridMetrics metrics;
    
    // Layout state
    int currentWidth = 0;
    int currentHeight = 0;
    bool isResizing = false;
    
    // Grid calculations
    void calculateGridMetrics();
    juce::Rectangle<int> calculateControlBounds(int row, int col, int width, int height) const;
    
    // Component positioning helpers
    void positionKnobCell(juce::Component& component, int row, int col);
    void positionButton(juce::Component& component, int row, int col);
    void positionComboBox(juce::Component& component, int row, int col);
    void positionSlider(juce::Component& component, int row, int col);
    void positionLabel(juce::Component& component, int row, int col);
};
