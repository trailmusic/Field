# Field Audio Plugin - Master Development Guide

## 🎯 Overview
This guide documents critical architectural decisions, debugging processes, and solutions discovered during the development of the Field audio plugin. This knowledge base prevents repeating complex debugging sessions and ensures consistent development practices.

---

## 🚨 Critical Crash Prevention Knowledge

### **Plugin Close Crash - The Ultimate Solution**

**Problem**: Plugin crashes when closing in DAW (Ableton Live, etc.) due to improper component destruction order and parameter attachment cleanup.

**Root Cause**: JUCE parameter attachments (`SliderAttachment`, `ButtonAttachment`, `ComboBoxAttachment`) were not being properly cleared before component destruction, causing use-after-free crashes.

**The Complete Fix**:

#### 1. **Always Add Destructors to Control Panes**
Every control pane that creates parameter attachments MUST have a destructor:

```cpp
// ✅ CORRECT - Always do this
class MyControlsPane : public juce::Component
{
public:
    ~MyControlsPane() override
    {
        // Clear parameter attachments before destruction to prevent crashes
        btnAtts.clear();
        cmbAtts.clear();
        sAtts.clear();
    }
    
private:
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> cmbAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
};
```

#### 2. **Always Add Destructors to Tab Components**
Every tab component that contains control panes MUST have a destructor with crash logging:

```cpp
// ✅ CORRECT - Always do this
class MyTab : public juce::Component
{
public:
    ~MyTab() override
    {
        // Add crash logging for debugging
        juce::File f = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Field_CrashLog.txt");
        f.appendText("MyTab Destructor: STARTED\n", false, false, "\n");
        
        // Destroy controls first
        controls.reset();
        f.appendText("MyTab Destructor: Controls destroyed\n", false, false, "\n");
        
        // Destroy other components
        otherComponent.reset();
        f.appendText("MyTab Destructor: Other component destroyed\n", false, false, "\n");
        
        f.appendText("MyTab Destructor: COMPLETE\n", false, false, "\n");
    }
    
private:
    std::unique_ptr<MyControlsPane> controls;
    std::unique_ptr<OtherComponent> otherComponent;
};
```

#### 3. **Timer-Based Components Must Stop Timers**
Any component that uses `juce::Timer` MUST call `stopTimer()` in its destructor:

```cpp
// ✅ CORRECT - Always do this
class MyTimerComponent : public juce::Component, private juce::Timer
{
public:
    ~MyTimerComponent() override
    {
        // Stop timer before destruction to prevent use-after-free
        stopTimer();
    }
};
```

#### 4. **AsyncUpdater Must Be Cancelled**
Any component using `juce::AsyncUpdater` MUST call `cancelPendingUpdate()` in destructors:

```cpp
// ✅ CORRECT - Always do this
class MyAsyncComponent : public juce::Component, private juce::AsyncUpdater
{
public:
    ~MyAsyncComponent() override
    {
        // Cancel AsyncUpdater to prevent use-after-free
        cancelPendingUpdate();
    }
};
```

---

## 🏗️ Architecture Patterns

### **Engine Initialization Pattern**

**Rule**: Engines should only be initialized when their respective enable parameters are true.

```cpp
// ✅ CORRECT - Conditional engine initialization
void MyPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Basic setup
    // ...
    
    // Conditional engine preparation
    if (params.motionEnabled)
    {
        motionEngine.prepare(sampleRate, samplesPerBlock);
    }
    
    if (params.reverbEnabled)
    {
        reverbEngine.prepare(sampleRate, samplesPerBlock);
    }
    
    if (params.delayEnabled)
    {
        delayEngine.prepare(sampleRate, samplesPerBlock);
    }
}
```

### **Parameter Attachment Safety Pattern**

**Rule**: Always check if parameters exist before creating attachments.

```cpp
// ✅ CORRECT - Safe parameter attachment
auto makeCell = [&](juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid)
{
    // Safety check: ensure parameter exists before creating attachment
    if (pid == nullptr || apvts.getParameter(juce::String(pid)) == nullptr)
    {
        // Skip this cell if parameter doesn't exist
        return;
    }
    
    // Create attachment only if parameter exists
    sAtts.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, pid, s));
};
```

### **Parameter ID Consistency Pattern**

**Rule**: Always use the `IDs::` namespace for parameter IDs, never string literals.

```cpp
// ✅ CORRECT - Use namespace constants
new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, IDs::delayTime, delayTimeSlider);

// ❌ WRONG - Don't use string literals
new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, "delay_time", delayTimeSlider);
```

---

## 🔧 Debugging Systems

### **Crash Logging System**

**Location**: `~/Documents/Field_CrashLog.txt`

**Usage**: Add crash logging to critical destructors to track destruction order:

```cpp
void MyComponent::~MyComponent()
{
    juce::File f = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Field_CrashLog.txt");
    f.appendText("MyComponent Destructor: STARTED\n", false, false, "\n");
    
    // Component-specific cleanup
    // ...
    
    f.appendText("MyComponent Destructor: COMPLETE\n", false, false, "\n");
}
```

**When to Use**:
- Plugin close crashes
- Component destruction issues
- Parameter attachment problems
- Timer-related crashes

### **Build Verification System**

**Script**: `./build_all.sh`

**Usage**: Always run after making changes to ensure all targets build successfully.

**Targets**:
- Standalone: `Field.app`
- AU Plugin: `Field.component`
- VST3 Plugin: `Field.vst3`

---

## 🚫 Common Pitfalls to Avoid

### **1. Missing Parameter Definitions**
**Problem**: UI components reference parameters that don't exist in `createParameterLayout()`.

**Solution**: Always verify all parameter IDs are defined in `createParameterLayout()`.

### **2. Parameter ID Mismatches**
**Problem**: UI uses string literals while parameters are defined with `IDs::` namespace.

**Solution**: Always use `IDs::` namespace constants consistently.

### **3. Missing Destructors**
**Problem**: Components with parameter attachments crash on destruction.

**Solution**: Always add destructors to clear parameter attachments.

### **4. Timer Not Stopped**
**Problem**: Timer-based components crash when destroyed.

**Solution**: Always call `stopTimer()` in destructors.

### **5. AsyncUpdater Not Cancelled**
**Problem**: AsyncUpdater continues after component destruction.

**Solution**: Always call `cancelPendingUpdate()` in destructors.

---

## 📋 Development Checklist

### **Before Adding New UI Components**

- [ ] Does the component create parameter attachments?
  - [ ] Add destructor to clear attachments
- [ ] Does the component use `juce::Timer`?
  - [ ] Add `stopTimer()` to destructor
- [ ] Does the component use `juce::AsyncUpdater`?
  - [ ] Add `cancelPendingUpdate()` to destructor
- [ ] Are all parameter IDs defined in `createParameterLayout()`?
- [ ] Are parameter IDs using `IDs::` namespace consistently?

### **Before Committing Changes**

- [ ] Run `./build_all.sh` - all targets must build successfully
- [ ] Test plugin load in DAW (Ableton Live)
- [ ] Test plugin close in DAW - must not crash
- [ ] Test all major UI interactions
- [ ] Verify crash log is clean (no infinite loops)

### **When Debugging Crashes**

- [ ] Check crash log at `~/Documents/Field_CrashLog.txt`
- [ ] Look for incomplete destructor sequences
- [ ] Verify parameter attachments are cleared
- [ ] Check for timer/AsyncUpdater issues
- [ ] Test with minimal component set to isolate issue

---

## 🎯 Engine Architecture

### **Custom vs JUCE DSP Components**

**Custom Engines** (use custom math):
- `DelayEngine` - Custom delay line with dual-reader crossfade
- `MotionEngine` - Custom envelope follower and filter algorithms
- `ReverbEngine` - Custom reverb algorithms

**Direct JUCE DSP** (use built-in components):
- `juce::dsp::LinkwitzRileyFilter` - Standard crossover filters
- `juce::dsp::IIR::Filter` - Standard IIR filters
- `juce::dsp::StateVariableTPTFilter` - Standard state variable filters
- `juce::dsp::Reverb` - Standard JUCE reverb

**Rule**: Custom engines provide unique sound characteristics, while JUCE DSP provides standard, reliable processing.

---

## 🔄 Git Workflow

### **Branch Management**
- `main` - Stable releases
- `ui-refactor-clean` - Active development branch

### **Commit Messages**
Use descriptive commit messages that explain the fix:

```
Fix plugin crash on close - comprehensive destructor fixes

- Added DelayTab destructor with crash logging
- Added DelayControlsPane destructor to clear parameter attachments
- Added ReverbTab destructor with crash logging  
- Added ReverbControlsPane2x16 destructor to clear parameter attachments
- Fixed parameter attachment cleanup to prevent use-after-free crashes
- Enhanced crash logging system for debugging destruction order
```

---

## 📚 Key Files Reference

### **Core Architecture**
- `Source/PluginProcessor.h` - Main processor class and FieldChain template
- `Source/PluginProcessor.cpp` - Audio processing and parameter layout
- `Source/PluginEditor.h` - UI component definitions
- `Source/PluginEditor.cpp` - UI implementation and parameter attachments

### **UI Components**
- `Source/ui/PaneManager.h` - Tab management system
- `Source/ui/delay/DelayTab.h` - Delay tab implementation
- `Source/ui/delay/DelayControlsPane.h` - Delay controls
- `Source/reverb/ui/ReverbTab.h` - Reverb tab implementation
- `Source/reverb/ui/ReverbControlsPane2x16.h` - Reverb controls

### **Build System**
- `build_all.sh` - Build script for all targets
- `verify_builds.sh` - Build verification script
- `CMakeLists.txt` - CMake configuration

---

## 🎉 Success Metrics

### **Plugin Stability Indicators**
- ✅ Plugin loads without crashing in DAW
- ✅ Plugin closes without crashing in DAW
- ✅ All UI controls are visible and functional
- ✅ Parameter changes affect audio processing
- ✅ No infinite paint loops or UI freezes
- ✅ Clean crash log (no error messages)

### **Development Efficiency Indicators**
- ✅ Build completes successfully in < 2 minutes
- ✅ All three targets (Standalone, AU, VST3) build
- ✅ Changes can be tested immediately
- ✅ Debugging information is readily available
- ✅ Architecture is consistent and maintainable

---

## 🚀 Future Development Guidelines

### **Adding New Features**
1. Follow the parameter attachment safety pattern
2. Add proper destructors to all new components
3. Use `IDs::` namespace for all parameter IDs
4. Test thoroughly in DAW environment
5. Document any new architectural patterns

### **Performance Optimization**
1. Profile audio processing performance
2. Optimize UI update frequencies
3. Minimize parameter attachment overhead
4. Use efficient destruction patterns

### **Maintenance**
1. Regularly review crash logs
2. Update this guide with new discoveries
3. Keep build system up to date
4. Monitor for new JUCE best practices

---

*This guide was created after a comprehensive debugging session that resolved critical plugin crash issues. It represents hard-won knowledge that should be preserved and followed to prevent similar issues in the future.*

**Last Updated**: December 2024  
**Version**: 1.0  
**Status**: Production Ready
