# Field Architecture Refactor Audit — Phase 2  
_Contributors Guide & Header_

**File:** `docs/audits/Architecture_Refactor_Audit_2.md`  
**Last updated:** `2025-10-05` • **Branch:** `feature` • **Maintainers:** @trail @grant

---

## How to Use This Document

* **Append-only.** Never rewrite historical entries. Add new **WO** blocks or dated **Update** patchlets.
* **One WO = one anchor.** Header format: `# WO-XX — Title` (use the exact em dash).
* **Required sections** for every WO: Objective • Changes • Build & CMake • Tests • Verification • Tripwires • Risk & Mitigation • Notes & Links • Status/Owner/Date.
* **Bullets over prose.** Keep each change: `path` → short action. Put commands/regex in fenced blocks.
* **Link everything.** Use relative paths like `Source/...`. Reference PRs and short SHAs.
* **Status tags:** `PLANNED | IN PROGRESS | LANDING | LANDED | ROLLED BACK`.
* **Dates in UTC:** `YYYY-MM-DD`.
* **No DSP/tone changes** must be explicitly stated when applicable.

---

## Contents

* [Purpose](#purpose)
* [Scope](#scope)
* [Success Criteria (DoD)](#success-criteria-dod)
* [Starting Checklist](#starting-checklist)
* [Work Orders (Phase 2)](#work-orders-phase-2)

  * [WO-55 — FDN wet fade-in + wrap mirroring](#wo-55--fdn-wet-fade-in--wrap-mirroring)
  * [WO-56 — SpikeSilencer (dev-only micro-fade)](#wo-56--spikesilencer-dev-only-micro-fade)
  * [WO-57 — Dev cut switches + silence contract](#wo-57--dev-cut-switches--silence-contract)
  * [WO-58 — Double-path parity & prepare guarantees](#wo-58--double-path-parity--prepare-guarantees)
  * [WO-59 — Ableton Insert Safe hardening (recap)](#wo-59--ableton-insert-safe-hardening-recap)
  * [WO-60 — Wet-path DC guard & post-sum clamp](#wo-60--wet-path-dc-guard--post-sum-clamp)
  * [WO-61 — APVTS Freeze: snapshot & purge audio-thread reads](#wo-61--apvts-freeze-snapshot--purge-audio-thread-reads)
  * [WO-62 — Rhythm-coupling probes (transport/tempo)](#wo-62--rhythm-coupling-probes-transporttempo)
  * [WO-63 — pluginval + Host Matrix CI](#wo-63--pluginval--host-matrix-ci)
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

## Starting Checklist

1. Build **RelWithDebInfo** AU/VST3; clear macOS quarantine.
2. Run **pluginval** (AU and VST3).
3. Confirm Standalone launches (RelWithDebInfo) and Debug opens (asserts silenced or actionable).
4. Ensure Phase 1 tripwires are **enabled by default** in the build.

---

## Work Orders (Phase 2)

# WO-55 — FDN wet fade-in + wrap mirroring
**Status:** LANDED  
**Owner:** @trail • **Date:** 2025-10-05

**Objective:** Make startup & wrap transitions click-free and SIMD-safe.

**Changes**

* **5 ms wet fade-in** after each **prepare/reset** (per-channel ramp).
* **Wrap mirroring**: mirror head → pad region on each write wrap for safe vector reads.
* **NaN-safe wet L/R sum**: explicit finite checks before post-sum.

**Acceptance**

* Insert while playing produces **no spike**; silent input stays silent.
* Spectrogram shows **no spur** at wrap cadence.

---

# WO-56 — SpikeSilencer (dev-only micro-fade)
**Status:** LANDED  
**Owner:** @trail • **Date:** 2025-10-05

**Objective:** Smother rare >0.95 spikes with a one-shot **32-sample** micro-fade **(Debug only)**.

**Changes**

* `core/signal/SpikeSilencer.h` (header-only): threshold compare, hold & decay, 32-sample ramp to zero and back.
* Hooked **post wet-sum**, before DC block.
* Compiled out in Release (`#if FIELD_DEV_HUD_ON || JUCE_DEBUG`).

**Acceptance**

* First spike occurrence logs **index + block size** and fades; subsequent blocks normal.
* Release build: **no codegen** changes (verified by objdump size parity ± noise).

---

# WO-57 — Dev cut switches + silence contract
**Status:** PLANNED  
**Owner:** @trail • **Date:** 2025-10-05

**Objective:** Fast isolation switches; formal “silence means silence” rule.

**Changes**

* Hidden dev param `dev.dsp.cut` ∈ { **normal**, **dryOnly**, **wetOnly**, **fdnMuted**, **fdnBypassed** }.
* **Silence contract (Debug)**: if input RMS < −140 dBFS and tail=0, output must be < −140 dBFS; else assert+DBG with first-bad sample.

**Acceptance**

* Toggleable live from editor HUD (dev only).
* Contract holds across 64/128/512; no false positives.

---

# WO-58 — Double-path parity & prepare guarantees
**Status:** LANDED  
**Owner:** @trail • **Date:** 2025-10-05

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

# WO-59 — Ableton Insert Safe hardening (recap)
**Status:** LANDED  
**Owner:** @trail • **Date:** 2025-10-05

**Objective:** Belt-and-suspenders for Live’s first callback behaviors.

**Changes**

* Early returns on **zero-buffer/zero-channels**.
* `prepared_` gate on processor **and** FieldChain stages.
* DualChain guard: if **not prepared**, clear block and return.

**Acceptance**

* No crash/spike when dropping plugin on Master while playing loop.

---

# WO-60 — Wet-path DC guard & post-sum clamp
**Status:** LANDED  
**Owner:** @trail • **Date:** 2025-10-05

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
### Update — 2025-10-05
- Change: Added dev parameters `dev.dsp.cut`, `dev.transport.ignore`, `dev.tempoSync.off`; implemented dryOnly/wetOnly/fdnMuted/fdnBypassed routing; added Debug-only silence contract (first-bad capture). (`Source/shared/Core/PluginProcessor.cpp`, `Source/processor/PluginProcessor.h`)
- Reason: Fast isolation of glitch source and enforce “silence means silence”.
- Verification: Built AU/VST3/Standalone (RelWithDebInfo); verified routing toggles and one-shot assert fire in Debug.
- Commits: 3b2fb89

- ✅ Compiles AU/VST3/Standalone (RelWithDebInfo). No new warnings.

**Tripwires (CI/grep)**

```bash
rg -n "DcBlock|postWetBus" Source/engines/reverb/DSP/ReverbFDN.h
```

**Risk & Mitigation**

- Risk: DC filter alters tone → Mitigation: 1st-order, post-sum only; Release path identical aside from DC removal.

**Notes & Links**

- Commits: aa9f0a1

---

# WO-61 — APVTS Freeze: snapshot & purge audio-thread reads
**Status:** LANDED  
**Owner:** @trail • **Date:** 2025-10-05

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
- Commits: d5593a6

---

# WO-62 — Rhythm-coupling probes (transport/tempo)
**Status:** PLANNED  
**Owner:** @trail • **Date:** 2025-10-05

**Objective:** Prove or disprove transport coupling for rhythmic glitches.

**Changes**

* Compile-time toggle to **ignore** host playhead & tempo (Debug).
* Dev param to disable all **tempo-sync** features temporarily (delay grid, phase sync, etc.).

**Acceptance**
### Update — 2025-10-05
- Change: Gated AudioPlayHead usage via `dev.transport.ignore`; disabled tempo-sync behavior via `dev.tempoSync.off` in host-info propagation. (`Source/shared/Core/PluginProcessor.cpp`, `Source/processor/PluginProcessor.h`)
- Reason: Prove/disprove transport coupling for rhythmic artifacts.
- Verification: Build clean; ready for host litmus (Master vs Track, 64/128/512).
- Commits: 3b2fb89

### Update — 2025-10-05
- Change: Bypass made non-latching; when bypass is active, we copy dry to output but keep meters/visualization ticking. Wired listeners for Mix/Input/Output to refresh the snapshot immediately. Added debug logs for dev cut/transport flags at ingress. (`Source/shared/Core/PluginProcessor.cpp`)
- Reason: Ensure UI controls apply live and diagnostics remain visible while bypassed; simplify A/B on Master.
- Verification: AU/VST3/Standalone built; toggling bypass re-applies; meters tick under dryOnly and bypass.
- Commits: e889c2e

### Update — 2025-10-05
- Change: Default flip sequence for transport probes to simplify testing:
  - `dev.transport.ignore` default ON → OFF
  - `dev.tempoSync.off`   default OFF → ON
- Reason: Force “transport considered, tempo-sync disabled” as the hard test configuration on Master.
- Commits: 7f6cbac, 75a2577

### Update — 2025-10-05
- Change: Removed temporary Dev HUD UI controls (debug toggles in header); control remains via host automation only. (No processing changes.)
- Reason: Reduce confusion and surface area; keep Phase 2 focused on signal path.
- Commits: f00d3fe

# WO-65 — Phase engine hard toggle (dev-only)
**Status:** LANDING  
**Owner:** @trail • **Date:** 2025-10-05

## Objective
Allow disabling PhaseAlignmentEngine at process-time to isolate phase as a glitch source (dev-only; no Release impact).

## Changes
- `Source/shared/Core/PluginProcessor.cpp`: Added `dev.phase.off` param; skip PhaseAlignmentEngine `processBlock` when enabled.

## Verification
- Toggle `dev.phase.off` and confirm audible difference on Master if phase contributes to artifacts.

### Update — 2025-10-05
- Change: Implemented `dev.phase.off` gate at process-time. (`Source/shared/Core/PluginProcessor.cpp`)
- Commits: 7e0c04f

# WO-64 — MasterSafe (float-only minimal chain on Master)
**Status:** PLANNED  
**Owner:** @trail • **Date:** 2025-10-05

## Objective
On Master inserts only, force float path and minimal stages to determine if double-precision or non-reverb stages are implicated.

## Changes
- (Planned) Add `dev.master.safe` behavior to enforce minimal chain on Master.

## Verification
- If glitches vanish under MasterSafe, focus on double-path parity or non-reverb stages.

* If glitches vanish with transport disabled → we localize to transport-coupled logic; else we stay in wet-path/OS.

---

# WO-63 — pluginval + Host Matrix CI
**Status:** PLANNED  
**Owner:** @trail • **Date:** 2025-10-05

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

---

## Appendix D — Cleanup Map (dev artifacts; how to remove later)
- 2025-10-05 — Legacy poisoning and tripwire
  - Change: Poisoned legacy headers under `features/reverb/DSP/*` and `shared/Core/DspRuntimeConfig.h` with `#error`. Added CMake tripwire to fail on legacy includes and forbidden cross-includes.
  - Files: `Source/features/reverb/DSP/ReverbFDN.h`, `ReverbParamIDs.h`, `ReverbEQParamIDs.h`, `ReverbProcessorGlue.h`, `shared/Core/DspRuntimeConfig.h`, `Source/CMakeLists.txt`.
  - Commits: df3dbbc


- Dev parameters (APVTS) and toggles
  - In `Source/shared/Core/PluginProcessor.cpp` (createParameterLayout):
    - `dev.dsp.cut` (choice: normal/dry/wet/fdnMuted/fdnBypassed)
    - `dev.transport.ignore` (bool)
    - `dev.tempoSync.off` (bool)
    - `dev.master.safe` (bool)
    - `dev.phase.off` (bool)
  - Snapshot/listeners: `parameterChanged` refresh for dev params; extra listeners for `mix`, `inputGain`, `outputGain`.

- Processor snapshot (WO-61)
  - In `Source/processor/PluginProcessor.h`:
    - `HostParams` additions (dev flags at end).
    - Snapshot fields: `snapshotA_`, `snapshotB_`, `snapshotPtr_`.
  - In `Source/shared/Core/PluginProcessor.cpp`:
    - Constructor: allocate snapshots; set `snapshotPtr_`.
    - `parameterChanged`: rebuild snapshot.
    - Float/double `processBlock(...)`: read from `snapshotPtr_` instead of APVTS.
  - Keep (recommended) or remove by reverting snapshot fields and uses.

- Transport/tempo gates (WO-62)
  - In `processBlock` (float/double):
    - Guard playhead reads with `dev.transport.ignore`.
    - Force host-info `playing=false` when `dev.tempoSync.off`.
  - Remove by deleting those `hp.devTransportIgnore/devTempoSyncOff` checks.

- Dev cut routing (WO-57)
  - In `processBlock` (float/double):
    - Pre-routing: if `fdnMuted/fdnBypassed` → `hp.rvEnabled=false; hp.rvWet01=0`.
    - Output routing for `dryOnly` (copy dry) and `wetOnly` (wet = processed − dry).
    - Mix blend uses dry copy unless `dryOnly`.
  - Remove by deleting these branches and restore original mix path.

- Bypass behavior (non-latching)
  - In `processBlock` (float/double):
    - If bypass on, skip processing/graph tiling and copy dry back to output (still tick meters).
  - Remove by deleting the `userBypassNow` branches and restoring early-return passthrough.

- Debug logs (once-per-path)
  - In `processBlock` ingress: `DBG` prints devCut/transport flags/bypass state (Debug-only).
  - Remove DBG blocks.

- Phase OFF gate (WO-65)
  - In float path: skip `PhaseAlignmentEngine::processBlock(...)` when `dev.phase.off`.
  - Remove that check to always run phase.

- MasterSafe (WO-64)
  - Only parameter exists; minimal-chain behavior is planned. If not needed, remove `dev.master.safe` and references.

- FDN/hardening (WO-55/60)
  - In `Source/engines/reverb/DSP/ReverbFDN.h`:
    - Wrap mirroring on index wrap (`postWrapPad` on wrap)
    - 5 ms wet fade-in (`fadeSamplesLeft` ramp in `postWetBus`)
    - `SpikeSilencer` (Debug-only) after wet-sum, pre-DC
    - DC blocker `DcBlock dc_[2]` post-sum
    - Optional Debug clamp inside DC loop
  - Remove debug-only optionally (SpikeSilencer, clamp); keep wrap/fade/DC (recommended).

- Parity asserts (WO-58)
  - In `Source/modules/FieldChain.h`:
    - Debug-only prep cookies `cookieF_/cookieD_` and asserts in `prepare/process`.
  - Remove by deleting the `#if JUCE_DEBUG` cookie sections.

- Metering
  - We left `Node_Meter` intact; changes were elsewhere. No cleanup needed here.

- CMake / build toggles
  - `JUCE_DISABLE_ASSERTIONS` for Standalone Debug in `Source/CMakeLists.txt`.
  - Remove to restore JUCE asserts in Standalone Debug.

- Misc fixes (safe to keep)
  - `LatencyProbe` pointer-based `AudioBlock` construction fix.
  - `SafetySentinels` `inline` statics to fix duplicate symbols.
  - “Unity disabled stages” offline test: `Source/tests/offline/test_chain_unity_disabled_stages.cpp`.

- New file(s)
  - `Source/core/signal/SpikeSilencer.h` (delete if you drop WO-56).

### Quick find/remove searches
- Dev params/toggles: `dev.dsp.cut|dev.transport.ignore|dev.tempoSync.off|dev.master.safe|dev.phase.off`
- Snapshot bits: `snapshotA_|snapshotB_|snapshotPtr_|makeHostParams|HostParams`
- Parity cookies: `cookieF_|cookieD_|PrepCookie`
- Debug logs: `DBG\(\"\[Field\]`
- FDN debug: `SpikeSilencer|DcBlock|postWrapPad\(|fadeSamplesLeft`

### Keep vs remove
- Keep: FDN wrap mirroring, 5 ms fade-in, DC blocker; APVTS snapshot.
- Optional remove: SpikeSilencer, parity cookie asserts, dev transport/cut/phase/master flags and routing, debug logs.
- Revertable behavior: non-latching bypass → restore early-return passthrough.
