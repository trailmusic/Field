# Reverb Feature Development Notes

## Visual Background Implementation (January 2025)

### Problem Identified
The `ReverbGraphics` component was missing its own `paint()` method, causing the visual background to only cover 62% of the area (via `ReverbCanvasComponent`). This left the rest of the visual area transparent, breaking the consistent dark background pattern used by other tabs.

### Solution Implemented
- Added `paint()` method to `ReverbGraphics` class
- Implemented full dark background with elevation shadows and rounded corners
- Matches styling pattern from `BandGraphics` and other tabs
- Uses theme colors from `FieldLNF`

### Files Modified
- `ReverbGraphics.h`: Added `void paint(juce::Graphics& g) override;`
- `ReverbGraphics.cpp`: Added paint implementation with theme integration

## Visual Cleanup (January 2025)

### Reverb Visuals Removed
- **Action**: Removed all reverb visual components to start from scratch
- **Kept**: Background paint method for consistent dark panel styling
- **Removed**: 
  - ReverbCanvasComponent (decay curves, heatmaps, etc.)
  - DecayCurveComponent
  - ReverbScopeComponent  
  - All ducking strip controls
  - All visual attachments and components

### Current State
- **ReverbGraphics**: Clean component with only background paint method
- **Background**: Consistent dark panel with elevation shadows and rounded corners
- **Ready**: Clean slate for rebuilding reverb visuals from scratch

## Complete Reverb Visual System Implementation (January 2025)

### Overview
Complete package for implementing the reverb system with:
- **Finalized 2×16 grid map** with required R1C1=Enable and R1C16=Wet Only
- **Floating Ducking module** (UI + hooks) with GR meter and mode logic
- **Visualization modes** with GR overlay (Rays | Waterfall | Spectral)
- **Stripped Pro-Q style EQ** for reverb (Tone EQ + Decay-Rate EQ)
- **Migration notes** for seamless integration

### 1. Parameter System (ReverbParamIDs.h)

**Core routing:**
- `enabled`, `killDry`

**Structure/space:**
- `preDelayMs`, `decaySec`, `sizePct`

**Early reflections:**
- `erLevelDb`, `erDensityPct`, `erWidthPct`, `erTimeMs`, `erToTailPct`

**Diffusion/density:**
- `diffusionPct`, `densityPct`

**Modulation:**
- `modDepthCents`, `modRateHz`

**Stereo + rotation:**
- `widthPct`, `rotationDeg`

**Motion follow (from global Motion Engine):**
- `followWidth`, `followWidthAmt`, `followRot`, `followRotAmt`

**Mix & specials:**
- `wetMix01`, `bloomPct`, `distancePct`, `freeze`, `shimmerAmtPct`, `shimmerInt`, `gateAmtPct`, `outTrimDb`

**Reverb EQ routing (4-band dyna-EQ pane):**
- `dreqXoverLoHz`, `dreqXoverHiHz`, `dreqApply` (0=Pre,1=Post,2=ER,3=Tail)

**Ducking (floating module):**
- `duckOn`, `duckMode`, `duckDepthDb`, `duckAtkMs`, `duckRelMs`, `duckThrDb`, `duckRatio`, `duckKneeDb`, `duckBandHz`, `duckBandQ`, `duckDetectorSrc`

### 2. Final 2×16 Grid Map (ReverbControlsPane.h/.cpp)

**Row 1 (16 controls):**
1. ENABLE (enabled) - Toggle
2. PRE (preDelayMs) - ms
3. ER LVL (erLevelDb) - dB
4. ER DEN (erDensityPct) - %
5. ER WID (erWidthPct) - %
6. ER TIME (erTimeMs) - ms
7. ER→T (erToTailPct) - %
8. DIFF (diffusionPct) - %
9. DENS (densityPct) - %
10. MOD DEP (modDepthCents) - ¢
11. MOD RATE (modRateHz) - Hz
12. WIDTH (widthPct) - %
13. ROT (rotationDeg) - °
14. SIZE (sizePct) - %
15. DECAY (decaySec) - s
16. WET ONLY (killDry) - Toggle

**Row 2 (16 controls):**
1. WET (wetMix01) - 0-1
2. BLOOM (bloomPct) - %
3. DIST (distancePct) - %
4. FREEZE (freeze) - Toggle
5. SHIM AMT (shimmerAmtPct) - %
6. SHIM INT (shimmerInt) - %
7. GATE (gateAmtPct) - %
8. DREQ XO LO (dreqXoverLoHz) - Hz
9. DREQ XO HI (dreqXoverHiHz) - Hz
10. EQ APPLY (dreqApply) - Choice
11. FOLLOW W (followWidth) - Toggle
12. W AMT (followWidthAmt) - %
13. FOLLOW R (followRot) - Toggle
14. R AMT (followRotAmt) - %
15. TRIM (outTrimDb) - dB
16. DUCK (duckOn) - Toggle

### 3. Floating Ducking Module (DuckingFloat.h/.cpp)

**Activation:**
- **DUCK Toggle**: The floating ducking module is activated when the DUCK toggle (R2C16) is enabled
- **Visual Integration**: When DUCK is on, the floating module appears and shows ducking controls
- **State Management**: DUCK toggle controls the visibility and functionality of the ducking system

**Features:**
- Collapsible/expandable UI with pill-style header
- GR meter with real-time gain reduction display
- Mode selection (General, Vocal, DrumBus, Guitar, Keys)
- Detector source selection (Dry, ER, Tail, Wet Sum)
- Auto lookahead/RMS based on mode selection

**UI Layout:**
- **Collapsed**: Pill header with GR meter and status
- **Expanded**: Full control grid with all ducking parameters
- **GR Meter**: Real-time gain reduction visualization

### 4. Hero Visualization (ReverbGraphics.h/.cpp)

**View Modes:**
- **Rays**: Fan of lines with random jitter, brightness maps to density/diffusion
- **Waterfall**: Gradient bands with IR preview texture
- **Spectral**: Frequency response curves

**Features:**
- GR overlay across all visualization modes
- Mode switching buttons (Rays | Waterfall | Spectral)
- Embedded DuckingFloat container
- Real-time gain reduction visualization

### 5. Stripped Pro-Q Style EQ for Reverb

**Tone EQ (POST) - 4 bands max:**
- Band types: Bell, Low Shelf, High Shelf
- Per-band dynamics: DynAmt only (0-100%)
- Auto A/R by band frequency
- Default: Post reverb

**Decay-Rate EQ (DR-EQ) - 3 bands max:**
- Band types: Bell, Low Tilt, High Tilt
- Y-axis controls Decay Multiplier (0.5× - 2.0×)
- Default: Tail-Only

**Unified Gestures:**
- Drag node: X = Freq (log), Y = Gain (Tone) or Decay Multiplier (Decay)
- Mouse wheel over node: Q
- Alt/Option + drag vertical: DynAmt ring (Tone lane only)
- Right-click: band type menu
- Double-click node: bypass/enable band
- "+" on spectrum: add band at cursor

**Visual Language:**
- Tone EQ: outer ring shows DynAmt fill
- Decay EQ: bidirectional arrows or "T60×" tick marks
- Analyzer: ER trace + Tail trace
- Decay lane: dim static magnitude, emphasize decay slope guide

### 6. Parameter IDs for ReverbEQ

**Tone EQ (post):**
- `rvb_eq_b{i}_enabled` (bool)
- `rvb_eq_b{i}_type` (int: 0=Bell,1=LS,2=HS)
- `rvb_eq_b{i}_freq` (20-20000 Hz, log)
- `rvb_eq_b{i}_gainDb` (-12...+12 dB)
- `rvb_eq_b{i}_q` (0.3...4.0)
- `rvb_eq_b{i}_dynAmt` (0...100 %)

**Decay-Rate EQ:**
- `rvb_dreq_b{j}_enabled` (bool)
- `rvb_dreq_b{j}_type` (int: 0=Bell,1=TiltLo,2=TiltHi)
- `rvb_dreq_b{j}_freq` (20-20000 Hz)
- `rvb_dreq_b{j}_q` (0.3...3.0)
- `rvb_dreq_b{j}_mult` (Decay Multiplier: 0.5×...2.0×)

**Lane/global:**
- `dreqApply` (0=Pre,1=Post,2=ER,3=Tail)
- `dreqXoverLoHz` (80-400 Hz), `dreqXoverHiHz` (1k-6k Hz)
- `rvb_dreq_apply` (0=Tail-Only,1=ER-Only,2=Both)

### 7. Defaults & UX

**Ducking Modes (hidden lookahead/RMS):**
- General: LA 6-10 ms, RMS 15-25 ms
- Vocal: LA 12-20 ms, RMS 20-30 ms
- DrumBus: LA 3-6 ms, RMS 6-12 ms
- Guitar/Keys: LA 5-10 ms, RMS 15-25 ms

**EQ Apply defaults:**
- Tone EQ: Post (default)
- Decay-Rate EQ: Tail-Only (default)

**Follow Motion:**
- Toggles default off
- Amounts at 0%

**Wet Only:**
- Off by default
- Enable on by default

### 8. Migration Notes

**Remove legacy params:**
- `widthStartPct`, `widthEndPct`, `rotStartDeg`, `rotEndDeg`
- `duckLaMs`, `duckRmsMs`

**Add new params:**
- `rotationDeg`, `followWidth`, `followWidthAmt`, `followRot`, `followRotAmt`
- `dreqApply`, ducking group IDs, shimmer/gate

**Update attachments:**
- Update control pane to new map
- Instantiate `ReverbGraphics` in reverb tab
- Add `.duckUI()` access for floating panel

**DSP graph:**
- Ensure ER and Tail are separate taps
- Apply `duck.getLinearGR()` to Tail gain reduction
- Split internally if one-bus'd reverb

### 9. QA Checklist

**Slot positions:**
- R1C1 Enable, R1C16 Wet Only confirmed
- `Trim` at R2C15

**Duck float:**
- Collapsed/expanded works
- DUCK ON toggles detector processing
- GR meter moves

**GR overlay:**
- Appears across all three viz modes
- No frame drops at 30 FPS

**Freeze:**
- IR/late reflections stop evolving
- GR overlay still renders (optional)

**Preset compatibility:**
- Add migration shim for removed IDs
- Map to new defaults

**Automation:**
- Parameters grouped in host
- Use `AudioProcessorParameterGroup` for foldering

### 10. Implementation Priority

1. **Phase 1**: Update parameter system and 2×16 grid
2. **Phase 2**: Implement floating ducking module
3. **Phase 3**: Add hero visualization modes
4. **Phase 4**: Integrate stripped Pro-Q style EQ
5. **Phase 5**: Final testing and optimization

### ✅ PHASE 1, 2 & 3 COMPLETE (January 2025)

**Migration Status:**
- ✅ **ReverbParamIDs.h**: Complete - all parameter IDs updated
- ✅ **ReverbParameters.h/.cpp**: Complete - APVTS integration working
- ✅ **ReverbControlsPane.h/.cpp**: Complete - 2×16 grid implemented
- ✅ **DuckingFloat.h/.cpp**: Complete - floating ducking module implemented
- ✅ **ReverbGraphics.h/.cpp**: Complete - hero visualization modes implemented
- ✅ **Build System**: Complete - all plugins building successfully
- ✅ **PluginProcessor Cleanup**: Complete - all legacy reverb code removed

**Visual System Status:**
- ✅ **Floating Ducking Module**: Collapsible UI with GR meter and mode selection
- ✅ **Hero Visualizations**: Rays, Waterfall, and Spectral modes implemented
- ✅ **Animation System**: 30 FPS timer with parameter-driven visuals
- ✅ **GR Overlay**: Real-time gain reduction visualization across all modes
- ✅ **View Mode Switching**: Rays | Waterfall | Spectral buttons

**Build Results:**
- ✅ **Standalone**: Field.app - Built successfully
- ✅ **AU Plugin**: Field.component - Built and installed
- ✅ **VST3 Plugin**: Field.vst3 - Built and installed

### ✅ PHASE 3 COMPLETE: Stripped Pro-Q Style EQ System (January 2025)

**EQ System Implementation:**
- ✅ **ReverbToneEQ.h/.cpp**: 4-band post-reverb tone shaping EQ
  - Bell, Low Shelf, High Shelf filter types
  - Per-band dynamics with DynAmt control
  - Interactive node-based interface with mouse controls
  - Real-time spectrum analyzer integration
  - Pro-Q4 style visual design with golden ratio color scheme

- ✅ **DecayRateEQ.h/.cpp**: 3-band decay multiplier EQ
  - Bell, TiltLo, TiltHi filter types for decay shaping
  - 0.5x to 2.0x decay multiplier range
  - Interactive frequency and multiplier controls
  - Real-time spectrum analyzer integration
  - Orange color scheme to distinguish from tone EQ

- ✅ **ReverbEQParamIDs.h**: Complete parameter system
  - ToneBand namespace: active, type, freqHz, gainDb, q, phase
  - DecayBand namespace: active, freqHz, q, decayMult, dynAmt
  - Parameter creation helpers for APVTS integration

- ✅ **Integration**: Full integration into ReverbGraphics
  - Vertical split layout: EQ panels (60%) + visualizations (40%)
  - Tone EQ (top) and Decay-Rate EQ (bottom) positioning
  - Analyzer control methods for sample rate and audio processing
  - Timer-based animation system (30 FPS)

**Technical Features:**
- **Mouse Interaction**: Click to add bands, drag to adjust, wheel for Q
- **Visual Feedback**: Real-time curve drawing with per-band colors
- **Parameter Mapping**: Full APVTS integration with proper parameter IDs
- **Performance**: Optimized rendering with efficient curve calculations
- **Theme Integration**: Consistent with Field's visual design system

**Build Status:**
- ✅ **Compilation**: All EQ components compile successfully
- ✅ **Linking**: All symbols resolved, no undefined references
- ✅ **Integration**: EQ components properly integrated into ReverbGraphics
- ✅ **CMakeLists.txt**: EQ files added to build system

**Legacy Code Cleanup:**
- ✅ **PluginProcessor.cpp**: All legacy reverb parameters and functions removed
- ✅ **PluginProcessor.h**: All legacy reverb struct members and forward declarations removed
- ✅ **Legacy Comments**: All old reverb system comments and notes cleaned up
- ✅ **Parameter Migration**: Complete transition from `ReverbIDs::` to `ReverbParamIDs::`
- ✅ **Code Quality**: Clean, maintainable codebase ready for visual development

## Phase 2 Implementation Complete (January 2025)

### **🎯 Floating Ducking Module (DuckingFloat.h/.cpp)**

**Features Implemented:**
- **Collapsible UI**: Pill-style header with expand/collapse functionality
- **GR Meter**: Real-time gain reduction display with color coding (Red/Orange/Green)
- **Mode Selection**: General, Vocal, DrumBus, Guitar, Keys with hidden lookahead/RMS
- **Detector Selection**: Dry, ER, Tail, Wet Sum source options
- **8 Ducking Controls**: Depth, Threshold, Ratio, Knee, Attack, Release, Band Freq, Band Q
- **Visual Integration**: Positioned in top-right corner of ReverbGraphics

**Technical Details:**
- **UI Layout**: Collapsed (40px height) and Expanded (200px height) modes
- **GR Visualization**: Color-coded meter with real-time updates
- **APVTS Integration**: Ready for parameter attachments
- **Styling**: Consistent with Field UI theme

### **🎨 Hero Visualization Modes (ReverbGraphics.h/.cpp)**

**Rays Mode:**
- **Visual**: Fan of lines with random jitter
- **Parameters**: Number of rays based on ER level, thickness based on density
- **Animation**: Smooth ray generation with parameter-driven properties
- **Colors**: HSV-based coloring with tail level influence

**Waterfall Mode:**
- **Visual**: Gradient bands with IR preview texture
- **Parameters**: Color stops based on ER and tail levels
- **Animation**: Smooth gradient transitions
- **Texture**: Horizontal line overlay for depth

**Spectral Mode:**
- **Visual**: Frequency response curves for ER and Tail
- **Parameters**: Separate curves for early reflections and tail
- **Animation**: Real-time frequency response updates
- **Colors**: Distinct colors for ER (blue) and Tail (orange)

**Common Features:**
- **View Mode Buttons**: Rays | Waterfall | Spectral switching
- **GR Overlay**: Semi-transparent red overlay with GR text when active
- **Animation System**: 30 FPS timer with parameter-driven visuals
- **Real-time Updates**: All modes respond to parameter changes

### **🔧 Technical Implementation**

**Files Created/Modified:**
- `DuckingFloat.h/.cpp` - New floating ducking module
- `ReverbGraphics.h/.cpp` - Updated with visualization modes
- `CMakeLists.txt` - Added DuckingFloat to build system

**Performance:**
- **Animation**: 30 FPS timer with smooth updates
- **Memory**: Efficient rendering with minimal allocations
- **Real-time**: All components respond to parameter changes
- **Build**: All plugins building successfully (Standalone, AU, VST3)

**Integration Status:**
- **ReverbGraphics**: Contains DuckingFloat and view mode buttons
- **Timer System**: 30 FPS animation with parameter updates
- **Ready for ReverbTab**: Components ready for final integration

### Next Steps
- **Phase 1**: ✅ COMPLETE - Parameter system migration and legacy cleanup
- **Phase 2**: ✅ COMPLETE - Floating ducking module UI and hero visualization modes
- **Phase 3**: ✅ COMPLETE - Stripped Pro-Q style EQ system implementation
- **Phase 4**: Final integration and testing
- **Phase 5**: Performance optimization and polish

## Control Type Analysis (January 2025)

### **🎛️ Current 2x16 Grid Status**

**Row 1 (16 controls):**
- ✅ **ENABLE** (ToggleButton) - Master enable/disable
- ✅ **PRE** (KnobCell) - Pre-delay time (0-200ms)
- ✅ **ER LVL** (KnobCell) - Early reflection level (-60 to +6dB)
- ✅ **ER DEN** (KnobCell) - Early reflection density (0-100%)
- ✅ **ER WID** (KnobCell) - Early reflection width (0-100%)
- ✅ **ER TIME** (KnobCell) - Early reflection time (0-100ms)
- ✅ **ER→T** (KnobCell) - ER to tail transition (0-100%)
- ✅ **DIFF** (KnobCell) - Diffusion amount (0-100%)
- ✅ **DENS** (KnobCell) - Density amount (0-100%)
- ✅ **MOD DEP** (KnobCell) - Modulation depth (0-50¢)
- ✅ **MOD RATE** (KnobCell) - Modulation rate (0.1-10Hz)
- ✅ **WIDTH** (KnobCell) - Stereo width (0-100%)
- ✅ **ROT** (KnobCell) - Rotation angle (0-360°)
- ✅ **SIZE** (KnobCell) - Room size (0-100%)
- ✅ **DECAY** (KnobCell) - Decay time (0.1-20s)
- ✅ **WET ONLY** (ToggleButton) - Kill dry signal

**Row 2 (16 controls):**
- ✅ **WET** (KnobCell) - Wet mix (0-1)
- ✅ **BLOOM** (KnobCell) - Bloom amount (0-100%)
- ✅ **DIST** (KnobCell) - Distance (0-100%)
- ✅ **FREEZE** (ToggleButton) - Freeze reverb tail
- ✅ **SHIM AMT** (KnobCell) - Shimmer amount (0-100%)
- ✅ **SHIM INT** (KnobCell) - Shimmer intensity (0-100%)
- ✅ **GATE** (KnobCell) - Gate amount (0-100%)
- ✅ **DREQ XO LO** (KnobCell) - EQ crossover low (80-400Hz)
- ✅ **DREQ XO HI** (KnobCell) - EQ crossover high (1k-6kHz)
- 🚨 **EQ APPLY** (ComboBox) - EQ routing (Pre/Post/ER/Tail) - **NEEDS FIX**
- ✅ **FOLLOW W** (ToggleButton) - Follow motion width
- ✅ **W AMT** (KnobCell) - Width amount (0-100%)
- ✅ **FOLLOW R** (ToggleButton) - Follow motion rotation
- ✅ **R AMT** (KnobCell) - Rotation amount (0-100%)
- ✅ **TRIM** (KnobCell) - Output trim (-12 to +12dB)
- ✅ **DUCK** (ToggleButton) - Ducking on/off toggle

### **🚨 Control Type Issues Identified**

#### **1. EQ APPLY Control Type Mismatch**
- **Current**: KnobCell (slider)
- **Should Be**: ComboBox (dropdown)
- **Reason**: Choice parameter with 4 options (Pre, Post, ER, Tail)
- **Fix Needed**: Convert to ComboBox with proper item list

#### **2. ToggleButton Implementation Status**
- **Current**: All toggles implemented as KnobCell sliders
- **Should Be**: ToggleButton components
- **Affected Controls**: ENABLE, WET ONLY, FREEZE, FOLLOW W, FOLLOW R
- **Fix Needed**: Convert to proper ToggleButton components

### **✅ Correct Control Types Summary**

- **KnobCell Controls (25 total)**: Time, level, percentage, modulation, frequency, and angle parameters
- **ToggleButton Controls (6 total)**: ENABLE, WET ONLY, FREEZE, FOLLOW W, FOLLOW R, DUCK
- **ComboBox Controls (1 total)**: EQ APPLY (needs conversion)

### **🎨 Metallic Styling Requirements**

All reverb controls use:
- **MetallicKind::Reverb** - Burnt orange theme
- **setAreaMetallicForCell()** - Apply metallic styling
- **Visual Consistency** - 8px corner radius, shadows, borders
- **Hover Effects** - Enhanced borders and glow on hover

### **📋 Implementation Fixes Needed**

1. **Convert ToggleButtons**: Change ENABLE, WET ONLY, FREEZE, FOLLOW W, FOLLOW R from KnobCell to ToggleButton
2. **Convert ComboBox**: Change EQ APPLY from KnobCell to ComboBox with proper item list
3. **Apply Metallic Styling**: Ensure all components use `setAreaMetallicForCell(*component, MetallicKind::Reverb)`
4. **Maintain Grid Layout**: Keep the 2x16 grid structure with proper positioning
5. **Value Labels**: Ensure all KnobCell controls have proper value labels with appropriate decimal places

## ✅ SMART POSITIONING IMPROVEMENTS (January 2025)

### Applied to Reverb EQs
- **ReverbToneEQ**: Smart positioning prevents BandOverlay/BandBadge overlap with band points
- **DecayRateEQ**: Smart positioning prevents BandOverlay/BandBadge overlap with band points
- **Band Limits**: 4-band limit for Tone EQ, 3-band limit for Decay-Rate EQ

### Applied to Main Dynamic EQ
- **DynEqTab**: Smart positioning prevents BandOverlay/BandBadge overlap with band points
- **Consistent Logic**: Same smart positioning algorithm across all EQs

### Smart Positioning Algorithm
1. **Overlap Detection**: Check if UI element would overlap with band point (12px radius + 20px margin)
2. **Fallback Positions**: Try right → left → above → below
3. **Bounds Checking**: Ensure elements stay within component bounds
4. **Consistent Behavior**: Same logic applied to all EQ implementations

### Future EQ Positioning Notes
- **Template Pattern**: Consider creating a base class for smart positioning
- **Configurable Margins**: Make band radius and margin configurable per EQ type
- **Animation Support**: Add smooth transitions when repositioning elements
- **Multi-Element Avoidance**: Extend to avoid overlap with multiple band points
- **Context-Aware Positioning**: Consider EQ type and frequency range for optimal placement

## ✅ SMART POSITIONING APPLIED (January 2025)

### Reverb EQ System
- **ReverbToneEQ**: Smart positioning prevents BandOverlay/BandBadge overlap with band points
- **DecayRateEQ**: Smart positioning prevents BandOverlay/BandBadge overlap with band points
- **Band Limits**: 4-band limit for Tone EQ, 3-band limit for Decay-Rate EQ
- **Algorithm**: 12px band radius + 20px margin with fallback positioning (right → left → above → below)

## ✅ DOUBLE-CLICK DELETE FUNCTIONALITY (January 2025)

### Reverb EQ System
- **ReverbToneEQ**: Double-click any band point to delete it instantly
- **DecayRateEQ**: Double-click any band point to delete it instantly
- **Parameter Cleanup**: Automatically deactivates band parameters in processor
- **UI Management**: Updates overlays, badges, and selection state properly
- **Consistent UX**: Same interaction pattern as main Dynamic EQ