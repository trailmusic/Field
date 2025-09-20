# DEC-0001 — Phase Modes: IIR stability, FIR normalization, defaults

Status: accepted  
Owner: @trailmusic  
Date: 2025-09-20  
Tags: dsp, phase, filters, fir, iir  
Links: `Source/PluginProcessor.cpp`, `DevNotesSystem.md`, PR: ui-refactor-clean/55f4729

## Context
Users reported crackle/warble and level jumps when engaging HP/LP and switching phase modes. Master bus revealed IIR sensitivity; FIR sounded clean.

## Decision
- Zero/Natural (IIR) switch to non‑resonant Linkwitz–Riley HP/LP.  
- IIR smoothing ≈ 90 ms; add ≥3 Hz epsilon gating for retunes; fixed DC blocker at 8 Hz.  
- Hybrid FIR kernel gain‑normalized; bypass FIR when tone is neutral.  
- Full builds composite FIR, reports latency.  
- Defaults: Standard → Hybrid; High → Full.

## Consequences
Pros: clean masters, stable sweeps, predictable gain; consistent defaults.  
Cons: Zero/Natural transient character differs from linear‑phase; small latency in FIR modes.

## Follow‑ups
- Consider optional “Master‑safe” toggle (auto‑select Hybrid/Full on master bus).  
- Evaluate raising IIR epsilon/smoothing for extremely bright material.
