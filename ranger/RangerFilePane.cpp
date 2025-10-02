#include "RangerFilePane.h"

RangerFilePane::RangerFilePane()
{
    // File list
    fileListLabel.setText("Loaded Files:", juce::dontSendNotification);
    fileListLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(fileListLabel);
    
    addAndMakeVisible(fileListBox);
    
    // Buttons
    addFileButton.setButtonText("Add File");
    addFileButton.onClick = [this] { addFile(); };
    addAndMakeVisible(addFileButton);
    
    removeFileButton.setButtonText("Remove Selected");
    removeFileButton.onClick = [this] { removeFile(); };
    addAndMakeVisible(removeFileButton);
    
    clearAllButton.setButtonText("Clear All");
    clearAllButton.onClick = [this] { clearAllFiles(); };
    addAndMakeVisible(clearAllButton);
    
    // File info
    fileInfoLabel.setText("File Information:", juce::dontSendNotification);
    fileInfoLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(fileInfoLabel);
    
    fileInfoEditor.setReadOnly(true);
    fileInfoEditor.setMultiLine(true);
    fileInfoEditor.setText("No file selected");
    addAndMakeVisible(fileInfoEditor);
    
    // Enable drag and drop
    setWantsKeyboardFocus(true);
}

RangerFilePane::~RangerFilePane()
{
}

void RangerFilePane::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));
    
    // Draw some visual elements
    g.setColour(juce::Colour(0xff3d3d3d));
    g.drawRect(getLocalBounds(), 2);
    
    // Draw drag and drop hint
    g.setColour(juce::Colour(0xff666666));
    g.setFont(juce::Font(12.0f));
    g.drawText("Drag and drop FIR files here (.csv, .txt, .dat, .fir)", 
               getLocalBounds().removeFromBottom(30), 
               juce::Justification::centred);
}

void RangerFilePane::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    // File list
    fileListLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    
    // Buttons
    auto buttonRow = bounds.removeFromTop(40);
    addFileButton.setBounds(buttonRow.removeFromLeft(100));
    buttonRow.removeFromLeft(10);
    removeFileButton.setBounds(buttonRow.removeFromLeft(120));
    buttonRow.removeFromLeft(10);
    clearAllButton.setBounds(buttonRow.removeFromLeft(100));
    
    bounds.removeFromTop(20);
    
    // File list box
    fileListBox.setBounds(bounds.removeFromTop(200));
    bounds.removeFromTop(20);
    
    // File info
    fileInfoLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    fileInfoEditor.setBounds(bounds);
}

void RangerFilePane::addFile()
{
    auto chooser = std::make_unique<juce::FileChooser>("Select FIR files to add...",
                                                       juce::File(),
                                                       "*.csv;*.txt;*.dat;*.fir");
    
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectMultipleItems;
    
    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        auto files = fc.getResults();
        for (auto& file : files)
        {
            // Add file to list (simplified for now)
            updateFileInfo();
        }
    });
}

void RangerFilePane::removeFile()
{
    // Remove selected file (simplified for now)
    updateFileInfo();
}

void RangerFilePane::clearAllFiles()
{
    // Clear all files (simplified for now)
    fileInfoEditor.setText("No file selected");
}

void RangerFilePane::updateFileInfo()
{
    fileInfoEditor.setText("File: example.fir\n"
                           "Size: 1.2 KB\n"
                           "Type: Linear-phase FIR\n"
                           "Order: 127\n"
                           "Status: Ready for conversion");
}

// Drag and drop implementation
bool RangerFilePane::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto& file : files)
    {
        auto fileObj = juce::File(file);
        auto extension = fileObj.getFileExtension().toLowerCase();
        if (extension == ".csv" || extension == ".txt" || extension == ".dat" || extension == ".fir")
            return true;
    }
    return false;
}

void RangerFilePane::fileDragEnter(const juce::StringArray& files, int x, int y)
{
    // Visual feedback for drag enter
    repaint();
}

void RangerFilePane::fileDragExit(const juce::StringArray& files)
{
    // Visual feedback for drag exit
    repaint();
}

void RangerFilePane::filesDropped(const juce::StringArray& files, int x, int y)
{
    juce::StringArray validFiles;
    
    for (auto& file : files)
    {
        auto fileObj = juce::File(file);
        auto extension = fileObj.getFileExtension().toLowerCase();
        if (extension == ".csv" || extension == ".txt" || extension == ".dat" || extension == ".fir")
        {
            validFiles.add(file);
        }
    }
    
    if (validFiles.size() > 0)
    {
        // Process dropped files
        for (auto& file : validFiles)
        {
            auto fileObj = juce::File(file);
            fileInfoEditor.setText("Dropped: " + fileObj.getFileName() + "\n"
                                 "Path: " + fileObj.getFullPathName() + "\n"
                                 "Size: " + juce::String(fileObj.getSize()) + " bytes\n"
                                 "Type: FIR filter file\n"
                                 "Status: Ready for processing");
        }
        
        // Update file list (simplified for now)
        updateFileInfo();
    }
}