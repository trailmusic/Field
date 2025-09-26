# FIELD PHASE ALIGNMENT SYSTEM
## Build Plan & Implementation Checklist

### 🎯 **Overview**
The Phase Alignment System provides comprehensive time/phase alignment capabilities for Field, integrating seamlessly with existing crossovers, meters, and routing while avoiding redundancy with existing features.

### 📋 **Build Checklist**

#### **Phase 1: Parameter System Integration**
- [ ] **1.1** Add all 32 new parameter IDs to PluginProcessor.h IDs struct
- [ ] **1.2** Update createParameterLayout() with new parameters
- [ ] **1.3** Add parameter state variables to PluginProcessor class
- [ ] **1.4** Update parameter initialization in constructor
- [ ] **1.5** Remove old phase parameters (phaseMode, centerPhaseRecOn, centerPhaseAmt01)
- [ ] **1.6** Add new parameter IDs to parameter validation

#### **Phase 2: DSP Engine Implementation**
- [ ] **2.1** Create PhaseAlignmentEngine class
- [ ] **2.2** Implement Farrow fractional delay (4-tap cubic)
- [ ] **2.3** Implement All-Pass filters (AP1/AP2) with coefficient mapping
- [ ] **2.4** Implement FIR phase matching for Studio mode
- [ ] **2.5** Implement GCC-PHAT coarse alignment algorithm
- [ ] **2.6** Implement sub-sample refinement via parabolic peak
- [ ] **2.7** Implement Dynamic Phase (transient-aware reduction)
- [ ] **2.8** Implement Audition Blend (50/50 parallel processing)

#### **Phase 3: UI Components**
- [ ] **3.1** Create PhaseAlignmentVisuals class (wide strip)
- [ ] **3.2** Implement dual-lane waveform display
- [ ] **3.3** Implement draggable transient anchors
- [ ] **3.4** Implement phase-vs-frequency unwrap display
- [ ] **3.5** Implement comb predictor visualization
- [ ] **3.6** Implement coherence mini-heatmap
- [ ] **3.7** Create 2×16 control grid using KnobCells
- [ ] **3.8** Implement ComboBoxes for enum parameters
- [ ] **3.9** Implement Switches for boolean parameters

#### **Phase 4: Integration & Wiring**
- [ ] **4.1** Integrate PhaseAlignmentEngine into PluginProcessor
- [ ] **4.2** Wire parameter attachments in PhaseTab
- [ ] **4.3** Implement crossover following (Band tab integration)
- [ ] **4.4** Implement meter integration (reuse existing analyzers)
- [ ] **4.5** Implement Master Mix guardrails
- [ ] **4.6** Implement engine switching (Live ↔ Studio)
- [ ] **4.7** Implement latency readout calculation

#### **Phase 5: Theme & Visual Integration**
- [ ] **5.1** Apply FieldLNF theme colors to all components
- [ ] **5.2** Implement standard accent border treatment
- [ ] **5.3** Apply consistent typography and spacing
- [ ] **5.4** Implement tooltip system
- [ ] **5.5** Implement parameter value displays
- [ ] **5.6** Implement visual feedback for automation

#### **Phase 6: Testing & Validation**
- [ ] **6.1** Unit tests for DSP algorithms
- [ ] **6.2** Integration tests with existing systems
- [ ] **6.3** Performance testing (CPU usage)
- [ ] **6.4** Visual consistency testing
- [ ] **6.5** Preset compatibility testing
- [ ] **6.6** Automation testing

---

## 🏗️ **Implementation Details**

### **Parameter System (32 Parameters)**

#### **Global/Routing (3 params)**
```cpp
phase_ref_source    // enum: AtoB, BtoA
phase_channel_mode  // enum: Stereo, MS, DualMono  
phase_follow_xo     // bool: Off, On
```

#### **Capture/Align (3 params)**
```cpp
phase_capture_len   // enum: 2s, 5s
phase_align_mode    // enum: Manual, Semi, Auto
phase_align_goal    // enum: MonoPunch, BassTight, StereoFocus
```

#### **Polarity/Delay (5 params)**
```cpp
phase_polarity_a    // bool: Normal, Invert
phase_polarity_b    // bool: Normal, Invert
phase_delay_ms_coarse  // float: -20.00 to +20.00
phase_delay_ms_fine    // float: -1.000 to +1.000
phase_delay_units      // enum: ms, samples
phase_link_mode        // enum: Off, TimeOnly, AllBands
```

#### **Engine/Latency/Commands (4 params)**
```cpp
phase_engine       // enum: Live, Studio
phase_latency_ro   // readout: 0-200ms
phase_reset_cmd    // enum: Time, Phase, All
phase_commit_cmd   // button: Trigger
```

#### **Banding/Crossovers (2 params)**
```cpp
phase_xo_lo_hz     // float: 40-400
phase_xo_hi_hz     // float: 800-6000
```

#### **Per-Band All-Pass (6 params)**
```cpp
phase_lo_ap_deg    // float: 0-180
phase_lo_q         // float: 0.30-4.00
phase_mid_ap_deg   // float: 0-180
phase_mid_q        // float: 0.30-6.00
phase_hi_ap_deg    // float: 0-180
phase_hi_q         // float: 0.30-8.00
```

#### **FIR Phase Match (1 param)**
```cpp
phase_fir_len      // int: 64, 128, 256, 512, 1024, 2048, 4096
```

#### **Dynamic Phase (1 param)**
```cpp
phase_dynamic_mode // enum: Off, Light, Med, Hard
```

#### **Monitoring/Output (4 params)**
```cpp
phase_monitor_mode // enum: Stereo, MonoMinus6, Mid, Side, A, B
phase_metric_mode  // enum: Corr, Coherence, DeltaPhiRMS, MonoLFRMS
phase_audition_blend // enum: Apply100, Blend50
phase_trim_db      // float: -12 to +12
```

#### **Logging/Preset Behavior (2 params)**
```cpp
phase_rec_enable   // bool: Off, On
phase_apply_on_load // bool: Off, On
```

### **UI Layout (2×16 Grid)**

#### **Row 1 — Routing, Capture, Time, Safety**
1. **Ref Source** (ComboBox) — A→B | B→A
2. **Channel Mode** (ComboBox) — Stereo | M/S | Dual-Mono
3. **Follow Crossovers** (Switch) — On/Off
4. **Capture** (ComboBox) — 2s | 5s
5. **Align Mode** (ComboBox) — Manual | Semi | Auto
6. **Align Goal** (ComboBox) — Mono Punch | Bass Tight | Stereo Focus
7. **Polarity A** (Switch) — Invert
8. **Polarity B** (Switch) — Invert
9. **Delay Coarse** (KnobCell) — ±20.0 ms
10. **Delay Fine** (KnobCell) — ±1.00 ms
11. **Units** (ComboBox) — ms | samples
12. **Link** (ComboBox) — Off | Time Only | All Bands
13. **Engine** (ComboBox) — Live | Studio
14. **Latency** (Label) — readout
15. **Reset** (ComboBox) — Time | Phase | All
16. **Commit to Bands** (Button) — writes current τ/AP

#### **Row 2 — Per-Band Phase**
1. **XO Low Hz** (KnobCell) — 40–400
2. **XO High Hz** (KnobCell) — 800–6000
3. **LOW AP°** (KnobCell) — 0–180
4. **LOW Q** (KnobCell) — 0.3–4.0
5. **MID AP°** (KnobCell) — 0–180
6. **MID Q** (KnobCell) — 0.3–6.0
7. **HIGH AP°** (KnobCell) — 0–180
8. **HIGH Q** (KnobCell) — 0.3–8.0
9. **FIR Length** (ComboBox) — 64…4096
10. **Dynamic Phase** (ComboBox) — Off | Light | Med | Hard
11. **Monitor** (ComboBox) — Stereo | Mono −6 | Mid | Side | A | B
12. **Metric** (ComboBox) — Corr | Coherence | Δφ RMS | Mono LF
13. **Audition Blend** (ComboBox) — Apply100 | Blend50
14. **Trim** (KnobCell) — −12…+12 dB
15. **Phase Rec** (Switch) — On/Off
16. **Apply @ Load** (Switch) — On/Off

### **Wide Visual Strip Components**

#### **Dual-Lane Capture Display**
- **A Lane** (Reference) — waveform display with transient markers
- **B Lane** (Target) — waveform display with draggable anchors
- **Transient Anchors** (◇) — draggable for time-nudge
- **ALT-drag** — fractional micro-nudge
- **Double-click** — add/clear anchor

#### **Phase-vs-Frequency Unwrap**
- **X-axis** — frequency (log scale)
- **Y-axis** — phase difference (degrees)
- **AP Handles** — draggable for per-band phase goals
- **Raw Δφ** — before processing
- **Processed Δφ** — after AP/FIR

#### **Comb Predictor**
- **Barcode display** — expected cancellation frequencies
- **Real-time updates** — as delay/AP changes
- **Frequency markers** — highlight problem areas

#### **Coherence Mini-Heatmap**
- **Frequency × Time** — thumbnail display
- **Color coding** — coherence strength
- **Reuses existing analyzer taps** — no new meter types

### **DSP Architecture**

#### **Signal Flow**
```
Input → Phase Alignment Engine → Rest of Plugin Chain → Master Mix → Output
         ↓
    Local Dry Tap (time-aligned) → Audition Blend (50/50)
```

#### **Engine Modes**
- **Live Mode** — AP only, zero latency
- **Studio Mode** — AP + FIR, adds latency

#### **Key Algorithms**
- **Farrow Delay** — 4-tap cubic interpolation
- **All-Pass Filters** — AP1 (1st-order) + AP2 (biquad)
- **FIR Phase Match** — magnitude-1 design
- **GCC-PHAT** — coarse alignment
- **Parabolic Peak** — sub-sample refinement
- **Dynamic Phase** — transient-aware reduction

### **Integration Points**

#### **Existing Systems**
- **Band Tab Crossovers** — when Follow Crossovers = On
- **Global Meters** — correlation, coherence, L/R
- **Master Mix** — guardrails for Audition Blend
- **Quality/OS** — affects FIR length and latency
- **Preset System** — parameter persistence

#### **New Systems**
- **Phase Alignment Engine** — core DSP processing
- **Visual Display System** — wide strip components
- **Control Grid System** — 2×16 parameter layout
- **Audition Blend System** — internal parallel processing

### **Performance Considerations**

#### **CPU Optimization**
- **Farrow Delay** — SIMD-optimized 4-tap cubic
- **All-Pass Filters** — efficient coefficient mapping
- **FIR Processing** — partitioned FFT, overlap-add
- **Visual Updates** — 30Hz refresh rate
- **Parameter Smoothing** — click-free automation

#### **Memory Management**
- **Delay Lines** — efficient circular buffers
- **FIR Buffers** — partitioned for streaming
- **Visual Buffers** — reuse existing analyzer data
- **State Management** — efficient parameter storage

### **Testing Strategy**

#### **Unit Tests**
- **DSP Algorithms** — impulse response, frequency response
- **Parameter Validation** — range checking, enum validation
- **State Management** — preset save/load, automation
- **Performance** — CPU usage, memory allocation

#### **Integration Tests**
- **Crossover Following** — Band tab integration
- **Meter Integration** — existing analyzer reuse
- **Master Mix Guardrails** — Audition Blend behavior
- **Engine Switching** — Live ↔ Studio transitions

#### **Visual Tests**
- **Theme Consistency** — FieldLNF integration
- **Layout Responsiveness** — different window sizes
- **Animation Smoothness** — parameter changes
- **Tooltip Accuracy** — parameter descriptions

---

## 🚀 **Next Steps**

1. **Start with Parameter System** — Add all 32 parameters to PluginProcessor
2. **Create DSP Engine** — Implement core algorithms
3. **Build UI Components** — Wide strip + 2×16 grid
4. **Integrate Systems** — Wire everything together
5. **Test & Validate** — Comprehensive testing suite
6. **Documentation** — Update FIELD_MASTER_GUIDE.md

This system will provide Field with professional-grade phase alignment capabilities while maintaining the plugin's existing workflow and visual consistency.
