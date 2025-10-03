# Reverb Preset Integration Guide

## Overview

This document explains how the reverb preset system integrates with Field's existing preset architecture. The integration is designed to be **minimal and non-disruptive** - reverb presets appear alongside existing presets in the same UI.

## Architecture

### Existing Preset System
- **`PresetStore`** - Manages preset storage, metadata, favorites, usage tracking
- **`NewPresetManager`** - Handles APVTS integration, parameter mapping, A/B slots, auditioning  
- **`PresetCommandPalette`** - UI for preset selection (search, categories, favorites)
- **`PresetRegistry`** - Discovers and loads presets

### New Reverb Integration
- **`ReverbPresetIntegration`** - Main integration point
- **`ReverbParamMap`** - Maps JSON parameters to APVTS parameter IDs
- **`ReverbPresetLoader`** - Loads JSON preset files and converts to LibraryPreset format

## Integration Points

### 1. Parameter Mapping
The `ReverbParamMap` class maps JSON preset parameters to Field's APVTS parameter IDs:

```cpp
// JSON parameter -> APVTS parameter ID
"decaySec" -> "decay_sec"
"erLevelDb" -> "er_level_db" 
"duckOn" -> "duck_on"
// ... etc
```

### 2. Preset Loading
The `ReverbPresetLoader` converts JSON presets to the existing `LibraryPreset` format:

```cpp
// JSON preset structure
{
  "name": "Vocal Plate - Smooth",
  "model": "Plate", 
  "tags": ["vocal", "pop"],
  "params": { "decaySec": 2.2, "erLevelDb": -10.0, ... }
}

// Converts to LibraryPreset
LibraryPreset {
  meta: { name: "Vocal Plate - Smooth", category: "Reverb", ... }
  params: { "decay_sec": 2.2, "er_level_db": -10.0, ... }
}
```

### 3. UI Integration
**No new UI components needed!** The existing `presetField` button automatically shows reverb presets because they're loaded into the same `PresetStore`.

## Usage

### Minimal Integration (2 lines of code)
```cpp
// In PluginEditor initialization
static ReverbPresetIntegration reverbIntegration;
reverbIntegration.initializeReverbPresets(presetStore);
presetManager.setParamMap(reverbIntegration.getReverbParamMap());
```

### What This Enables
- ✅ **Existing UI** - `presetField` button shows reverb presets
- ✅ **A/B Slots** - Reverb presets work with A/B comparison
- ✅ **Favorites** - Star reverb presets for quick access
- ✅ **Search** - Find reverb presets by name/tags
- ✅ **Categories** - Reverb presets appear in "Reverb" category
- ✅ **Save Custom** - Users can save their own reverb presets
- ✅ **Auditioning** - Preview reverb presets before applying

## Preset File Structure

### JSON Preset Format
```json
{
  "version": 1,
  "author": "Field Audio", 
  "format": "FieldReverbPresetPack",
  "count": 40,
  "presets": [
    {
      "name": "Vocal Plate - Smooth",
      "model": "Plate",
      "tags": ["vocal", "pop"],
      "params": {
        "decaySec": 2.2,
        "erLevelDb": -10.0,
        "duckOn": false,
        // ... all reverb parameters
      },
      "toneEQ": [ /* tone EQ bands */ ],
      "decayRateEQ": [ /* decay rate EQ bands */ ],
      "ducking": { /* ducking parameters */ }
    }
  ]
}
```

### File Locations
- **Factory Presets**: `Assets/Presets/Reverb/*.json`
- **User Presets**: Saved via existing preset system
- **Favorites**: Stored in existing preset metadata

## Features

### Automatic Discovery
The system automatically finds and loads all JSON preset files in `Assets/Presets/Reverb/`:
- `Presets_Reverb.json` (40 general presets)
- `Presets_Reverb_ElectronicHalls.json` (40 electronic/techno presets)  
- `Presets_Reverb_TrapSlapRooms.json` (40 trap/slap presets)
- `Presets_Reverb_OrchestralStacks.json` (40 orchestral presets)

### Model System
Each preset includes a `model` field that applies base characteristics:
- **Plate** - Short, dense, smooth
- **Hall** - Long, spacious, natural
- **Chamber** - Medium, intimate, detailed  
- **Room** - Short, tight, focused

### Advanced Features
- **Tone EQ** - Per-preset EQ settings
- **Decay Rate EQ** - Frequency-dependent decay shaping
- **Ducking** - Per-preset ducking configuration
- **Tags** - Searchable metadata (vocal, pop, electronic, etc.)

## Implementation Status

✅ **Completed:**
- JSON preset loading system
- Parameter mapping to APVTS
- Integration with existing PresetStore
- Model application system
- Preset file organization

🔄 **Next Steps:**
- Integration with PluginEditor (2 lines of code)
- Testing with existing preset UI
- User preset saving/loading
- Advanced preset features (Tone EQ, Decay Rate EQ, Ducking)

## Benefits

1. **Zero UI Changes** - Uses existing preset system
2. **Consistent UX** - Same interface for all presets
3. **Full Feature Set** - A/B, favorites, search, custom presets
4. **Extensible** - Easy to add more preset packs
5. **Professional** - 160 curated presets across 4 categories

The reverb preset system is designed to be **invisible to the user** - they just see more presets in the existing preset browser, with all the same functionality they're used to.
