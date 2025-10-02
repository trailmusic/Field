#include "RangerDesigner.h"

RangerDesigner::RangerDesigner()
{
    // Title
    titleLabel.setText("Field Ranger - Min-Phase FIR Designer", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // Status
    statusLabel.setText("Ready to design filters", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(14.0f));
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);
    
    // Buttons
    loadButton.setButtonText("Load FIR File");
    loadButton.onClick = [this] { loadFile(); };
    addAndMakeVisible(loadButton);
    
    generateButton.setButtonText("Generate Min-Phase");
    generateButton.onClick = [this] { generateFilter(); };
    addAndMakeVisible(generateButton);
    
    exportButton.setButtonText("Export Results");
    exportButton.onClick = [this] { exportResults(); };
    addAndMakeVisible(exportButton);
    
    clearButton.setButtonText("Clear All");
    clearButton.onClick = [this] { clearAll(); };
    addAndMakeVisible(clearButton);
    
    // File info
    fileInfoLabel.setText("File Path:", juce::dontSendNotification);
    addAndMakeVisible(fileInfoLabel);
    
    filePathEditor.setReadOnly(true);
    filePathEditor.setMultiLine(false);
    addAndMakeVisible(filePathEditor);
    
    // Filter parameters
    orderLabel.setText("Filter Order:", juce::dontSendNotification);
    addAndMakeVisible(orderLabel);
    
    orderSlider.setRange(63, 255, 1);
    orderSlider.setValue(127);
    orderSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 60, 20);
    addAndMakeVisible(orderSlider);
    
    cutoffLabel.setText("Cutoff Frequency:", juce::dontSendNotification);
    addAndMakeVisible(cutoffLabel);
    
    cutoffSlider.setRange(0.1, 0.9, 0.01);
    cutoffSlider.setValue(0.5);
    cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 60, 20);
    addAndMakeVisible(cutoffSlider);
    
    filterTypeLabel.setText("Filter Type:", juce::dontSendNotification);
    addAndMakeVisible(filterTypeLabel);
    
    filterTypeCombo.addItem("Halfband FIR", 1);
    filterTypeCombo.addItem("Lowpass FIR", 2);
    filterTypeCombo.addItem("Highpass FIR", 3);
    filterTypeCombo.setSelectedId(1);
    addAndMakeVisible(filterTypeCombo);
    
    // Results
    resultsLabel.setText("Results:", juce::dontSendNotification);
    addAndMakeVisible(resultsLabel);
    
    resultsEditor.setReadOnly(true);
    resultsEditor.setMultiLine(true);
    resultsEditor.setText("No results yet. Load a file and generate filters.");
    addAndMakeVisible(resultsEditor);
}

RangerDesigner::~RangerDesigner()
{
}

void RangerDesigner::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));
    
    // Draw some visual elements
    g.setColour(juce::Colour(0xff3d3d3d));
    g.drawRect(getLocalBounds(), 2);
}

void RangerDesigner::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    // Title
    titleLabel.setBounds(bounds.removeFromTop(40));
    bounds.removeFromTop(10);
    
    // Status
    statusLabel.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(20);
    
    // Buttons row
    auto buttonRow = bounds.removeFromTop(40);
    loadButton.setBounds(buttonRow.removeFromLeft(120));
    buttonRow.removeFromLeft(10);
    generateButton.setBounds(buttonRow.removeFromLeft(150));
    buttonRow.removeFromLeft(10);
    exportButton.setBounds(buttonRow.removeFromLeft(120));
    buttonRow.removeFromLeft(10);
    clearButton.setBounds(buttonRow.removeFromLeft(100));
    
    bounds.removeFromTop(20);
    
    // File info
    fileInfoLabel.setBounds(bounds.removeFromTop(20));
    filePathEditor.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(20);
    
    // Parameters
    auto paramRow1 = bounds.removeFromTop(30);
    orderLabel.setBounds(paramRow1.removeFromLeft(100));
    orderSlider.setBounds(paramRow1.removeFromLeft(200));
    
    bounds.removeFromTop(10);
    
    auto paramRow2 = bounds.removeFromTop(30);
    cutoffLabel.setBounds(paramRow2.removeFromLeft(120));
    cutoffSlider.setBounds(paramRow2.removeFromLeft(200));
    
    bounds.removeFromTop(10);
    
    auto paramRow3 = bounds.removeFromTop(30);
    filterTypeLabel.setBounds(paramRow3.removeFromLeft(100));
    filterTypeCombo.setBounds(paramRow3.removeFromLeft(150));
    
    bounds.removeFromTop(20);
    
    // Results
    resultsLabel.setBounds(bounds.removeFromTop(20));
    resultsEditor.setBounds(bounds);
}

void RangerDesigner::loadFile()
{
    auto chooser = std::make_unique<juce::FileChooser>("Select FIR file to convert...",
                                                       juce::File(),
                                                       "*.csv;*.txt;*.dat");
    
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    
    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file != juce::File{})
        {
            filePathEditor.setText(file.getFullPathName());
            updateStatus("File loaded: " + file.getFileName());
        }
    });
}

void RangerDesigner::generateFilter()
{
    if (filePathEditor.getText().isEmpty())
    {
        updateStatus("Please load a file first");
        return;
    }
    
    updateStatus("Generating min-phase filter...");
    
    // Get the console tools path
    auto consoleToolsPath = juce::File::getCurrentWorkingDirectory().getChildFile("Ranger/console/build");
    auto minphaseExe = consoleToolsPath.getChildFile("minphase");
    
    if (!minphaseExe.exists())
    {
        updateStatus("Console tools not found. Please build them first.");
        return;
    }
    
    // Prepare command
    juce::StringArray args;
    args.add(minphaseExe.getFullPathName());
    args.add(filePathEditor.getText());
    args.add("--order");
    args.add(juce::String(orderSlider.getValue()));
    args.add("--cutoff");
    args.add(juce::String(cutoffSlider.getValue()));
    
    // Run the console tool
    juce::ChildProcess process;
    if (process.start(args))
    {
        juce::String output;
        char buffer[10000];
        process.readProcessOutput(buffer, 10000);
        output = juce::String(buffer);
        
        if (process.waitForProcessToFinish(5000))
        {
            resultsEditor.setText("Min-Phase Filter Generated:\n\n"
                                 "Order: " + juce::String(orderSlider.getValue()) + "\n"
                                 "Cutoff: " + juce::String(cutoffSlider.getValue()) + "\n"
                                 "Type: " + filterTypeCombo.getText() + "\n\n"
                                 "Console Output:\n" + output + "\n\n"
                                 "Status: Ready for export");
            
            updateStatus("Filter generated successfully!");
        }
        else
        {
            updateStatus("Console tool timed out");
        }
    }
    else
    {
        updateStatus("Failed to start console tool");
    }
}

void RangerDesigner::exportResults()
{
    if (resultsEditor.getText().isEmpty() || resultsEditor.getText() == "No results yet. Load a file and generate filters.")
    {
        updateStatus("Please generate a filter first");
        return;
    }
    
    auto chooser = std::make_unique<juce::FileChooser>("Save MinPhaseBank.h...",
                                                       juce::File(),
                                                       "*.h");
    
    auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;
    
    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file != juce::File{})
        {
            file.replaceWithText(resultsEditor.getText());
            updateStatus("Exported to: " + file.getFileName());
        }
    });
}

void RangerDesigner::clearAll()
{
    filePathEditor.clear();
    resultsEditor.setText("No results yet. Load a file and generate filters.");
    updateStatus("Cleared all data");
}

void RangerDesigner::updateStatus(const juce::String& message)
{
    statusLabel.setText(message, juce::dontSendNotification);
    repaint();
}