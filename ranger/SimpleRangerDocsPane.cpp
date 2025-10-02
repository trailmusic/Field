#include "SimpleRangerDocsPane.h"

SimpleRangerDocsPane::SimpleRangerDocsPane()
{
    addAndMakeVisible(docContent);
    docContent.setReadOnly(true);
    docContent.setMultiLine(true);
    docContent.setReturnKeyStartsNewLine(true);
    docContent.setScrollbarsShown(true);
    
    setupDocumentation();
}

void SimpleRangerDocsPane::resized()
{
    docContent.setBounds(getLocalBounds().reduced(10));
}

void SimpleRangerDocsPane::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
}

void SimpleRangerDocsPane::setupDocumentation()
{
    juce::String content = R"(
# Field Ranger Documentation

## Overview
Field Ranger is a professional filter design tool for creating minimum-phase FIR filters for the Field audio plugin.

## Getting Started
1. Load a linear-phase FIR filter file (.csv, .txt, .dat, .fir)
2. Configure filter parameters
3. Generate minimum-phase conversion
4. Export results

## Filter Types
- **Halfband FIR**: Symmetric halfband filters for oversampling
- **Lowpass FIR**: General lowpass filters
- **Highpass FIR**: General highpass filters

## Filter Orders
- Range: 63-255 taps
- Higher orders provide better stopband rejection
- Lower orders are more efficient

## Quality Settings
- **Eco**: Basic quality, fast processing
- **Standard**: Balanced quality and performance
- **High**: Maximum quality, slower processing

## Export Options
- CSV format for analysis
- C++ header files for plugin integration
- Batch processing for multiple filters

## Tips
- Use higher filter orders for critical applications
- Test filters with different sample rates
- Verify frequency response characteristics
- Check for aliasing artifacts

## Troubleshooting
- Ensure input files are properly formatted
- Check filter order is within valid range
- Verify sample rate compatibility
- Test with known good reference filters
)";
    
    docContent.setText(content);
}
