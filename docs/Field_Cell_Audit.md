# Field Cell Audit - Metallic Styling System

## 🎯 **Audit Overview**
This document tracks the current state of metallic styling across all Field control areas and identifies inconsistencies that need to be resolved.

---

## 🏗️ **FieldLookAndFeel Architecture (NEW - January 2025)**

### **Modular System Overview**
The FieldLookAndFeel system has been reorganized into a clean, modular architecture:

```
FieldLookAndFeel System:
├── FieldTheme.h          - Theme management & color palettes
├── FieldMetallic.h       - Metallic rendering system
├── FieldRendering.h      - Component-specific drawing methods
└── FieldLookAndFeel.h    - Core LNF class (delegates to above)
```

### **Theme System**
- **5 Complete Themes**: Ocean (default), Green, Pink, Yellow, Grey
- **ThemeManager**: Centralized theme switching and management
- **Color Palettes**: EQ, metallic, meter colors per theme
- **Backward Compatibility**: All existing theme functionality preserved

### **Metallic System**
- **MetallicKind Enum**: None, Neutral, Reverb, Delay, Band, Phase, Motion, XY
- **Helper Functions**: `metallicFromProps()`, `setAreaMetallic()`, `setAreaMetallicForCell()`
- **MetallicRenderer**: Static methods for `paintMetal()` and `paintPhaseMetal()`
- **ComboBox Integration**: Full metallic styling support with KnobCell visual consistency

### **Rendering System**
- **FieldRendering**: Component-specific drawing methods
- **Delegation Pattern**: Core LNF delegates to specialized renderers
- **Consistent API**: All drawing methods take `FieldTheme const&` parameter

### **ComboBox Styling System (NEW - January 2025)**
- **Metallic ComboBoxes**: Full integration with metallic rendering system
- **Visual Consistency**: Matches KnobCell styling (corner radius, shadows, borders)
- **Text Wrapping**: Automatic two-line text for two-word labels
- **Interior Windows**: Recessed button windows with proper spacing
- **Border System**: Uses `theme.accentSecondary` for consistent visual weight

---

## 📊 **Current Status: FULLY UNIFIED** ✅

### **🔍 Control Area Analysis**

| Control Area | KnobCells | Buttons/Switches | ComboBoxes | Metallic Application | Status |
|--------------|-----------|------------------|------------|---------------------|---------|
| **Delay** | ✅ Pane-level | ✅ Pane-level | ✅ **KnobCell Visual** | Pane-only | 🟢 **ENHANCED** |
| **Motion** | ✅ Registry-based | ✅ Registry-based | ✅ **KnobCell Visual** | Canonical 32-slot | 🟢 **ENHANCED** |
| **XY** | ✅ Pane-level | ✅ Pane-level | ✅ Pane-level | Pane-only | 🟢 Consistent |
| **Phase** | ✅ Pane-level | ✅ Pane-level | ✅ **KnobCell Visual** | Pane-only | 🟢 **ENHANCED** |
| **Reverb** | ✅ Pane-level | ✅ Pane-level | ✅ Pane-level | Pane-only | 🟢 Consistent |
| **Band** | ✅ Pane-level | ✅ Pane-level | ✅ Pane-level | Pane-only | 🟢 Consistent |

### **🎨 ComboBox Visual Enhancements (NEW - January 2025)**

#### **KnobCell Visual Consistency**
- **Corner Radius**: Updated from 5.0f to 8.0f to match KnobCell
- **Shadows**: Two-layer drop shadows (dark + light) matching KnobCell
- **Inner Rim**: 0.8f thickness with 0.16f alpha for depth
- **Border System**: Uses `theme.accentSecondary` with 1.5f thickness
- **Interior Windows**: Recessed button windows with proper spacing compensation

#### **Text Wrapping System**
- **Automatic Detection**: Two-word labels automatically split into two lines
- **Manual Override**: Direct `\n` embedding in item lists for precise control
- **Font Optimization**: 10.0f font size for optimal two-line display
- **No Flash**: Embedded line breaks prevent selection flickering

#### **Sizing System**
- **setMetrics() Integration**: ComboBoxes use same sizing system as KnobCell
- **Visual Weight Matching**: Interior window sizing compensates for styling changes
- **Consistent Padding**: 4px padding matching KnobCell behavior

### **🏗️ Architecture Patterns**

#### **Pattern A: Pane-Only (Delay) - CORRECT**
```
DelayControlsPane.h:
├── KnobCells: setAreaMetallicForCell(cell, MetallicKind::Delay)
├── ComboBoxes: setAreaMetallicForCell(combo, MetallicKind::Delay) → SimpleSwitchCell
└── BlankCells: setAreaMetallicForCell(cell, MetallicKind::Delay)
```

#### **Pattern B: Registry-Based (Motion) - NEW CANONICAL SYSTEM**
```
MotionSlot.h:
├── Canonical 32-slot parameter registry
├── ParamRef definitions with types, IDs, ranges, defaults
└── Single source of truth for all Motion controls

MotionControlsPane.h:
├── buildControls() iterates through MotionSlot::canonicalParams
├── createButton/ComboBox/Knob based on ParamRef.type
└── All styling applied via registry-driven creation
```

#### **Pattern C: Self-Contained (XY) - CORRECT**
```
XYControlsPane.h:
├── KnobCells: setAreaMetallicForCell(cell, MetallicKind::XY)
├── KnobCellWithAux: setAreaMetallicForCell(cell, MetallicKind::XY) + aux styling
├── Buttons: setAreaMetallicForCell(button, MetallicKind::XY) → SimpleSwitchCell
└── ComboBoxes: setAreaMetallicForCell(combo, MetallicKind::XY) → SimpleSwitchCell
```

#### **KnobCellWithAux Layout System - UPDATED**
```
KnobCellWithAux.cpp:
├── Layout: 50/50 split with compensated middle gap
├── Knob Positioning: leftHalfCenter = fullWidth.getX() + (fullWidth.getWidth() / 4)
├── Label Centering: 60% of knob size, centered within knob bounds
└── Aux Components: Right 50% area with proper spacing
```

#### **Pattern D: Consistent (Phase, Reverb, Band, Delay) - CORRECT**
```
PhaseTab.cpp / ReverbControlsPane.h / BandControlsPane.h / DelayControlsPane.h:
├── All components: setAreaMetallicForCell(component, MetallicKind::X)
└── All styling applied within the control pane
```

### **🔧 Component Type Analysis**

#### **KnobCell Metallic Support**
- ✅ **Standard KnobCell**: Full metallic support
- ✅ **KnobCellWithAux**: Full metallic support + aux component styling
- ✅ **Blank KnobCell**: Full metallic support for placeholders

#### **Button/Switch Metallic Support**
- ✅ **Raw ToggleButton**: Direct metallic properties
- ✅ **SimpleSwitchCell**: Metallic delegation to child
- ✅ **SwitchCell**: Metallic delegation to child
- ❌ **Custom LookAndFeel**: Bypasses metallic system (FIXED)

#### **ComboBox Metallic Support** 🚨 **NEEDS ATTENTION**
- ✅ **Raw ComboBox**: Direct metallic properties
- ✅ **SimpleSwitchCell**: Metallic delegation to child
- ❌ **Custom LookAndFeel**: Bypasses metallic system (FIXED)
- 🚨 **ComboBox Item Population**: Need to verify all ComboBoxes have proper item lists
- 🚨 **ComboBox Styling Consistency**: Need to verify all ComboBoxes render metallic correctly
- 🚨 **ComboBox Sizing**: Need to ensure consistent sizing across all control areas

### **🎨 Metallic Kind Usage**

| MetallicKind | Control Areas | Color | Status |
|---------------|---------------|-------|---------|
| `None` | Default | Panel | ✅ Working |
| `Neutral` | Generic | Steel | ✅ Working |
| `Reverb` | Reverb | Burnt Orange | ✅ Working |
| `Delay` | Delay | Light Yellow-Green | ✅ Working |
| `Band` | Band | Metallic Blue | ✅ Working |
| `Phase` | Phase | Blue | ✅ Working |
| `Motion` | Motion | Purple | ✅ **Registry-Based** |
| `XY` | XY | Grey | ✅ Working |

### **🔘 Button Visual Components - Complete Reference**

#### **Button Types in Field System:**
1. **juce::ToggleButton** - Standard toggle buttons
2. **juce::TextButton** - Text-based buttons  
3. **BypassButton** - Special bypass button with custom rendering
4. **ThemedIconButton** - Icon-based buttons with theme support
5. **SimpleSwitchCell** - Wrapper for buttons with caption and border

#### **Visual Parts of Buttons:**

##### **🎨 Core Visual Elements:**
- **Background Fill** - Main button body color
- **Border** - Outline around the button
- **Corner Radius** - Rounded corners (typically 4-6px)
- **Shadow** - Drop shadow for depth
- **Text/Icon** - Content displayed on button

##### **🎨 State-Based Visual Elements:**
- **Active State** - When button is toggled/pressed
- **Inactive State** - When button is off/unpressed  
- **Hover State** - When mouse is over button
- **Down State** - When button is being pressed
- **Metallic State** - Special metallic rendering for themed buttons

##### **🎨 Metallic Button Elements:**
- **Metallic Background** - Painted using `paintMetal()` function
- **Metallic Border** - Themed border colors
- **Metallic Text** - Text color matching metallic theme
- **Metallic Icon** - Icon color matching metallic theme

##### **🎨 Wrapper Elements (SimpleSwitchCell):**
- **Cell Background** - Panel background for the cell
- **Cell Border** - Border around the entire cell
- **Caption** - Text label above/below the button
- **Child Bounds** - Area where the actual button is placed

##### **🎨 Special Button Elements:**
- **Icon Rendering** - Custom icon drawing via IconSystem
- **Label Text** - Custom text rendering
- **Invert Active** - Property to invert active/inactive states
- **Icon Override** - Custom icon colors via properties

#### **Button Rendering Pipeline:**
1. **Background** - `drawButtonBackground()` in FieldLookAndFeel
2. **Metallic Check** - Check for metallic properties
3. **State Colors** - Apply active/inactive/hover colors
4. **Border Drawing** - Draw border with appropriate colors
5. **Text/Icon** - Render content via `drawButtonText()` or `drawIcon()`
6. **Wrapper Rendering** - SimpleSwitchCell handles cell background

### **✅ ComboBox-Specific Issues** ✅ **ALL RESOLVED**

#### **✅ Issue 1: ComboBox Item Population - RESOLVED**
- **Problem**: ComboBoxes may not have proper item lists populated
- **Impact**: Empty or incorrect dropdown options
- **Areas Affected**: Motion (Panner, Path, Quant, Mode), Delay, Phase, XY
- **Solution**: ✅ **COMPLETED** - All ComboBoxes now have proper item lists
  - **Phase**: ✅ Already had comprehensive `addItem()` calls
  - **Motion**: ✅ Uses `motion::choiceList*()` functions with proper items
  - **Delay**: ✅ **FIXED** - Added proper `addItem()` calls for all 4 ComboBoxes
  - **XY**: ✅ No ComboBoxes (as expected)

#### **✅ Issue 2: ComboBox Metallic Rendering - RESOLVED**
- **Problem**: ComboBoxes may not render metallic styling consistently
- **Impact**: Visual inconsistency with other controls
- **Areas Affected**: All control areas with ComboBoxes
- **Solution**: ✅ **COMPLETED** - All ComboBoxes use `FieldLNF::drawComboBox` with metallic support
  - **Phase**: ✅ Uses `setAreaMetallicForCell(c, MetallicKind::Phase)`
  - **Motion**: ✅ Uses `setAreaMetallicForCell(*combo, MetallicKind::Motion)`
  - **Delay**: ✅ Uses `setAreaMetallicForCell(c, MetallicKind::Delay)`
  - **Rendering**: ✅ `FieldRendering::drawComboBox` handles metallic rendering via `MetallicRenderer::paintMetal`

#### **✅ Issue 3: ComboBox Sizing Inconsistency - RESOLVED**
- **Problem**: ComboBoxes may have different sizes across control areas
- **Impact**: Visual grid misalignment
- **Areas Affected**: All control areas
- **Solution**: ✅ **COMPLETED** - All ComboBoxes use `SimpleSwitchCell` wrapper for consistent sizing

#### **✅ Issue 4: ComboBox Wrapper Inconsistency - RESOLVED**
- **Problem**: Some ComboBoxes wrapped in `SimpleSwitchCell`, others not
- **Impact**: Inconsistent styling and behavior
- **Areas Affected**: Motion, Delay, XY, Phase
- **Solution**: ✅ **COMPLETED** - All ComboBoxes now use `SimpleSwitchCell` wrapper consistently
  - **Phase**: ✅ Uses `SimpleSwitchCell` wrapper
  - **Motion**: ✅ Uses `SimpleSwitchCell` wrapper
  - **Delay**: ✅ Uses `SimpleSwitchCell` wrapper
  - **XY**: ✅ No ComboBoxes (as expected)

### **🎉 All Critical Issues Resolved**

#### **✅ Issue 1: Mixed Styling Application - RESOLVED**
- **Problem**: Delay and Motion controls split between main editor and panes
- **Solution**: ✅ All styling moved to control panes + Motion uses canonical registry
- **Status**: **COMPLETED**

#### **✅ Issue 2: KnobCellWithAux Aux Styling - RESOLVED**
- **Problem**: XY controls use KnobCellWithAux but aux components may not get proper metallic
- **Solution**: ✅ Aux components properly styled with `setAreaMetallic(*auxComp, MetallicKind::XY)`
- **Status**: **COMPLETED**

#### **✅ Issue 2.1: KnobCellWithAux Layout & Positioning - RESOLVED**
- **Problem**: KnobCellWithAux used 2/3 split causing knob positioning issues
- **Solution**: ✅ Updated to 50/50 split with compensated middle gap
- **Knob Positioning**: ✅ Added compensation math to center knob in left half of full width
- **Label Centering**: ✅ Fixed label positioning to center within knob area (not full cell)
- **Status**: **COMPLETED**

#### **✅ Issue 3: Custom LookAndFeel Conflicts - RESOLVED**
- **Problem**: Custom LookAndFeel classes bypass metallic system
- **Solution**: ✅ Delegate to FieldLNF when metallic properties present
- **Status**: **COMPLETED**

#### **✅ Issue 4: SimpleSwitchCell Metallic Delegation - RESOLVED**
- **Problem**: SimpleSwitchCell overrides child rendering
- **Solution**: ✅ Delegate to child's drawButtonBackground and drawComboBox
- **Status**: **COMPLETED**

### **📋 Action Plan**

#### **Phase 1: Standardize Delay Controls** ✅ **COMPLETED**
- [x] Move Delay ToggleButton styling from PluginEditor to DelayControlsPane
- [x] Move Delay ComboBox styling from PluginEditor to DelayControlsPane
- [x] Ensure consistent metallic application

#### **Phase 2: Implement Canonical Motion Registry** ✅ **COMPLETED**
- [x] Create MotionSlot.h with complete 32-slot parameter registry
- [x] Update MotionControlsPane to use canonical registry for all controls
- [x] Remove all Motion control creation from PluginEditor.cpp/h
- [x] Apply metallic styling to all Motion controls via registry
- [x] Ensure MotionControlsPane is single source of truth for Motion UI

#### **Phase 3: Validate XY Controls** ✅ **COMPLETED**
- [x] Verify KnobCellWithAux aux components get proper metallic
- [x] Ensure XY controls follow consistent patterns
- [x] Test all XY component types

#### **Phase 4: Create Unified Helpers** ✅ **COMPLETED**
- [x] Create standardized `makeMetallicCell()` helpers
- [x] Support all component types consistently
- [x] Document metallic application patterns

#### **Phase 5: Testing & Validation** ✅ **COMPLETED**
- [x] Test all control areas for consistent metallic styling
- [x] Verify KnobCellWithAux aux component styling
- [x] Ensure no regressions in existing functionality

### **🎯 Success Criteria** ✅ **ALL ACHIEVED**

- [x] All control areas use consistent metallic application patterns
- [x] No mixed styling between main editor and panes
- [x] KnobCellWithAux aux components properly styled
- [x] All MetallicKind types working correctly
- [x] No custom LookAndFeel conflicts
- [x] SimpleSwitchCell properly delegates metallic rendering

### **📋 ComboBox Inventory Across All Control Areas**

#### **🎯 Motion ComboBoxes (4 total)**
- **Panner** (Slot 2): `motion.panner_select` - Algorithm selection
- **Path** (Slot 3): `motion.p1.path` - Geometric path preset
- **Quant** (Slot 11): `motion.p1.quantize_div` - Lock motion to grid
- **Mode** (Slot 13): `motion.p1.mode` - Free/Sync/Input Env/Sidechain/One-shot

#### **🎯 Delay ComboBoxes (4 total)**
- **Mode**: Delay algorithm selection
- **Grid Flavor**: Grid timing selection  
- **Duck Source**: Sidechain source selection
- **Filter Type**: Filter algorithm selection

#### **🎯 Phase ComboBoxes (1 total)**
- **Ref Source**: Reference source selection

#### **🎯 XY ComboBoxes (0 total)**
- No ComboBoxes in XY controls

#### **🎯 Band ComboBoxes (0 total)**
- No ComboBoxes in Band controls

#### **🎯 Reverb ComboBoxes (0 total)**
- No ComboBoxes in Reverb controls

### **📋 Complete 2x16 Controls Inventory - 32-Slot Tables**

#### **🎯 Phase Controls (PhaseTab) - 32 Controls**

| Slot | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|------|---|---|---|---|---|---|---|---|----|----|----|----|----|----|----|----|
| **Row 1** | Ref Source | Channel Mode | Follow XO | Capture | Align Mode | Align Goal | Polarity A | Polarity B | Delay Coarse | Delay Fine | Units | Link | Engine | Latency | Reset | Commit |
| **Row 2** | XO Low | XO High | Low AP° | Low Q | Mid AP° | Mid Q | High AP° | High Q | FIR Length | Dynamic | Monitor | Metric | Audition | Trim | Phase Rec | Apply @ Load |

**ComboBoxes:** Ref Source (1)

---

#### **🎯 XY Controls (XYControlsPane) - 32 Controls**

| Slot | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|------|---|---|---|---|---|---|---|---|----|----|----|----|----|----|----|----|
| **Row A** | MONO(2) | HP(1) | BASS(2) | TILT(2) | SCOOP(2) | AIR(2) | LP(1) | Q+QLink(2) | S(1) | BLANK(1) | | | | | | |
| **Row B** | ROT | ASYM | PAN | SAT MIX | 7×BLANK | PUNCH | CNTR | LO | HI | BLANK | | | | | | |

**Double-Wide Controls:** MONO(2), BASS(2), TILT(2), SCOOP(2), AIR(2), Q+QLink(2)  
**ComboBoxes:** None

---

#### **🎯 Band Controls (BandControlsPane) - 32 Controls**

| Slot | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|------|---|---|---|---|---|---|---|---|----|----|----|----|----|----|----|----|
| **Row A** | WIDTH | W LO | W MID | W HI | XO LO | XO HI | SHF L | SHF H | SHF X | TLT S | PVT | A DEP | A THR | ATT | REL | MAX |
| **Row B** | 16×BLANK | | | | | | | | | | | | | | | |

**ComboBoxes:** None

---

#### **🎯 Motion Controls (MotionControlsPane) - 32 Controls** ✅ **CANONICAL REGISTRY**

| Slot | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|------|---|---|---|---|---|---|---|---|----|----|----|----|----|----|----|----|
| **Row A** | Enable | Panner | Path | Rate | Depth | Phase | Spread | Elev | Bounce | Jitter | Quant | Swing | Mode | Retrig | Hold | Sens |
| **Row B** | Offset | Inertia | Front | Doppler | Send | Anchor | Bass | Occlusion | Start Angle | Path Scale | Path Morph | Center Bias | Stereo Link | Random Seed | Motion Smooth | Motion Mix |

**ComboBoxes:** Panner (2), Path (3), Quant (11), Mode (13)  
**Buttons:** Enable (1), Retrig (14), Anchor (22), Stereo Link (29), Random Seed (30)

---

#### **🎯 Reverb Controls (ReverbControlsPane2x16) - 32 Controls**

| Slot | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|------|---|---|---|---|---|---|---|---|----|----|----|----|----|----|----|----|
| **Row A** | PRE | ER L | ER D | ER W | DIFF | MOD DEP | MOD RATE | HP | LP | TILT | EQ MIX | ER→TAIL | LOW× | MID× | HIGH× | TL WID |
| **Row B** | WET | DECAY | SIZE | BLOOM | DIST | DEC XO LO | DEC XO HI | DUCK | ATT | REL | THR | RAT | 4×BLANK | | | |

**ComboBoxes:** None

---

#### **🎯 Delay Controls (DelayControlsPane) - 32 Controls**

| Slot | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|------|---|---|---|---|---|---|---|---|----|----|----|----|----|----|----|----|
| **Row A** | Enabled | Mode | Sync | Grid | Pingpong | Freeze | Filter | Kill Dry | TIME | FB | WET | RATE | DEPTH | SPREAD | WIDTH | PRE |
| **Row B** | SAT | DIFF | SIZE | HP | LP | TILT | WOW | JITTER | Source | Post | THR | DEPTH | ATT | REL | LA | RAT |

**ComboBoxes:** Mode (2), Grid Flavor (4), Duck Source (9), Filter Type (7)  
**Buttons:** Enabled (1), Sync (3), Pingpong (5), Freeze (6), Kill Dry (8)

### **📝 Notes**

- **Current State**: 6/6 control areas consistent (Phase, Reverb, Band, Delay, Motion, XY) ✅ **ALL COMPLETE**
- **Problem Areas**: None - all control areas now use unified enum-based metallic system
- **Priority**: ✅ **COMPLETED** - All controls validated and standardized
- **Architecture**: Successfully moved to pane-only metallic application with enum-based system + canonical registry for Motion
- **Motion System**: ✅ **NEW CANONICAL SYSTEM** - MotionSlot.h provides single source of truth for all 32 Motion controls
- **PluginEditor Cleanup**: ✅ **COMPLETED** - All Motion control creation removed from PluginEditor.cpp/h
- **Blank Placeholders**: ✅ **COMPLETED** - All blank placeholders now use proper metallic styling

### **🎉 Final Status: ALL PHASES COMPLETE**

#### **✅ Phase 1: Delay Controls Standardization** 
- ✅ Moved Delay ToggleButton styling from PluginEditor to DelayControlsPane
- ✅ Moved Delay ComboBox styling from PluginEditor to DelayControlsPane  
- ✅ Applied consistent `MetallicKind::Delay` styling within DelayControlsPane

#### **✅ Phase 2: Motion Controls Canonical Registry Implementation**
- ✅ Created MotionSlot.h with complete 32-slot parameter registry
- ✅ Updated MotionControlsPane to use canonical registry for all controls
- ✅ Removed all Motion control creation from PluginEditor.cpp/h
- ✅ Applied metallic styling to all Motion controls via registry
- ✅ Ensured MotionControlsPane is single source of truth for Motion UI

#### **✅ Phase 3: XY Controls Validation**
- ✅ Validated XY controls already using proper `setAreaMetallicForCell(*cell, MetallicKind::XY)`
- ✅ Confirmed KnobCellWithAux aux components properly styled with `setAreaMetallic(*auxComp, MetallicKind::XY)`
- ✅ Updated BandControlsPane blank placeholders to use `setAreaMetallicForCell(*cell, MetallicKind::Band)`
- ✅ Updated ReverbControlsPane2x16 blank placeholders to use `setAreaMetallicForCell(*cell, MetallicKind::Reverb)`

---

**Last Updated**: January 2025
**Status**: 🎉 **ALL PHASES COMPLETE - METALLIC STYLING SYSTEM FULLY UNIFIED + CANONICAL MOTION REGISTRY + KNOBCELLWITHAUX LAYOUT OPTIMIZED + KNOB LAYOUT REFINED**

### **🔧 Latest Updates (January 2025)**
- **Knob Size**: Increased from 48px to 52px across all control areas
- **Top Padding**: Increased to 12px for better vertical breathing room
- **Gap Optimization**: Reduced gap between knob and data label to 1px minimum
- **Data Label Padding**: Added 3px bottom padding for better visual spacing
- **Gap Calculation**: `G - 8` (accounts for all padding and tightest possible spacing)
