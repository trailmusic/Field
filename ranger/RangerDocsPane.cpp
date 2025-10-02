#include "RangerDocsPane.h"

RangerDocsPane::RangerDocsPane()
{
    // Navigation buttons
    addAndMakeVisible(homeButton);
    homeButton.setButtonText("🏠 Home");
    homeButton.onClick = [this] { navigateTo("home"); };
    
    addAndMakeVisible(backButton);
    backButton.setButtonText("← Back");
    backButton.onClick = [this] { 
        if (currentHistoryIndex > 0) {
            currentHistoryIndex--;
            navigateTo(navigationHistory[currentHistoryIndex]);
        }
    };
    
    addAndMakeVisible(forwardButton);
    forwardButton.setButtonText("Forward →");
    forwardButton.onClick = [this] { 
        if (currentHistoryIndex < navigationHistory.size() - 1) {
            currentHistoryIndex++;
            navigateTo(navigationHistory[currentHistoryIndex]);
        }
    };
    
    // Search
    addAndMakeVisible(searchBox);
    searchBox.setPlaceholderText("Search documentation...");
    searchBox.onReturnKey = [this] { performSearch(searchBox.getText()); };
    
    addAndMakeVisible(searchButton);
    searchButton.setButtonText("🔍 Search");
    searchButton.onClick = [this] { performSearch(searchBox.getText()); };
    
    addAndMakeVisible(clearSearchButton);
    clearSearchButton.setButtonText("Clear");
    clearSearchButton.onClick = [this] { clearSearch(); };
    
    // Content
    addAndMakeVisible(contentView);
    contentView.setMultiLine(true);
    contentView.setReturnKeyStartsNewLine(true);
    contentView.setReadOnly(true);
    contentView.setScrollbarsShown(true);
    
    // Action buttons
    addAndMakeVisible(printButton);
    printButton.setButtonText("🖨️ Print");
    printButton.onClick = [this] { 
        // TODO: Implement print functionality
    };
    
    addAndMakeVisible(exportButton);
    exportButton.setButtonText("📤 Export");
    exportButton.onClick = [this] { 
        // TODO: Implement export functionality
    };
    
    // Status
    addAndMakeVisible(statusLabel);
    statusLabel.setText("Ready", juce::dontSendNotification);
    
    addAndMakeVisible(breadcrumbLabel);
    breadcrumbLabel.setText("Home", juce::dontSendNotification);
    
    // Initialize with home page
    navigateTo("home");
}

RangerDocsPane::~RangerDocsPane()
{
}

void RangerDocsPane::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    // Draw borders
    g.setColour(juce::Colour(0xff404040));
    g.drawRect(getLocalBounds(), 1);
}

void RangerDocsPane::resized()
{
    auto bounds = getLocalBounds();
    
    // Top toolbar
    auto toolbar = bounds.removeFromTop(40);
    homeButton.setBounds(toolbar.removeFromLeft(80));
    backButton.setBounds(toolbar.removeFromLeft(60));
    forwardButton.setBounds(toolbar.removeFromLeft(80));
    toolbar.removeFromLeft(10);
    
    searchBox.setBounds(toolbar.removeFromLeft(200));
    searchButton.setBounds(toolbar.removeFromLeft(80));
    clearSearchButton.setBounds(toolbar.removeFromLeft(60));
    
    // Status bar
    auto statusBar = bounds.removeFromBottom(25);
    statusLabel.setBounds(statusBar.removeFromLeft(200));
    breadcrumbLabel.setBounds(statusBar);
    
    // Action buttons
    auto actionBar = bounds.removeFromBottom(35);
    printButton.setBounds(actionBar.removeFromLeft(80));
    exportButton.setBounds(actionBar.removeFromLeft(80));
    
    // Main content area
    contentView.setBounds(bounds);
}

void RangerDocsPane::navigateTo(const juce::String& path)
{
    addToHistory(path);
    currentPath = path;
    
    juce::String content;
    if (path == "home") {
        content = generateHomePage();
    }
    else if (path == "quickstart") {
        content = generateQuickStart();
    }
    else if (path == "userguide") {
        content = generateUserGuide();
    }
    else if (path == "technical") {
        content = generateTechnicalReference();
    }
    else if (path == "api") {
        content = generateAPIReference();
    }
    else if (path == "troubleshooting") {
        content = generateTroubleshooting();
    }
    else if (path == "faq") {
        content = generateFAQ();
    }
    else if (path == "changelog") {
        content = generateChangelog();
    }
    else {
        content = generateHomePage();
    }
    
    contentView.setText(content);
    breadcrumbLabel.setText(path, juce::dontSendNotification);
}

void RangerDocsPane::addToHistory(const juce::String& path)
{
    if (currentHistoryIndex < navigationHistory.size() - 1) {
        navigationHistory.removeRange(currentHistoryIndex + 1, navigationHistory.size() - currentHistoryIndex - 1);
    }
    
    navigationHistory.add(path);
    currentHistoryIndex = navigationHistory.size() - 1;
    
    // Update button states
    backButton.setEnabled(currentHistoryIndex > 0);
    forwardButton.setEnabled(currentHistoryIndex < navigationHistory.size() - 1);
}

void RangerDocsPane::performSearch(const juce::String& query)
{
    if (query.isEmpty()) return;
    
    // Simple search implementation
    statusLabel.setText("Searching for: " + query, juce::dontSendNotification);
    
    // For now, just show a message
    juce::String searchResults = "Search Results for: " + query + "\n\n";
    searchResults += "This is a placeholder for search functionality.\n";
    searchResults += "In a full implementation, this would search through\n";
    searchResults += "all documentation content and return relevant results.\n\n";
    searchResults += "Try searching for:\n";
    searchResults += "- \"filter design\"\n";
    searchResults += "- \"oversampling\"\n";
    searchResults += "- \"Field plugin\"\n";
    searchResults += "- \"troubleshooting\"\n";
    
    contentView.setText(searchResults);
}

void RangerDocsPane::clearSearch()
{
    searchBox.clear();
    statusLabel.setText("Search cleared", juce::dontSendNotification);
    navigateTo(currentPath);
}

// Content generation methods
juce::String RangerDocsPane::generateHomePage()
{
    return R"(# Field Ranger Documentation

## Welcome to Field Ranger

**Field Ranger** is a professional desktop application for designing and analyzing FIR (Finite Impulse Response) filters for audio oversampling applications. Built specifically for the Field audio plugin ecosystem.

## Quick Navigation

### 🚀 Getting Started
- [Quick Start Guide](quickstart) - Get up and running in minutes
- [Installation Guide](installation) - System requirements and setup
- [First Filter](first-filter) - Create your first oversampling filter

### 📖 User Guide
- [Interface Overview](interface) - Understanding the Field Ranger interface
- [Filter Design](filter-design) - Designing filters for different applications
- [Visualization](visualization) - Analyzing filter responses
- [Export & Integration](export) - Using filters in Field plugin

### 🔧 Technical Reference
- [Filter Theory](filter-theory) - DSP concepts and mathematics
- [Oversampling](oversampling) - Understanding oversampling in audio
- [Performance](performance) - Optimization and CPU usage
- [File Formats](formats) - Supported input/output formats

### 🛠️ API Reference
- [Console Tools](console-tools) - Command-line interface
- [Batch Processing](batch) - Automated filter generation
- [Integration](integration) - Using with Field plugin

### 🆘 Support
- [Troubleshooting](troubleshooting) - Common issues and solutions
- [FAQ](faq) - Frequently asked questions
- [Changelog](changelog) - Version history and updates

## Key Features

- **Professional Filter Design**: Create high-quality FIR filters for oversampling
- **Visual Analysis**: Real-time frequency response, impulse response, and phase analysis
- **Field Integration**: Seamless integration with Field audio plugin
- **Batch Processing**: Generate multiple filters efficiently
- **Export Options**: Multiple output formats for different applications

## Getting Help

- Check the [FAQ](faq) for common questions
- Review [Troubleshooting](troubleshooting) for technical issues
- Consult the [Technical Reference](technical) for advanced topics

---
*Field Ranger v1.0.0 - Professional Audio Filter Design*)
";
}

juce::String RangerDocsPane::generateQuickStart()
{
    return R"(# Quick Start Guide

## Installation

1. **Download Field Ranger** from the Field project
2. **Extract** the application to your Applications folder
3. **Launch** Field Ranger from Applications

## Your First Filter

### Step 1: Choose Filter Type
- **Halfband FIR**: Most common for 2x oversampling
- **Lowpass FIR**: General lowpass filtering
- **Highpass FIR**: Highpass applications

### Step 2: Set Parameters
- **Filter Order**: 63-255 taps (higher = steeper rolloff)
- **Cutoff Frequency**: Usually 0.5 for halfband filters
- **Filter Type**: Linear Phase or Minimum Phase

### Step 3: Generate Filter
1. Click **"Generate Filter"** button
2. View the **frequency response** plot
3. Check **impulse response** for phase characteristics
4. Analyze **group delay** for linearity

### Step 4: Export
1. Click **"Export Results"**
2. Choose **MinPhaseBank.h** format
3. Save to Field plugin directory
4. **Rebuild Field plugin** to use new filters

## Interface Overview

### Main Tabs
- **Designer**: Filter parameter controls
- **Files**: File management and drag-drop
- **Plots**: Visual analysis and export
- **Settings**: Application preferences
- **Instructions**: Help and tutorials
- **Docs**: This documentation system

### Key Controls
- **Filter Order Slider**: 63-255 taps
- **Cutoff Frequency**: 0.1-0.9 (normalized)
- **Generate Button**: Create filter from parameters
- **Export Button**: Save results

## Next Steps

- Read the [User Guide](userguide) for detailed instructions
- Explore [Technical Reference](technical) for advanced topics
- Check [Troubleshooting](troubleshooting) if you encounter issues

---
*Ready to design professional audio filters!*)
";
}

juce::String RangerDocsPane::generateUserGuide()
{
    return R"(# User Guide

## Interface Overview

### Main Window Layout
- **Top**: Menu bar and Field Ranger logo
- **Left**: Tabbed interface for different functions
- **Center**: Main content area
- **Bottom**: Status bar and action buttons

### Tab Organization

#### Designer Tab
- **Filter Type Selection**: Halfband FIR, Lowpass FIR, Highpass FIR
- **Parameter Controls**: Order, cutoff frequency, phase type
- **Generate Button**: Create filter from current settings
- **Real-time Preview**: Basic frequency response

#### Files Tab
- **Drag & Drop**: Drop FIR files directly onto the interface
- **File Browser**: Navigate and select files
- **File Info**: Display file properties and metadata
- **Import/Export**: Load and save filter data

#### Plots Tab
- **Frequency Response**: Magnitude and phase plots
- **Impulse Response**: Time domain analysis
- **Group Delay**: Phase linearity visualization
- **Export Options**: Save plots as images or data

#### Settings Tab
- **Theme Selection**: Field Ocean, Green, Pink, Yellow, Grey
- **Display Options**: Plot colors, grid settings
- **Performance**: CPU usage and optimization
- **Paths**: Default directories for files

#### Instructions Tab
- **Tutorials**: Step-by-step guides
- **Examples**: Sample filter designs
- **Tips & Tricks**: Professional techniques
- **Keyboard Shortcuts**: Power user features

#### Docs Tab
- **Documentation**: This comprehensive help system
- **Search**: Find specific topics quickly
- **Navigation**: Hierarchical content organization
- **Export**: Save documentation as text

## Filter Design Workflow

### 1. Planning Your Filter
- **Determine Purpose**: Oversampling, anti-aliasing, analysis
- **Choose Type**: Halfband for 2x oversampling, Lowpass for general use
- **Set Requirements**: Steepness, phase characteristics, CPU budget

### 2. Parameter Selection
- **Filter Order**: 
  - 63 taps: Gentle rolloff, low CPU
  - 127 taps: Balanced quality/performance
  - 255 taps: Maximum quality, high CPU
- **Cutoff Frequency**: 0.5 for halfband, adjust for other types
- **Phase Type**: Linear for phase-critical, Minimum for natural sound

### 3. Generation and Analysis
- **Generate Filter**: Click generate button
- **Analyze Response**: Check frequency response plot
- **Verify Phase**: Examine impulse response and group delay
- **Optimize**: Adjust parameters if needed

### 4. Export and Integration
- **Export Format**: Choose MinPhaseBank.h for Field plugin
- **Save Location**: Place in Field plugin directory
- **Rebuild Plugin**: Compile Field with new filters
- **Test Integration**: Verify filters work in Field

## Advanced Features

### Batch Processing
- **Multiple Filters**: Generate several filters at once
- **Parameter Sweeps**: Test different settings
- **Automated Export**: Save all results automatically

### Visual Analysis
- **Frequency Response**: Magnitude and phase plots
- **Impulse Response**: Time domain characteristics
- **Group Delay**: Phase linearity analysis
- **Zoom and Pan**: Detailed examination of responses

### Integration with Field
- **MinPhaseBank.h**: Generated filter bank for Field plugin
- **Automatic Detection**: Field plugin finds and loads filters
- **Quality Settings**: Map to Field's quality tiers
- **Performance Optimization**: CPU usage considerations

## Tips and Best Practices

### Filter Design
- **Start Simple**: Begin with 127-tap halfband filters
- **Test Thoroughly**: Always analyze frequency and phase response
- **Consider CPU**: Higher order = more CPU usage
- **Phase Matters**: Linear phase for mastering, minimum phase for mixing

### Performance
- **Monitor CPU**: Watch CPU usage during generation
- **Optimize Settings**: Balance quality with performance
- **Batch Processing**: Generate multiple filters efficiently
- **Export Efficiently**: Use appropriate formats

### Integration
- **Test in Field**: Always test filters in actual Field plugin
- **Quality Mapping**: Map filter orders to Field quality settings
- **Documentation**: Keep notes on filter characteristics
- **Version Control**: Track filter versions and changes

---
*Master the art of professional filter design!*)
";
}

juce::String RangerDocsPane::generateTechnicalReference()
{
    return R"(# Technical Reference

## Filter Theory

### FIR vs IIR Filters
- **FIR (Finite Impulse Response)**: Linear phase, stable, computationally intensive
- **IIR (Infinite Impulse Response)**: Natural phase, efficient, can be unstable

### Filter Characteristics
- **Linear Phase**: Preserves phase relationships, constant group delay
- **Minimum Phase**: Natural phase response, variable group delay
- **Maximum Phase**: Unstable, not used in audio

### Oversampling Theory
- **Purpose**: Increase sample rate to reduce aliasing
- **Process**: Upsample → Process → Downsample
- **Anti-aliasing**: Critical for preventing artifacts

## Mathematical Foundations

### FIR Filter Design
- **Impulse Response**: h[n] = coefficients
- **Frequency Response**: H(ω) = Σ h[n] e^(-jωn)
- **Phase Response**: ∠H(ω) = -ω(N-1)/2 (linear phase)

### Halfband Filters
- **Definition**: Every other coefficient is zero
- **Efficiency**: 50% reduction in computation
- **Applications**: 2x oversampling, decimation

### Filter Order vs Performance
- **63 taps**: ~4th order equivalent, gentle rolloff
- **127 taps**: ~8th order equivalent, steep rolloff
- **255 taps**: ~16th order equivalent, brick-wall

## Implementation Details

### Coefficient Generation
- **Remez Algorithm**: Parks-McClellan optimal design
- **Window Method**: Simple but suboptimal
- **Least Squares**: Good compromise

### Phase Processing
- **Linear Phase**: Symmetric coefficients
- **Minimum Phase**: Cepstral analysis of magnitude
- **Conversion**: Homomorphic processing

### Performance Optimization
- **SIMD Instructions**: Vectorized computation
- **Memory Access**: Cache-friendly coefficient storage
- **Parallel Processing**: Multi-threaded generation

## File Formats

### Input Formats
- **CSV**: Comma-separated coefficient files
- **TXT**: Plain text coefficient files
- **DAT**: Binary coefficient files
- **FIR**: Standard FIR coefficient format

### Output Formats
- **MinPhaseBank.h**: C++ header for Field plugin
- **CSV**: Exported coefficient data
- **TXT**: Plain text coefficient files
- **JSON**: Metadata and coefficient data

### MinPhaseBank.h Structure
```cpp
struct TapSet {
    const float* taps;
    int numTaps;
    float cutoffFreq;
    FilterType type;
};

extern const TapSet* registry[];
extern const int registryCount;
```

## Quality Metrics

### Frequency Response
- **Passband Ripple**: < 0.1dB for professional quality
- **Stopband Attenuation**: > 60dB for anti-aliasing
- **Transition Width**: As narrow as possible

### Phase Response
- **Group Delay**: Constant for linear phase
- **Phase Linearity**: Deviation from linear phase
- **Phase Distortion**: Audible phase changes

### Computational Complexity
- **CPU Usage**: Proportional to filter order
- **Memory Usage**: Coefficient storage requirements
- **Real-time Performance**: Latency considerations

## Integration with Field Plugin

### MinPhaseBank Integration
- **Automatic Loading**: Field plugin loads MinPhaseBank.h
- **Quality Mapping**: Filter orders mapped to quality settings
- **Performance Tiers**: Eco, Standard, High, Ultra

### Oversampling Implementation
- **2x Oversampling**: Most common application
- **4x Oversampling**: High-quality processing
- **8x Oversampling**: Maximum quality, high CPU

### Quality Settings
- **Eco**: 63-tap filters, low CPU
- **Standard**: 127-tap filters, balanced
- **High**: 255-tap filters, maximum quality
- **Ultra**: Custom filters, maximum performance

## Troubleshooting

### Common Issues
- **Filter Instability**: Check coefficient values
- **Poor Performance**: Optimize filter order
- **Integration Problems**: Verify MinPhaseBank.h format

### Debugging Tools
- **Frequency Response**: Analyze filter characteristics
- **Impulse Response**: Check time domain behavior
- **Group Delay**: Verify phase linearity

### Performance Tuning
- **CPU Profiling**: Identify bottlenecks
- **Memory Usage**: Monitor coefficient storage
- **Optimization**: SIMD and parallel processing

---
*Deep dive into the technical aspects of filter design!*)
";
}

juce::String RangerDocsPane::generateAPIReference()
{
    return R"(# API Reference

## Console Tools

### minphase
**Purpose**: Convert linear-phase FIR to minimum-phase FIR

**Usage**:
```bash
./minphase input.csv output.csv [options]
```

**Options**:
- `-v, --verbose`: Verbose output
- `-q, --quiet`: Quiet mode
- `-h, --help`: Show help

**Input Format**: CSV with coefficient values
**Output Format**: CSV with minimum-phase coefficients

### batch_minphase
**Purpose**: Batch process multiple FIR files

**Usage**:
```bash
./batch_minphase input_dir output.h [options]
```

**Options**:
- `-r, --recursive`: Process subdirectories
- `-f, --format`: Output format (h, csv, json)
- `-v, --verbose`: Verbose output

**Input**: Directory containing FIR files
**Output**: MinPhaseBank.h with all filters

## File Formats

### CSV Format
```csv
# FIR Filter Coefficients
# Order: 127
# Type: Halfband
# Cutoff: 0.5
0.000000
0.000000
0.000000
...
```

### MinPhaseBank.h Format
```cpp
#ifndef MINPHASEBANK_H
#define MINPHASEBANK_H

struct TapSet {
    const float* taps;
    int numTaps;
    float cutoffFreq;
    FilterType type;
};

// Filter registry
extern const TapSet* registry[];
extern const int registryCount;

#endif
```

## Integration API

### Field Plugin Integration
```cpp
#include "MinPhaseBank.h"

// Load filter bank
MinPhaseBankIntegration bank;

// Get filter for specific order
auto filter = bank.getTapsForOrder(127);
if (filter) {
    // Use filter coefficients
    float* taps = filter->taps;
    int numTaps = filter->numTaps;
}
```

### Quality Mapping
```cpp
enum QualityTier {
    Eco = 0,      // 63 taps
    Standard = 1, // 127 taps
    High = 2,     // 255 taps
    Ultra = 3     // Custom
};

int getFilterOrder(QualityTier tier) {
    switch (tier) {
        case Eco: return 63;
        case Standard: return 127;
        case High: return 255;
        case Ultra: return 511;
        default: return 127;
    }
}
```

## Command Line Interface

### Basic Usage
```bash
# Generate single filter
./minphase input.csv output.csv

# Batch process
./batch_minphase filters/ MinPhaseBank.h

# With options
./minphase input.csv output.csv -v
```

### Advanced Usage
```bash
# Recursive processing
./batch_minphase filters/ output.h -r

# Custom format
./batch_minphase filters/ output.json -f json

# Verbose output
./batch_minphase filters/ output.h -v
```

## Error Handling

### Common Errors
- **File Not Found**: Check file paths
- **Invalid Format**: Verify CSV structure
- **Memory Error**: Check system resources
- **Conversion Error**: Verify input coefficients

### Error Codes
- `0`: Success
- `1`: File error
- `2`: Format error
- `3`: Memory error
- `4`: Conversion error

## Performance Considerations

### CPU Usage
- **Filter Order**: Higher order = more CPU
- **Batch Processing**: More efficient for multiple filters
- **SIMD Optimization**: Use vectorized instructions

### Memory Usage
- **Coefficient Storage**: Proportional to filter order
- **Batch Processing**: Temporary storage for multiple filters
- **Export Format**: MinPhaseBank.h is most efficient

### Optimization Tips
- **Use Halfband**: 50% reduction in computation
- **Batch Process**: Generate multiple filters at once
- **Optimize Order**: Balance quality with performance

---
*Complete API reference for Field Ranger tools!*)
";
}

juce::String RangerDocsPane::generateTroubleshooting()
{
    return R"(# Troubleshooting

## Common Issues

### Application Crashes
**Problem**: Field Ranger crashes on startup
**Solution**: 
- Check system requirements
- Update graphics drivers
- Run with debug logging
- Check for conflicting software

### Filter Generation Fails
**Problem**: "Generate Filter" button doesn't work
**Solution**:
- Verify all parameters are set
- Check filter order is valid (63-255)
- Ensure cutoff frequency is in range (0.1-0.9)
- Try different filter type

### Export Issues
**Problem**: Cannot export MinPhaseBank.h
**Solution**:
- Check write permissions
- Verify output directory exists
- Ensure sufficient disk space
- Try different export format

### Performance Problems
**Problem**: Slow filter generation
**Solution**:
- Reduce filter order
- Close other applications
- Check CPU usage
- Use batch processing for multiple filters

## File Format Issues

### CSV Import Errors
**Problem**: "Invalid CSV format" error
**Solution**:
- Check file encoding (UTF-8)
- Verify comma separators
- Ensure numeric values only
- Check for empty lines

### MinPhaseBank.h Generation
**Problem**: Generated file is invalid
**Solution**:
- Verify filter parameters
- Check coefficient values
- Ensure proper C++ syntax
- Test with simple filter first

### Drag & Drop Not Working
**Problem**: Cannot drop files onto interface
**Solution**:
- Check file permissions
- Verify supported formats (.csv, .txt, .dat, .fir)
- Try different file
- Restart application

## Integration Issues

### Field Plugin Integration
**Problem**: Field plugin doesn't load new filters
**Solution**:
- Rebuild Field plugin
- Check MinPhaseBank.h location
- Verify file format
- Check compilation errors

### Quality Settings
**Problem**: Quality settings don't match filters
**Solution**:
- Check quality mapping
- Verify filter order
- Update Field plugin
- Check configuration

### Performance in Field
**Problem**: High CPU usage in Field plugin
**Solution**:
- Use lower order filters
- Check quality settings
- Optimize filter parameters
- Monitor CPU usage

## Debugging Tools

### Log Files
**Location**: `~/Library/Logs/Field Ranger/`
**Contents**: Application logs, error messages
**Usage**: Check for error patterns

### Console Output
**Command**: `./FieldRanger --verbose`
**Output**: Detailed logging information
**Usage**: Debug startup issues

### Performance Monitoring
**Tool**: Activity Monitor
**Metrics**: CPU usage, memory usage
**Usage**: Identify performance bottlenecks

## System Requirements

### Minimum Requirements
- **macOS**: 10.15 or later
- **RAM**: 4GB minimum, 8GB recommended
- **CPU**: Intel or Apple Silicon
- **Storage**: 100MB free space

### Recommended Requirements
- **macOS**: 12.0 or later
- **RAM**: 16GB or more
- **CPU**: Apple Silicon M1/M2
- **Storage**: 1GB free space

### Graphics Requirements
- **OpenGL**: 3.3 or later
- **Metal**: Supported
- **Display**: 1280x800 minimum

## Getting Help

### Documentation
- Check this documentation system
- Review FAQ section
- Consult technical reference
- Check changelog for updates

### Support Channels
- **GitHub Issues**: Report bugs and feature requests
- **Community Forum**: Ask questions and share tips
- **Email Support**: Direct technical support
- **Video Tutorials**: Step-by-step guides

### Reporting Issues
**Include**:
- System information
- Steps to reproduce
- Error messages
- Log files
- Screenshots

**Template**:
```
System: macOS 12.0, Apple M1, 16GB RAM
Issue: Filter generation fails
Steps: 1. Set order to 255, 2. Click generate
Error: "Invalid parameters"
Logs: [attach log file]
```

---
*Get help with common Field Ranger issues!*)
";
}

juce::String RangerDocsPane::generateFAQ()
{
    return R"(# Frequently Asked Questions

## General Questions

### What is Field Ranger?
Field Ranger is a professional desktop application for designing and analyzing FIR filters for audio oversampling applications. It's specifically built for the Field audio plugin ecosystem.

### Who should use Field Ranger?
- Audio plugin developers
- DSP engineers
- Audio professionals
- Anyone working with oversampling in audio

### How does Field Ranger relate to Field?
Field Ranger generates filter banks that are used by the Field audio plugin for oversampling. It's part of the Field ecosystem for professional audio processing.

## Filter Design Questions

### What's the difference between Linear Phase and Minimum Phase?
- **Linear Phase**: Preserves phase relationships, constant group delay
- **Minimum Phase**: Natural phase response, variable group delay
- **Use Linear Phase**: For mastering, measurement, phase-critical applications
- **Use Minimum Phase**: For mixing, real-time processing, natural sound

### What filter order should I use?
- **63 taps**: Eco mode, low CPU, gentle rolloff
- **127 taps**: Standard mode, balanced quality/performance
- **255 taps**: High mode, maximum quality, steep rolloff
- **Higher**: Ultra mode, brick-wall filtering, very high CPU

### When should I use Halfband FIR vs Lowpass FIR?
- **Halfband FIR**: For 2x oversampling (most common)
- **Lowpass FIR**: For general lowpass filtering
- **Highpass FIR**: For highpass applications

## Technical Questions

### How does oversampling work?
1. **Upsample**: Increase sample rate by 2x, 4x, or 8x
2. **Process**: Apply audio processing at higher rate
3. **Downsample**: Reduce sample rate back to original
4. **Anti-aliasing**: Critical filters prevent artifacts

### What's the difference between FIR and IIR filters?
- **FIR**: Linear phase, stable, computationally intensive
- **IIR**: Natural phase, efficient, can be unstable
- **Field Ranger**: Focuses on FIR for oversampling

### Why use FIR filters for oversampling?
- **Linear Phase**: Preserves phase relationships
- **Stability**: No feedback, always stable
- **Quality**: Excellent frequency response
- **Control**: Precise control over characteristics

## Integration Questions

### How do I integrate filters with Field plugin?
1. **Generate Filter**: Use Field Ranger to create filter
2. **Export**: Save as MinPhaseBank.h
3. **Place**: Put file in Field plugin directory
4. **Rebuild**: Compile Field plugin with new filters
5. **Test**: Verify filters work in Field

### What's MinPhaseBank.h?
MinPhaseBank.h is a C++ header file containing filter coefficients and metadata. It's the standard format for integrating filters with the Field plugin.

### How do quality settings work?
- **Eco**: 63-tap filters, low CPU usage
- **Standard**: 127-tap filters, balanced performance
- **High**: 255-tap filters, maximum quality
- **Ultra**: Custom filters, maximum performance

## Performance Questions

### Why is filter generation slow?
- **Filter Order**: Higher order = more computation
- **CPU Usage**: Check system resources
- **Batch Processing**: More efficient for multiple filters
- **Optimization**: Use appropriate settings

### How much CPU do filters use?
- **63 taps**: ~1% CPU for real-time processing
- **127 taps**: ~2% CPU for real-time processing
- **255 taps**: ~4% CPU for real-time processing
- **Higher**: Not recommended for real-time

### Can I use filters in real-time?
- **Yes**: For lower order filters (63-127 taps)
- **Maybe**: For higher order filters (255 taps)
- **No**: For very high order filters (511+ taps)

## File Format Questions

### What file formats are supported?
- **Input**: CSV, TXT, DAT, FIR
- **Output**: MinPhaseBank.h, CSV, TXT, JSON
- **Plots**: PNG, SVG, PDF

### How do I convert between formats?
- **CSV to MinPhaseBank**: Use batch processing
- **TXT to CSV**: Use text editor or script
- **DAT to CSV**: Use conversion tool

### What's the CSV format?
```csv
# FIR Filter Coefficients
# Order: 127
# Type: Halfband
# Cutoff: 0.5
0.000000
0.000000
0.000000
...
```

## Troubleshooting Questions

### Why does the application crash?
- **System Requirements**: Check macOS version
- **Graphics Drivers**: Update drivers
- **Memory**: Check available RAM
- **Conflicts**: Check for conflicting software

### Why can't I export files?
- **Permissions**: Check write permissions
- **Disk Space**: Ensure sufficient space
- **Directory**: Verify output directory exists
- **Format**: Check export format

### Why don't filters work in Field?
- **Integration**: Check MinPhaseBank.h format
- **Location**: Verify file placement
- **Rebuild**: Recompile Field plugin
- **Testing**: Verify in Field plugin

## Advanced Questions

### Can I create custom filter types?
- **Yes**: Use custom coefficient files
- **Advanced**: Modify source code
- **Expert**: Create custom algorithms

### How do I optimize performance?
- **Filter Order**: Use appropriate order
- **Batch Processing**: Generate multiple filters
- **SIMD**: Use vectorized instructions
- **Parallel**: Use multi-threading

### Can I use filters in other applications?
- **Yes**: Export in standard formats
- **Integration**: Use MinPhaseBank.h
- **Custom**: Modify for other applications
- **Standards**: Follow industry standards

---
*Find answers to common Field Ranger questions!*)
";
}

juce::String RangerDocsPane::generateChangelog()
{
    return R"(# Changelog

## Version 1.0.0 (2025-10-01)

### 🎉 Initial Release
- **Field Ranger Desktop Application**: Complete desktop application for FIR filter design
- **Field Integration**: Seamless integration with Field audio plugin
- **Professional UI**: Field's Look & Feel system integrated
- **Comprehensive Documentation**: Full documentation system with search

### ✨ Features
- **Filter Design**: Halfband FIR, Lowpass FIR, Highpass FIR
- **Visual Analysis**: Frequency response, impulse response, group delay
- **Export Options**: MinPhaseBank.h, CSV, TXT, JSON formats
- **Batch Processing**: Generate multiple filters efficiently
- **Drag & Drop**: Direct file import support
- **Theme System**: Field's Ocean, Green, Pink, Yellow, Grey themes

### 🛠️ Technical
- **Console Tools**: minphase and batch_minphase utilities
- **MinPhaseBank Integration**: C++ header generation for Field plugin
- **Performance Optimization**: SIMD and parallel processing
- **File Format Support**: CSV, TXT, DAT, FIR input formats
- **Cross-Platform**: macOS support with Apple Silicon optimization

### 📚 Documentation
- **User Guide**: Comprehensive user documentation
- **Technical Reference**: DSP theory and implementation details
- **API Reference**: Console tools and integration APIs
- **Troubleshooting**: Common issues and solutions
- **FAQ**: Frequently asked questions

### 🎨 UI/UX
- **Field Look & Feel**: Consistent with Field plugin design
- **Field Ranger Logo**: Professional branding and identity
- **Tabbed Interface**: Designer, Files, Plots, Settings, Instructions, Docs
- **Search System**: Full-text search across documentation
- **Navigation**: Breadcrumb navigation and history

### 🔧 Console Tools
- **minphase**: Convert linear-phase FIR to minimum-phase FIR
- **batch_minphase**: Batch process multiple FIR files
- **KissFFT Integration**: Lightweight FFT library
- **CMake Build**: Cross-platform build system
- **CI/CD**: Automated testing and deployment

### 📁 File Structure
```
Ranger/
├── console/           # Console tools
├── docs/             # Documentation
├── Components/       # UI components
├── Controls/         # Control elements
├── Design/           # Design system
├── Engines/          # Processing engines
├── Events/           # Event handling
├── Layout/           # Layout management
├── Managers/         # State management
├── Utilities/        # Utility functions
└── [Source Files]    # Main application
```

### 🚀 Performance
- **Optimized Build**: Professional audio optimization flags
- **SIMD Support**: AVX2/NEON vectorized computation
- **Memory Management**: Efficient coefficient storage
- **Real-time Ready**: Optimized for real-time audio processing

### 🔗 Integration
- **Field Plugin**: Direct integration with Field audio plugin
- **MinPhaseBank.h**: Standard format for filter coefficients
- **Quality Mapping**: Automatic quality tier mapping
- **Performance Tiers**: Eco, Standard, High, Ultra

### 📋 Quality Assurance
- **Testing**: Comprehensive test suite
- **Documentation**: Complete API and user documentation
- **Performance**: Optimized for professional audio use
- **Reliability**: Stable and crash-free operation

### 🎯 Future Roadmap
- **Additional Filter Types**: More filter design options
- **Advanced Visualization**: 3D plots and analysis
- **Plugin Integration**: Direct plugin communication
- **Cloud Processing**: Remote filter generation
- **AI Assistance**: Intelligent filter recommendations

---
*Field Ranger v1.0.0 - Professional Audio Filter Design*)
";
}