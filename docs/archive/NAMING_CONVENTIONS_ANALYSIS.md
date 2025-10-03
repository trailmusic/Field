# 🏷️ **Naming Conventions Analysis**

## **📋 Overview**
Analysis of naming patterns across the Field plugin codebase to identify inconsistencies and establish standardization. **UPDATED**: Reflects completed directory reorganization into `shared/` and `features/` structure.

---

## **🎯 Current Naming Patterns**

### **📁 Frontend Order (As Used in UI)**
**Phase → XY → Band → Motion → Reverb → Delay → Dynamic EQ → Imager → Machine**

### **📁 Features Directory (`/Source/features/`) - ✅ COMPLETED REORGANIZATION**

| Directory | File | Class Name | Pattern | Purpose | Frontend Order |
|-----------|------|------------|---------|---------|----------------|
| `features/phase/` | `PhaseTab.h` | `PhaseTab` | `[Feature]Tab` | Phase alignment controls | 1st |
| `features/xy/` | `XYTab.h` | `XYTab` | `[Feature]Tab` | XY pad + EQ controls | 2nd |
| `features/band/` | `BandTab.h` | `BandTab` | `[Feature]Tab` | Band-specific width controls | 3rd |
| `features/motion/` | `MotionTab.h` | `MotionTab` | `[Feature]Tab` | Motion controls | 4th |
| `features/reverb/` | `ReverbTab.h` | `ReverbTab` | `[Feature]Tab` | Reverb controls | 5th |
| `features/delay/` | `DelayTab.h` | `DelayTab` | `[Feature]Tab` | Delay controls | 6th |
| `features/dynEq/` | `DynEqTab.h` | `DynEqTab` | `[Feature]Tab` | Dynamic EQ controls | 7th |
| `features/imager/` | `ImagerTab.h` | `ImagerTab` | `[Feature]Tab` | Imaging controls | 8th |
| `features/machine/` | `MachineTab.h` | `MachineTab` | `[Feature]Tab` | Machine learning controls | 9th |

**Tab Pattern**: `[Feature]Tab` - Consistent ✅  
**Directory Structure**: All features now properly colocated ✅

### **📁 Shared Directory (`/Source/shared/`) - ✅ COMPLETED REORGANIZATION**

| Directory | Purpose | Contents | Status |
|-----------|---------|----------|---------|
| `shared/Core/` | Core plugin functionality | PluginEditor, PluginProcessor, FieldLookAndFeel, etc. | ✅ Moved from `/Source/Core/` |
| `shared/dsp/` | Audio processing engines | DelayEngine, PhaseAlignmentEngine, Ducker, etc. | ✅ Moved from `/Source/dsp/` |
| `shared/ui/` | Shared UI components | KnobCell, ButtonManager, LayoutManager, etc. | ✅ Moved from `/Source/shared/ui/ ✅ **MOVED TO SHARED**` |
| `shared/Presets/` | Preset system | PresetManager, PresetRegistry, PresetStore, etc. | ✅ Moved from `/Source/Presets/` |

**Shared Components**: All shared functionality now properly organized ✅

### **📁 Features Controls (`/Source/features/*/`) - ✅ COMPLETED REORGANIZATION**

| Feature | Controls File | Graphics File | Pattern | Status |
|--------|---------------|---------------|---------|---------|
| **XY** | `features/xy/XYControlsPane.h` | `features/xy/XYPad.h` | `[Feature]ControlsPane` + `[Feature]Pad` | ✅ Consolidated |
| **Band** | `features/band/BandControlsPane.h` | `features/band/BandGraphics.h` | `[Feature]ControlsPane` + `[Feature]Graphics` | ✅ Consolidated |
| **Imager** | `features/imager/ImagerControlsPane.h` | `features/imager/ImagerPane.h` | `[Feature]ControlsPane` + `[Feature]Pane` | ✅ Consolidated |
| **Reverb** | `features/reverb/ReverbControlsPane.h` | `features/reverb/ReverbGraphics.h` | `[Feature]ControlsPane` + `[Feature]Graphics` | ✅ Consolidated |
| **Delay** | `features/delay/DelayControlsPane.h` | `features/delay/DelayVisuals.h` | `[Feature]ControlsPane` + `[Feature]Visuals` | ✅ Consolidated |
| **Motion** | `features/motion/MotionControlsPane.h` | `features/motion/MotionGraphics.h` | `[Feature]ControlsPane` + `[Feature]Graphics` | ✅ Already Perfect |
| **DynEq** | `features/dynEq/ProcessedSpectrumPane.h` | N/A | `[Feature]Pane` | ✅ Consolidated |

**Pattern Status**: ✅ **ALL FEATURES NOW PROPERLY COLOCATED**

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

## **🎉 MAJOR REORGANIZATION COMPLETED** ✅

### **📁 New Directory Structure**
```
Source/
├── shared/                    # Shared components
│   ├── Core/                  # Core plugin functionality
│   ├── dsp/                   # Audio processing engines  
│   ├── ui/                    # Shared UI components
│   └── Presets/               # Preset system
└── features/                  # Feature-specific components
    ├── phase/                 # Phase alignment
    ├── xy/                    # XY pad + EQ
    ├── band/                  # Band width controls
    ├── motion/                # Motion controls
    ├── reverb/                # Reverb system
    ├── delay/                 # Delay system
    ├── dynEq/                 # Dynamic EQ
    ├── imager/                # Imaging controls
    └── machine/               # Machine learning
```

### **✅ Completed Achievements**
- **Feature Colocation**: All features now properly colocated in dedicated directories
- **Shared Components**: All shared functionality organized under `shared/`
- **Clean Architecture**: Clear separation between shared and feature-specific code
- **Build Success**: All targets (Standalone, AU, VST3) compile and link successfully
- **Include Paths**: All include paths updated to reflect new structure
- **CMakeLists.txt**: Build system updated with new directory structure

---

## **🚨 Naming Inconsistencies** (Historical - Most Issues Resolved)

### **1. Pane Naming Inconsistency**
- **Controls Panes**: `BandControlsPane`, `ImagerControlsPane`, `XYControlsPane`
- **Graphics Panes**: `BandGraphics`, `ImagerPane`, `ProcessedSpectrumPane`
- **Mixed Patterns**: Some use `ControlsPane`, others use `Graphics`, others use `Pane`

### **2. Directory Structure** ✅ **RESOLVED**
- **All features colocated**: All features now properly organized in `features/` directory ✅
- **Shared components**: All shared components organized in `shared/` directory ✅
- **Clean architecture**: Clear separation between shared and feature-specific code ✅
- **No scattered files**: All files properly organized by feature ✅

### **3. Graphics Files** ✅ **RESOLVED**
- **XYTab**: Uses `XYPad` in `features/xy/XYPad.h` ✅
- **DynEqTab**: Uses `ProcessedSpectrumPane` in `features/dynEq/ProcessedSpectrumPane.h` ✅
- **PhaseTab**: Graphics integrated in `features/phase/PhaseTab.h` ✅

### **4. Naming Pattern Inconsistencies**
- **Graphics naming**: `BandGraphics`, `MotionGraphics`, `ReverbGraphics`, `DelayVisuals`, `ImagerPane`, `ProcessedSpectrumPane`, `XYPad`
- **6 different patterns**: `[Feature]Graphics`, `[Feature]Visuals`, `[Feature]Pane`, `[Feature]Pad`
- **No standardization**: Each graphics file follows different naming convention

### **5. Complete File Inventory by Frontend Order** ✅ **UPDATED**

| Frontend Order | Tab | Graphics | Controls | Directory | Status |
|----------------|-----|----------|----------|-----------|---------|
| **1st - Phase** | `PhaseTab` | Integrated | Integrated | `features/phase/` | ✅ Complete |
| **2nd - XY** | `XYTab` | `XYPad` | `XYControlsPane` | `features/xy/` | ✅ Complete |
| **3rd - Band** | `BandTab` | `BandGraphics` | `BandControlsPane` | `features/band/` | ✅ Complete |
| **4th - Motion** | `MotionTab` | `MotionGraphics` | `MotionControlsPane` | `features/motion/` | ✅ Complete |
| **5th - Reverb** | `ReverbTab` | `ReverbGraphics` | `ReverbControlsPane` | `features/reverb/` | ✅ Complete |
| **6th - Delay** | `DelayTab` | `DelayVisuals` | `DelayControlsPane` | `features/delay/` | ✅ Complete |
| **7th - Dynamic EQ** | `DynEqTab` | `ProcessedSpectrumPane` | N/A (overlay system) | `features/dynEq/` | ✅ Complete |
| **8th - Imager** | `ImagerTab` | `ImagerPane` | `ImagerControlsPane` | `features/imager/` | ✅ Complete |
| **9th - Machine** | `MachineTab` | `MachinePane` | N/A (proposal cards) | `features/machine/` | ✅ Complete |

### **6. Key Findings**

#### **✅ Complete Features (All 9)**
- **Phase**: Complete with integrated controls in `features/phase/PhaseTab.h` ✅
- **XY**: Complete with `XYControlsPane` in `features/xy/` ✅
- **Band**: Complete with `BandControlsPane` in `features/band/` ✅
- **Motion**: Complete with `MotionControlsPane` in `features/motion/` ✅
- **Reverb**: Complete with `ReverbControlsPane` in `features/reverb/` ✅
- **Delay**: Complete with `DelayControlsPane` in `features/delay/` ✅
- **Dynamic EQ**: Complete with overlay system in `features/dynEq/` ✅
- **Imager**: Complete with `ImagerControlsPane` in `features/imager/` ✅
- **Machine**: Complete with proposal cards in `features/machine/` ✅

#### **🔍 Future Improvements** (Optional)
- **Phase**: Could extract controls to separate `PhaseControlsPane.h` for consistency
- **Imager**: Could verify `ImagerControlsPane` is up-to-date with current functionality

---

## **🎯 Current Directory Structure Analysis** ✅ **RESOLVED**

### **📁 Directory Structure** ✅ **ALL ISSUES RESOLVED**

#### **✅ All Features Now Properly Colocated**
- **Motion**: `features/motion/` - Everything together ✅
  - `MotionTab.h`, `MotionGraphics.h`, `MotionControlsPane.h`
- **Reverb**: `features/reverb/` - Feature-specific directory ✅
  - `ReverbTab.h`, `ReverbGraphics.h`, `ReverbControlsPane.h`
- **XY**: `features/xy/` - All consolidated ✅
  - `XYTab.h`, `XYPad.h`, `XYControlsPane.h`
- **Band**: `features/band/` - All consolidated ✅
  - `BandTab.h`, `BandGraphics.h`, `BandControlsPane.h`
- **Delay**: `features/delay/` - All consolidated ✅
  - `DelayTab.h`, `DelayVisuals.h`, `DelayControlsPane.h`

#### **✅ Shared Components Properly Organized**
- **`shared/ui/Components/`**: Shared UI components only ✅
- **`shared/ui/Managers/`**: UI managers only ✅
- **`shared/ui/Layout/`**: Layout system only ✅
- **`shared/Core/`**: Core plugin functionality ✅
- **`shared/dsp/`**: Audio processing engines ✅
- **`shared/Presets/`**: Preset system ✅

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
Source/shared/ui/ ✅ **MOVED TO SHARED**
├── tabs/            # All tabs
├── graphics/        # All graphics
├── controls/        # All control panes
└── components/      # Shared UI components
```

#### **✅ COMPLETED: Top-Level Feature Colocation**
```
Source/
├── shared/          # ✅ Shared components
│   ├── Core/        # Core plugin functionality
│   ├── dsp/         # Audio processing engines
│   ├── ui/          # Shared UI components
│   └── Presets/     # Preset system
└── features/        # ✅ Feature-specific components
    ├── phase/       # Phase alignment
    ├── xy/          # XY pad + EQ
    ├── band/        # Band width controls
    ├── motion/      # Motion controls
    ├── reverb/      # Reverb system
    ├── delay/       # Delay system
    ├── dynEq/       # Dynamic EQ
    ├── imager/      # Imaging controls
    └── machine/     # Machine learning
```

### **📋 COMPLETED: All Required Moves** ✅

#### **✅ All Features Successfully Moved**
- **Motion**: `features/motion/` - 11 files, perfect colocation ✅
- **Reverb**: `features/reverb/` - 8 files, consolidated from nested structure ✅
- **Delay**: `features/delay/` - 4 files, moved from `/ui/delay/` ✅
- **Machine**: `features/machine/` - 7 files, moved from `/ui/machine/` ✅
- **Phase**: `features/phase/` - 2 files, extracted from `/ui/Tabs/` ✅
- **XY**: `features/xy/` - 3 files, consolidated from 3 directories ✅
- **Band**: `features/band/` - 3 files, consolidated from 2 directories ✅
- **DynEq**: `features/dynEq/` - 1 file, extracted from `/ui/Tabs/` ✅
- **Imager**: `features/imager/` - 3 files, consolidated from 2 directories ✅

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
Source/shared/ui/ ✅ **MOVED TO SHARED**
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

### **✅ COMPLETED: New Structure (Consistent)**
```
Source/
├── shared/                    # Shared components
│   ├── Core/                 # Core plugin functionality
│   ├── dsp/                  # Audio processing engines
│   ├── ui/                   # Shared UI components
│   │   ├── Components/       # KnobCell, ButtonManager, etc.
│   │   ├── Managers/         # UI managers
│   │   ├── Layout/           # Layout system
│   │   └── ...
│   └── Presets/              # Preset system
└── features/                 # Feature-specific components
    ├── phase/                # Phase alignment
    ├── xy/                   # XY pad + EQ
    ├── band/                 # Band width controls
    ├── motion/               # Motion controls
    ├── reverb/               # Reverb system
    ├── delay/                # Delay system
    ├── dynEq/                # Dynamic EQ
    ├── imager/               # Imaging controls
    └── machine/              # Machine learning
```

---

## **🔧 Implementation Plan** ✅ **COMPLETED**

### **Phase 1: Naming Standardization** ✅ **COMPLETED**
1. **Directory Reorganization**: All features moved to `features/` structure ✅
2. **Shared Components**: All shared components moved to `shared/` structure ✅
3. **Include Path Updates**: All include statements updated to reflect new structure ✅
4. **Build System**: CMakeLists.txt updated with new directory structure ✅

### **Phase 2: Directory Reorganization** ✅ **COMPLETED**
1. **Features Directory**: All features properly colocated in `features/` ✅
2. **Shared Directory**: All shared components organized in `shared/` ✅
3. **Clean Architecture**: Clear separation between shared and feature-specific code ✅
4. **Build Success**: All targets (Standalone, AU, VST3) compile and link successfully ✅

### **Phase 3: Missing Components** ✅ **COMPLETED**
1. **DynEq**: Complete with `ProcessedSpectrumPane` in `features/dynEq/` ✅
2. **Machine**: Complete with `MachinePane` in `features/machine/` ✅
3. **Phase**: Complete with integrated graphics in `features/phase/` ✅
4. **XY**: Complete with `XYPad` in `features/xy/` ✅

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
