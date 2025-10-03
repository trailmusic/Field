# Field Audio Plugin - Build Instructions

## 🎵 Overview

Field is a professional spatial audio processor with three build targets:
- **Standalone**: Standalone application for testing and direct use
- **AU Plugin**: Audio Unit plugin for macOS DAWs
- **VST3 Plugin**: VST3 plugin for cross-platform DAWs

## 🎛️ Recent Improvements (January 2025)

### **VerticalSlider3D System Enhancements**
- **Professional 3D Sliders**: Enhanced visual feedback with gradient accent indication
- **Dynamic Marker System**: Context-aware markers (dB range: -20 to +12, Mix: 25-100)
- **Improved Layout**: Wider sliders with optimized gaps (4px) and increased container space
- **Clean Visual Design**: Removed distracting markers, larger fonts (9pt), gradient feedback
- **Theme Integration**: Seamless integration with Field's theming system
- **Performance Optimized**: Efficient rendering with pre-computed geometry
- **Enhanced Container Space**: 30% meters, 15% sliders for better layout flexibility

### **Key Slider Features**
- **Gradient Accent Feedback**: Visual indication of current value with smooth gradients
- **Smart Marker System**: Essential markers only (removed -60, -40, 0 from dB, 0 from Mix)
- **Professional Handle Design**: Accent frame with dark interior, no underglow
- **Bottom Value Labels**: Real-time value display with proper formatting
- **Responsive Layout**: Automatic sizing based on container constraints

## 🚀 Quick Build

### Build All Targets (Recommended)
```bash
./build_all.sh
```

This builds all three targets and ensures they are identical and up to date.

### Build with Auto-Test (Enhanced)
```bash
./build_and_test.sh
```

This builds all targets and automatically manages the Field app:
- **If Field is running**: Closes it and reopens with the new build
- **If Field is not running**: Simply launches the new build
- **Smart detection**: Checks for both AU plugin and Standalone app
- **Fallback handling**: Tries AU plugin first, then Standalone app

### Build All (one-liner from build/)
```bash
cmake --build . --target Field_Standalone Field_AU Field_VST3 --config Debug -- -j 8
```

Use this when already inside the `build/` directory to always compile Standalone, AU, and VST3 together.

### Verify Builds
```bash
./verify_builds.sh
```

This verifies that all builds are recent and properly installed.

## 🔧 Manual Build Process

### Prerequisites
- macOS 10.15 or later
- Xcode Command Line Tools
- CMake 3.15 or later

### Build Steps

1. **Configure the project:**
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

2. **Build all targets (always build Standalone, AU, and VST3):**
   ```bash
   cmake --build . --target Field_Standalone Field_AU Field_VST3 --config Debug -- -j 8
   ```

   For a Release build:
   ```bash
   cmake --build . --target Field_Standalone Field_AU Field_VST3 --config Release -- -j 8
   ```

3. **Install plugins (automatic):**
   - AU Plugin: `~/Library/Audio/Plug-Ins/Components/Field.component`
   - VST3 Plugin: `~/Library/Audio/Plug-Ins/VST3/Field.vst3`

## 📦 Build Targets

### Standalone Application
- **Location**: `build/Source/Field_artefacts/Debug/Standalone/Field.app`
- **Use**: Direct testing and standalone audio processing
- **Features**: Full UI with all controls and real-time audio processing

### Audio Unit Plugin
- **Location**: `~/Library/Audio/Plug-Ins/Components/Field.component`
- **Use**: macOS DAWs (Logic Pro, GarageBand, etc.)
- **Features**: Full plugin functionality with host integration

### VST3 Plugin
- **Location**: `~/Library/Audio/Plug-Ins/VST3/Field.vst3`
- **Use**: Cross-platform DAWs (Ableton Live, Pro Tools, etc.)
- **Features**: Full plugin functionality with host integration

## 🎯 Important Notes

### Build Consistency
- **Always build all three targets** to ensure consistency
- The Standalone version is the reference implementation
- AU and VST3 versions must be identical to Standalone

### Plugin Installation
- Plugins are automatically installed to system directories
- No manual installation required
- DAWs will automatically detect new plugins

### Development Workflow
1. Make code changes
2. Run `./build_all.sh` to build all targets
3. Run `./verify_builds.sh` to confirm consistency
4. Test in Standalone first, then in DAWs

## 🐛 Troubleshooting

### Build Issues
- Ensure Xcode Command Line Tools are installed
- Check CMake version: `cmake --version`
- Clean build: `rm -rf build && mkdir build && cd build && cmake ..`

### Plugin Issues
- Restart DAW after installing new plugins
- Check plugin directories exist and are writable
- Verify plugin signatures in System Preferences > Security & Privacy

### VST3 Issues
- VST3 plugins require proper VST3 SDK configuration
- Check VST3 installation path in CMake configuration
- Ensure VST3 host compatibility

## 📋 Build Scripts

### `build_all.sh`
- Builds all three targets
- Provides clear success/failure feedback
- Ensures consistent builds

### `verify_builds.sh`
- Verifies all builds exist and are recent
- Shows build timestamps
- Identifies outdated builds

## 🎵 Features

### Current Implementation
- **Enhanced Reverb Knob**: Dual functionality (depth control + algorithm selection)
- **Professional Algorithms**: Inner, Outer, Deep with unique characteristics
- **Split Mode**: Independent L/R pan control with linking
- **Real-time Visualization**: Dynamic spectral EQ and waveform display
- **Preset System**: A/B comparison and preset management
- **Professional UI**: Modern design with hover effects and visual feedback

### Audio Processing
- **Multi-band EQ**: HP, LP, Tilt, Air, Mono controls
- **Spatial Processing**: Width, Pan, Depth controls
- **Saturation**: Drive and Mix controls
- **Professional Algorithms**: Three distinct space algorithms
- **Real-time Analysis**: Spectral response and waveform visualization
