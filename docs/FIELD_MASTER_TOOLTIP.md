# FIELD MASTER TOOLTIP GUIDE
# ================================================================================
# 
# 📍 LOCATION: docs/FIELD_MASTER_TOOLTIP.md
# 🎯 PURPOSE: Comprehensive tooltip system for FIELD plugin controls
# 🔧 INTEGRATION: Works with FieldLookAndFeel tooltip system
# 📊 FEATURES: Context-aware tooltips, control explanations, parameter guidance
# 
# ================================================================================

## 🎛️ BAND CONTROL SYSTEM - Tooltips & Explanations

### **Control Type Tooltips**

#### **SHUF Controls (Shuffle Randomization)**

**SHUF LO** (Shuffle Low)
- **Tooltip**: "Shuffle amount for low frequencies (affects bass stereo spread)"
- **Range**: 0-200%
- **Default**: 100%
- **Purpose**: Adds controlled randomness to low-frequency stereo positioning
- **Use Case**: Prevents static bass imaging, adds subtle movement to kick/bass

**SHUF HI** (Shuffle High)  
- **Tooltip**: "Shuffle amount for high frequencies (affects treble stereo spread)"
- **Range**: 0-200%
- **Default**: 100%
- **Purpose**: Adds controlled randomness to high-frequency stereo positioning
- **Use Case**: Prevents static treble imaging, adds airy movement to cymbals/highs

**SHUF XO** (Shuffle Crossover)
- **Tooltip**: "Crossover frequency separating low vs high shuffle processing"
- **Range**: 150-2000 Hz
- **Default**: 700 Hz
- **Purpose**: Defines where LO vs HI shuffling applies
- **Use Case**: Fine-tune shuffle frequency split for optimal effect

#### **XO Controls (Crossover Boundaries)**

**XO LO** (Crossover Low)
- **Tooltip**: "Low crossover point separating bass from midrange (40-400 Hz)"
- **Range**: 40-400 Hz
- **Default**: 150 Hz
- **Purpose**: Creates frequency band boundaries for processing
- **Use Case**: Set bass/mid split point for independent width control

**XO HI** (Crossover High)
- **Tooltip**: "High crossover point separating midrange from treble (800-6000 Hz)"
- **Range**: 800-6000 Hz
- **Default**: 2000 Hz
- **Purpose**: Creates frequency band boundaries for processing
- **Use Case**: Set mid/treble split point for independent width control

#### **WIDTH Controls (Stereo Expansion)**

**WIDTH** (Master Width)
- **Tooltip**: "Master stereo width control affecting all frequency bands"
- **Range**: 0.5-2.0
- **Default**: 1.0
- **Purpose**: Global stereo width scaling
- **Use Case**: Overall stereo field expansion/contraction

**W LO** (Width Low)
- **Tooltip**: "Low frequency stereo width (bass stereo spread)"
- **Range**: 0-2.0
- **Default**: 1.0
- **Purpose**: Independent bass width control
- **Use Case**: Keep bass tight (≤1.0) or add subtle width

**W MID** (Width Mid)
- **Tooltip**: "Mid frequency stereo width (midrange stereo spread)"
- **Range**: 0-2.0
- **Default**: 1.0
- **Purpose**: Independent midrange width control
- **Use Case**: Add width to vocals, instruments, presence

**W HI** (Width High)
- **Tooltip**: "High frequency stereo width (treble stereo spread)"
- **Range**: 0-2.0
- **Default**: 1.1
- **Purpose**: Independent treble width control
- **Use Case**: Add air and sparkle to high frequencies

### **Control Hierarchy Tooltips**

#### **Typical Workflow**
1. **"Set XO boundaries first"** - "XO controls define frequency bands (foundation)"
2. **"Adjust width per band"** - "WIDTH controls expand/contract stereo field"
3. **"Add shuffle for movement"** - "SHUF controls prevent static positioning"

#### **Key Differences**
- **"SHUF = Randomization"** - "Adds controlled chaos to avoid static stereo"
- **"XO = Frequency boundaries"** - "Defines processing bands (LO/MID/HI)"
- **"WIDTH = Stereo expansion"** - "Makes things wider/narrower within bands"

### **Advanced Tooltips**

#### **Mono Safety**
- **"Keep bass tight"** - "W LO ≤ 1.0 prevents mono collapse on kick/bass"
- **"Watch correlation"** - "Low correlation = mono risk, high = safe"
- **"Use Mono Maker"** - "Collapse lows to mono below cutoff for safety"

#### **Creative Applications**
- **"Wide chorus effect"** - "W HI > 1.2 + SHUF HI for airy movement"
- **"Tight bass, wide top"** - "W LO 0.9, W HI 1.3 for modern sound"
- **"Subtle shuffle"** - "SHUF 90-110% for gentle movement without chaos"

#### **Troubleshooting**
- **"Static stereo"** - "Add SHUF for movement, check XO placement"
- **"Mono collapse"** - "Reduce W LO, increase Mono Maker cutoff"
- **"Harsh highs"** - "Reduce W HI, adjust XO HI placement"

### **Context-Aware Tooltips**

#### **Genre-Specific Guidance**
- **EDM/Electronic**: "Wide highs (W HI 1.2-1.4), tight bass (W LO 0.9-1.0)"
- **Rock/Alternative**: "Balanced width (W MID 1.1), subtle shuffle (SHUF 100-110%)"
- **Jazz/Acoustic**: "Conservative width (all ≤ 1.2), minimal shuffle"
- **Hip-Hop/Trap**: "Tight bass (W LO 0.8-0.9), wide mids (W MID 1.2-1.3)"

#### **Mix Position Guidance**
- **"Early in chain"** - "Set XO boundaries before other processing"
- **"After EQ"** - "Width controls work best post-EQ for clean separation"
- **"Before compression"** - "Shuffle before compression for natural movement"

### **Parameter Relationships**

#### **XO Dependencies**
- **"XO LO < XO HI"** - "Low crossover must be below high crossover"
- **"XO affects all"** - "Crossover changes affect all width and shuffle controls"
- **"Typical ranges"** - "LO: 80-200 Hz, HI: 1500-4000 Hz for most content"

#### **Width Interactions**
- **"Master overrides"** - "WIDTH scales all band widths proportionally"
- **"Band independence"** - "W LO/MID/HI work independently within bands"
- **"Sum to mono"** - "All width changes affect final stereo image"

#### **Shuffle Behavior**
- **"Frequency dependent"** - "SHUF LO affects bass, SHUF HI affects treble"
- **"XO determines split"** - "SHUF XO sets where LO vs HI shuffling applies"
- **"Additive effect"** - "Shuffle adds to existing width, doesn't replace it"

### **Visual Feedback Tooltips**

#### **Band Visual Indicators**
- **"LO MID HI labels"** - "Show current frequency band boundaries"
- **"Width visualization"** - "Real-time display of stereo field changes"
- **"XO markers"** - "Vertical lines show crossover frequencies"

#### **Meter Integration**
- **"Correlation meter"** - "Shows mono safety (green=safe, red=risk)"
- **"L/R meters"** - "Display channel levels and stereo balance"
- **"Width analysis"** - "Real-time stereo field measurement"

### **Performance Tooltips**

#### **CPU Considerations**
- **"XO changes"** - "Crossover changes require filter coefficient updates"
- **"Width processing"** - "Per-band width adds minimal CPU overhead"
- **"Shuffle randomization"** - "Controlled randomness, not CPU intensive"

#### **Latency Notes**
- **"Zero latency"** - "All Band controls operate in real-time"
- **"No lookahead"** - "Immediate parameter response for live use"
- **"Smooth transitions"** - "Parameter changes are smoothed to prevent clicks"

---

## 🎯 Tooltip System Integration

### **Implementation Notes**
- **Context Awareness**: Tooltips adapt based on current parameter values
- **Progressive Disclosure**: Basic info first, advanced details on hover
- **Visual Consistency**: All tooltips follow FIELD theme and styling
- **Performance**: Tooltips don't impact audio processing or UI performance

### **Future Enhancements**
- **Interactive Examples**: Audio examples for different control combinations
- **Preset Integration**: Tooltips show how presets use different control settings
- **Learning Mode**: Progressive tooltip system for new users
- **Expert Mode**: Advanced tooltips for experienced users

---

## 🎛️ DYNAMIC EQ SYSTEM - Tooltips & Explanations

### **Global & Pane Tooltips**

**Analyzer Tap**
- **Tooltip**: "Switch the analyzer between Pre and Post DynEQ"
- **Purpose**: Compare input vs output for EQ analysis
- **Use Case**: See how Dynamic EQ affects the signal

**Latency Readout**
- **Tooltip**: "Shows added latency from look-ahead and linear phase"
- **Purpose**: Display processing latency for host PDC
- **Use Case**: Understand delay compensation requirements

### **Band Editor Gestures**

**Add Band**
- **Tooltip**: "Double-click empty space to add a band at that frequency"
- **Purpose**: Quick band creation at specific frequency
- **Use Case**: Add bands where you hear issues

**Delete Band**
- **Tooltip**: "Double-click a band point to remove it"
- **Purpose**: Quick band removal
- **Use Case**: Remove unwanted bands

**Drag Band**
- **Tooltip**: "Drag to set Freq (X) and Gain (Y)"
- **Purpose**: Interactive frequency and gain adjustment
- **Use Case**: Fine-tune band parameters visually

**Adjust Q**
- **Tooltip**: "Scroll on a band; hold Shift to adjust faster"
- **Purpose**: Bandwidth control with mouse wheel
- **Use Case**: Narrow or widen band focus

**Range Handle (Dynamic)**
- **Tooltip**: "Drag the center handle to set ±dB dynamic range"
- **Purpose**: Set dynamic processing range
- **Use Case**: Control how much dynamics affect the band

### **Band Parameters**

**Type**
- **Tooltip**: "Choose filter shape (Bell, Shelf, HP/LP, Notch, BP, All-Pass)"
- **Purpose**: Select appropriate filter type for the task
- **Use Case**: Bell for surgical cuts, Shelf for broad changes

**Freq**
- **Tooltip**: "Set the band's center/cutoff frequency (20 Hz–20 kHz)"
- **Purpose**: Position the band in frequency spectrum
- **Use Case**: Target specific problem frequencies

**Gain**
- **Tooltip**: "Set static boost/cut in dB (−24 to +24 dB)"
- **Purpose**: Static EQ gain adjustment
- **Use Case**: Basic frequency response shaping

**Q**
- **Tooltip**: "Set bandwidth; higher Q is narrower"
- **Purpose**: Control band width and selectivity
- **Use Case**: Narrow Q for surgical cuts, wide Q for gentle shaping

**Phase**
- **Tooltip**: "Choose Zero, Natural, or Linear phase (latency varies)"
- **Purpose**: Select phase response for the band
- **Use Case**: Zero for live, Linear for mastering

**Channel**
- **Tooltip**: "Target Stereo, Mid, Side, Left, or Right"
- **Purpose**: Select processing target channel
- **Use Case**: Mid for center focus, Side for width control

### **Dynamic Processing**

**Dyn On**
- **Tooltip**: "Enable level-dependent processing for this band"
- **Purpose**: Activate dynamic EQ processing
- **Use Case**: Compress peaks or expand quiet content

**Mode**
- **Tooltip**: "Down compresses peaks; Up expands quiet content"
- **Purpose**: Choose dynamic behavior direction
- **Use Case**: Down for peak control, Up for presence enhancement

**Range**
- **Tooltip**: "Max gain change (±dB) applied by the dynamics"
- **Purpose**: Set maximum dynamic processing amount
- **Use Case**: Control how much dynamics can change the band

**Threshold**
- **Tooltip**: "Level where dynamics begin to act"
- **Purpose**: Set activation point for dynamics
- **Use Case**: Trigger dynamics at appropriate levels

**Attack**
- **Tooltip**: "Time to react to increases in level"
- **Purpose**: Set dynamic response speed
- **Use Case**: Fast attack for transients, slow for smooth control

**Release**
- **Tooltip**: "Time to recover after signal falls"
- **Purpose**: Set recovery time for dynamics
- **Use Case**: Match release to program material

**Hold**
- **Tooltip**: "Minimum time to sustain gain change"
- **Purpose**: Prevent rapid gain changes
- **Use Case**: Smooth out erratic dynamics

**Look-Ahead**
- **Tooltip**: "Anticipate peaks for cleaner control (adds latency)"
- **Purpose**: Enable predictive dynamic processing
- **Use Case**: Cleaner peak control with slight delay

**Detector**
- **Tooltip**: "Choose tap (Pre/Post/External) and side-chain filters"
- **Purpose**: Select signal source for dynamics detection
- **Use Case**: Pre for input-based, Post for output-based control

**T/S Amount**
- **Tooltip**: "Separate amounts for Transient and Sustain energy"
- **Purpose**: Independent control of transient and sustain dynamics
- **Use Case**: Control attack and body separately

### **Spectral Processing**

**Spec On**
- **Tooltip**: "Enable resonance-aware shaping inside the band"
- **Purpose**: Activate spectral processing
- **Use Case**: Target specific resonant frequencies

**Selectivity**
- **Tooltip**: "How narrowly resonant bins are targeted"
- **Purpose**: Control spectral processing precision
- **Use Case**: Narrow for specific resonances, wide for general shaping

**Resolution**
- **Tooltip**: "FFT density (Low/Med/High); higher costs more CPU"
- **Purpose**: Set spectral analysis resolution
- **Use Case**: High for precision, Low for performance

**Adaptive**
- **Tooltip**: "Auto-adjust threshold to track program material"
- **Purpose**: Automatic threshold adjustment
- **Use Case**: Maintain consistent processing across varying material

### **Visual Feedback**

**Band Contribution Curve**
- **Tooltip**: "Individual band's response (light path)"
- **Purpose**: Show individual band EQ curve
- **Use Case**: Visualize each band's contribution

**Macro EQ Curve**
- **Tooltip**: "Composite response of all bands (thicker path)"
- **Purpose**: Show combined EQ response
- **Use Case**: See overall frequency response

**Dynamic Region**
- **Tooltip**: "Gradient area between static and dynamic paths (Dyn On)"
- **Purpose**: Visualize dynamic processing range
- **Use Case**: See how much dynamics can change the band

**Spectral Underfill**
- **Tooltip**: "Subtle gradient indicating spectral shaping (Spec On)"
- **Purpose**: Show spectral processing activity
- **Use Case**: Visualize resonance targeting

### **Quality & Safety**

**Auto Linear Guard**
- **Tooltip**: "High-Q HF boosts may blend to Natural to reduce pre-ringing"
- **Purpose**: Automatic phase mode adjustment for stability
- **Use Case**: Prevent artifacts from high-Q high-frequency boosts

**Oversampling**
- **Tooltip**: "Engages automatically for demanding settings; shown in header"
- **Purpose**: Automatic quality enhancement
- **Use Case**: Maintain quality with complex settings

### **Short Hints (Inline)**

**Band Management**
- **Hint**: "Double-click empty space to add; double-click a point to delete"
- **Hint**: "Scroll over a point to change Q; hold Shift to go faster"
- **Hint**: "Turn on Dyn or Spec to reveal advanced controls and visuals"

### **Context-Aware Guidance**

#### **Genre-Specific Tips**
- **Rock/Metal**: "Use Dyn Down for guitar resonance control"
- **Electronic**: "Spec On for precise frequency targeting"
- **Jazz/Acoustic**: "Gentle Q values, Natural phase for musicality"
- **Pop/Vocal**: "Mid channel targeting for vocal clarity"

#### **Processing Chain Tips**
- **"Early in chain"**: "DynEQ before compression for clean dynamics"
- **"After EQ"**: "Static EQ first, then dynamic for refinement"
- **"Parallel processing"**: "Use Side channel for width control"

#### **Performance Optimization**
- **"Start simple"**: "Basic bands first, then add dynamics/spectral"
- **"Monitor CPU"**: "High resolution costs more processing power"
- **"Use presets"**: "Learn from included Dynamic EQ presets"

---

*This comprehensive tooltip system provides detailed guidance for all Dynamic EQ controls, ensuring users understand the purpose, function, and optimal use of each control in the FIELD plugin's Dynamic EQ system.*
