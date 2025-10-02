# Glossary (A–Z)

**Abbreviation Mode** – UI mode that displays short tokens (e.g., "G", "V", "DB") in ComboBoxes, while menus keep full names.

**Chevron-only** – ComboBox trigger shows only a triangle glyph; full text appears inside the popup menu.

**ColourIds** – Stable integer keys used by components to fetch theme colors from Look&Feel at paint-time.

**DC Normalization** – Scales taps so the filter's DC (0 Hz) gain is 1.0.

**FIR / IIR** – Finite/Infinite Impulse Response filters. FIR can be linear/minimum phase; IIR is minimum-phase by design.

**JUCE** – C++ framework used for cross-platform audio/plugins/UI.

**KissFFT** – Lightweight FFT library used by our console/UI tools.

**LNF (Look & Feel)** – JUCE class that centralizes theming and control rendering (`FieldLNF`).

**Linear Phase** – Filter with symmetric taps; no phase rotation; can create pre-ringing.

**Minimum Phase** – Same magnitude as linear phase but with energy front-loaded; no pre-ringing; different phase.

**Oversampling** – Processing at a higher rate to reduce aliasing; needs **up/down** filters.

**Theme Variant** – Named palette (Ocean/Green/…) switched at runtime via `FieldLNF::applyTheme()`.

**True-Peak (TP-Safe)** – Prevents intersample overs when downsampling; clamps/limits before return to base rate.

**Cepstral Spectral Factorization** – Mathematical process used to convert linear-phase FIR to minimum-phase FIR while preserving magnitude response.

**DropShadow Cache** – Pre-rasterized shadow image to avoid CPU-heavy shadow draws every frame.

**EQ ColourIds** – Dedicated ids: label, border, zero line, grid, handle, handle-active, analyzer trace.

**FieldLNF** – Global Look&Feel + theme broadcaster; sends change messages on palette swap.

**MetallicRenderer** – Brushed/metal gradients used for special buttons & combos.

**MinPhaseBank** – Generated header file containing constexpr arrays of minimum-phase FIR taps and registry for lookup.

**Normalization** – Process of scaling filter taps to achieve desired gain characteristics (None/Unity/DC).

**TapSet** – Registry structure containing pointer to taps, length, and filter order for runtime lookup.

**Theme Invariants** – Rules ensuring consistent theming: components query colors at paint-time, no color caching, theme changes trigger repaints.

**Unity Normalization** – Scales taps so the filter's L2 energy is 1.0.

**Visualization State Machine** – Logic managing visualization behavior based on plugin parameters and signal levels (Disabled, ActiveSignal, IdlePreview, Frozen).
