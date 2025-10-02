# batch_minphase

Batch converts a folder of linear tap CSVs (e.g., HB63/HB95/HB127), emits `_min.csv` and a combined `MinPhaseBank.h`.

## Usage

```bash
./batch_minphase --out-header MinPhaseBank.h --prefix HB --normalize dc --emit-csv --in file1.csv file2.csv ...
```

## Arguments

- `--out-header` : output header name (default `MinPhaseBank.h`)
- `--prefix` : symbol prefix for arrays (`HB` → `HB63_min`, `HB95_min`, …)
- `--normalize` : `none|unity|dc` (use `dc` for halfband)
- `--emit-csv` : also write per-design `<Prefix><Order>_min.csv`
- `--in` : list of input CSVs. **Order** is parsed from filename digits (e.g., `HB63_linear.csv` → `63`).

## Example

```bash
./batch_minphase \
  --out-header MinPhaseBank.h \
  --prefix HB \
  --normalize dc \
  --emit-csv \
  --in HB63_linear.csv HB95_linear.csv HB127_linear.csv
```

## Output

- `MinPhaseBank.h` (constexpr arrays + registry)
- `HB63_min.csv`, `HB95_min.csv`, `HB127_min.csv` (if `--emit-csv`)

See the [complete documentation](../docs/README.md) for detailed usage instructions.
