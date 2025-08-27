Here’s a cleaned, consolidated, **industry-standard** README that merges the *relevant* parts of your old doc with everything we’ve planned in this thread—while dropping stale or redundant bits.

---

# FIELD — Professional Spatial & Imaging Processor

A modern spatial-imaging plugin built with JUCE. FIELD blends an expressive **XY Pad** (pan/depth), **multiband imaging** (per-band width + shuffler + rotation/asymmetry), tasteful **mono management**, **stereoize** options, **Brauer-style motion**, clear metering, and a **searchable preset system**—all under a consistent Look\&Feel and a maintainable codebase.

This README is optimized for **humans** and **AI builders**: explicit, actionable, and unambiguous.

---

## Contents

* [At a Glance](#at-a-glance)
* [Why FIELD](#why-field)
* [Feature Set](#feature-set)
* [Signal Flow](#signal-flow)
* [Parameters (APVTS IDs)](#parameters-apvts-ids)
* [UI / UX Spec](#ui--ux-spec)
* [Preset System](#preset-system)
* [Meters & Visualization](#meters--visualization)
* [Build & Run](#build--run)
* [Project Structure](#project-structure)
* [Code & GUI Rules (Must-Follow)](#code--gui-rules-mustfollow)
* [Testing & QA Checklist](#testing--qa-checklist)
* [Roadmap](#roadmap)
* [Credits & License](#credits--license)

---

## At a Glance

* **Framework:** JUCE **7+** (works on 8), C++17
* **Formats:** VST3, AU, (optional AAX), Standalone
* **OS:** macOS & Windows, HiDPI ready
* **Pan Law:** **Ableton-accurate constant-power** (0 dB center, +3 dB at extremes)
* **UI:** Container-based, vector icon system, responsive scaling, fullscreen mode
* **Presets:** Searchable, categorized, A/B with copy, JSON storage
* **New Imaging:** 3-band width, shuffler (LF/HF emphasis), rotation, asymmetry
* **Safety:** Mono maker with slope + audition; stereoize mono-fold safety
* **Motion:** Brauer-style autopan (sync/free), depth, width, phase, center-bias
* **Meters:** L/R + M/S, correlation, mini vectorscope, waveform background

---

## Why FIELD

We drew from tools like **Basslane/Basslane Pro** (low-end mono & width treatment), **Waves S1** (rotation/asymmetry imaging), and **Brauer Motion** (animated movement) while keeping the **fast feel** of modern one-screen plugins. FIELD aims to be musical first, visual and predictable, and easy to automate.

---

## Feature Set

### XY Pad (Hero)

* **X:** Pan (Ableton-accurate)
* **Y:** Depth/Space intensity
* **Split mode:** Independent L/R points with **Link** & **Snap Grid**
* **Motion overlay:** Shows autopan path when enabled

### Imaging

* **3-Band Width:** Lo / Mid / Hi width with adjustable crossovers
* **Shuffler:** LF/HF emphasis & crossover for stereo perception
* **Rotation & Asymmetry:** S1-style field rotation and center offset
* **Accurate Curves:** RBJ biquad-magnitude visualization (HP/LP/Shelves/Peak) with soft-knee pixel mapping

### Mono Management

* **Mono Maker:** Sums under a cutoff with selectable **slope (6/12/24 dB/oct)**
* **Audition:** Listen to “what’s being mono’d” for confidence checks

### Stereoize (P1)

* **Algorithms:** Haas, All-Pass, MicroPitch
* **Mono safety:** Delay/time clamped vs. mono cutoff to prevent combing on fold-down

### Motion (Brauer-Style)

* **Shapes:** Sine, Triangle, Square, Random
* **Sync/Free:** DAW-sync divisions & free Hz rates
* **Depth/Width/Phase/Center Bias** + optional **band-limited** motion

### Presets

* **Searchable** by text, **categorized**, **A/B** with **Copy A↔B**, JSON storage

### Visuals & Meters

* **Correlation**, **mini vectorscope**, **L/R + M/S bars**, **waveform background**
* **EQ curves:** HP/LP, Tilt, Air, Scoop/Boost — true RBJ magnitude; neutral at HP=20 Hz and LP=20 kHz
* **Rulers & Grid:** dB ruler (±18 dB with soft mapping) and Pan subgrid (every 5 units, 50L…0…50R)

---

## Signal Flow

```
[Input L/R]
 → Pre-EQ: HP / LP / Tilt / Bass / Scoop / Air (HP/LP bypassed when neutral: HP≤20 Hz and LP≥20 kHz)
 → Band-Split (LR24): xover_lo_hz, xover_hi_hz
    → Per-Band M/S Width (Lo/Mid/Hi)
    → Shuffler LF/HF (with shuffler_xover_hz)
 → Global M/S: Rotation, Asymmetry
 → Stereoize (Haas/AP/MicroPitch) + mono safety
 → Mono Maker (below mono_hz, slope, audition tap)
 → Saturation / Drive / Mix (existing)
 → Output L/R

Meters tap at safe points (LR+MS peaks, correlation, scope feed).
```

---

## Parameters (APVTS IDs)

> **IDs are snake\_case.** Values normalized in UI; APVTS stores normalized. Only the **relevant & current** set is listed.

**Core / Existing (shipping)**

* `gain_db` (-12…+12, 0), `sat_drive_db` (0…36, 0), `sat_mix` (0…1, 1), `bypass` (0/1, 0)
* `width` (0.5…2.0, 1.0), `pan` (-1…+1, 0), `pan_l` / `pan_r` (split), `split_mode` (0/1, 0)
* `space_algo` (Inner/Outer/Deep, Inner), `depth` (0…1, 0.0), `ducking` (0…1, 0), `os_mode` (Off/2x/4x)
* `hp_hz` (20…1000, 20), `lp_hz` (1000…20000, 20000) — bypassed at extremes
* `tilt` (-12…+12, 0), `air_db` (0…6, 0), `bass_db` (-12…+12, 0), `scoop` (-12…+12, 0)
* (Mini-sliders) `tilt_freq` (100…1000), `scoop_freq` (200…2000), `bass_freq` (50…500), `air_freq` (2k…20k)

**Imaging (NEW)**

* `xover_lo_hz` (40…400, **150**)
* `xover_hi_hz` (800…6000, **2000**)
* `width_lo` (0…2, **1.0**), `width_mid` (0…2, **1.0**), `width_hi` (0…2, **1.1**)
* `rotation_deg` (-45…+45, **0**), `asymmetry` (-1…+1, **0**)
* `shuffler_lo_pct` (0…200, **100**), `shuffler_hi_pct` (0…200, **110**), `shuffler_xover_hz` (150…2000, **700**)

**Stereoize**

* `stereoize_enabled` (0/1, **0**)
* `stereoize_algo` (Haas/AllPass/MicroPitch, **Haas**)
* `stereoize_time_ms` (0…20, **6.0**) (Haas)
* `stereoize_ap_amount` (0…1, **0.5**) (AllPass)
* `stereoize_detune_cents` (0…15, **5**) (MicroPitch)

**Mono Maker**

* `mono_hz` (0…300, **120**)
* `mono_slope_db_oct` (6/12/24, **12**)
* `mono_audition` (0/1, **0**)

**Motion**

* `autopan_enabled` (0/1, **0**)
* `autopan_shape` (Sine/Tri/Square/Random, **Sine**)
* `autopan_sync` (Sync/Free, **Sync**)
* `autopan_rate_hz` (0.01…20, **0.5**)
* `autopan_rate_div` (1/4, 1/2, 1/1, 2/1, 4/1, **1/2**)
* `autopan_depth` (0…1, **0.6**), `autopan_width` (0…1, **1.0**)
* `autopan_phase_deg` (0…180, **0**), `autopan_center_bias` (-1…+1, **0**)
* `autopan_filter_hp` (20…500, **20**), `autopan_filter_lp` (2k…20k, **20000**)

**Meters (read-only)**

* `meter_corr` (-1…+1), `meter_lr_peak`, `meter_ms_peak`, `scope_feed_x/y`

**Pan Law (spec)**

* Sinusoidal constant-power crossfade, center = 0 dB, extremes = +3 dB.
* Split mode exposes `pan_l` / `pan_r` internally (UI presents as L/R balls).

---

## UI / UX Spec

### Layout (Containers)

* **Header:** Bypass, Preset combo (searchable), Prev/Next, Save, A/B with Copy, Split, Link, Snap Grid, Color Mode, Fullscreen.
* **Hero:** **XY Pad** (pan/depth) with split markers; waveform background; motion path overlay.
* **Right Strip:** Correlation pill, mini vectorscope, L/R + M/S meters.
* **Bottom Rows:**

  * **Volume row:** Pan (or L/R), Depth, Space Algo, Ducking, Gain, Drive, Width, Mix.
  * **EQ row:** Bass, Air, Tilt (+ mini-freq sliders), HP, LP, Mono.
  * **Image row:** Width Lo/Mid/Hi, Rotation, Asymmetry, Shuffler, Stereoize card.

### Look & Feel

* Single **Look\&Feel** (`FieldLNF`) owns colors/gradients/shadows.
* **Color Modes:** Ocean (default), Green, Pink, Yellow, Grey. Use the palette button in the header to cycle modes; visuals are driven entirely by `FieldLNF::theme`.
* Vector **icon language** (20+ icons): Lock/Save/Power/Options, Pan/Space/Width/Tilt, HP/LP/Drive/Mix/Air, Link/Stereo/Split/Fullscreen, Help.
* Centered value labels below knobs; responsive scaling (≈50–200%); fullscreen toggle.

---

## Preset System

* **Searchable**: type-to-search by name, description, author, category.
* **Categories**: Studio, Mixing, Mastering, Creative (+ user categories).
* **A/B with Copy**; **JSON** storage in user-data app folder (platform-appropriate).
* Ships with a **starter bank** (e.g., *Low Anchor*, *Wide Air*, *Club*, *Brauer Glide*, *Motion ∞*).

---

## Meters & Visualization

* **Waveform background** with elegant thin lines (direction configurable).
* **EQ curves:** HP/LP (blue), Tilt (orange dashed), Air (white).
* **Drive visualization** with fine lines.
* **Pan split borders** show L/R split percentage.
* **Meters:** L/R + M/S bars, **Correlation** (-1…+1), **mini vectorscope**.

---

## Build & Run

### Requirements

* **JUCE 7+**, **C++17**, CMake 3.22+ (or Projucer), Xcode 14+/VS 2022
* VST3 SDK (for VST3), AU enabled on macOS

### CMake (recommended)

```bash
git clone <repo>
cd Field
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

**Artifacts (typical):**

* macOS AU: `~/Library/Audio/Plug-Ins/Components/Field.component`
* macOS VST3: `~/Library/Audio/Plug-Ins/VST3/Field.vst3`
* Windows VST3: `C:\Program Files\Common Files\VST3\Field.vst3`
* Standalone app under `*/Field_artefacts/*/Standalone/`

*(Projucer projects also supported if preferred.)*

---

## Project Structure

```
Source/
  dsp/
    BandSplitter.*         // LR24 crossovers
    Imaging.*              // width (banded), rotation, asymmetry, shuffler
    Stereoize.*            // Haas/AP/MicroPitch + mono safety
    Motion.*               // autopan core + tempo sync
    Meters.*               // atomics/lock-free scope feeds
  ui/
    XYPad.*                // split mode, link, grid, motion overlay
    Controls.*             // knobs, sliders, toggles, containers
    Containers.*           // Space/Pan/Volume/EQ/Image rows
    FieldLookAndFeel.*     // unified theme & drawing
    Presets.*              // searchable UI, A/B, save
  PluginProcessor.*        // APVTS, dsp graph, state
  PluginEditor.*           // layout, attachments, scaling
```

---

## Code & GUI Rules (Must-Follow)

**Layout & Paint**

1. **Size only in `resized()`**. Never in constructors or `paint()`.
2. **Draw only in `paint()`/`paintOverChildren()`**. No layout or parameter changes there.
3. **Cache geometry**: precompute rects in `resized()`; keep `paint()` math trivial.
4. **Use `juce::Grid`** and shared `Layout` constants; avoid magic numbers.
5. **DP scaling**: all sizes via dp helpers; honor display scale.

**State & Params**
6\. **APVTS is the single source of truth**. Use attachments or listeners; no ad-hoc wiring.
7\. **Parameter IDs** exactly as listed; keep preset serialization stable.
8\. **Begin/End change gesture** for host automation; smooth user-visible jumps.

**DSP & Threads**
9\. **No allocations/locks** in audio thread. SIMD where sensible.
10\. **Meters** via atomics/lock-free queues; UI polls safely.
11\. **Stereoize mono-safety**: clamp delay/time vs. `mono_hz`; taper mix to prevent combing.
12\. **Band-split coherence**: LR24 filters matched; verify nulls on recombine.

**Look\&Feel**
13\. **All visuals via `FieldLNF`** (colors, strokes, shadows).
14\. No inline styling in components except transient highlights.

**Accessibility & UX**
15\. Minimum hit target ≥ **24 dp**; hand cursor for interactive; keyboard focus where useful.
16\. Consistent iconography & centered labels for discoverability.

**AI Builder Prompt (paste into your tool)**

> Use JUCE 7+, C++17. Size in `resized()`, draw in `paint()`. Use `Grid`, dp helpers, and `FieldLNF`. Wire all controls to APVTS via attachments. Keep DSP/UI decoupled (no UI headers in DSP). No allocations/locks in audio thread. Implement the parameters and IDs exactly as in this README. Add A/B & searchable presets. Add mono-safety for stereoize and LR24 crossovers for band width. Write tests/checks from the QA list.

---

## Testing & QA Checklist

**Audio**

* Mono fold-down free of combing with Stereoize on (verify clamps vs. `mono_hz`).
* Crossovers null when recombined (phase-coherent LR24).
* Rotation/Asymmetry maintain energy; automatable without clicks.
* Motion remains in sync across transport start/stop & offline render.

**UI**

* Split mode: L/R points, Link, and Snap Grid behave; labels stay centered.
* Scaling: 0.5×–2.0× without clipping; fullscreen returns to original size.
* Presets: load/save, search, A/B with Copy; all IDs map correctly.
* Meters: correlation within \[-1,+1]; vectorscope stable; waveform smooth.

**Performance**

* No heap allocs per block; stable CPU at 48/96/192 kHz.
* UI redraws do not starve audio; meter update throttled.

---

## Roadmap

* **P0 (Ship):** Multiband imaging (width/shuffler/rotation/asym), Mono Maker slope+audition, meters, searchable presets, polished LNF.
* **P1:** Stereoize Haas + mono safety, Motion (sine/tri), more presets.
* **P2:** Stereoize All-Pass & MicroPitch, advanced Motion (square/random, circle/∞), per-band meters & solos.
* **Nice-to-have:** Optional oversampling island for saturation; 64-bit path where host requests; preset cloud sync.

---





