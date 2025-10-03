# Field Debug Troubleshoot Playbook

## 🎯 **QUICK DIAGNOSTIC CHECKLIST**

### **Audio Glitch Isolation Protocol**
1. **Component Toggle Testing** - Systematic bypass approach
2. **Buffer Reset Verification** - Ensure clean state transitions
3. **Audio Path Validation** - Confirm signal flow integrity
4. **Performance Monitoring** - Track CPU and memory usage

---

## 🔧 **COMPONENT TOGGLE SYSTEM**

### **Toggle Locations**
```cpp
// Source/shared/Core/PluginProcessor.cpp - Lines ~100-110
static bool bypassWidth = false;      // Toggle 1: Bypass width/MS processing
static bool bypassReverb = false;    // Toggle 2: Bypass reverb processing
static bool bypassDelay = false;     // Toggle 3: Bypass delay processing
static bool bypassTone = false;      // Toggle 4: Bypass tone processing
static bool bypassOversampling = false; // Toggle 5: Bypass oversampling
```

### **Toggle Testing Protocol**
1. **Test One Component at a Time**
2. **Build and Test in Ableton**
3. **Document Results**
4. **Revert if Audio Breaks**

---

## 🎵 **AUDIO PATH DIAGNOSTICS**

### **Critical Files for Audio Glitch Investigation**
- `Source/shared/Core/PluginProcessor.cpp` ⭐ **MAIN AUDIO PROCESSOR**
- `Source/shared/Core/PluginProcessor.h` ⭐ **MAIN AUDIO PROCESSOR HEADER**
- `Source/features/reverb/Core/ReverbEngine.cpp` ⭐ **REVERB ENGINE**
- `Source/shared/dsp/DelayEngine.h` ⭐ **DELAY ENGINE**

### **Audio Processing Chain**
```
Input → Width/MS → Reverb → Delay → Tone → Oversampling → Output
```

### **Buffer Reset Requirements**
```cpp
// FieldChain::reset() must include:
delayLineL.reset();
delayLineR.reset();
delayPrepared = false;
```

---

## 🚨 **COMMON ISSUES & SOLUTIONS**

### **Issue: No Audio Output**
**Symptoms**: Plugin receives input but produces silence
**Causes**:
- Reverb bypass breaking audio path
- Width/MS bypass breaking audio path
- Oversampling bypass breaking audio path

**Solution**: Ensure all component toggles are `false` for normal operation

### **Issue: Persistent Popping After Stop**
**Symptoms**: Audio continues to fade out after transport stops
**Causes**: Delay buffers not being reset
**Solution**: Add delay buffer resets to `FieldChain::reset()`

### **Issue: Audio Glitches During Playback**
**Symptoms**: Crackling, popping, or distortion during playback
**Causes**: Component-specific processing issues
**Solution**: Use component toggles to isolate the problematic component

---

## 🔍 **DEBUGGING WORKFLOW**

### **Step 1: Verify Audio Path**
```bash
# Build and test with all components enabled
make -j4 -C build
# Test in Ableton - should have audio with glitches
```

### **Step 2: Isolate Glitch Source**
```cpp
// Test each component systematically:
// 1. Set bypassWidth = true, test
// 2. Set bypassReverb = true, test  
// 3. Set bypassDelay = true, test
// 4. Set bypassTone = true, test
// 5. Set bypassOversampling = true, test
```

### **Step 3: Document Results**
- Which toggle fixes the glitches?
- Which toggles break audio completely?
- What are the exact symptoms?

### **Step 4: Implement Fix**
- If a component is the source, investigate that component
- If no component fixes it, investigate the audio chain itself

---

## 📊 **PERFORMANCE MONITORING**

### **CPU Usage Indicators**
- High CPU during bypass = Component not properly bypassed
- CPU spikes during processing = Buffer size issues
- Consistent high CPU = Inefficient algorithms

### **Memory Usage Indicators**
- Memory leaks = Improper cleanup in `releaseResources()`
- Buffer overruns = Incorrect buffer size calculations
- Persistent audio = Unreset buffers

---

## 🛠️ **BUILD & TEST COMMANDS**

### **Build Commands**
```bash
# Full build
make -j4 -C build

# Clean build
rm -rf build && mkdir build && cd build && cmake .. && make -j4

# Test build
./build_and_test.sh
```

### **Test Locations**
- **Standalone**: `build/Source/Field_artefacts/Standalone/Field.app`
- **AU**: `build/Source/Field_artefacts/AU/Field.component`
- **VST3**: `build/Source/Field_artefacts/VST3/Field.vst3`

---

## 📝 **REPORTING TEMPLATE**

### **Test Results Format**
```
Component: [Component Name]
Toggle: bypass[Component] = [true/false]
Result: [Audio Working/No Audio/Glitches Fixed/Glitches Persist]
Symptoms: [Detailed description]
Build Status: [Success/Failure]
```

### **Example Report**
```
Component: Reverb
Toggle: bypassReverb = true
Result: No Audio
Symptoms: Complete silence, no signal processing
Build Status: Success
Notes: Reverb bypass breaks entire audio path
```

---

## 🔄 **RECOVERY PROCEDURES**

### **If Audio Completely Breaks**
1. **Revert all toggles to `false`**
2. **Clean build**: `rm -rf build && mkdir build && cd build && cmake .. && make -j4`
3. **Test in Ableton**
4. **If still broken, check `releaseResources()` implementation**

### **If Build Fails**
1. **Check for syntax errors in modified files**
2. **Verify all includes are correct**
3. **Clean build directory**
4. **Check CMake configuration**

---

## 🎯 **SUCCESS CRITERIA**

### **Audio Working**
- ✅ Plugin processes audio without glitches
- ✅ No persistent popping after stop
- ✅ Clean audio path through all components
- ✅ Proper cleanup when plugin stops

### **Build Success**
- ✅ All targets compile (Standalone, AU, VST3)
- ✅ No compilation errors
- ✅ No linker errors
- ✅ Proper installation

---

## 📚 **REFERENCE MATERIALS**

### **Key Documentation**
- `docs/audits/Audio_Glitch_Fix_Audit.md` - Current audit status
- `Source/features/reverb/ReverbDocs/` - Reverb system documentation
- `Source/shared/dsp/` - DSP engine documentation

### **Critical Code Sections**
- `PluginProcessor::processBlock()` - Main audio processing
- `FieldChain::process()` - Audio chain processing
- `FieldChain::reset()` - State reset functionality
- `releaseResources()` - Cleanup when plugin stops

---

*Generated: January 2025*  
*Playbook ID: FIELD-DEBUG-PLAYBOOK-001*  
*Status: ACTIVE - Use for systematic audio glitch debugging*
