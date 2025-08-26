# Field - Professional Spatial Audio Processor

A sophisticated spatial audio processing plugin built with JUCE, featuring **Ableton-accurate panning**, advanced DSP processing, real-time waveform visualization, **comprehensive preset management**, and a modern **container-based UI** with custom icon language. Field provides complete stereo field manipulation, saturation, filtering, and spatial enhancement tools with professional-grade precision.

## 🎯 Overview

Field is a professional-grade spatial audio processor designed for modern music production. It combines traditional stereo processing tools with **Ableton Live-accurate panning behavior**, innovative visual feedback, advanced DSP algorithms, and a **comprehensive preset system** to give you complete control over your stereo field.

## ✨ Key Features

### 🎨 **Modern Container-Based UI**
- **Professional container layout** with organized control groupings
- **Custom icon language** with 23+ vector-based icons
- **Larger lock icons** (24x24px) for better visibility
- **Centered value labels** positioned directly below each knob
- **Soft-shadow design** with ocean accent colors
- **Responsive layout** with draggable resize handle (50%-200% scaling)
- **Full screen mode** with top-right corner toggle button
- **Real-time visual feedback** for all parameters
- **Split percentage visualization** on Pan knob borders

### 📦 **Organized Control Containers**

#### **SPACE Container**
- **Space Knob** - Spatial depth and reverb amount
- **Ducking Knob** - Sidechain-style compression with reduced padding
- **Container styling** matching XY Pad aesthetic

#### **PAN Container**  
- **Pan Knob** - Ableton-accurate constant-power panning
- **Lock Button** - Parameter locking with larger, more visible icons
- **Professional container** with consistent visual design

#### **VOLUME Container**
- **Gain** - Input level control (-24dB to +24dB)
- **Drive** - Saturation drive amount with harmonic visualization
- **Mix** - Saturation wet/dry blend
- **Width** - Enhanced stereo width control (0-500%)

#### **EQ Container**
- **Air** - Maag-style high-shelf enhancement (0-6dB)
- **Tilt** - Frequency balance control with visual curve
- **HP** - High-pass filter (20Hz-1kHz) with blue curve
- **LP** - Low-pass filter (2kHz-20kHz) with blue curve
- **Mono** - Low-frequency mono summing for phase coherence

### 🎛️ **Comprehensive Preset System**

#### **Searchable Preset Management**
- **200px wide dropdown** with search functionality
- **Type-to-search** through all presets by name, description, author, or category
- **Category organization** with professional grouping
- **Save button** with custom floppy disk icon
- **JSON-based storage** for user presets

#### **Built-in Preset Categories**

**📊 Studio Presets**
- **Default** - Clean, balanced starting point
- **Wide Stereo** - Expansive stereo field with air enhancement
- **Narrow Focus** - Tight, focused stereo image
- **Center Focus** - Emphasizes center content

**🎵 Mixing Presets**
- **Drum Bus** - Wide, punchy drum processing
- **Vocal Space** - Airy vocal enhancement
- **Bass Mono** - Solid bass foundation with mono summing
- **Guitar Width** - Expansive guitar processing

**🎚️ Mastering Presets**
- **Master Wide** - Wide mastering chain
- **Master Punch** - Punchy mastering with impact
- **Master Air** - Airy mastering with high-end enhancement

**🎨 Creative Presets**
- **Extreme Width** - Maximum stereo expansion (450%)
- **Hard Pan** - Ableton-style hard panning technique
- **Space Echo** - Echo-like spatial effects

#### **User Preset Features**
- **Custom categories** and subcategories
- **Author attribution** for preset sharing
- **Automatic parameter capture** from current settings
- **Professional preset management** workflow

### 🎨 **Custom Icon Language**
Field features a comprehensive **21-icon vector system**:

**🔒 Control Icons**
- **Lock/Unlock** - Parameter locking states
- **Save** - Preset saving functionality
- **Power** - Bypass button with clean on/off states
- **Options** - Settings and configuration menu

**🎛️ Parameter Icons**
- **Speaker** - Volume/gain controls
- **Pan** - Panning and spatial positioning
- **Space** - Spatial depth and reverb
- **Width** - Stereo width processing
- **Tilt** - Frequency tilt and balance

**🎚️ Processing Icons**
- **HP/LP** - High-pass and low-pass filters
- **Drive** - Saturation and harmonic processing
- **Mix** - Wet/dry blend controls
- **Air** - High-frequency enhancement
- **Duck** - Sidechain compression effects

**🔗 Interface Icons**
- **Link** - Parameter linking and synchronization
- **Stereo/Split** - Processing mode indicators
- **Bypass** - Effect bypass visualization
- **Full Screen/Exit Full Screen** - Full screen mode toggle

### 🎛️ **Complete DSP Processing Chain**
1. **Input Gain** - Pre-processing level control (-24dB to +24dB)
2. **High-Pass Filter** - Low-frequency removal (20Hz-1kHz, 2nd order Butterworth)
3. **Low-Pass Filter** - High-frequency shaping (1kHz-20kHz, 2nd order Butterworth)
4. **Tilt EQ** - Frequency balance control with visual curve feedback
5. **Air Band** - Maag-style high-shelf enhancement (0-6dB, positive only)
6. **Space Algorithms** - Three distinct spatial enhancement modes
7. **Pan Control** - **Ableton-accurate constant-power panning** (center=0dB, extremes=+3dB)
8. **Stereo Width** - Enhanced mid-side processing (0-500%, 10x precision)
9. **Mono Maker** - Low-frequency mono summing for phase coherence
10. **Saturation** - Musical soft-clipping with harmonic enhancement
11. **Output Gain** - Final level control with color-coded feedback

### 🎵 **Advanced Spatial Processing**
- **STEREO/SPLIT Mode** - Single ball vs dual independent ball control
- **Ball Linking** - Synchronize dual ball movement in split mode
- **Ableton-Style Split Panning** - Independent L/R channel placement
- **Space Algorithms**:
  - **Inner**: Clean and bright with scooped mids
  - **Outer**: Moderate reverb with balanced character
  - **Deep**: Long, soft reverb with heavy damping

### 📊 **Real-Time Visualization**
- **Live Waveform Display** - Input vs processed signal comparison
- **Elegant Drive Lines** - Thin, refined harmonic visualization
- **EQ Response Curves** - Real-time filter response with separate Air curve
- **Tilt Curve Visualization** - Orange dashed curve shows frequency tilt
- **Color-Coded Processing** - Orange/blue Drive, white Air, blue HP/LP curves
- **Directional Flow Control** - Configurable waveform sample direction

## 🎛️ Controls & Parameters

### **Container-Based Layout**

#### **SPACE Container (Top Left)**
- **Space Knob** - Spatial depth and reverb amount with custom space icon
- **Ducking Knob** - Positioned with reduced padding for better workflow

#### **PAN Container (Top Right)**  
- **Pan Knob** - **Ableton-accurate panning** with visual split percentage borders
- **Lock Button** - Larger 28x28px lock icons for better visibility

#### **VOLUME Container (Middle Left)**
- **Gain** - Input level control (-24dB to +24dB) with speaker icon
- **Drive** - Saturation drive amount (0-36dB) with drive icon
- **Mix** - Saturation wet/dry blend (0-100%) with mix icon
- **Width** - Enhanced stereo width control (0-500%) with width icon

#### **EQ Container (Bottom Full Width)**
- **Air** - High-shelf enhancement (0-6dB) with air icon
- **Tilt** - Frequency balance control with tilt icon
- **HP Hz** - High-pass filter frequency (20Hz-1kHz) with HP icon
- **LP Hz** - Low-pass filter frequency (2kHz-20kHz) with LP icon
- **Mono Hz** - Low-frequency mono summing (0-300Hz) with mono icon

#### **Mini Frequency Sliders**
- **Tilt Frequency** - Controls the center frequency of the tilt EQ (100Hz-1kHz, logarithmic)
- **Scoop Frequency** - Sets the center frequency for scoop/boost processing (200Hz-2kHz, logarithmic)
- **Bass Frequency** - Controls the bass shelf frequency (50Hz-500Hz, logarithmic)
- **Air Frequency** - Sets the air enhancement frequency (2kHz-20kHz, logarithmic)

### **Header Controls**
- **Options Button** - Settings menu with cog wheel icon (bottom-left)
- **Bypass Button** - Plugin bypass with power icon (bottom-left)
- **Full Screen Button** - Toggle full screen mode (top-right corner)
- **Preset Dropdown** - 200px searchable preset selection
- **Save Button** - Preset saving with floppy disk icon

### **Mode Controls**
- **STEREO/SPLIT Toggle** - Mode switching with custom stereo/split icons
- **Link Button** - Ball synchronization with link icon
- **Lock Buttons** - Individual parameter locking (28x28px icons)

### **Value Display Improvements**
- **Centered labels** - All parameter values positioned directly below knobs
- **40px width** - Consistent label sizing for better readability
- **Professional alignment** - Clean, organized value display

## 🎨 UI Features

### **XY Pad**
- **Interactive 2D Control** - Pan (X-axis) and Depth (Y-axis)
- **Live Waveform Background** - Real-time audio visualization with elegant thin lines
- **EQ Response Curves** - Multiple filter response overlays
- **Grid Markers** - Frequency and pan position indicators
- **Ball Visualization** - Single or dual ball display with smooth animations

### **Container System**
- **Professional card styling** - Matching XY Pad aesthetic
- **Container icons** - Visual identification for each control group
- **Consistent padding** - 20px internal spacing for all containers
- **Shadow effects** - Subtle depth with professional appearance

### **Visual Feedback**
- **Custom icon integration** - 21 vector icons throughout the interface
- **Larger lock indicators** - 24-28px icons for better visibility
- **Centered value labels** - Professional alignment and spacing
- **Container organization** - Logical grouping of related controls
- **Tilt Curve Display** - Orange dashed curve shows frequency tilt effect
- **Drive Visualization** - Thin orange/blue lines show harmonic saturation
- **Air Band Curve** - White transparent curve for high-shelf enhancement
- **Split Percentage Borders** - Blue/red arcs on Pan knob show split percentages

### **Mini Slider Graphical Design**
- **Horizontal Linear Style** - Compact horizontal sliders positioned below their respective EQ knobs
- **Custom Drawing** - Neomorphic design with gradient backgrounds and enhanced thumb visualization
- **Smooth Interaction** - Responsive mouse handling with hover effects and drag feedback
- **Logarithmic Positioning** - Thumb position correctly reflects the logarithmic frequency ranges
- **Visual Hierarchy** - Subtle shadows and gradients create depth and professional appearance
- **Interactive Feedback** - Glow effects on hover and inner highlights during dragging
- **Bounds Clamping** - Thumb never disappears outside the slider track bounds
- **Accent Color Integration** - Uses the theme's accent color (blue in normal mode, green in green mode)

### **Layout Improvements**
- **Container-based organization** - Logical control grouping
- **Responsive Design** - Scales from 50% to 200% maintaining aspect ratio
- **Full Screen Mode** - Toggle between normal and full screen with memory of original size
- **Professional Spacing** - Clean, uncluttered interface with proper padding
- **Icon consistency** - Unified vector-based icon system
- **Draggable Resize Handle** - Bottom-right corner resize with aspect ratio lock

## 🔧 Technical Specifications

### **Audio Processing**
- **Sample Rates**: 44.1kHz to 192kHz
- **Bit Depth**: 32-bit floating point
- **Oversampling**: Configurable Off/2x/4x with high-quality saturation
- **Latency**: Minimal (depends on oversampling setting)
- **Pan Law**: **Ableton-accurate constant-power with sinusoidal curves**

### **64‑bit Processing Upgrade**
- **Precision**: Supports both 32‑bit float and 64‑bit double processing (host controlled)
- **Templated DSP**: Unified `FieldChain<Sample>` instantiated for `float` and `double`
- **Float Reverb Adapter**: Double path uses a float reverb adapter with wet‑only parallel mix to preserve double precision elsewhere
- **Oversampling Island**: Oversampling (Off/2x/4x) is applied only to saturation for performance and quality
- **Parameter Snapshot**: Per‑block `HostParams` snapshot passed into chains; internal `FieldParams` stored in sample domain
- **Panning/Width**: Ableton‑accurate constant‑power panning and M/S width implemented identically across precisions

### **Panning Specifications (Ableton-Accurate)**
- **Center Position**: 0dB (no attenuation)
- **Hard Pan Extremes**: +3dB boost (constant-power behavior)
- **Pan Law**: Sinusoidal constant-power crossfade
- **Split Mode**: Independent L/R channel placement
- **Width Integration**: Proper width reduction before hard panning

### **Filter Specifications**
- **High-Pass**: 2nd order Butterworth (12dB/octave, 20Hz-1kHz)
- **Low-Pass**: 2nd order Butterworth (12dB/octave, 2kHz-20kHz)
- **Air Band**: High-shelf at 20kHz with Q=0.7 (0-6dB positive only)
- **Tilt EQ**: Low shelf (150Hz) + High shelf (6kHz) combination
- **Mono Maker**: Linkwitz-Riley crossover (0-300Hz)

### **Mini Frequency Slider Specifications**
- **Tilt Frequency**: 100Hz-1kHz (logarithmic, skew factor 0.5f) - Controls the center frequency of the tilt EQ response
- **Scoop Frequency**: 200Hz-2kHz (logarithmic, skew factor 0.5f) - Sets the center frequency for scoop/boost bell curve processing
- **Bass Frequency**: 50Hz-500Hz (logarithmic, skew factor 0.5f) - Controls the bass shelf frequency for low-end enhancement
- **Air Frequency**: 2kHz-20kHz (logarithmic, skew factor 0.5f) - Sets the air enhancement frequency for high-end processing

**DSP Implementation:**
- **Logarithmic Ranges**: All frequency sliders use `NormalisableRange` with skew factor 0.5f for natural frequency perception
- **Real-time Processing**: Frequency changes are applied immediately to the DSP chain with smooth parameter transitions
- **Visual Feedback**: Slider thumbs move smoothly across the track with proper logarithmic positioning
- **Parameter Integration**: Frequency values directly control the center frequencies of their respective EQ bands

### **Width Processing**
- **Range**: 0-500% (67% more than previous 300% limit)
- **Precision**: 100,000 steps (10x more precise than standard)
- **Algorithm**: Enhanced mid-side processing with aggressive widening
- **Saturation**: Soft clipping for extreme width settings

### **Preset System Specifications**
- **Storage Format**: JSON with UTF-8 encoding
- **Location**: `~/Library/Application Support/Field/Presets/`
- **Categories**: Studio, Mixing, Mastering, Creative + Custom
- **Search Algorithm**: Multi-field text matching (name, description, author, category)
- **Parameter Capture**: All 12 main processing parameters

### **Icon System**
- **Format**: Vector-based juce::Path objects
- **Scalability**: Resolution-independent scaling
- **Count**: 23 custom-designed icons (including full screen icons)
- **Style**: Consistent line weights and proportions
- **Performance**: Lightweight, cached rendering

## 🚀 Build Instructions

### **Prerequisites**
- **CMake** 3.22 or higher
- **C++17** compatible compiler
- **JUCE 8.0.0** (automatically fetched via CPM)
- **Xcode** (macOS) or **Visual Studio** (Windows)

### **Quick Start**

```bash
# Clone and setup
git clone <repository-url>
cd Field

# Configure build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build all targets
cmake --build .

# Or build specific targets:
cmake --build . --target Field_Standalone --config Debug  # Fast development
cmake --build . --target Field_AU --config Debug          # macOS Audio Units
cmake --build . --target Field_VST3 --config Debug        # VST3 (requires SDK)
```

### **Launch Applications**

**macOS:**
```bash
# Standalone
open build/Field_artefacts/Debug/Standalone/Field.app

# AU Location (auto-installed)
~/Library/Audio/Plug-Ins/Components/Field.component

# VST3 Location
~/Library/Audio/Plug-Ins/VST3/Field.vst3
```

**Windows:**
```bash
# Standalone
build/Field_artefacts/Debug/Standalone/Field.exe

# VST3 Location
C:\Program Files\Common Files\VST3\Field.vst3
```

## 📁 Project Structure

```
Field/
├── CMakeLists.txt              # Root CMake configuration
├── cmake/
│   └── CPM.cmake              # CPM dependency manager
└── Source/
    ├── CMakeLists.txt         # Source CMake configuration
    ├── PluginProcessor.h/cpp  # 64-bit templated DSP engine (float/double), Ableton-accurate panning
    ├── PluginEditor.h/cpp     # Container-based UI with custom icons
    ├── FieldLookAndFeel.h/cpp # Neomorphic UI theme with custom components
    ├── IconSystem.h/cpp       # Custom vector icon system (21 icons)
    └── PresetSystem.h/cpp     # Comprehensive preset management system
```

## 🎵 Usage Examples

### **Container-Based Workflow**
1. **VOLUME Container**: Start with Gain, add Drive for warmth, adjust Mix for blend
2. **EQ Container**: Shape with HP/LP filters, enhance with Air, balance with Tilt
3. **SPACE Container**: Set spatial depth, add ducking for dynamic control
4. **PAN Container**: Position in stereo field with Ableton-accurate panning

### **Preset-Based Workflow**
1. **Search presets** by typing in the dropdown (e.g., "vocal", "wide", "master")
2. **Browse by category** - Studio, Mixing, Mastering, Creative
3. **Load presets** and fine-tune with container controls
4. **Save custom presets** with descriptive names and categories

### **Professional Stereo Enhancement**
1. Set **STEREO** mode for standard processing
2. Use **VOLUME Container** for gain staging and saturation
3. Shape with **EQ Container** filters and enhancement
4. Position with **PAN Container** and **SPACE Container** depth

### **Advanced Spatial Processing**
1. Choose **Space Algorithm** (Inner/Outer/Deep) 
2. Use **SPACE Container** for depth and ducking
3. Apply **EQ Container** filters to shape spatial character
4. Fine-tune with **VOLUME Container** drive and width

### **Split Processing with Locks**
1. Switch to **SPLIT** mode with stereo/split toggle
2. Position balls independently for complex stereo shaping
3. Use **larger lock buttons** (28x28px) to freeze specific parameters
4. **Link** button synchronizes ball movement when needed

## 🔍 Advanced Features

### **Container System Benefits**
- **Logical organization** - Related controls grouped together
- **Professional workflow** - Industry-standard control layout
- **Visual clarity** - Clean separation of processing stages
- **Scalable design** - Consistent styling across all containers

### **Enhanced Icon Language**
- **23 vector icons** covering all interface elements
- **Consistent styling** with unified line weights and proportions
- **Professional appearance** matching modern DAW standards
- **Scalable rendering** for high-DPI displays
- **Full screen icons** with expand/contract arrow designs

### **Full Screen Mode**
- **Top-right corner button** with expand/contract arrow icons
- **Toggle functionality** - Click to enter/exit full screen mode
- **Memory of original size** - Restores previous window dimensions
- **Professional workflow** - Maximize workspace when needed
- **Consistent design** - Matches options button styling

### **Improved Lock System**
- **Larger icons** (24-28px) for better visibility and usability
- **Visual feedback** with clear locked/unlocked states
- **Parameter isolation** - Lock specific controls in split mode
- **Professional workflow** for complex processing scenarios

### **Centered Value Display**
- **Direct positioning** - Values appear exactly below their controls
- **Consistent sizing** - 40px width for all parameter labels
- **Better readability** - Improved alignment and spacing
- **Professional appearance** - Clean, organized value display

### **Comprehensive Preset Management**
- **14 built-in presets** across 4 professional categories
- **Searchable interface** - Type to find presets quickly
- **User preset system** - Save and organize custom settings
- **Category organization** - Professional workflow with subcategories
- **JSON storage** - Portable and human-readable preset files

### **Ableton-Accurate Panning**
- **Constant-power crossfade** with sinusoidal curves
- **Center = 0dB**, extremes = +3dB behavior
- **Both channels contribute** to both outputs (proper crossfade)
- **Split stereo mode** for independent L/R placement
- **Visual feedback** with split percentage borders on Pan knob

## 🔍 Troubleshooting

### **Build Issues**
- Ensure CMake 3.22+ is installed
- Check C++17 compiler compatibility
- VST3 build requires VST3 SDK (AU build works without)
- Clear build directory and reconfigure if needed

### **Audio Issues**
- Check sample rate compatibility (44.1kHz-192kHz)
- Verify plugin format installation paths
- Test with different oversampling settings
- Ensure proper Ableton-style panning behavior

### **UI Issues**
- Try different scale factors (50%-200%)
- Reset plugin parameters using presets
- Check container layout and icon display
- Verify lock button functionality and sizing

### **Preset Issues**
- Check preset directory permissions: `~/Library/Application Support/Field/Presets/`
- Verify JSON file format and encoding
- Clear preset cache and reload plugin
- Test search functionality with different terms

## 📋 Development Checklist

- [x] **Core Functionality**
  - [x] Ableton-accurate panning with constant-power crossfade
  - [x] Enhanced width processing (0-500%, 100k steps)
  - [x] Real-time parameter updates with smoothing
  - [x] VST3/AU/Standalone builds
  - [x] Parameter automation support

- [x] **UI Implementation**
  - [x] Container-based layout with professional organization
  - [x] Custom icon language with 23 vector icons
  - [x] Larger lock icons (24-28px) for better visibility
  - [x] Centered value labels positioned below knobs
  - [x] Neomorphic design with professional aesthetics
  - [x] Responsive layout with aspect ratio preservation
  - [x] Full screen mode with top-right corner toggle button

- [x] **Advanced Features**
  - [x] Comprehensive preset system with search functionality
  - [x] 14 built-in presets across 4 categories
  - [x] User preset management with JSON storage
  - [x] STEREO/SPLIT modes with independent ball control
  - [x] Enhanced parameter locking system with visual feedback
  - [x] Live waveform display with directional control
  - [x] EQ response curves with separate Air visualization

- [x] **Performance & Quality**
  - [x] Efficient DSP processing with proper pan law
  - [x] Smooth parameter transitions with SmoothedValue
  - [x] Optimized UI rendering with container system
  - [x] Professional-grade audio quality
  - [x] Memory-efficient circular buffer for waveforms
  - [x] Lightweight vector icon rendering

## 📄 License

This project uses the JUCE framework. Please refer to JUCE licensing terms for commercial use.

## 🤝 Contributing

Field is actively developed with a focus on professional audio processing, modern UI design, and comprehensive preset management. Contributions are welcome for:
- DSP algorithm improvements and optimizations
- UI/UX enhancements and container system improvements
- Icon system expansion and visual design
- Preset development and category organization
- Performance optimizations and memory efficiency
- Documentation updates and usage examples

## 📞 Support

For technical support, feature requests, or bug reports, please refer to the project repository or contact the development team.

---

**Field** - Professional Spatial Audio Processor  
*Built with JUCE 8.0.0*  
*Features Container-Based UI with Custom Icons*  
*Comprehensive Preset Management System*  
*Ableton-Accurate Panning*  
*Full Screen Mode with Top-Right Toggle*  
*Version: 3.1.0* 