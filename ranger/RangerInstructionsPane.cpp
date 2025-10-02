#include "RangerInstructionsPane.h"

RangerInstructionsPane::RangerInstructionsPane()
{
    titleLabel.setText("Field Ranger Instructions", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);
    
    instructionsEditor.setReadOnly(true);
    instructionsEditor.setMultiLine(true);
    instructionsEditor.setScrollbarsShown(true);
    instructionsEditor.setCaretVisible(false);
    instructionsEditor.setFont(juce::Font(12.0f));
    setupInstructions();
    addAndMakeVisible(instructionsEditor);
}

RangerInstructionsPane::~RangerInstructionsPane()
{
}

void RangerInstructionsPane::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));
    g.setColour(juce::Colour(0xff3d3d3d));
    g.drawRect(getLocalBounds(), 2);
}

void RangerInstructionsPane::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    titleLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10); // Spacer
    
    instructionsEditor.setBounds(bounds);
}

void RangerInstructionsPane::setupInstructions()
{
    juce::String instructions = 
        "FIELD RANGER - MIN-PHASE FIR TOOLCHAIN\n"
        "==========================================\n\n"
        "OVERVIEW\n"
        "--------\n"
        "Field Ranger is a professional desktop application for designing and analyzing \n"
        "minimum-phase FIR filters. It integrates with the Field audio plugin to provide \n"
        "high-quality oversampling and phase processing.\n\n"
        "QUICK START\n"
        "----------\n"
        "1. DESIGNER TAB: Load linear-phase FIR files and generate min-phase versions\n"
        "2. FILES TAB: Manage your FIR file collection with drag-and-drop support\n"
        "3. PLOTS TAB: Visualize frequency response, impulse response, phase, and group delay\n"
        "4. SETTINGS TAB: Configure sample rates, filter parameters, and export options\n"
        "5. INSTRUCTIONS TAB: This help guide (you are here!)\n\n"
        "DESIGNER TAB\n"
        "------------\n"
        "- Load Linear FIR: Click Load Linear FIR (CSV) to import your filter files\n"
        "- Filter Order: Adjust the number of taps (1-255) for your filter\n"
        "- Cutoff Frequency: Set the filter's corner frequency (20Hz - 20kHz)\n"
        "- Filter Type: Choose between Linear Phase and Minimum Phase FIR\n"
        "- Generate: Convert your linear-phase filter to minimum-phase\n"
        "- Export: Save the results as MinPhaseBank.h for Field plugin integration\n\n"
        "FILES TAB\n"
        "---------\n"
        "- Drag & Drop: Drag FIR files (.csv, .txt, .dat, .fir) directly onto the interface\n"
        "- File Management: Add, remove, and organize your filter collection\n"
        "- File Info: View detailed information about each loaded file\n"
        "- Batch Processing: Select multiple files for batch conversion\n\n"
        "PLOTS TAB\n"
        "---------\n"
        "- Plot Types: Choose from Frequency Response, Impulse Response, Phase Response, or Group Delay\n"
        "- Generate Plot: Create visual representations of your filter characteristics\n"
        "- Export Plot: Save plot data as CSV files for further analysis\n"
        "- Real-time Updates: Plots update automatically when you change filter parameters\n\n"
        "SETTINGS TAB\n"
        "------------\n"
        "- Sample Rate: Configure the target sample rate for your filters\n"
        "- Filter Parameters: Adjust precision, windowing, and analysis settings\n"
        "- Export Options: Set default paths and file formats\n"
        "- Console Tools: Configure integration with the Min-Phase FIR toolchain\n\n"
        "CONSOLE TOOLS INTEGRATION\n"
        "-------------------------\n"
        "Field Ranger automatically integrates with the console tools:\n"
        "- minphase: Converts linear-phase FIR to minimum-phase FIR\n"
        "- batch_minphase: Processes multiple filters and generates MinPhaseBank.h\n"
        "- Built-in FFT: Uses KissFFT for efficient spectral analysis\n\n"
        "WORKFLOW EXAMPLE\n"
        "----------------\n"
        "1. Load a linear-phase FIR file in the Designer tab\n"
        "2. Adjust parameters (order, cutoff, type) as needed\n"
        "3. Generate the minimum-phase version\n"
        "4. View the results in the Plots tab\n"
        "5. Export as MinPhaseBank.h for Field plugin integration\n"
        "6. Use the generated filters in your Field audio plugin\n\n"
        "FIELD PLUGIN INTEGRATION\n"
        "-------------------------\n"
        "The generated MinPhaseBank.h file can be directly integrated into the Field plugin:\n"
        "- Copy the generated file to the Field plugin source directory\n"
        "- The Field plugin will automatically use the new filter coefficients\n"
        "- Improved oversampling quality with custom minimum-phase filters\n"
        "- Professional-grade audio processing with your custom filters\n\n"
        "TROUBLESHOOTING\n"
        "---------------\n"
        "- File Loading Issues: Ensure your FIR files are in CSV format with numeric data\n"
        "- Build Errors: Make sure console tools are built before using Field Ranger\n"
        "- Plot Issues: Check that filter data is valid and parameters are within bounds\n"
        "- Export Problems: Verify write permissions in the target directory\n\n"
        "KEYBOARD SHORTCUTS\n"
        "------------------\n"
        "- Cmd+O: Open file dialog\n"
        "- Cmd+S: Save current results\n"
        "- Cmd+E: Export plot data\n"
        "- Cmd+R: Generate new plot\n"
        "- Tab: Switch between tabs\n"
        "- Esc: Close dialogs\n\n"
        "SUPPORT\n"
        "-------\n"
        "For technical support and advanced usage:\n"
        "- Check the documentation in /Ranger/docs/\n"
        "- Review the console tools in /Ranger/console/\n"
        "- Examine example files in /Ranger/console/examples/\n\n"
        "VERSION INFO\n"
        "------------\n"
        "Field Ranger v1.0.0\n"
        "Built with JUCE Framework\n"
        "Min-Phase FIR Toolchain Integration\n"
        "Field Plugin Compatible\n\n"
        "Happy Filter Designing!";

    instructionsEditor.setText(instructions);
}
