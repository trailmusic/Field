# Null @ Unity (Host sanity)

1) Put Field on a copy of an audio track. Set Field to unity: all modules OFF, mix = 100% through.
2) Duplicate the source track; invert polarity on the duplicate.
3) Route both to the same bus. Expect full null (−∞) on the bus meter.

If it doesn't null:
- Verify reported latency (DBG) equals LatencyProbe reading.
- Ensure setLatencySamples() is only called at prepare and not during playback.
- Sanitize first/last blocks to avoid non-finites.
