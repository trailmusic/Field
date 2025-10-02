# minphase (CLI)

Convert linear FIR taps → min-phase.

## Usage

```bash
./minphase input.csv --norm dc --fft auto --out output_min.csv
```

## Arguments

- `--in` : path to linear CSV (one tap per line; first column used)
- `--out-base` : base name for outputs → `<Name>_min.csv` + `<Name>_min.h`
- `--normalize` : `none|unity|dc` (use `dc` for halfband)
- `--fft-pad` : optional FFT size (power-of-two ≥ 2×taps). Defaults to `nextpow2(2*N)`.

## Example

```bash
./minphase --in HB63_linear.csv --out-base HB63 --normalize dc
# Outputs: HB63_min.csv, HB63_min.h
```

## Output

- `<Name>_min.csv` – minimum-phase taps, one per line
- `<Name>_min.h` – C++ header with constexpr array

See the [complete documentation](../docs/README.md) for detailed usage instructions.
