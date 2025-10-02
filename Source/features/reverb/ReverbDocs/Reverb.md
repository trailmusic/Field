# Reverb.md — Field Reverb System

*Version:* 2.0 (Jan 2025) • *Owner:* Audio/DSP • *Status:* Phase 2 FDN tank implemented and production-ready

---

## Clickable Index

* [1. Overview](#1-overview)
* [2. What Shipped This Cycle](#2-what-shipped-this-cycle)
* [3. Architecture & Topology](#3-architecture--topology)

  * [3.1 High-Level Signal Flow](#31-high-level-signal-flow)
  * [3.2 UI/Engine Boundaries](#32-uiengine-boundaries)
* [4. Parameters & IDs](#4-parameters--ids)

  * [4.1 Core & Structure](#41-core--structure)
  * [4.2 Early Reflections](#42-early-reflections)
  * [4.3 Diffusion/Modulation/Stereo](#43-diffusionmodulationstereo)
  * [4.4 Mix, Specials & Motion Follow](#44-mix-specials--motion-follow)
  * [4.5 Reverb EQ (Tone & Decay-Rate)](#45-reverb-eq-tone--decay-rate)
  * [4.6 Ducking](#46-ducking)
  * [4.7 Phase 2+ Infrastructure (New)](#47-phase-2-infrastructure-new)
* [5. UI System](#5-ui-system)

  * [5.1 2×16 Controls Pane Map](#51-216-controls-pane-map)
  * [5.2 Ducking Float](#52-ducking-float)
  * [5.3 Reverb Graphics (Rays / Waterfall / Spectral)](#53-reverb-graphics-rays--waterfall--spectral)
* [6. DSP Details](#6-dsp-details)

  * [6.1 Early Reflections (Phase 1)](#61-early-reflections-phase-1)
  * [6.2 Tail (Phase 1) & Phase 2 FDN Plan](#62-tail-phase-1--phase-2-fdn-plan)
  * [6.3 Ducking Design](#63-ducking-design)
  * [6.4 EQ Placement](#64-eq-placement)
  * [6.5 Phase 2+ Infrastructure Details](#65-phase-2-infrastructure-details)
  * [6.6 Performance & Optimization](#66-performance--optimization)
* [7. Theming, LNF & Robustness](#7-theming-lnf--robustness)
* [8. QA & Measurement](#8-qa--measurement)
* [9. Known Issues & Open Items](#9-known-issues--open-items)
* [10. Roadmap](#10-roadmap)
* [11. Change Log](#11-change-log)
* [12. Developer Integration Guide](#12-developer-integration-guide)
* [Glossary](#glossary)

---

## 1. Overview

Field's Reverb system delivers a modern, musical reverb with a pro UI, robust ducking, and an EQ stack (Tone + Decay-Rate). **Phase 2 FDN tank is now implemented** with mathematically correct decay mapping, per-cycle feedback gains, and real decay-rate shaping. The system provides professional-quality reverb behavior with accurate T60 measurements.

---

## 2. What Shipped This Cycle

* **UI:**

  * 2×16 control grid finalized (Enable at R1C1, Wet-Only at R1C16).
  * Floating **Ducking module** with GR meter, mode-based lookahead/RMS, band focus.
  * **ReverbGraphics** views: Rays, Waterfall (theme grey), Spectral.
  * **Tone EQ** (post), **Decay-Rate EQ** (decay mult×) with smart positioning, band limits, double-click delete, point toggle.
  * Visualization control panel (polished, themed).
* **Engine:**

  * ER taps with per-tap filters; ducking compressor with mode presets.
  * **Phase 2 FDN tank** with 8 delay lines, Hadamard feedback matrix, per-cycle feedback gains.
  * **Real decay-rate shaping** with mathematically correct T60 mapping.
  * **Ducking latency reporting** for proper PDC compensation (host-visible).
  * Metering (ER RMS, Tail RMS, Duck GR).
* **Infra:**

  * `FIELD_REVERB_PHASE2=1` enabled by default.
  * **DecayLossDesigner** for UI-to-FDN coefficient mapping.
  * **ReverbFDN.h** with production-ready FDN core.
  * Processor glue (APVTS→ReverbParams).
  * Unit-test IR exporter; SIMD stubs.
* **Robustness:**

  * Theme/LNF propagation fixed across all reverb UI.
  * Timer lifecycle hardening; editor teardown order; leak detectors.
  * **Denormal protection** in FDN hot loop.
  * **Thread-safe parameter updates** with double-buffered runtime.
  * **Output safety** with soft clipper for extreme presets.
  * **Comprehensive validation** with T60 measurement and stereo decorrelation checks.

---

## 3. Architecture & Topology

### 3.1 High-Level Signal Flow

```
Input → (PreDelay) → EarlyReflections → FDN Tank (Phase 2)
                      │                 │
                      └── meters        └── Tone EQ (routing) → Ducking → Wet out
                             │                        ▲
                        ReverbGraphics               Sidechain (Dry/ER/Tail/Wet)
```

* **Phase 2 FDN Tank:** 8 delay lines with Hadamard feedback matrix, per-cycle feedback gains, real decay-rate shaping.
* **Decay-Rate EQ:** Maps UI multipliers to T60(f) curve, converts to per-line feedback gains.

### 3.2 UI/Engine Boundaries

* **APVTS ↔ ReverbParams**: glue layer packs/reads.
* **ReverbGraphics** reads metering + toggles visualization modes and hosts EQ UIs.
* **DuckingFloat** manipulates duck parameters; engine applies all DSP.

---

## 4. Parameters & IDs

### 4.1 Core & Structure

* `enabled`, `killDry`
* `preDelayMs`, `decaySec`, `sizePct`

### 4.2 Early Reflections

* `erLevelDb`, `erDensityPct`, `erWidthPct`, `erTimeMs`, `erToTailPct`

### 4.3 Diffusion/Modulation/Stereo

* `diffusionPct`, `densityPct`
* `modDepthCents`, `modRateHz`
* `widthPct`, `rotationDeg`

### 4.4 Mix, Specials & Motion Follow

* `wetMix01`, `bloomPct`, `distancePct`, `freeze`, `shimmerAmtPct`, `shimmerInt`, `gateAmtPct`, `outTrimDb`
* `followWidth`, `followWidthAmt`, `followRot`, `followRotAmt`

### 4.5 Reverb EQ (Tone & Decay-Rate)

**Tone EQ (post) per-band:**
`rvb_eq_b{i}_enabled`, `rvb_eq_b{i}_type (Bell/LS/HS)`, `rvb_eq_b{i}_freq`, `rvb_eq_b{i}_gainDb`, `rvb_eq_b{i}_q`, `rvb_eq_b{i}_dynAmt`
**Decay-Rate EQ per-band:**
`rvb_dreq_b{j}_enabled`, `rvb_dreq_b{j}_type (Bell/TiltLo/TiltHi)`, `rvb_dreq_b{j}_freq`, `rvb_dreq_b{j}_q`, `rvb_dreq_b{j}_mult`
**Lane/global:** `dreqXoverLoHz`, `dreqXoverHiHz`, `dreqApply (Pre/Post/ER/Tail)`

### 4.6 Ducking

`duckOn`, `duckMode`, `duckDepthDb`, `duckThrDb`, `duckRatio`, `duckKneeDb`, `duckAtkMs`, `duckRelMs`, `duckBandHz`, `duckBandQ`, `duckDetectorSrc`

### 4.7 Phase 2+ Infrastructure (New)

* **FieldReverbConfig.h**: Compile-time switches (`FIELD_REVERB_PHASE2`, `FIELD_ENABLE_SIMD`)
* **ReverbFDN.h**: FDN core with Hadamard feedback, prime delay lengths, per-line loss filters
* **ReverbProcessorGlue**: APVTS → ReverbParams bridge with sidechain handling
* **ReverbIRExportTest**: 10-second IR export to desktop for validation
* **SimdBiquad.h**: Structure-of-Arrays biquad for future SIMD optimization

---

## 5. UI System

### 5.1 2×16 Controls Pane Map

**Row 1:** ENABLE, PRE, ER LVL, ER DEN, ER WID, ER TIME, ER→T, DIFF, DENS, MOD DEP, MOD RATE, WIDTH, ROT, SIZE, DECAY, WET ONLY
**Row 2:** WET, BLOOM, DIST, FREEZE, SHIM AMT, SHIM INT, GATE, DREQ XO LO, DREQ XO HI, EQ APPLY (Combo), FOLLOW W, W AMT, FOLLOW R, R AMT, TRIM, DUCK

**Control types**

* Knobs: all continuous values.
* Toggles: ENABLE, WET ONLY, FREEZE, FOLLOW W, FOLLOW R, DUCK.
* Combo: **EQ APPLY** (Pre/Post/ER/Tail).

### 5.2 Ducking Float

* Always visible (expansion removed).
* **States:** Inactive (duck off), Ready (on but no GR), Active (GR > 0).
* Mode presets: General, Vocal, DrumBus, Guitar, Keys (auto lookahead/RMS).
* Detector source: Dry, ER, Tail, Wet Sum.
* GR meter at top; color-coded zones; units label.

### 5.3 Reverb Graphics (Rays / Waterfall / Spectral)

* **Rays:** density/diffusion-mapped ray fan, parameter-driven jitter.
* **Waterfall:** theme-greys, audio-reactive intensity, dual texture lines.
* **Spectral:** ER vs Tail curves.
* GR overlay available in all views. Button row centered in panel header.

---

## 6. DSP Details

### 6.1 Early Reflections (Phase 1)

* Up to 16 taps; exponential delay spread (≈5–55 ms), exp decay gains, alternating pan.
* Simple per-tap filter placeholder; equal-power panning; ring buffers; zero-alloc in process.

### 6.2 Phase 2 FDN Tank (Implemented)

* **FDN Core:** 8 delay lines with prime-ish lengths (31-149ms @48k), Hadamard feedback matrix.
* **Per-Cycle Feedback Gains:** `g = 10^(-3 * T_rt / T60)` where T_rt is round-trip delay time.
* **Decay-Rate EQ Integration:** Maps UI multipliers to T60(f) curve, converts to per-line feedback gains.
* **Input/Output Diffusion:** Decorrelated input spread weights, multi-line stereo output tapping.
* **Denormal Protection:** `juce::ScopedNoDenormals` in hot loop for CPU stability.

### 6.3 Ducking Design

* Mode-based lookahead/RMS; soft knee; threshold/ratio; depth cap; band focus via peaking filter.
* Detector sources: Dry/ER/Tail/Wet; envelope smoothing via attack/release exponentials.
* **Latency Reporting**: Ducking look-ahead latency is automatically reported to host for proper PDC compensation.
  - **Source of Truth**: Latency comes from ducking FIFO `gaAhead` (in samples), not attack time
  - **Mode-Dependent**: Each ducking mode has specific look-ahead times (8-16ms)
  - **SR-Aware**: Look-ahead scales correctly with sample rate changes
  - **Parameter Updates**: Latency refreshes on `duckOn`, `duckMode`, `duckDetectorSrc` changes
  - **Bypass Handling**: Reports 0 latency when ducking disabled or plugin bypassed
  - **Future-Proof**: Architecture ready for oversampling latency addition

### 6.4 EQ Placement

* **Tone EQ:** default Post; supports Pre/ER/Tail per `dreqApply`.
* **Decay-Rate EQ:** conceptually inside FDN loop (for decay), but UI allows ER/Tail-only displays. Engine hook planned at tank feedback.

### 6.5 Phase 2+ Infrastructure Details (Implemented)

* **FDN Core**: 8 delay lines with prime-ish lengths (31-149ms @48k), Hadamard feedback matrix
* **DecayLossDesigner**: Converts Decay-Rate EQ UI to per-line feedback gains with smoothing
* **Per-Cycle Feedback**: Mathematically correct `g = 10^(-3 * T_rt / T60)` formula
* **Frequency Mapping**: Line delays mapped to representative frequencies for T60 curve interpolation
* **Processor Glue**: Handles APVTS parameter mapping and sidechain routing
* **IR Export**: UnitTest framework for offline validation and analysis

### 6.6 Performance & Optimization

* **Audio Thread**: Zero allocations in processWet()
* **Memory**: All buffers pre-sized in prepare()
* **SIMD Ready**: BiquadSoA structure for future vectorization
* **Denormal Safety**: `juce::ScopedNoDenormals` in FDN hot loop
* **Thread Safety**: Double-buffered runtime with atomic parameter updates
* **Output Safety**: Soft clipper prevents spikes in extreme presets
* **Latency**: PDC reporting for ducking look-ahead (host-visible, parameter-aware)

---

## 7. Theming, LNF & Robustness

* All reverb components now inherit LNF dynamically (no pinned pointers; no cached colors).
* `lookAndFeelChanged()` and `parentHierarchyChanged()` propagate LNF to the entire tree.
* **Timer lifecycle:** visibility-aware timers; stop in destructors; editor teardown safety; leak detectors added.

---

## 8. QA & Measurement

* **IR Export Test:** 10 s IR writer (UnitTest) with comprehensive validation.
* **T60 Measurement:** Mathematical T60 fitting with ±5% tolerance validation.
* **Stereo Decorrelation:** Cross-correlation analysis with ρ < 0.6 target.
* **Thread Safety:** Double-buffered parameter updates prevent automation races.
* **Output Safety:** Soft clipper prevents spikes in extreme presets.
* **Checklist:**

  * Slot positions (R1C1 Enable, R1C16 Wet Only, R2C15 Trim).
  * Duck float states & GR meter movement.
  * View modes switch; Waterfall displays; clipping correct.
  * Theme switch updates all elements.
  * Preset migration: legacy IDs removed/mapped.
  * **T60 accuracy:** Measured vs UI decay time within ±5%.
  * **Stereo spread:** Cross-correlation below 0.6 for proper decorrelation.
  * **Thread safety:** No parameter update races during automation.
* **Comprehensive Testing:** See `ReverbTesting.md` for detailed validation procedures.

---

## 9. Known Issues & Open Items

* **Band Indicators:** refactored counters/ID-finder in place, but indicators still not reflecting active band counts → verify APVTS ID creation and callbacks; add DBG traces.
* **Waterfall Past Bug:** layout percentage math fixed; keep guard tests.
* **Thread Safety:** Double-buffered parameter updates for live automation (pending).
* **Latency Reporting:** Ducking look-ahead latency reporting to host (pending).

---

## 10. Roadmap

* **Phase 2 Complete:** FDN tank implemented with mathematically correct decay mapping and real decay-rate shaping.
* **Room Models & Presets:** Plate/Hall/Chamber/Room; factory set.
* **Performance:** SIMD passes for filters/mix; FTZ/DAZ; parameter smoothing.
* **UX:** Analyzer tap points (Pre/ER/Tail/Post), macros (Size/Color/Motion), wet-lock.
* **QA:** IR/T60 tests; swept-sine EQ checks; ducking step tests.
* **Thread Safety:** Double-buffered parameter updates for live automation.
* **Latency Reporting:** Ducking look-ahead latency reporting to host.

---

## 11. Change Log

* **Jan 2025 v2.0**

  * **Phase 2 FDN Tank Implemented:** Production-ready FDN core with 8 delay lines, Hadamard feedback matrix.
  * **Mathematically Correct Decay Mapping:** Per-cycle feedback gains `g = 10^(-3 * T_rt / T60)` instead of 1-pole filtering.
  * **Real Decay-Rate Shaping:** DecayLossDesigner converts UI multipliers to T60(f) curve and per-line feedback gains.
  * **Frequency Mapping:** Line delays mapped to representative frequencies for accurate T60 curve interpolation.
  * **Input/Output Diffusion:** Decorrelated input spread weights and multi-line stereo output tapping.
  * **Denormal Protection:** `juce::ScopedNoDenormals` in FDN hot loop for CPU stability.
  * **Smoothing Math Fixed:** Correct per-block smoothing coefficient calculation.
  * **Thread Safety:** Double-buffered runtime with atomic parameter updates for automation safety.
  * **Output Safety:** Soft clipper prevents spikes in extreme presets.
  * **Comprehensive Validation:** T60 measurement, stereo decorrelation, and thread safety testing.
  * ReverbGraphics background paint added; visuals reset & rebuilt (Rays/Waterfall/Spectral).
  * 2×16 grid finalized; control types corrected (Combo: EQ APPLY; Toggles for ENABLE, WET ONLY, etc.).
  * DuckingFloat redesigned: always visible; GR meter with three states; mode/Detector selection.
  * Theme compliance: dynamic LNF, no cached colors; propagation across tree.
  * Waterfall: theme-grey gradient + textures; coordinate/clipping fixes.
  * Robustness: timer lifecycle; cleanup manager; leak detectors.
  * EQ UX: smart positioning, band limits, point toggle, double-click delete.
  * Infra: FDN skeleton, APVTS glue, IR UnitTest, SIMD stubs.

---

## 12. Developer Integration Guide

### Phase 2 FDN (Enabled by Default)
```cpp
// FIELD_REVERB_PHASE2=1 is enabled by default in CMakeLists.txt
// The FDN tank is now production-ready with mathematically correct decay mapping
```

### Using the Processor Glue
```cpp
// In your AudioProcessor
std::unique_ptr<ReverbProcessorGlue> reverbGlue;
ReverbEngine reverbEngine;

// Constructor
reverbGlue = std::make_unique<ReverbProcessorGlue>(apvts, reverbEngine);

// prepareToPlay
reverbGlue->prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());

// processBlock
reverbGlue->processBlock(buffer, midiMessages);
```

### Running IR Export Tests
```cpp
// Enable JUCE unit tests in your build
// Run: juce::UnitTestRunner::runAllTests()
// Output: FIELD_Reverb_IR.wav on Desktop
```

### Build Flags Reference
```cpp
// Compile-time switches (Phase 2 FDN enabled by default)
#define FIELD_REVERB_PHASE2 1        // FDN tank with mathematically correct decay mapping
#define FIELD_ENABLE_SIMD 1          // Enable SIMD optimizations
#define FIELD_REVERB_DEFAULT_IR_SECONDS 10  // IR export duration
```

---

## Glossary

* **APVTS** — JUCE `AudioProcessorValueTreeState`; param store + host automation.
* **ER (Early Reflections)** — The first discrete echoes from nearby surfaces (5–80 ms).
* **FDN (Feedback Delay Network)** — A reverb tank architecture with multiple delay lines mixed by a unitary matrix.
* **T60** — Time for decay to drop by 60 dB; main perceptual "decay time."
* **Decay-Rate EQ (DR-EQ)** — Frequency-dependent **decay** shaping (multiplier× on T60), distinct from magnitude EQ.
* **Tone EQ** — Static/dynamic magnitude EQ applied pre/post or to ER/Tail only.
* **GR (Gain Reduction)** — Amount of attenuation applied by ducking.
* **Lookahead** — Detector sees the future by delaying program path (or advancing detector) to react sooner.
* **Soft Knee** — Smooth transition into compression; reduces audible pumping.
* **LNF (Look & Feel)** — JUCE theming/styling system used by Field.
* **Freeze** — Forces tank feedback ≈ 1; reverb sustains indefinitely (with loss shaping).

---

### Appendix: Implementation Notes (Quick Hits)

* **Clickable routing summary:** `dreqApply → {Pre | Post | ER | Tail}`; DR-EQ integrated inside FDN tank loop.
* **Meters:** `getErRms()`, `getTailRms()`, `getCurrentDuckGrDb()` (atomics).
* **Build Flags:** `FIELD_REVERB_PHASE2=1` (enabled), `FIELD_ENABLE_SIMD`, `FIELD_REVERB_DEFAULT_IR_SECONDS`.
* **Unit Tests:** IR export writes `FIELD_Reverb_IR.wav` to Desktop.
* **FDN Tank:** 8 delay lines, Hadamard feedback matrix, per-cycle feedback gains, denormal protection.

---

*This document is the single source of truth for Field Reverb's design, shipped state, and near-term roadmap. Keep it updated when parameter IDs, routing, or UX change.*
