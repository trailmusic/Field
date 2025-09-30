# 🗺️ **Control Ownership Map**

## **📋 Overview**
This document maps all controls across the Field plugin features to their single owners, eliminating redundancy and clarifying responsibilities. **UPDATED**: Reflects completed directory reorganization into `features/` structure.

---

## **🎯 Control Ownership by Pane**

### **1. XYControlsPane.h** (`Source/features/xy/XYControlsPane.h`) ✅ **MOVED TO FEATURES**
**Primary Owner: EQ + Imaging + Center Processing**

#### **EQ Controls (Slots 1-15)**
- **MONO** (slots 1-2): `monoHz`, `monoV`, `monoSlopeChoice`, `monoAuditionButton`
- **HP** (slot 3): `hp`, `hpV` 
- **BASS** (slots 4-5): `bass`, `bassV`, `bassFreq`, `bassFreqV`
- **TILT** (slots 6-7): `tilt`, `tiltV`, `tiltFreq`, `tiltFreqV`
- **SCOOP** (slots 8-9): `scoop`, `scoopV`, `scoopFreq`, `scoopFreqV`
- **AIR** (slots 10-11): `air`, `airV`, `airFreq`, `airFreqV`
- **LP** (slot 12): `lp`, `lpV`
- **Q LINK** (slots 13-14): `q`, `qV`, `qLink`, `hpQ`, `hpQV`, `lpQ`, `lpQV`
- **SHELF SHAPE** (slot 15): `shelfS`, `shelfSV`

#### **Imaging Controls (Moved from Imager)**
- **ROTATION**: `rotation`, `rotationV`
- **ASYMMETRY**: `asym`, `asymV`
- **PAN**: `pan`, `panV`
- **SAT MIX**: `satMix`, `satMixV`

#### **Center Processing Controls (Slots 27-32)**
- **PUNCH MODE** (slot 27): `punchMode` (UpwardComboBox)
- **PUNCH** (slot 28): `punchAmt`, `punchAmtV`
- **CNTR** (slot 29): `promDb`, `promDbV`
- **LO** (slot 30): `focusLo`, `focusLoV`
- **HI** (slot 31): `focusHi`, `focusHiV`
- **PHASE REC** (slot 32): `phaseRecOn`
- **PHASE** (slot 33): `phaseAmt`, `phaseAmtV`
- **CNTR LOCK** (slot 34): `centerLockOn`

---

### **2. BandControlsPane.h** (`Source/features/band/BandControlsPane.h`) ✅ **MOVED TO FEATURES**
**Primary Owner: WIDTH + Crossover + Shuffle + Designer Controls**

#### **WIDTH Controls**
- **WIDTH**: `width`, `widthV`
- **W LO**: `widthLo`, `widthLoV`
- **W MID**: `widthMid`, `widthMidV`
- **W HI**: `widthHi`, `widthHiV`

#### **Crossover Controls**
- **XO LO**: `xoLo`, `xoLoV`
- **XO HI**: `xoHi`, `xoHiV`

#### **Shuffle Controls**
- **SHF L**: `shufLo`, `shufLoV`
- **SHF H**: `shufHi`, `shufHiV`
- **SHF X**: `shufX`, `shufXV`

#### **Designer Controls (7)**
- **TLT S**: `sideTiltDbOct`, `valSideTilt`
- **PVT**: `pivotHz`, `valPivot`
- **A DEP**: `autoDepth`, `valAutoDepth`
- **A THR**: `autoThrDb`, `valAutoThr`
- **ATT**: `autoAtkMs`, `valAtk`
- **REL**: `autoRelMs`, `valRel`
- **MAX**: `maxWidth`, `valMax`

---

### **3. ImagerControlsPane.h**
**Primary Owner: [EMPTY - Controls Removed]**
- **Status**: WIDTH controls moved to BandControlsPane.h
- **Status**: Imaging controls moved to XYControlsPane.h
- **Current**: Empty pane with blank placeholders only

---

### **4. ProcessedSpectrumPane.h**
**Primary Owner: Spectrum Analysis**
- **Status**: Graphics/visualization pane
- **Controls**: Spectrum display and analysis tools

---

## **🚫 Eliminated Redundancy**

### **Previously Duplicate Controls (Now Single Owner)**
1. **WIDTH Controls** - Was in both `BandControlsPane.h` AND `ImagerControlsPane.h`
   - **Resolution**: Now only in `BandControlsPane.h`
   - **Removed from**: `ImagerControlsPane.h`

2. **Imaging Controls** - Was in both `ImagerPane.h` AND `XYControlsPane.h`
   - **Resolution**: Now only in `XYControlsPane.h`
   - **Moved from**: `ImagerPane.h` to `XYControlsPane.h`

---

## **📊 Control Distribution Summary**

| Pane | EQ Controls | Imaging | Center Processing | WIDTH | Crossover | Shuffle | Designer | Total Controls |
|------|-------------|---------|-------------------|-------|-----------|---------|----------|----------------|
| **XYControlsPane** | ✅ 15 | ✅ 4 | ✅ 8 | ❌ | ❌ | ❌ | ❌ | **27** |
| **BandControlsPane** | ❌ | ❌ | ❌ | ✅ 4 | ✅ 2 | ✅ 3 | ✅ 7 | **16** |
| **ImagerControlsPane** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | **0** |
| **ProcessedSpectrumPane** | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | **Graphics** |

---

## **🎯 Key Benefits**

### **✅ Eliminated Redundancy**
- **WIDTH controls**: 4 duplicate controls removed
- **Imaging controls**: 4 duplicate controls consolidated
- **Total redundancy eliminated**: 8+ duplicate controls

### **✅ Clear Ownership**
- Each control has exactly one owner
- No confusion about which pane controls what
- Easy to locate and modify controls

### **✅ Simplified Architecture**
- `ImagerControlsPane.h` now empty (ready for removal or repurposing)
- `XYControlsPane.h` owns EQ + Imaging + Center Processing
- `BandControlsPane.h` owns WIDTH + Crossover + Shuffle + Designer

---

## **🔄 Next Steps**

### **Phase 1: Complete Redundancy Elimination**
- [ ] Verify no other duplicate controls exist
- [ ] Consider removing empty `ImagerControlsPane.h`
- [ ] Update any references to moved controls

### **Phase 2: Architecture Optimization**
- [ ] Standardize control creation patterns across panes
- [ ] Create shared control factory methods
- [ ] Implement consistent grid positioning system

### **Phase 3: Documentation**
- [ ] Update all pane documentation
- [ ] Create control reference guide
- [ ] Document control parameter mappings

---

## **📝 Notes**

- **Control Count**: Total unique controls across all panes
- **Redundancy**: Previously had 8+ duplicate controls, now eliminated
- **Architecture**: Clean separation of concerns with single ownership
- **Maintenance**: Much easier to locate and modify controls
- **Testing**: Reduced complexity for testing and validation

---

*Last Updated: Control redundancy elimination completed*
*Status: ✅ Redundancy eliminated, ownership clarified*
