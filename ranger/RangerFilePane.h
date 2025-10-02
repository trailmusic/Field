#pragma once

#include <JuceHeader.h>

class RangerFilePane : public juce::Component,
                       public juce::FileDragAndDropTarget,
                       public juce::ListBoxModel
{
public:
    RangerFilePane();
    ~RangerFilePane() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // File drag and drop
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    // UI Components
    juce::Label titleLabel;
    juce::TextButton openButton;
    juce::ListBox fileList;
    juce::TextButton baselineButton;
    juce::Label baselineLabel;
    
    // File management
    juce::StringArray loadedFiles;
    juce::String baselineFolder;
    
    void openFiles();
    void setBaselineFolder();
    void updateFileList();
    
    // ListBoxModel implementation
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged(int lastRowSelected) override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerFilePane)
};
