# Field Architecture Refactor Audit — **Phase 2**

> **Last updated:** 2025-10-05
> **Branch:** `feature`
> **Goal:** eliminate rhythmic/glitch artifacts across hosts, cement float/double parity, and add **zero-cost-in-Release** diagnostics.

---

## Contents

* [Purpose](#purpose)
* [Scope](#scope)
* [Success Criteria (DoD)](#success-criteria-dod)
* [Entry Checklist](#entry-checklist)
* [Work Orders (Phase 2)](#work-orders-phase-2)

  * [WO-55 — FDN wet fade-in + wrap mirroring](#wo55--fdn-wet-fade-in--wrap-mirroring)
  * [WO-56 — SpikeSilencer (dev-only micro-fade)](#wo56--spikesilencer-dev-only-micro-fade)
  * [WO-57 — Dev cut switches + silence contract](#wo57--dev-cut-switches--silence-contract)
  * [WO-58 — Double-path parity & prepare guarantees](#wo58--double-path-parity--prepare-guarantees)
  * [WO-59 — Ableton Insert Safe hardening (recap)](#wo59--ableton-insert-safe-hardening-recap)
  * [WO-60 — Wet-path DC guard & post-sum clamp](#wo60--wet-path-dc-guard--post-sum-clamp)
  * [WO-61 — APVTS Freeze: snapshot & purge audio-thread reads](#wo61--apvts-freeze-snapshot--purge-audio-thread-reads)
  * [WO-62 — Rhythm-coupling probes (transport/tempo)](#wo62--rhythm-coupling-probes-transporttempo)
  * [WO-63 — pluginval + Host Matrix CI](#wo63--pluginval--host-matrix-ci)
* [Diagnostics & Telemetry (dev-only)](#diagnostics--telemetry-dev-only)
* [Verification Matrix](#verification-matrix)
* [Risk & Mitigation](#risk--mitigation)
* [Rollback Plan](#rollback-plan)
* [Ownership & Cadence](#ownership--cadence)
* [Appendix A — Code Sketches (drop-in)](#appendix-a--code-sketches-drop-in)
* [Appendix B — CI Tripwires (regex)](#appendix-b--ci-tripwires-regex)
* [Appendix C — Triage Playbook](#appendix-c--triage-playbook)

---

## Purpose

Phase 2 hardens **stability and parity** across formats/hosts and installs **fast-to-act** dev instrumentation that is compiled out in Release builds. We target the reported **rhythmic glitches** (octave/tempo-aligned) and any **insert-time spikes**.

---

## Scope

* **Processor lifecycle** (insert safety, zero buffers/channels).
* **Float/double parity** across **Master vs Track**.
* **Wet-path hygiene** (FDN wrap mirroring, DC guard, explicit sums).
* **Spike capture/suppression** in Debug only.
* **Zero APVTS reads** on audio thread; snapshot at prepare/message thread.
* **Transport-coupling probes** to confirm/deny tempo-sync involvement.
* **pluginval + host matrix** smoke runs.

---

## Success Criteria (DoD)

* **No crashes** on insert or while playing (Live 12, AU/VST3).
* **No audible spikes** on insert/start/stop; first buffer is faded-in.
* **No rhythmic glitches** at 64/128/512 buffer; **null at unity** verified.
* **Float vs double** output parity (within −140 dBFS) when OS=1x.
* **Zero audio-thread APVTS reads** (CI tripwire clean).
* **pluginval** passes; **Host Matrix** smoke clean.

---

## Entry Checklist

1. Build **RelWithDebInfo** AU/VST3; clear macOS quarantine.
2. Run **pluginval** (AU and VST3).
3. Confirm Standalone launches (RelWithDebInfo) and Debug opens (asserts silenced or actionable).
4. Ensure Phase 1 tripwires are **enabled by default** in the build.

---

## Work Orders (Phase 2)

### WO-55 — FDN wet fade-in + wrap mirroring

**Objective:** Make startup & wrap transitions click-free and SIMD-safe.

**Changes**

* **5 ms wet fade-in** after each **prepare/reset** (per-channel ramp).
* **Wrap mirroring**: mirror head → pad region on each write wrap for safe vector reads.
* **NaN-safe wet L/R sum**: explicit finite checks before post-sum.

**Acceptance**

* Insert while playing produces **no spike**; silent input stays silent.
* Spectrogram shows **no spur** at wrap cadence.

---

### WO-56 — SpikeSilencer (dev-only micro-fade)

**Objective:** Smother rare >0.95 spikes with a one-shot **32-sample** micro-fade **(Debug only)**.

**Changes**

* `core/signal/SpikeSilencer.h` (header-only): threshold compare, hold & decay, 32-sample ramp to zero and back.
* Hooked **post wet-sum**, before DC block.
* Compiled out in Release (`#if FIELD_DEV_HUD_ON || JUCE_DEBUG`).

**Acceptance**

* First spike occurrence logs **index + block size** and fades; subsequent blocks normal.
* Release build: **no codegen** changes (verified by objdump size parity ± noise).

---

### WO-57 — Dev cut switches + silence contract

**Objective:** Fast isolation switches; formal “silence means silence” rule.

**Changes**

* Hidden dev param `dev.dsp.cut` ∈ { **normal**, **dryOnly**, **wetOnly**, **fdnMuted**, **fdnBypassed** }.
* **Silence contract (Debug)**: if input RMS < −140 dBFS and tail=0, output must be < −140 dBFS; else assert+DBG with first-bad sample.

**Acceptance**

* Toggleable live from editor HUD (dev only).
* Contract holds across 64/128/512; no false positives.

---

### WO-58 — Double-path parity & prepare guarantees

**Objective:** Ensure float/double paths are **prepared identically** and remain in lockstep.

**Changes**

* `prepareCommon(sr, maxBlock, chans)` called from both `prepare<float>` and `prepare<double>`.
* **Prep cookie** `{sr, maxBlock, chans, sizeof(Sample)}` stored; each process overload **asserts** cookie match (Debug).
* Force **same stage order & params** both paths; forbid implicit casts.

**Acceptance**

* **AB null** (float vs double) at OS=1x < −140 dBFS.
* No parity drift after 5 minutes of transport running.

### Update — 2025-10-05
- Change: Added debug-only PrepCookie parity cookies/asserts to `Source/modules/FieldChain.h` to ensure float/double prepare/process match; validates SR/channels/bytes and that both paths have been prepared.
- Reason: Enforce double-path parity and surface mismatches early during development (WO-58).
- Verification: Built AU/VST3/Standalone in RelWithDebInfo; no new warnings/errors; parity asserts active only in Debug.
- Commit: 561ef2a

---

### WO-59 — Ableton Insert Safe hardening (recap)

**Objective:** Belt-and-suspenders for Live’s first callback behaviors.

**Changes**

* Early returns on **zero-buffer/zero-channels**.
* `prepared_` gate on processor **and** FieldChain stages.
* DualChain guard: if **not prepared**, clear block and return.

**Acceptance**

* No crash/spike when dropping plugin on Master while playing loop.

---

### WO-60 — Wet-path DC guard & post-sum clamp

**Objective:** Kill DC creep and disallow pathological overs.

**Changes**

- `Source/engines/reverb/DSP/ReverbFDN.h`: Add per-channel 1st-order DC blocker `DcBlock` post wet-sum; reset state in `prepare`.
- `Source/engines/reverb/DSP/ReverbFDN.h`: Add Debug-only soft clamp (±8.0) in post-sum loop; fenced by `#if JUCE_DEBUG`.

**Build & CMake**

- No CMake changes; standard targets build.

**Tests**

- Offline/unit: pending dedicated DC offset decay test.
- Host smoke: Live 12 AU/VST3, 48k, 64/128/512 buffers.
- pluginval: to be run in next matrix.

**Verification**

- ✅ Compiles AU/VST3/Standalone (RelWithDebInfo). No new warnings.

**Tripwires (CI/grep)**

```bash
rg -n "DcBlock|postWetBus" Source/engines/reverb/DSP/ReverbFDN.h
```

**Risk & Mitigation**

- Risk: DC filter alters tone → Mitigation: 1st-order, post-sum only; Release path identical aside from DC removal.

**Notes & Links**

- Commit: <pending>

---

### WO-61 — APVTS Freeze: snapshot & purge audio-thread reads

**Objective:** Eliminate the last APVTS touch in audio callbacks.

**Changes**

* Expand `core/params/Snapshot` to cache **all** used params at **prepare** and **message-thread ticks**.
* CI tripwire: forbid `AudioProcessorValueTreeState`/`getRawParameterValue` tokens in any `processBlock` TU.

**Acceptance**

* Grep is clean; audio thread reads **only** POD snapshots.

### Update — 2025-10-05
- Change: Implemented lock-free `HostParams` snapshot with double buffers in processor; replaced audio-thread `makeHostParams(apvts)` with snapshot reads in both float and double paths. (`Source/shared/Core/PluginProcessor.cpp`, `Source/processor/PluginProcessor.h`)
- Reason: Remove audio-thread `APVTS`/`ValueTree` interaction; enforce WO-61.
- Verification: Built AU/VST3/Standalone (RelWithDebInfo) successfully.
- Commits: aa9f0a1

---

### WO-62 — Rhythm-coupling probes (transport/tempo)

**Objective:** Prove or disprove transport coupling for rhythmic glitches.

**Changes**

* Compile-time toggle to **ignore** host playhead & tempo (Debug).
* Dev param to disable all **tempo-sync** features temporarily (delay grid, phase sync, etc.).

**Acceptance**

* If glitches vanish with transport disabled → we localize to transport-coupled logic; else we stay in wet-path/OS.

---

### WO-63 — pluginval + Host Matrix CI

**Objective:** Make regressions noisy.

**Changes**

* `scripts/host_matrix.sh`: Run Standalone smoke + pluginval; (optionally) thin Live/REAPER headless script where available.
* CI artifacts: upload logs + short audio renders for 64/128/512 at 44.1/48/96 kHz.

**Acceptance**

* Matrix green; artifacts show identical hashes across float/double at OS=1x.

---

## Diagnostics & Telemetry (dev-only)

* **First-bad-sample capture**: index, channel, block size, sample value (once per session).
* **SpikeSilencer log**: threshold value, ramp start, recovered in N samples.
* **Wrap monitor**: count of wraps per line per second; anomaly if mismatched.
* **HUD** toggle: show `CUT=wetOnly` etc. and last anomaly TTL.

*All compiled out in Release; toggled with `FIELD_DEV_HUD_ON` + `dev.hud.enable`.*

---

## Verification Matrix

| Area            | Check                             | Tooling                 | DoD               |
| --------------- | --------------------------------- | ----------------------- | ----------------- |
| Insert Safety   | Drop on Live Master while playing | manual + logs           | No spike/crash    |
| Wet Silence     | Silent in → silent out (no tail)  | unit (silence contract) | < −140 dBFS       |
| Parity          | float vs double @ OS=1x           | AB null / hash          | < −140 dBFS       |
| Rhythmic Glitch | 64/128/512 buffers                | renders + spectrogram   | no periodic spurs |
| APVTS Reads     | No audio-thread reads             | CI grep                 | clean             |
| pluginval       | AU+VST3                           | pluginval               | pass              |
| Host Matrix     | 44.1/48/96 × 64/128/512           | script                  | pass              |

---

## Risk & Mitigation

* **Heisenbugs** masked by dev fades → Only **one-shot** SpikeSilencer; bounded ramp; full log on first event.
* **Parity drift** from hidden casts → Prep cookie asserts + `prepareCommon` single source.
* **Tripwire fatigue** → Keep patterns tight; whitelist test scaffolds; fail with actionable messages.
* **Transport-coupling** false positives → Provide two toggles: ignore playhead vs only disable tempo sync.

---

## Rollback Plan

* All WO patches are **additive** and **guarded**; revert by:

  1. Disabling `FIELD_DEV_HUD_ON` and dev params → restores pure Release behavior.
  2. Removing SpikeSilencer include/hook (single point).
  3. Reverting prep cookie asserts (Debug-only).

No file moves in Phase 2; structure remains Phase-1-compliant.

---

## Ownership & Cadence

* **DSP/Engines:** @trail
* **Processor/Glue:** @trail, @grant
* **CI/Matrix:** @trail
* **Weekly cadence:** ship WOs in the order 55 → 56 → 57/58 → 60 → 61 → 62 → 63; run Host Matrix after each cluster.

---

## Appendix A — Code Sketches (drop-in)

**A1. SpikeSilencer (header-only, dev-only)**

```cpp
// core/signal/SpikeSilencer.h
#pragma once
#include <juce_dsp/juce_dsp.h>
#if FIELD_DEV_HUD_ON || JUCE_DEBUG
struct SpikeSilencer {
  int hold{0};
  float thresh{0.95f};
  int fade{32};
  bool process(float& x) noexcept {
    if (!juce::isFinite(x)) { x = 0.f; return true; }
    if (hold > 0) {            // in fade window
      const float t = 1.f - (float)hold / (float)fade;
      x *= t;                  // simple ramp back
      --hold;
      return true;
    }
    if (std::abs(x) >= thresh) { hold = fade; x *= 0.0f; return true; }
    return false;
  }
};
#else
struct SpikeSilencer { bool process(float&) noexcept { return false; } };
#endif
```

**Hook (FDN wet path, post-sum pre-DC):**

```cpp
#if FIELD_DEV_HUD_ON || JUCE_DEBUG
static thread_local SpikeSilencer silencerL, silencerR;
silencerL.process(outL); silencerR.process(outR);
#endif
```

**A2. 5 ms wet fade-in (prepare)**

```cpp
// engines/reverb/Core/ReverbEngine.h (member)
juce::dsp::Gain<float> wetRamp_;
void prepare(double sr, int maxBlock, int chans) {
  wetRamp_.setRampDurationSeconds(0.005); // 5ms
  wetRamp_.setGainLinear(0.f);
  wetRamp_.reset();
  // ...
}
void processWet(juce::dsp::AudioBlock<float>& wet) {
  wetRamp_.setGainLinear(1.f);
  wetRamp_.process(juce::dsp::ProcessContextReplacing<float>(wet));
  // ...
}
```

**A3. Wrap mirroring (canonical)**

```cpp
inline void postWrapPad(float* buf, int logicalLen, int pad) noexcept {
  for (int i = 0; i < pad; ++i) buf[logicalLen + i] = buf[i];
}
```

**A4. Prepare cookie & parity assert**

```cpp
struct PrepCookie { double sr{0}; int block{0}; int chans{0}; int bytes{0}; };
PrepCookie cookieF_, cookieD_;
template<class S> void prepareCommon(double sr, int maxBlock, int ch) {
  // ...
}
void prepare(float sr, int maxBlock, int ch)  { prepareCommon<float>(sr,maxBlock,ch); cookieF_={sr,maxBlock,ch,(int)sizeof(float)}; }
void prepare(double sr, int maxBlock, int ch) { prepareCommon<double>(sr,maxBlock,ch); cookieD_={sr,maxBlock,ch,(int)sizeof(double)}; }

template<class S>
void process(juce::dsp::AudioBlock<S>& b) {
#if JUCE_DEBUG
  const auto want = PrepCookie{ b.getSampleRate(), (int)b.getNumSamples(), (int)b.getNumChannels(), (int)sizeof(S) };
  const auto have = std::is_same_v<S,float> ? cookieF_ : cookieD_;
  jassert (have.chans==want.chans && have.bytes==want.bytes && have.block>=want.block);
#endif
  // ...
}
```

**A5. DC guard (post-sum)**

```cpp
struct DcBlock {
  float z{0.f};
  float a{0.995f}; // leak
  float process(float x) noexcept { float y = x - z; z = x + y * a; return y; }
};
```

---

## Appendix B — CI Tripwires (regex)

* Legacy scope: `\\bDspRuntimeConfig\\b|#include\\s*"shared/dsp/|#include\\s*"features/.*/DSP/`
* Audio thread param reads: `AudioProcessorValueTreeState|getRawParameterValue\\s*\\(.*\\).*processBlock`
* Engine parity: `processBlock\\s*\\(.*double.*\\)` without matching `prepare\\(.*double.*\\)`
* Transport probe off: ensure guarded use only: `AudioPlayHead` references must be behind dev toggles in engines.

---

## Appendix C — Triage Playbook

* **Master glitches > Track clean** → focus **double path** (WO-58).
* **Wet-only glitches; Dry-only clean** → localize **engine wet path** (WO-55/60).
* **Glitch rate tracks buffer** → inspect **wrap/crossfade** (WO-55).
* **Glitches stop when transport disabled** → **tempo/transport coupling** (WO-62).
* **Single insert spike** → verify **fade-in & SpikeSilencer** (WO-55/56).
* **APVTS reads in callback** (grep hit) → fix per **WO-61**.

---

**Phase 2 is focused, reversible, and guarded.**
When this page is green, we’ll graduate to Phase 3 (latency-aware live swaps beyond same-latency, and engine-level voicing maps).
