#pragma once

#include <JuceHeader.h>

class RangerFilePane : public juce::Component,
                       public juce::FileDragAndDropTarget
{
public:
    RangerFilePane();
    ~RangerFilePane() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Drag and drop
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    // File list
    juce::ListBox fileListBox;
    juce::Label fileListLabel;
    
    // File operations
    juce::TextButton addFileButton;
    juce::TextButton removeFileButton;
    juce::TextButton clearAllButton;
    
    // File info
    juce::Label fileInfoLabel;
    juce::TextEditor fileInfoEditor;
    
    // Methods
    void addFile();
    void removeFile();
    void clearAllFiles();
    void updateFileInfo();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerFilePane)
};