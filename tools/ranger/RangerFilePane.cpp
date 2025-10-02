#include "RangerFilePane.h"

RangerFilePane::RangerFilePane()
{
    // Set up title
    titleLabel.setText("Files", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);
    
    // Set up open button
    openButton.setButtonText("Open taps");
    openButton.onClick = [this] { openFiles(); };
    addAndMakeVisible(openButton);
    
    // Set up file list
    fileList.setModel(this);
    addAndMakeVisible(fileList);
    
    // Set up baseline
    baselineButton.setButtonText("Set Baseline");
    baselineButton.onClick = [this] { setBaselineFolder(); };
    addAndMakeVisible(baselineButton);
    
    baselineLabel.setText("Baseline: Not set", juce::dontSendNotification);
    baselineLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(baselineLabel);
}

RangerFilePane::~RangerFilePane()
{
}

void RangerFilePane::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a)); // Dark panel background
    
    // Draw border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds(), 1);
}

void RangerFilePane::resized()
{
    auto bounds = getLocalBounds().reduced(8);
    
    titleLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(8);
    
    openButton.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(8);
    
    fileList.setBounds(bounds.removeFromTop(200));
    bounds.removeFromTop(8);
    
    baselineButton.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(4);
    
    baselineLabel.setBounds(bounds.removeFromTop(20));
}

bool RangerFilePane::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto& file : files)
    {
        if (file.endsWithIgnoreCase(".csv"))
            return true;
    }
    return false;
}

void RangerFilePane::fileDragEnter(const juce::StringArray& files, int x, int y)
{
    repaint();
}

void RangerFilePane::fileDragExit(const juce::StringArray& files)
{
    repaint();
}

void RangerFilePane::filesDropped(const juce::StringArray& files, int x, int y)
{
    for (auto& file : files)
    {
        if (file.endsWithIgnoreCase(".csv"))
        {
            loadedFiles.add(file);
        }
    }
    updateFileList();
}

void RangerFilePane::openFiles()
{
    juce::FileChooser chooser("Select CSV files", juce::File(), "*.csv");
    if (chooser.browseForMultipleFilesToOpen())
    {
        auto files = chooser.getResults();
        for (auto& file : files)
        {
            loadedFiles.add(file.getFullPathName());
        }
        updateFileList();
    }
}

void RangerFilePane::setBaselineFolder()
{
    juce::FileChooser chooser("Select baseline folder", juce::File(), "*");
    if (chooser.browseForDirectory())
    {
        baselineFolder = chooser.getResult().getFullPathName();
        baselineLabel.setText("Baseline: " + juce::File(baselineFolder).getFileName(), juce::dontSendNotification);
    }
}

void RangerFilePane::updateFileList()
{
    fileList.updateContent();
    repaint();
}

// ListBoxModel implementation
int RangerFilePane::getNumRows()
{
    return loadedFiles.size();
}

void RangerFilePane::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::darkblue);
    
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    g.drawText(loadedFiles[rowNumber], 4, 0, width - 8, height, juce::Justification::centredLeft);
}

void RangerFilePane::selectedRowsChanged(int lastRowSelected)
{
    // Handle file selection
}
