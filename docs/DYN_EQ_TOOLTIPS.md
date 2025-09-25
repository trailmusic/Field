## DynEQ Tooltips (seed copy)

Last updated: 2025-09-24

Guidelines
- Keep to one sentence; lead with action/impact; avoid jargon.
- Use units and defaults; reflect FIELD_UI_RULES naming.
- Prefer verbs over nouns; hint at modifiers (Shift/Alt) when helpful.

Global / Pane
- Analyzer Tap: Switch the analyzer between Pre and Post DynEQ.
- Latency Readout: Shows added latency from look-ahead and linear phase.

Gestures (Band Editor)
- Add Band: Double‑click empty space to add a band at that frequency.
- Delete Band: Double‑click a band point to remove it.
- Drag Band: Drag to set Freq (X) and Gain (Y).
- Adjust Q: Scroll on a band; hold Shift to adjust faster.
- Range Handle (Dynamic): Drag the center handle to set ±dB dynamic range.

Band Parameters
- Type: Choose filter shape (Bell, Shelf, HP/LP, Notch, BP, All‑Pass).
- Freq: Set the band’s center/cutoff frequency (20 Hz–20 kHz).
- Gain: Set static boost/cut in dB (−24 to +24 dB).
- Q: Set bandwidth; higher Q is narrower.
- Phase: Choose Zero, Natural, or Linear phase (latency varies).
- Channel: Target Stereo, Mid, Side, Left, or Right.

Dynamic (time‑varying)
- Dyn On: Enable level‑dependent processing for this band.
- Mode: Down compresses peaks; Up expands quiet content.
- Range: Max gain change (±dB) applied by the dynamics.
- Threshold: Level where dynamics begin to act.
- Attack: Time to react to increases in level.
- Release: Time to recover after signal falls.
- Hold: Minimum time to sustain gain change.
- Look‑Ahead: Anticipate peaks for cleaner control (adds latency).
- Detector: Choose tap (Pre/Post/External) and side‑chain filters.
- T/S Amount: Separate amounts for Transient and Sustain energy.

Spectral (resonance/tonal shaping)
- Spec On: Enable resonance‑aware shaping inside the band.
- Selectivity: How narrowly resonant bins are targeted.
- Resolution: FFT density (Low/Med/High); higher costs more CPU.
- Adaptive: Auto‑adjust threshold to track program material.

Visuals (Pane)
- Band Contribution Curve: Individual band’s response (light path).
- Macro EQ Curve: Composite response of all bands (thicker path).
- Dynamic Region: Gradient area between static and dynamic paths (Dyn On).
- Spectral Underfill: Subtle gradient indicating spectral shaping (Spec On).

Quality & Safety
- Auto Linear Guard: High‑Q HF boosts may blend to Natural to reduce pre‑ringing.
- Oversampling: Engages automatically for demanding settings; shown in header.

Short Hints (inline)
- Hint: Double‑click empty space to add; double‑click a point to delete.
- Hint: Scroll over a point to change Q; hold Shift to go faster.
- Hint: Turn on Dyn or Spec to reveal advanced controls and visuals.


