# 🏷️ **Naming Conventions Analysis**

## **📋 Overview**
Analysis of naming patterns across `/Source/ui/Tabs` and `/Source/ui/Panes` directories to identify inconsistencies and establish standardization.

---

## **🎯 Current Naming Patterns**

### **📁 Frontend Order (As Used in UI)**
**Phase → XY → Band → Motion → Reverb → Delay → Dynamic EQ → Imager → Machine**

### **📁 Tabs Directory (`/Source/ui/Tabs`)**

| File | Class Name | Pattern | Purpose | Frontend Order |
|------|------------|---------|---------|----------------|
| `PhaseTab.h` | `PhaseTab` | `[Feature]Tab` | Phase alignment controls | 1st |
| `XYTab.h` | `XYTab` | `[Feature]Tab` | XY pad + EQ controls | 2nd |
| `BandTab.h` | `BandTab` | `[Feature]Tab` | Band-specific width controls | 3rd |
| `DynEqTab.h` | `DynEqTab` | `[Feature]Tab` | Dynamic EQ controls | 7th |
| `ImagerTab.h` | `ImagerTab` | `[Feature]Tab` | Imaging controls | 8th |
| `MachineTab.h` | `MachineTab` | `[Feature]Tab` | Machine learning controls | 9th |

**Tab Pattern**: `[Feature]Tab` - Consistent ✅

### **📁 Additional Tab Directories**

| Directory | File | Class Name | Pattern | Purpose | Frontend Order |
|-----------|------|------------|---------|---------|----------------|
| `/motion/` | `MotionTab.h` | `MotionTab` | `[Feature]Tab` | Motion controls | 4th |
| `/reverb/ui/` | `ReverbTab.h` | `ReverbTab` | `[Feature]Tab` | Reverb controls | 5th |
| `/delay/` | `DelayTab.h` | `DelayTab` | `[Feature]Tab` | Delay controls | 6th |
| `/machine/` | `MachinePane.h` | `MachinePane` | `[Feature]Pane` | Machine learning visuals | 9th |

**Note**: Delay and Machine moved to top-level directories ✅

### **📁 Panes Directory (`/Source/ui/Panes`)**

| File | Class Name | Pattern | Purpose |
|------|------------|---------|---------|
| `BandControlsPane.h` | `BandControlsPane` | `[Feature]ControlsPane` | Band width controls |
| `ImagerControlsPane.h` | `ImagerControlsPane` | `[Feature]ControlsPane` | Imaging controls |
| `XYControlsPane.h` | `XYControlsPane` | `[Feature]ControlsPane` | XY EQ controls |
| `BandGraphics.h` | `BandGraphics` | `[Feature]Graphics` | Band visuals |
| `ImagerPane.h` | `ImagerPane` | `[Feature]Pane` | Imager visuals |
| `ProcessedSpectrumPane.h` | `ProcessedSpectrumPane` | `[Feature]Pane` | Spectrum analysis |

**Pane Patterns**: Mixed ❌
- `[Feature]ControlsPane` (3 files)
- `[Feature]Graphics` (1 file) 
- `[Feature]Pane` (2 files)

### **📁 Additional Graphics Files**

| Directory | File | Class Name | Pattern | Purpose | Frontend Order |
|-----------|------|------------|---------|---------|----------------|
| `/motion/` | `MotionGraphics.h` | `MotionGraphics` | `[Feature]Graphics` | Motion graphics | 4th |
| `/reverb/ui/` | `ReverbGraphics.h` | `ReverbGraphics` | `[Feature]Graphics` | Reverb graphics | 5th |
| `/delay/` | `DelayVisuals.h` | `DelayVisuals` | `[Feature]Visuals` | Delay graphics | 6th |
| `/machine/` | `MachinePane.h` | `MachinePane` | `[Feature]Pane` | Machine graphics | 9th |
| `/ui/Components/` | `XYPad.h` | `XYPad` | `[Feature]Pad` | XY graphics | 2nd |

**Note**: Graphics files are scattered across multiple directories with inconsistent naming

---

## **🚨 Naming Inconsistencies**

### **1. Pane Naming Inconsistency**
- **Controls Panes**: `BandControlsPane`, `ImagerControlsPane`, `XYControlsPane`
- **Graphics Panes**: `BandGraphics`, `ImagerPane`, `ProcessedSpectrumPane`
- **Mixed Patterns**: Some use `ControlsPane`, others use `Graphics`, others use `Pane`

### **2. Directory Structure Chaos**
- **Tabs scattered**: Main tabs in `/ui/Tabs/`, but `DelayTab` now in `/delay/` ✅
- **Graphics scattered**: `BandGraphics` in `/ui/Panes/`, `DelayVisuals` now in `/delay/` ✅, `MachinePane` now in `/machine/` ✅
- **XYPad**: In `Components/` but used by `XYTab`
- **No clear organization**: Files are spread across multiple directories

### **3. Missing Graphics Files**
- **XYTab**: Uses `XYPad` (not in Panes directory)
- **DynEqTab**: No corresponding graphics file
- **PhaseTab**: No corresponding graphics file

### **4. Naming Pattern Inconsistencies**
- **Graphics naming**: `BandGraphics`, `MotionGraphics`, `ReverbGraphics`, `DelayVisuals`, `ImagerPane`, `ProcessedSpectrumPane`, `XYPad`
- **6 different patterns**: `[Feature]Graphics`, `[Feature]Visuals`, `[Feature]Pane`, `[Feature]Pad`
- **No standardization**: Each graphics file follows different naming convention

### **5. Complete File Inventory by Frontend Order**

| Frontend Order | Tab | Graphics | Controls | Directory | Status |
|----------------|-----|----------|----------|-----------|---------|
| **1st - Phase** | `PhaseTab` | ❌ Missing | ✅ **EXISTS** (in PhaseTab.cpp) | `/ui/Tabs/` | **Controls need extraction** |
| **2nd - XY** | `XYTab` | `XYPad` | `XYControlsPane` | `/ui/Tabs/`, `/ui/Components/`, `/ui/Panes/` | ✅ Complete |
| **3rd - Band** | `BandTab` | `BandGraphics` | `BandControlsPane` | `/ui/Tabs/`, `/ui/Panes/` | ✅ Complete |
| **4th - Motion** | `MotionTab` | `MotionGraphics` | `MotionControlsPane` | `/motion/` | ✅ Complete |
| **5th - Reverb** | `ReverbTab` | `ReverbGraphics` | `ReverbControlsPane` | `/reverb/ui/` | ✅ Complete |
| **6th - Delay** | `DelayTab` | `DelayVisuals` | `DelayControlsPane` | `/ui/delay/` | ✅ Complete |
| **7th - Dynamic EQ** | `DynEqTab` | ❌ Missing | ❌ Missing | `/ui/Tabs/` | **No 2x16 controls** |
| **8th - Imager** | `ImagerTab` | `ImagerPane` | `ImagerControlsPane` | `/ui/Tabs/`, `/ui/Panes/` | **Controls may be outdated** |
| **9th - Machine** | `MachineTab` | `MachinePane` | ❌ Missing | `/ui/Tabs/`, `/ui/machine/` | **No 2x16 controls** |

### **6. Key Findings**

#### **✅ Complete with 2x16 Controls (First 6)**
- **Phase**: Controls exist in `PhaseTab.cpp` but need extraction to `PhaseControlsPane.h`
- **XY**: Complete with `XYControlsPane` 
- **Band**: Complete with `BandControlsPane`
- **Motion**: Complete with `MotionControlsPane`
- **Reverb**: Complete with `ReverbControlsPane`
- **Delay**: Complete with `DelayControlsPane`

#### **❌ Missing 2x16 Controls (Last 3)**
- **Dynamic EQ**: No bottom controls (uses overlay system)
- **Imager**: `ImagerControlsPane` exists but may be outdated
- **Machine**: No 2x16 controls (uses proposal cards)

#### **🔍 Controls Extraction Needed**
- **Phase**: 32 controls in `PhaseTab.cpp` need extraction to `PhaseControlsPane.h`
- **Imager**: `ImagerControlsPane` may need verification/update

---

## **🎯 Current Directory Structure Analysis**

### **📁 Directory Level Issues**

#### **✅ Good Examples (Colocated)**
- **Motion**: `/motion/` - Everything together
  - `MotionTab.h`, `MotionGraphics.h`, `MotionControlsPane.h`
- **Reverb**: `/reverb/ui/` - Feature-specific subdirectory
  - `ReverbTab.h`, `ReverbGraphics.h`, `ReverbControlsPane.h`

#### **❌ Problematic Examples (Scattered)**
- **XY**: Scattered across 3 directories
  - `XYTab.h` → `/ui/Tabs/`
  - `XYPad.h` → `/ui/Components/`
  - `XYControlsPane.h` → `/ui/Panes/`
- **Band**: Split across 2 directories
  - `BandTab.h` → `/ui/Tabs/`
  - `BandGraphics.h`, `BandControlsPane.h` → `/ui/Panes/`
- **Delay**: Split across 2 directories
  - `DelayTab.h` → `/ui/delay/`
  - `DelayVisuals.h` → `/ui/delay/`
  - `DelayControlsPane.h` → `/ui/delay/`

#### **🔍 UI Directory Misuse**
- **`/ui/Panes/`**: Mix of graphics and controls
- **`/ui/Components/`**: Has `XYPad` (should be graphics)
- **`/ui/Tabs/`**: Only has tabs, missing their graphics/controls
- **`/ui/`**: General UI components mixed with feature-specific

### **🎯 Recommended Colocation Strategy**

#### **Option 1: Feature-Based Colocation (Like Motion)**
```
Source/
├── phase/           # Phase feature
│   ├── PhaseTab.h
│   ├── PhaseGraphics.h
│   └── PhaseControlsPane.h
├── xy/              # XY feature  
│   ├── XYTab.h
│   ├── XYPad.h
│   └── XYControlsPane.h
├── band/            # Band feature
│   ├── BandTab.h
│   ├── BandGraphics.h
│   └── BandControlsPane.h
├── motion/          # Motion feature (already good)
├── reverb/          # Reverb feature (already good)
├── delay/           # Delay feature (already good)
├── dynEq/           # Dynamic EQ feature
├── imager/          # Imager feature
└── machine/         # Machine feature
```

#### **Option 2: UI-Level Organization**
```
Source/ui/
├── tabs/            # All tabs
├── graphics/        # All graphics
├── controls/        # All control panes
└── components/      # Shared UI components
```

#### **Option 3: Top-Level Feature Colocation (Recommended)**
```
Source/
├── motion/          # ✅ Already perfect (11 files)
├── reverb/          # ✅ Already good (8 files + ui/ subdir)
├── delay/           # ❌ Move from /ui/delay/ (4 files)
├── machine/         # ❌ Move from /ui/machine/ (7 files)
├── phase/           # ❌ Extract from /ui/Tabs/ (2 files + create graphics/controls)
├── xy/              # ❌ Consolidate from 3 directories (3 files)
├── band/            # ❌ Consolidate from 2 directories (3 files)
├── dynEq/           # ❌ Extract from /ui/Tabs/ (1 file + create graphics/controls)
├── imager/          # ❌ Consolidate from 2 directories (3 files)
└── ui/              # Shared UI components only
    ├── components/  # Shared components (KnobCell, etc.)
    ├── managers/    # UI managers (ButtonManager, etc.)
    └── layout/      # Layout system
```

### **📋 Required Moves to Match Motion Pattern**

#### **✅ Already Good (No Changes Needed)**
- **Motion**: `/motion/` - 11 files, perfect colocation ✅
- **Reverb**: `/reverb/` - 8 files + `/reverb/ui/` subdirectory ✅

#### **✅ Completed Moves to Top Level**
- **Delay**: ✅ Moved from `/ui/delay/` → `/delay/` (4 files)
- **Machine**: ✅ Moved from `/ui/machine/` → `/machine/` (7 files)

#### **🆕 Need to Create Top Level**
- **Phase**: Extract from `/ui/Tabs/` → `/phase/` (2 files + create missing)
- **XY**: Consolidate from 3 directories → `/xy/` (3 files)
- **Band**: Consolidate from 2 directories → `/band/` (3 files)
- **DynEq**: Extract from `/ui/Tabs/` → `/dynEq/` (1 file + create missing)
- **Imager**: Consolidate from 2 directories → `/imager/` (3 files)

### **📋 Core, DSP, and UI Directory Strategy**

#### **✅ Core Directory (Foundation Layer - No Changes)**
- **Purpose**: Core plugin functionality
- **Contents**: PluginEditor, PluginProcessor, FieldLookAndFeel, FieldMetallic, IconSystem
- **Status**: ✅ **Stays as-is** - foundation layer that everything builds on
- **Files**: 14 files including core plugin logic

#### **✅ DSP Directory (Audio Processing Layer - No Changes)**  
- **Purpose**: Audio processing engines
- **Contents**: DelayEngine, PhaseAlignmentEngine, Ducker, PhaseModes
- **Status**: ✅ **Stays as-is** - audio processing layer
- **Files**: 8 files including all DSP engines

#### **🔄 UI Directory (Shared Components Only - Cleanup Needed)**
- **Current Problem**: Still contains feature-specific files that should be moved
- **Goal**: Only contain **shared UI components** (KnobCell, ButtonManager, etc.)
- **Current Structure**:
  - ✅ **Keep**: `/ui/Components/` (52 files) - shared UI components
  - ✅ **Keep**: `/ui/Managers/` (17 files) - UI managers
  - ✅ **Keep**: `/ui/Layout/` (4 files) - layout system
  - ✅ **Keep**: `/ui/Design/`, `/ui/Engines/`, `/ui/Events/` - shared systems
  - ❌ **Move**: `/ui/Tabs/` (10 files) → Move to top-level feature directories
  - ❌ **Move**: `/ui/Panes/` (9 files) → Move to top-level feature directories

#### **🎯 Final UI Directory Structure**
```
Source/ui/
├── Components/     # Shared UI components (KnobCell, etc.)
├── Managers/       # UI managers (ButtonManager, etc.)
├── Layout/         # Layout system
├── Design/         # Design system
├── Engines/        # UI engines
├── Events/         # Event system
└── Controls/       # Control components
```

---

## **📊 Current vs. Recommended Structure**

### **Current Structure (Inconsistent)**
```
Source/ui/
├── Tabs/
│   ├── BandTab.h
│   ├── XYTab.h
│   └── ...
├── Panes/
│   ├── BandControlsPane.h    # Controls
│   ├── BandGraphics.h        # Graphics
│   ├── XYControlsPane.h      # Controls
│   └── ...
└── Components/
    ├── XYPad.h               # Graphics (misplaced)
    └── ...
```

### **Recommended Structure (Consistent)**
```
Source/ui/
├── Tabs/
│   ├── BandTab.h
│   ├── XYTab.h
│   └── ...
├── Controls/
│   ├── BandControlsPane.h
│   ├── XYControlsPane.h
│   └── ...
├── Graphics/
│   ├── BandGraphics.h
│   ├── XYGraphics.h
│   └── ...
└── Components/
    ├── KnobCell.h
    └── ...
```

---

## **🔧 Implementation Plan**

### **Phase 1: Naming Standardization**
1. **Rename Graphics Classes**: `ImagerPane` → `ImagerGraphics`
2. **Rename Graphics Classes**: `ProcessedSpectrumPane` → `ProcessedSpectrumGraphics`
3. **Create Missing Graphics**: `XYGraphics.h` (move from `XYPad.h`)
4. **Standardize Pane Names**: Ensure all follow `[Feature]ControlsPane` pattern

### **Phase 2: Directory Reorganization**
1. **Create `Graphics/` directory**: Move graphics files from `Panes/`
2. **Rename `Panes/` to `Controls/`**: Move control panes
3. **Update includes**: Fix all import paths
4. **Update CMakeLists.txt**: Update build configuration

### **Phase 3: Missing Components**
1. **Create DynEqGraphics.h**: For DynEqTab visuals
2. **Create MachineGraphics.h**: For MachineTab visuals  
3. **Create PhaseGraphics.h**: For PhaseTab visuals
4. **Move XYPad.h**: From `Components/` to `Graphics/` as `XYGraphics.h`

---

## **📈 Benefits of Standardization**

### **✅ Consistency**
- All tabs follow `[Feature]Tab` pattern
- All controls follow `[Feature]ControlsPane` pattern
- All graphics follow `[Feature]Graphics` pattern

### **✅ Organization**
- Clear separation between tabs, controls, and graphics
- Easy to locate components by type
- Logical directory structure

### **✅ Maintainability**
- Predictable naming conventions
- Easy to add new features
- Clear component responsibilities

### **✅ Developer Experience**
- Intuitive file locations
- Consistent import paths
- Clear component relationships

---

## **🎯 Next Steps**

1. **Analyze current usage**: Check how each component is used
2. **Plan migration**: Create detailed migration plan
3. **Implement changes**: Execute naming and directory changes
4. **Update documentation**: Reflect new structure in docs
5. **Test thoroughly**: Ensure no regressions

---

*Last Updated: Naming conventions analysis completed*
*Status: ✅ Patterns identified, standardization plan created*
