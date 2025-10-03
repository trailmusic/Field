# ReverbTesting.md

> Field Reverb — validation & measurement playbook
> Audience: DSP, QA, sound design, build/release

---

## Contents

* [1. Purpose & Scope](#1-purpose--scope)
* [2. Test Lab Setup](#2-test-lab-setup)
* [3. Golden Metrics (What we measure)](#3-golden-metrics-what-we-measure)
* [4. Tooling Overview (RX, MATLAB, etc.)](#4-tooling-overview-rx-matlab-etc)
* [5. Core Test Suites (step-by-step)](#5-core-test-suites-step-by-step)

  * [5.1 IR Export & Sanity](#51-ir-export--sanity)
  * [5.2 RT60 / T60 Accuracy](#52-rt60--t60-accuracy)
  * [5.3 Frequency-Dependent Decay (DR-EQ)](#53-frequency-dependent-decay-dr-eq)
  * [5.4 Early Reflections (ETC)](#54-early-reflections-etc)
  * [5.5 Predelay](#55-predelay)
  * [5.6 Ducking Transfer & Timing](#56-ducking-transfer--timing)
  * [5.7 Modulation & Aliasing](#57-modulation--aliasing)
  * [5.8 Stereo Image & Mono Fold-Down](#58-stereo-image--mono-fold-down)
  * [5.9 CPU, Denormals, Stability](#59-cpu-denormals-stability)
  * [5.10 Latency Reporting (PDC)](#510-latency-reporting-pdc)
  * [5.11 Block-Size & Sample-Rate Invariance](#511-block-size--sample-rate-invariance)
  * [5.12 Automation Zippering](#512-automation-zippering)
  * [5.13 Freeze / Safety / Headroom](#513-freeze--safety--headroom)
* [6. Pass/Fail Targets & Benchmarks](#6-passfail-targets--benchmarks)
* [7. Release Checklist](#7-release-checklist)
* [8. Glossary](#8-glossary)
* [9. Appendix A — One-Shot IR Exporter](#9-appendix-a--one-shot-ir-exporter)
* [10. Appendix B — Quick T60 Fit (Python)](#10-appendix-b--quick-t60-fit-python)

---

## 1. Purpose & Scope

Provide a repeatable, objective way to validate Field Reverb's **DSP correctness**, **musical behavior**, and **production readiness** across platforms and DAWs.

Covers ER, FDN/decay shaping, tone/decay EQ, ducking, modulation, stereo image, CPU/denormals, latency (PDC), automation, and freeze/safety.

---

## 2. Test Lab Setup

* **DAW:** REAPER, Logic, Live, Studio One (use at least two).
* **SR/Block combos:** 44.1/48/96 kHz × 64/128/256/1024.
* **Meters/Analyzers:** LUFS, correlation meter, spectrum, phase scope.
* **Silence & denormal checks:** very low-level material (−120 dBFS).
* **Host PDC visible:** confirm reported latency updates with duck look-ahead.

---

## 3. Golden Metrics (What we measure)

* **T60 (RT60):** seconds to decay −60 dB from steady state (or extrapolated from a window).
* **EDT:** early decay time (−10 dB extrapolated to −60).
* **DRR:** direct-to-reverberant ratio (dB).
* **ETC:** energy-time curve for ER timing/amplitude.
* **Clarity (C50/C80):** early/late energy ratios in dB.
* **IACC / L–R ρ:** stereo interaural correlation / simple L–R correlation.
* **CPU & Denormals:** % CPU at idle/tail, no denormal spikes.
* **Latency (PDC):** host-reported samples (duck look-ahead only).
* **Automation smoothness:** no zippering/clicks on param sweeps.

---

## 4. Tooling Overview (RX, MATLAB, etc.)

* **iZotope RX (Audio Editor):** Visual analysis of IRs and decays; easy slope fitting and spectrograms.
* **MATLAB / GNU Octave (free):** Numeric analysis, curve fitting, filter design, batch plots.
* **Python (NumPy/SciPy/Matplotlib):** Free/portable; great for T60 fits and batch checks (see Appendix B).
* **Room EQ Wizard (REW):** Free; measures decay, clarity, ETC; handy even for plugin IRs.
* **Voxengo SPAN / Blue Cat FreqAnalyst:** Real-time spectrum checks.
* **Reaper JSFX / SWS:** Host-side impulse/ping tools and batch rendering.
* **Plugin Doctor:** Linear analyses; useful for ER/tone EQ (reverbs are time-varying—interpret with caution).
* **Audacity:** Free wave editor for quick zoom/measure.

> If you've never used them: **RX** is a waveform/spectrogram editor with measurement tools; **MATLAB/Octave** are numerical computing environments for custom measurements and plots.

---

## 5. Core Test Suites (step-by-step)

### 5.1 IR Export & Sanity

**Goal:** Obtain a clean 8–10 s impulse response for objective measurements.

**Method:** Use our one-shot exporter (Appendix A) or DAW bounce with a single-sample impulse through the plugin (Wet Only, preset neutral).

**Pass:** IR has correct length, no DC offset, no unexpected clipping.

---

### 5.2 RT60 / T60 Accuracy

**Goal:** Measured broadband T60 matches UI `decaySec`.

**Method:**

1. Export IR (48 kHz).
2. Fit decay between 0.5–3.5 s (avoid ER & late noise).
3. Compare measured T60 to target.

**Targets:** ±5% at 48 kHz; ±7% at 44.1/96 (rounding and window differences).

---

### 5.3 Frequency-Dependent Decay (DR-EQ)

**Goal:** DR-EQ multipliers change T60 by the intended factor.

**Method:**

1. Base `decaySec = 2.0 s`, DR-EQ off → measure T60base.
2. Set **High bell @ 4 kHz, ×2.0** → T60(4 kHz) ≈ 4.0 s.
3. Set **Low bell @ 200 Hz, ×0.5** → T60(200 Hz) ≈ 1.0 s.

**Pass:** Within ±10% of requested multiplier; smooth transition across bands.

---

### 5.4 Early Reflections (ETC)

**Goal:** ER timing/energy consistent with parameterization; width maps to pan law.

**Method:** ETC plot (RX/REW). Verify:

* First ER after predelay.
* Tap envelope decays as designed; no "invisible" giant tap.
* Width: equal-power panning behavior.

---

### 5.5 Predelay

**Goal:** Acoustic onset offset equals UI predelay.

**Method:** Impulse with predelay set (e.g., 75 ms). Measure delta between direct and onset of reverberant energy.

**Pass:** ±1 ms.

---

### 5.6 Ducking Transfer & Timing

**Goals:**

* Transfer curve (threshold/ratio/knee/depth) matches design.
* Attack/release and look-ahead timings are correct.

**Method:**

* Feed stepped tone bursts (e.g., −18 dBFS on/off).
* Plot wet level over time; read attack (10→90%) and release (90→10%).
* Change mode → verify look-ahead alters timing and PDC latency updates.

**Pass:** Attack/release within ±10%; GR cap obeys `depthDb`; PDC equals look-ahead (±1 sample tolerance).

---

### 5.7 Modulation & Aliasing

**Goal:** No audible aliasing or cyclic shimmer; rate matches UI.

**Method:**

* Drive with sustained narrowband content; examine spectrogram for sideband smear.
* Vary SR (44.1→96 kHz).
* Listen for periodic "flutter" in tails.

**Pass:** Sidebands fall within expected symmetrical spread; no high-HF alias combs.

---

### 5.8 Stereo Image & Mono Fold-Down

**Goals:** Good decorrelation; mono safe.

**Method:**

* Compute L–R correlation |ρ|max on IR (window mid tail).
* Fold to mono; check level & tone.

**Targets:** |ρ|max < 0.6 (Hall), < 0.5 (Plate/Ambient). Mono fold stable (≤1 dB swing, no phasey nulls).

---

### 5.9 CPU, Denormals, Stability

**Goal:** Flat CPU at tail end; no denormal spikes; no allocations in audio thread.

**Method:**

* Feed −120 dBFS noise; watch CPU.
* Scrub transport; start/stop; automate params.
* Verify Instruments/Allocations (Xcode/Visual Studio): zero allocs per buffer.

**Targets (48 kHz / 128-smpl / stereo):**
ER < 0.2% • FDN(8) < 1.2% • Duck < 0.1% • Tone EQ < 0.2% (approx per core)

---

### 5.10 Latency Reporting (PDC)

**Goal:** Reported latency equals duck look-ahead.

**Method:**

* Toggle modes; read plugin latency in host.
* Solo reverb return; ping alignment.

**Pass:** Exactly matches computed look-ahead samples.

---

### 5.11 Block-Size & Sample-Rate Invariance

**Goal:** No level/timbre shifts when buffer size/SR change.

**Method:**

* Render the same preset across (64/128/256/1024) & (44.1/48/96).
* Compare null residuals or loudness & spectral tilt.

**Pass:** Residual below −50 dBFS and spectrally benign; or LUFS within ±0.2 dB, tilt < 0.5 dB/oct variance.

---

### 5.12 Automation Zippering

**Goal:** No clicks/zipper when automating decay, DR-EQ, duck params.

**Method:**

* Ramp key params over 2–4 s on sustained content.
* Inspect waveform; listen critically.

**Pass:** No discrete steps; no clicks.

---

### 5.13 Freeze / Safety / Headroom

**Goal:** Freeze is stable; safety limiter (if enabled) prevents spikes without tone damage.

**Method:**

* Engage Freeze at various levels; monitor spectrum & CPU.
* Drive extreme presets; check for hard clipping vs. soft safety behavior.

**Pass:** No runaway; soft limiter only under abuse; bypass leaves no coloration.

---

## 6. Pass/Fail Targets & Benchmarks

| Area            | Metric              | Target                          |            |                                 |
| --------------- | ------------------- | ------------------------------- | ---------- | ------------------------------- |
| T60 (broadband) | T60_measured vs UI  | ±5% (48 kHz), ±7% (44.1/96)     |            |                                 |
| DR-EQ           | T60 multiplier      | ±10% at band center             |            |                                 |
| Predelay        | Onset offset        | ±1 ms                           |            |                                 |
| Ducking         | Attack/Release      | ±10% of UI                      |            |                                 |
| Ducking         | GR depth cap        | ±0.5 dB                         |            |                                 |
| PDC             | Reported latency    | = look-ahead ±1 sample          |            |                                 |
| Stereo          |                     | ρ                               | max (tail) | < 0.6 (Hall), < 0.5 (Plate/Amb) |
| Mono            | Fold-down stability | ≤1 dB swing, no phasey holes    |            |                                 |
| CPU             | See 5.9             | Under budget at 48k/128         |            |                                 |
| Denormals       | CPU tail            | Flat; no spikes                 |            |                                 |
| SR/Block        | Invariance          | LUFS ±0.2 dB; tilt < 0.5 dB/oct |            |                                 |

---

## 7. Release Checklist

* [ ] IR exports pass T60 & DR-EQ tests (48/96 kHz).
* [ ] ER ETC consistent with UI density/width/time.
* [ ] Ducking transfer & timings verified in all 5 modes.
* [ ] Stereo decorrelation & mono fold-down pass.
* [ ] CPU/denormals flat; no audio-thread allocations.
* [ ] PDC matches look-ahead in all modes.
* [ ] No zippering on automated params.
* [ ] Freeze stable; safety limiter only under abuse.
* [ ] Preset pack auditioned (vocal, hall, drum, ambient).
* [ ] Build passes in at least two DAWs and on macOS + Windows.

---

## 8. Glossary

* **IR (Impulse Response):** Output when input is a single sample; captures system's time response.
* **RT60 / T60:** Time until decay by 60 dB.
* **EDT:** Early decay time; extrapolated from initial slope.
* **ETC:** Energy-Time Curve; reveals ER timing/energy.
* **Clarity (C50/C80):** Early/late energy ratio (dB) for speech/music.
* **IACC:** Interaural cross-correlation; stereo spaciousness proxy.
* **PDC:** Plugin Delay Compensation; host alignment of latency.
* **Denormals:** Tiny floats that spike CPU; we suppress them.
* **Look-ahead:** Delay added to anticipate detector events (ducking).
* **FDN:** Feedback Delay Network; late-reverb "tank".
* **DR-EQ:** Decay-Rate EQ; frequency-dependent T60 shaping.

---

## 9. Appendix A — One-Shot IR Exporter

> Small JUCE console app: renders an impulse through **ReverbEngine** and writes `~/Desktop/Field_Rev_IR.wav` (10 s). Adjust paths/names as needed.

```cpp
// File: tools/FieldIRExport.cpp
#include <JuceHeader.h>
#include "features/reverb/Core/ReverbEngine.h" // Updated path after reorganization

using namespace juce;

int main (int, char**)
{
    const double fs = 48000.0;
    const int block = 256;
    const int channels = 2;
    const double seconds = 10.0;
    const int total = (int) std::round (seconds * fs);

    // Engine
    ReverbEngine eng;
    eng.prepare (fs, block, channels);

    // Params (neutral-ish, adjust as desired)
    ReverbParams p{};
    p.decaySec = 2.0f;
    p.erLevelDb = -18.0f;
    p.erTimeMs = 55.0f;
    p.erDensity = 0.7f;
    p.erWidthPct = 1.0f;
    p.duckOn = false;
    eng.setParams (p);

    // Optional: set Decay-Rate profile / Tone EQ here

    AudioBuffer<float> wet (channels, block), side (channels, block);
    AudioBuffer<float> out (channels, total);
    out.clear();

    // Seed: single-sample impulse on L
    std::vector<float> imp (total, 0.0f);
    imp[0] = 1.0f;

    int written = 0;
    while (written < total)
    {
        const int N = jmin (block, total - written);
        wet.clear(); side.clear();

        // copy impulse chunk to both channels (mono excite)
        for (int c=0;c<channels;++c)
            memcpy (wet.getWritePointer(c), imp.data()+written, sizeof(float)*N);

        // process 100% wet
        eng.processWet (wet, side);

        // write to output
        for (int c=0;c<channels;++c)
            memcpy (out.getWritePointer(c)+written, wet.getReadPointer(c), sizeof(float)*N);

        written += N;
    }

    WavAudioFormat fmt;
    File file (File::getSpecialLocation(File::userDesktopDirectory).getChildFile("Field_Rev_IR.wav"));
    std::unique_ptr<FileOutputStream> os (file.createOutputStream());
    if (!os) { DBG("Cannot open output file"); return 1; }

    std::unique_ptr<AudioFormatWriter> w (fmt.createWriterFor (os.get(), fs, (unsigned int)channels, 24, {}, 0));
    if (!w) { DBG("Cannot create writer"); return 1; }
    os.release();

    w->writeFromAudioSampleBuffer (out, 0, total);
    DBG("Wrote: " << file.getFullPathName());
    return 0;
}
```

**Build hint (CMake):**

```cmake
add_executable(FieldIRExport tools/FieldIRExport.cpp)
target_link_libraries(FieldIRExport PRIVATE juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_core juce::juce_audio_basics juce::juce_audio_formats)
target_link_libraries(FieldIRExport PRIVATE FieldPluginLib) # your engine lib target

# Include the reorganized reverb source files
target_sources(FieldIRExport PRIVATE
    Source/features/reverb/Core/ReverbEngine.cpp
    Source/features/reverb/Core/ReverbTypes.h
    Source/features/reverb/Core/FieldReverbConfig.h
    # Add other required reverb files as needed
)
```

Run → check `Field_Rev_IR.wav` on Desktop in RX/MATLAB.

---

## 10. Appendix B — Quick T60 Fit (Python)

> Minimal script to estimate T60 from a WAV IR. Install `numpy`, `scipy`, `soundfile`, `matplotlib` locally.

```python
import numpy as np, soundfile as sf, matplotlib.pyplot as plt

wav, fs = sf.read("Field_Rev_IR.wav")
mono = wav.mean(axis=1) if wav.ndim == 2 else wav
mono = mono / (np.max(np.abs(mono)) + 1e-12)

t = np.arange(len(mono)) / fs
win = (t >= 0.5) & (t <= 3.5)  # adjust per preset
y = mono[win]
y = np.maximum(np.abs(y), 1e-12)
logy = np.log10(y)

# Linear least-squares: log10(y) = m*t + b
tt = t[win]
A = np.vstack([tt, np.ones_like(tt)]).T
m, b = np.linalg.lstsq(A, logy, rcond=None)[0]
T60 = 6.0 / abs(m)

print(f"T60 estimate: {T60:.3f} s")

# Plot
plt.figure()
plt.plot(t, 20*np.log10(np.maximum(np.abs(mono), 1e-12)), label="IR dB")
fit = (m*tt + b)
plt.plot(tt, 20*fit, label="Fit (dB)", linewidth=2)
plt.xlabel("Time (s)"); plt.ylabel("Level (dB)"); plt.legend(); plt.grid(True)
plt.show()
```

---

### Notes

* Keep the **IR exporter** and **Python T60 fit** under `tools/` so QA can run them without a DAW.
* Benchmarks above are conservative; if we consistently beat them, we can tighten before release.

---

If you want, I can also hand you a tiny **REAPER project** with ready-made tracks (impulse bus, analyzer bus, automation lanes) for fast manual validation—just say the word.
