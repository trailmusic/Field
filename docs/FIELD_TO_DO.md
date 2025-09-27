# Field Plugin - TODO List

## Current Issues (High Priority)

### 1. Metallic System Debugging
- **Issue**: Phase controls not showing Phase metallic (Deep Cobalt Interference)
- **Issue**: XY controls still using old metallic rendering instead of new system
- **Issue**: Moving lines in metallics (continuous painting issue)
- **Status**: In progress - debug borders added but visual changes not taking effect

### 2. Phase Controls Metallic Detection
- **Problem**: `phaseMetallic` property not being detected properly
- **Expected**: Red borders around Phase controls
- **Current**: No colored borders or wrong metallic type
- **Files**: `Source/ui/Components/KnobCell.cpp`, `Source/ui/PhaseTab.cpp`

### 3. XY Controls Metallic Rendering
- **Problem**: XY controls falling through to old metallic rendering
- **Expected**: Cyan borders (neutral metallic) around XY controls
- **Current**: Orange borders (fallback metallic)
- **Files**: `Source/ui/Components/KnobCellWithAux.cpp`, `Source/ui/XYControlsPane.h`

### 4. Continuous Painting Issue
- **Problem**: Moving lines in metallics due to continuous repainting
- **Status**: Partially fixed - removed animated interference motif
- **Remaining**: Brushed lines may still be causing continuous painting
- **Files**: `Source/Core/FieldLookAndFeel.cpp`

## Debugging Steps Needed

### Visual Debugging
- [ ] Check colored borders around Phase controls (should be red)
- [ ] Check colored borders around XY controls (should be cyan)
- [ ] Verify gradient transitions are smoother
- [ ] Confirm no moving lines in metallics

### Code Investigation
- [ ] Verify `phaseMetallic` property is being set correctly in PhaseTab.cpp
- [ ] Check if Phase controls are using KnobCell vs KnobCellWithAux
- [ ] Verify metallic rendering logic in KnobCell.cpp and KnobCellWithAux.cpp
- [ ] Check if `showPanel` flag is being set correctly

### Console Debugging
- [ ] Add temporary console output to verify property detection
- [ ] Check if metallic rendering path is being entered
- [ ] Verify component types and property values

## Technical Notes

### Metallic System Architecture
- **Main Function**: `FieldLNF::paintMetal()` in `FieldLookAndFeel.cpp`
- **Phase Function**: `FieldLNF::paintPhaseMetal()` in `FieldLookAndFeel.cpp`
- **Component Types**: 
  - Phase controls use `KnobCell` (not `KnobCellWithAux`)
  - XY controls use `KnobCellWithAux`
- **Property Detection**: `getProperties().getWithDefault("phaseMetallic", false)`

### Debug Borders Color Code
- **Red**: Phase metallic detected ✅
- **Cyan**: Neutral metallic detected ✅
- **Orange**: Fallback metallic ❌
- **Blue**: Band metallic
- **Green**: Reverb metallic
- **Yellow**: Delay metallic
- **Purple**: Motion metallic

## Next Steps

1. **Immediate**: Test current build and report border colors
2. **Debug**: Add console output to verify property detection
3. **Fix**: Correct metallic rendering logic based on findings
4. **Cleanup**: Remove debug borders once system is working
5. **Documentation**: Update metallic system docs once fixed

## Files Modified
- `Source/ui/Components/KnobCell.cpp` - Added debug borders and metallic detection
- `Source/ui/Components/KnobCellWithAux.cpp` - Added debug borders and metallic detection  
- `Source/ui/PhaseTab.cpp` - Added debug properties and console output
- `Source/Core/FieldLookAndFeel.cpp` - Softened gradient transitions, removed continuous painting
- `Source/Core/FieldLookAndFeel.h` - Added Phase metallic system

## Status
- **Build**: ✅ Successful
- **Gradient Transitions**: ✅ Improved
- **Phase Metallic**: ❌ Not working
- **XY Metallic**: ❌ Not working  
- **Continuous Painting**: ⚠️ Partially fixed
- **Debug System**: ✅ In place
