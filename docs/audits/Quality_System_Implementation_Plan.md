# Quality System Implementation Plan - Gold Clip Parity

## 🎯 **IMPLEMENTATION ROADMAP**

### **Phase 1: SR-Aware Oversampling (IMMEDIATE)**

#### **1.1 Update DspRuntimeConfig.h**
```cpp
// Add to DspRuntimeConfig struct
struct DspRuntimeConfig {
    // ... existing members ...
    double sampleRate = 48000.0;
    int osRealtime = 0;    // 0=Auto, 1-5=Off,2x,4x,8x,16x
    int osOffline = 1;     // 0=Auto, 1-5=Off,2x,4x,8x,16x
    int osFilterType = 0;  // 0=Linear, 1=Minimum
    bool tpSafe = true;    // True-peak safe mode
    
    // SR-aware resolver
    static int resolveOSFactor(double sr, int qualityTier) {
        const bool loSR = (sr <= 48000.0);
        const bool midSR = (sr > 48000.0 && sr <= 96000.0);
        
        switch (qualityTier) {
            case 0: // Eco/High
                if (loSR)  return 4;  // 4× @ 44.1/48
                if (midSR) return 2;  // 2× @ 88.2/96
                return 1;             // 1× @ 192+
            case 1: // Standard/Pristine
                if (loSR)  return 8;  // 8× @ 44.1/48
                if (midSR) return 4;  // 4× @ 88.2/96
                return 2;             // 2× @ 192+
            case 2: // High/Extra Pristine
                if (loSR)  return 16; // 16× @ 44.1/48
                if (midSR) return 8;  // 8× @ 88.2/96
                return 4;             // 4× @ 192+
            default: return 1;
        }
    }
    
    int getActiveOSFactor() const {
        int targetOS = isNonRealtime() ? osOffline : osRealtime;
        if (targetOS == 0) { // Auto by Quality
            return resolveOSFactor(sampleRate, quality);
        }
        return targetOS; // Manual override
    }
};
```

#### **1.2 Update Parameter Layout**
```cpp
// Add to createParameterLayout() in PluginProcessor.cpp
// Realtime/Offline OS parameters
params.push_back(std::make_unique<juce::AudioParameterChoice>(
    juce::ParameterID{ IDs::osRealtime, 1 }, 
    "Oversampling Realtime", 
    juce::StringArray{ "Auto by Quality", "Off", "2x", "4x", "8x", "16x" }, 0));

params.push_back(std::make_unique<juce::AudioParameterChoice>(
    juce::ParameterID{ IDs::osOffline, 1 }, 
    "Oversampling Offline", 
    juce::StringArray{ "Auto by Quality", "Off", "2x", "4x", "8x", "16x" }, 1));

// OS Filter Type
params.push_back(std::make_unique<juce::AudioParameterChoice>(
    juce::ParameterID{ IDs::osFilterType, 1 }, 
    "Oversampling Type", 
    juce::StringArray{ "Linear Phase", "Minimum Phase" }, 0));

// True-Peak Safe
params.push_back(std::make_unique<juce::AudioParameterBool>(
    juce::ParameterID{ IDs::tpSafe, 1 }, 
    "True-Peak Safe", true));
```

#### **1.3 Update Parameter IDs**
```cpp
// Add to IDs namespace in PluginProcessor.h
namespace IDs {
    // ... existing IDs ...
    static constexpr const char* osRealtime = "oversampling_realtime";
    static constexpr const char* osOffline = "oversampling_offline";
    static constexpr const char* osFilterType = "oversampling_filter_type";
    static constexpr const char* tpSafe = "true_peak_safe";
}
```

### **Phase 2: Enhanced Parameter Handlers**

#### **2.1 Update onQualityChanged()**
```cpp
void MyPluginAudioProcessor::onQualityChanged(int quality)
{
    auto cfg = rtCfg.load();
    cfg.quality = juce::jlimit(0, 2, quality);
    cfg.sampleRate = getSampleRate();
    
    // Apply SR-aware OS if using Auto mode
    if (cfg.osRealtime == 0) { // Auto by Quality
        cfg.os = DspRuntimeConfig::resolveOSFactor(cfg.sampleRate, cfg.quality);
    }
    if (cfg.osOffline == 0) { // Auto by Quality
        cfg.os = DspRuntimeConfig::resolveOSFactor(cfg.sampleRate, cfg.quality);
    }
    
    if (!cfg.userOverrodePhase) cfg.phase = DspRuntimeConfig::kQMap[cfg.quality].phase;
    
    scheduleDspRebuildIfNeeded(cfg);
    rtCfg.store(cfg);
}
```

#### **2.2 Add New Parameter Handlers**
```cpp
// Add to parameterChanged()
else if (parameterID == IDs::osRealtime)
{
    onOSRealtimeChanged(static_cast<int>(newValue * 5.0f));
}
else if (parameterID == IDs::osOffline)
{
    onOSOfflineChanged(static_cast<int>(newValue * 5.0f));
}
else if (parameterID == IDs::osFilterType)
{
    onOSFilterTypeChanged(static_cast<int>(newValue));
}
else if (parameterID == IDs::tpSafe)
{
    onTPSafeChanged(newValue > 0.5f);
}

// Add method declarations to header
void onOSRealtimeChanged(int os);
void onOSOfflineChanged(int os);
void onOSFilterTypeChanged(int type);
void onTPSafeChanged(bool enabled);
```

#### **2.3 Implement New Handlers**
```cpp
void MyPluginAudioProcessor::onOSRealtimeChanged(int os)
{
    auto cfg = rtCfg.load();
    cfg.osRealtime = juce::jlimit(0, 5, os);
    cfg.userOverrodeOS = true;
    scheduleDspRebuildIfNeeded(cfg);
    rtCfg.store(cfg);
}

void MyPluginAudioProcessor::onOSOfflineChanged(int os)
{
    auto cfg = rtCfg.load();
    cfg.osOffline = juce::jlimit(0, 5, os);
    cfg.userOverrodeOS = true;
    scheduleDspRebuildIfNeeded(cfg);
    rtCfg.store(cfg);
}

void MyPluginAudioProcessor::onOSFilterTypeChanged(int type)
{
    auto cfg = rtCfg.load();
    cfg.osFilterType = juce::jlimit(0, 1, type);
    scheduleDspRebuildIfNeeded(cfg);
    rtCfg.store(cfg);
}

void MyPluginAudioProcessor::onTPSafeChanged(bool enabled)
{
    auto cfg = rtCfg.load();
    cfg.tpSafe = enabled;
    scheduleDspRebuildIfNeeded(cfg);
    rtCfg.store(cfg);
}
```

### **Phase 3: Enhanced DSP Rebuild**

#### **3.1 Update rebuildDspForConfig()**
```cpp
template <typename Sample>
void MyPluginAudioProcessor::rebuildDspForConfig(const DspRuntimeConfig& cfg, juce::AudioBuffer<Sample>& buffer)
{
    // 1) Get active OS factor (SR-aware + realtime/offline)
    const int factor = cfg.getActiveOSFactor();
    
    // 2) Get filter type
    auto filterType = (cfg.osFilterType == 0) 
        ? juce::dsp::Oversampling<Sample>::filterHalfBandFIR
        : juce::dsp::Oversampling<Sample>::filterHalfBandPolyphaseIIR;
    
    // 3) Calculate latency
    int latencySamples = 0;
    
    if constexpr (std::is_same_v<Sample, float>)
    {
        if (factor == 1) 
        {
            osF.reset();
        }
        else 
        {
            const int stages = juce::roundToInt(std::log2(factor));
            osF = std::make_unique<juce::dsp::Oversampling<float>>(
                juce::jmin(2, buffer.getNumChannels()), stages,
                filterType, true, true);
            osF->reset();
        }
        phaseBanksF.prepare(getSampleRate() * factor, getBlockSize() * factor, buffer.getNumChannels(), cfg.phase);
        latencySamples = (cfg.os ? osLatencySamples(factor) : 0) + phaseBanksF.latencyFor(cfg.phase);
    }
    else
    {
        if (factor == 1) 
        {
            osD.reset();
        }
        else 
        {
            const int stages = juce::roundToInt(std::log2(factor));
            osD = std::make_unique<juce::dsp::Oversampling<double>>(
                juce::jmin(2, buffer.getNumChannels()), stages,
                filterType, true, true);
            osD->reset();
        }
        phaseBanksD.prepare(getSampleRate() * factor, getBlockSize() * factor, buffer.getNumChannels(), cfg.phase);
        latencySamples = (cfg.os ? osLatencySamples(factor) : 0) + phaseBanksD.latencyFor(cfg.phase);
    }
    
    // 4) Report latency to host
    setLatencySamples(latencySamples);
    
    // 5) Create updated config with latency and commit
    DspRuntimeConfig updatedCfg = cfg;
    updatedCfg.latencySamples = latencySamples;
    rtCfg.store(updatedCfg, std::memory_order_release);
    
    // 6) Start topology crossfade
    startTopologyCrossfadeMs(15.0f);
}
```

### **Phase 4: True-Peak Anti-Overshoot**

#### **4.1 TruePeakGuard Implementation**
```cpp
// Add to PhaseBanks.h or new file
struct TruePeakGuard {
    float peakHold = 0.0f;
    float smoothedGain = 1.0f;
    float releaseTime = 0.5f; // 0.5ms release
    float threshold = 0.95f;   // 0.95 dBFS threshold
    
    void process(juce::dsp::AudioBlock<float>& block) {
        if (!enabled) return;
        
        // 1. Detect peaks in oversampled domain
        float currentPeak = 0.0f;
        for (int ch = 0; ch < block.getNumChannels(); ++ch) {
            auto* data = block.getChannelPointer(ch);
            for (int i = 0; i < (int)block.getNumSamples(); ++i) {
                currentPeak = juce::jmax(currentPeak, std::abs(data[i]));
            }
        }
        
        // 2. Update peak hold with release
        peakHold = juce::jmax(peakHold * 0.999f, currentPeak);
        
        // 3. Apply gentle gain trim if needed
        if (peakHold > threshold) {
            smoothedGain = juce::jmin(smoothedGain, threshold / peakHold);
        } else {
            smoothedGain = juce::jmin(1.0f, smoothedGain * 1.001f); // Gentle recovery
        }
        
        // 4. Apply gain to block
        for (int ch = 0; ch < block.getNumChannels(); ++ch) {
            auto* data = block.getChannelPointer(ch);
            for (int i = 0; i < (int)block.getNumSamples(); ++i) {
                data[i] *= smoothedGain;
            }
        }
    }
    
    void reset() {
        peakHold = 0.0f;
        smoothedGain = 1.0f;
    }
    
    bool enabled = true;
};
```

#### **4.2 Integrate TP Guard**
```cpp
// Add to FieldChain or main processing
struct FieldChain {
    // ... existing members ...
    TruePeakGuard tpGuard;
    
    void process(Block block) {
        // ... existing processing ...
        
        // Apply TP guard if enabled and oversampling active
        if (cfg.tpSafe && oversampling) {
            tpGuard.process(block);
        }
    }
};
```

### **Phase 5: UI Integration**

#### **5.1 Quality Button Enhancement**
```cpp
// In ButtonManager or QualityButton
void updateQualityButton() {
    auto cfg = proc.rtCfg.load();
    bool hasOverrides = cfg.userOverrodeOS || cfg.userOverrodePhase;
    
    // Build status string
    juce::String status = "";
    if (cfg.os > 0) {
        status += juce::String(cfg.os) + "×";
        status += (cfg.osFilterType == 0) ? " Linear" : " Min";
        if (cfg.tpSafe) status += " • TP Safe";
    }
    
    qualityButton.setSuffix(hasOverrides ? " • manual" : "");
    qualityButton.setTooltip("Quality: " + getQualityName(cfg.quality) + 
                           (status.isNotEmpty() ? "\nActive: " + status : ""));
}
```

#### **5.2 Parameter Display Sync**
```cpp
// In EventManager or UI update
void syncParameterDisplays() {
    auto cfg = proc.rtCfg.load();
    
    // Update OS displays
    osRealtimeCombo.setSelectedId(cfg.osRealtime + 1, juce::dontSendNotification);
    osOfflineCombo.setSelectedId(cfg.osOffline + 1, juce::dontSendNotification);
    
    // Update filter type
    osFilterTypeCombo.setSelectedId(cfg.osFilterType + 1, juce::dontSendNotification);
    
    // Update TP Safe
    tpSafeButton.setToggleState(cfg.tpSafe, juce::dontSendNotification);
}
```

---

## 🧪 **TESTING CHECKLIST**

### **SR-Aware Testing**
- [ ] 44.1kHz + Eco → 4× OS
- [ ] 44.1kHz + Standard → 8× OS  
- [ ] 44.1kHz + High → 16× OS
- [ ] 96kHz + Eco → 2× OS
- [ ] 96kHz + Standard → 4× OS
- [ ] 96kHz + High → 8× OS
- [ ] 192kHz + Eco → 1× OS
- [ ] 192kHz + Standard → 2× OS
- [ ] 192kHz + High → 4× OS

### **Realtime/Offline Testing**
- [ ] Realtime mode uses `osRealtime` setting
- [ ] Offline mode uses `osOffline` setting
- [ ] Auto by Quality respects SR-aware resolver
- [ ] Manual override preserved

### **Filter Type Testing**
- [ ] Linear Phase uses FIR filters
- [ ] Minimum Phase uses IIR filters
- [ ] Switch during playback: No clicks
- [ ] Latency reporting accurate

### **TP-Safe Testing**
- [ ] Drive signal to 0dBFS
- [ ] Verify no overshoot after downsample
- [ ] Test with different OS rates
- [ ] Verify transparency when not needed

---

## 🎯 **SUCCESS METRICS**

### **Feature Parity with Gold Clip**
- ✅ **SR-aware oversampling tiers** (4×@44.1, 2×@96, 1×@192)
- ✅ **Separate realtime/offline** control
- ✅ **Linear/Minimum phase OS** selectable
- ✅ **True-peak safe** downsampling
- ✅ **No clicks** during topology changes

### **Beyond Gold Clip**
- ✅ **Zero/Hybrid/FullLinear** processing modes
- ✅ **Manual override** protection
- ✅ **Quality macro** with smart defaults
- ✅ **Reverb-specific** features (ducking, motion, visuals)

This implementation plan provides a clear roadmap to achieve feature parity with Gold Clip while maintaining our unique reverb and motion features.
