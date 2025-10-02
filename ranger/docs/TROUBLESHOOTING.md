# Troubleshooting

## Build fails: JUCE modules missing
- Ensure submodules are pulled: `git submodule update --init --recursive`
- Check CMake preset matches your IDE toolchain.

## Plots blank in UI
- Verify CSV uses single column of floats (one tap per line).
- Check Preferences → Precision (Float/Double) matches CLI converter.

## MinPhase mismatch vs CLI
- Confirm identical FFT pad (Auto/4k/8k/16k/32k).
- Normalization mode must match (None/Unity/DC).
- On CI, compare generated `MinPhaseBank.h` checksum with baseline artifact.

## CSV parse errors
- Ensure first column is numeric, one tap per line.
- Check for empty lines or non-numeric characters.
- Verify file encoding is UTF-8 or ASCII.

## Weird ringing in output
- Confirm input is **linear-phase** (symmetric). Prefer **odd length**.
- Check that the input taps are properly designed halfband filters.

## Magnitude mismatch between linear and min-phase
- Don't double-normalize; for halfband, use `--normalize dc`.
- Ensure the same FFT size is used for both conversions.
- Verify the input linear-phase taps are correctly designed.

## Performance issues
- These tools are offline; runtime performance depends on your FIR engine, not these generators.
- For large tap counts, consider using the batch processor instead of individual conversions.

## CI/CD failures
- Check that baseline files exist in `tools/baseline/` directory.
- Verify threshold settings are appropriate for your design requirements.
- Ensure all required Python dependencies are installed in the CI environment.

## Theme integration issues
- Components must query colors at paint-time via `findColour(FieldLNF::<id>, true)`.
- No color caching in members (only cache geometry/path).
- On theme change, `FieldLNF::applyTheme()` → `sendChangeMessage()` must trigger repaints.

## ComboBox display issues
- Abbreviation visible in closed state, full names in popup.
- Right chevron lane reserved: `max(18px, height/3)`.
- Use shared mapping: `FieldRendering::mapAbbrev()`.

## Accessibility problems
- Ensure all controls have proper accessible names.
- Test with VoiceOver (macOS) or Narrator (Windows).
- Verify keyboard navigation works for all interactive elements.
