# Reverb Preset System Integration Guide

*Version 2.1 (Jan 2025) - Complete Preset System Integration*

This document provides comprehensive guidance for integrating the Field Reverb preset system with the main Field plugin architecture.

## 📋 Overview

The Field Reverb preset system provides 320 professional presets across 8 categories, with complete JSON-based parameter mapping and seamless integration with Field's existing preset management infrastructure.

## 🎯 System Architecture

### **Core Components**

```cpp
// Main integration point
ReverbPresetIntegration     // Entry point for preset system integration

// Preset management
ReverbPresetManager         // Preset loading and application
ReverbPresetLoader          // JSON file loading and parsing
ReverbParamMap              // JSON ↔ APVTS parameter mapping
ModelMacros                 // Model defaults and macros

// UI components
ReverbPresetBrowser         // Preset browser UI component
```

### **File Structure**
```
Source/features/reverb/Presets/
├── ReverbPresetManager.h/.cpp      // Main preset management
├── ReverbPresetLoader.h/.cpp       // JSON file loading
├── ReverbParamMap.h/.cpp           // Parameter mapping
├── ReverbPresetIntegration.h/.cpp  // System integration
├── ReverbPresetIntegrationExample.h // Usage examples
├── ReverbPresetBrowser.h           // UI browser component
└── ModelMacros.h                   // Model defaults

Assets/Presets/Reverb/
├── Presets_Reverb_General.json     // 40 general reverb presets
├── Presets_Reverb_AmbientPads.json // 40 ambient pad presets
├── Presets_Reverb_DrumPlates.json // 40 drum plate presets
├── Presets_Reverb_ElectronicHalls.json // 40 electronic hall presets
├── Presets_Reverb_GuitarRooms.json // 40 guitar room presets
├── Presets_Reverb_OrchestralStacks.json // 40 orchestral stack presets
├── Presets_Reverb_Retro80s.json    // 40 retro 80s presets
└── Presets_Reverb_TrapSlapRooms.json // 40 trap slap room presets
```

## 🚀 Integration Steps

### **Step 1: Initialize Preset System**

```cpp
// In your main plugin initialization
#include "features/reverb/Presets/ReverbPresetIntegration.h"

class MyPluginAudioProcessor : public juce::AudioProcessor
{
private:
    std::unique_ptr<ReverbPresetIntegration> reverbPresetIntegration;
    
public:
    MyPluginAudioProcessor()
    {
        // Initialize preset system
        reverbPresetIntegration = std::make_unique<ReverbPresetIntegration>();
    }
    
    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        // Initialize reverb presets with your preset store
        reverbPresetIntegration->initializeReverbPresets(*presetStore);
    }
};
```

### **Step 2: Load Preset Packs**

```cpp
// Load specific preset pack
void loadReverbPresetPack(const juce::File& jsonFile)
{
    reverbPresetIntegration->loadReverbPresetPack(jsonFile, *presetStore);
}

// Auto-discover and load all preset packs
void loadAllReverbPresets()
{
    auto reverbPresetDir = juce::File("Assets/Presets/Reverb");
    if (reverbPresetDir.exists())
    {
        auto jsonFiles = reverbPresetDir.findChildFiles(
            juce::File::findFiles, false, "*.json");
            
        for (auto& file : jsonFiles)
        {
            loadReverbPresetPack(file);
        }
    }
}
```

### **Step 3: Apply Presets**

```cpp
// Apply preset to reverb engine
void applyReverbPreset(int presetIndex, ReverbParams& reverbParams)
{
    ReverbPresetManager presetManager;
    presetManager.applyPreset(presetIndex, reverbParams);
}

// Get preset information
PresetInfo getPresetInfo(int presetIndex)
{
    ReverbPresetManager presetManager;
    return presetManager.getPresetInfo(presetIndex);
}
```

## 📊 Preset Categories

### **1. General Reverb (40 presets)**
- **Purpose**: Versatile all-purpose reverb
- **Characteristics**: Balanced decay times, moderate diffusion
- **Use Cases**: General mixing, vocals, instruments

### **2. Ambient Pads (40 presets)**
- **Purpose**: Long decays with shimmer effects
- **Characteristics**: Extended decay times, high diffusion, shimmer
- **Use Cases**: Ambient music, pads, atmospheric effects

### **3. Drum Plates (40 presets)**
- **Purpose**: Tight decay with ducking for drums
- **Characteristics**: Short decay times, high density, ducking enabled
- **Use Cases**: Drum processing, percussion, rhythm instruments

### **4. Electronic Halls (40 presets)**
- **Purpose**: Techno/electronic with high diffusion
- **Characteristics**: High diffusion, electronic character
- **Use Cases**: Electronic music, techno, EDM

### **5. Guitar Rooms (40 presets)**
- **Purpose**: Compact/low-pre-delay for guitar
- **Characteristics**: Short pre-delay, moderate decay, guitar-optimized
- **Use Cases**: Guitar processing, amp simulation, instrument effects

### **6. Orchestral Stacks (40 presets)**
- **Purpose**: Room→chamber→hall combinations
- **Characteristics**: Layered reverb, orchestral character
- **Use Cases**: Orchestral music, classical, cinematic

### **7. Retro 80s (40 presets)**
- **Purpose**: Gated effects with bright shelves
- **Characteristics**: Gated decay, bright EQ, 80s character
- **Use Cases**: Retro music, 80s style, vintage effects

### **8. Trap Slap Rooms (40 presets)**
- **Purpose**: Short/bright, pre-delayed, gated
- **Characteristics**: Short decay, high pre-delay, gated
- **Use Cases**: Trap music, hip-hop, modern production

## 🔧 Parameter Mapping

### **JSON Structure**
```json
{
  "presetName": "Hall Reverb",
  "presetCategory": "General",
  "presetAuthor": "Field Audio",
  "presetVersion": "1.0",
  "parameters": {
    "decaySec": 2.4,
    "erLevelDb": -18.0,
    "erTimeMs": 55.0,
    "erDensityPct": 70.0,
    "erWidthPct": 100.0,
    "diffusionPct": 80.0,
    "densityPct": 75.0,
    "modDepthCents": 6.0,
    "modRateHz": 0.35,
    "widthPct": 100.0,
    "rotationDeg": 0.0,
    "wetMix01": 0.3,
    "bloomPct": 0.0,
    "distancePct": 0.0,
    "freeze": false,
    "shimmerAmtPct": 0.0,
    "shimmerInt": 0.0,
    "gateAmtPct": 0.0,
    "outTrimDb": 0.0,
    "followWidth": false,
    "followWidthAmt": 0.0,
    "followRot": false,
    "followRotAmt": 0.0,
    "duckOn": false,
    "duckMode": "General",
    "duckDepthDb": -6.0,
    "duckThrDb": -18.0,
    "duckRatio": 3.0,
    "duckKneeDb": 2.0,
    "duckAtkMs": 10.0,
    "duckRelMs": 100.0,
    "duckBandHz": 1000.0,
    "duckBandQ": 1.0,
    "duckDetectorSrc": "Dry"
  }
}
```

### **APVTS Mapping**
```cpp
// Parameter mapping from JSON to APVTS
ReverbParamMap paramMap;
paramMap.jsonToApvts(jsonParams, apvtsParams);
paramMap.apvtsToJson(apvtsParams, jsonParams);
```

## 🎛️ UI Integration

### **Preset Browser Component**
```cpp
// Add preset browser to your UI
class ReverbPresetBrowser : public juce::Component
{
public:
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    
private:
    std::vector<PresetInfo> presets;
    int selectedPreset = -1;
};
```

### **Preset Selection**
```cpp
// Handle preset selection
void onPresetSelected(int presetIndex)
{
    // Apply preset to reverb engine
    applyReverbPreset(presetIndex, reverbParams);
    
    // Update UI
    updateReverbControls();
}
```

## 🧪 Testing & Validation

### **Preset Loading Test**
```cpp
// Test preset loading
void testPresetLoading()
{
    ReverbPresetLoader loader;
    auto presets = loader.loadPresetsFromDirectory("Assets/Presets/Reverb");
    
    // Verify all 320 presets loaded
    assert(presets.size() == 320);
    
    // Verify each category has 40 presets
    for (const auto& category : {"General", "AmbientPads", "DrumPlates", 
                                "ElectronicHalls", "GuitarRooms", 
                                "OrchestralStacks", "Retro80s", "TrapSlapRooms"})
    {
        auto categoryPresets = filterByCategory(presets, category);
        assert(categoryPresets.size() == 40);
    }
}
```

### **Parameter Mapping Test**
```cpp
// Test parameter mapping
void testParameterMapping()
{
    ReverbParamMap paramMap;
    
    // Test JSON to APVTS conversion
    juce::var jsonParams = loadJsonPreset("test_preset.json");
    juce::NamedValueSet apvtsParams;
    paramMap.jsonToApvts(jsonParams, apvtsParams);
    
    // Verify parameter values
    assert(apvtsParams["decaySec"].getValue() == 2.4);
    assert(apvtsParams["erLevelDb"].getValue() == -18.0);
}
```

## 📈 Performance Considerations

### **Loading Performance**
- **Lazy Loading**: Load presets on-demand rather than all at startup
- **Caching**: Cache frequently used presets in memory
- **Background Loading**: Load preset metadata in background thread

### **Memory Usage**
- **Preset Size**: Each preset ~2KB JSON, total ~640KB for all presets
- **Parameter Objects**: Minimal memory overhead for parameter mapping
- **UI Components**: Efficient preset browser with virtual scrolling

## 🔍 Debugging & Troubleshooting

### **Common Issues**

1. **Preset Loading Failures**
   - Check JSON file format and syntax
   - Verify file paths and permissions
   - Ensure all required parameters are present

2. **Parameter Mapping Errors**
   - Verify parameter ID consistency between JSON and APVTS
   - Check parameter value ranges and types
   - Validate parameter names and case sensitivity

3. **UI Integration Problems**
   - Ensure preset browser is properly initialized
   - Check event handling and callback registration
   - Verify UI component lifecycle management

### **Debug Tools**
```cpp
// Enable debug logging
#define REVERB_PRESET_DEBUG 1

// Debug preset loading
void debugPresetLoading()
{
    #if REVERB_PRESET_DEBUG
    DBG("Loading preset: " << presetName);
    DBG("Parameters: " << parameterCount);
    #endif
}
```

## 📚 Additional Resources

- **Main Documentation**: `Reverb.md` - Complete system documentation
- **Testing Guide**: `ReverbTesting.md` - Validation procedures
- **API Reference**: `ReverbPresetIntegration.h` - Complete API documentation
- **Examples**: `ReverbPresetIntegrationExample.h` - Usage examples

## 🎯 Best Practices

1. **Preset Organization**: Keep presets organized by category and use case
2. **Parameter Validation**: Always validate parameter values before application
3. **Error Handling**: Implement robust error handling for preset loading failures
4. **Performance**: Use lazy loading and caching for large preset collections
5. **User Experience**: Provide clear preset names and categories for easy selection

---

*This document provides comprehensive guidance for integrating the Field Reverb preset system. For additional support or questions, refer to the main Reverb documentation or contact the development team.*
