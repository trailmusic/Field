# Ableton Live + Logic Pro: PDC & Tail Notes

- Change to plugin-reported latency (PDC) is cached on insert and on prepareToPlay().
- Do NOT call `setLatencySamples()` while transport is running. Defer to next prepare.
- Tail (`getTailLengthSeconds`) is sampled by the host; mid-play changes can affect bounce/tail-cut.
- Safe policy we follow:
  - Recompute latency & tail only during `prepareToPlay()` or when transport is stopped.
  - If a UI toggle changes latency (OS factor, linear-phase, look-ahead), we gate rebuild and defer.
  - Same-latency graph swaps are allowed mid-play with a short crossfade (DualChain), never touching PDC.
