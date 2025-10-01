# Delay Feature Development Notes

## Theme Change Fix Reference (January 2025)

### **🎯 EQ Theme Compliance Solution**

**Reference Implementation:**
- **Reverb.md**: See "THEME CHANGE FIX IMPLEMENTATION" section for complete solution
- **Problem**: EQs not responding to theme changes due to LookAndFeel pointer pinning
- **Solution**: Remove LNF injection, eliminate color caching, use dynamic color querying
- **Result**: EQs now properly respond to theme changes with clean architecture

**Key Principles for Delay EQs:**
1. **No LookAndFeel Injection**: Don't pass `&getLookAndFeel()` to EQ constructors
2. **No Color Caching**: Don't store colors in member variables
3. **Dynamic Querying**: Query colors fresh in `paint()` methods using `lf.findColour()`
4. **Theme Integration**: Use `FieldLNF` color IDs for consistent theming

**Implementation Pattern:**
```cpp
// EQ Constructor - NO LNF parameter
DelayToneEQ::DelayToneEQ(MyPluginAudioProcessor& p) : proc(p) { }

// Paint Method - Dynamic color querying
void DelayToneEQ::paint(juce::Graphics& g) {
    auto& lf = getLookAndFeel();
    auto accent = lf.findColour(FieldLNF::eqLabelTextColourId);
    auto border = lf.findColour(FieldLNF::eqBorderColourId);
    // ... use colors dynamically
}
```

**Files to Reference:**
- `Source/features/reverb/Reverb.md` - Complete theme fix implementation
- `Source/features/reverb/ReverbToneEQ.h/.cpp` - Example implementation
- `Source/features/reverb/DecayRateEQ.h/.cpp` - Example implementation
- `Source/shared/Core/FieldLookAndFeel.h` - Color ID definitions

**Build Status:**
- **✅ Theme Compliance**: Reverb EQs now properly respond to theme changes
- **✅ Clean Architecture**: No LNF injection, no color caching
- **✅ Dynamic Colors**: All colors queried fresh on every paint call
- **✅ Performance**: No performance impact from dynamic color querying