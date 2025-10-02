#!/usr/bin/env bash
set -euo pipefail

# Where outputs go
OUT_DIR="out"
CSV_DIR="${OUT_DIR}/csv"
REPORT_DIR="${OUT_DIR}/report"
PLOTS_DIR="${OUT_DIR}/plots"
mkdir -p "${CSV_DIR}" "${REPORT_DIR}" "${PLOTS_DIR}"

# Inputs: point at your LINEAR designs
# Example assumes tools/examples contains HB63/HB95/HB127 linear taps
LIN_DIR="tools/examples"
LINEAR_FILES=(
  "${LIN_DIR}/HB63_linear.csv"
  "${LIN_DIR}/HB95_linear.csv"
  "${LIN_DIR}/HB127_linear.csv"
)

# Run batch_minphase
BIN="tools/batch_minphase/build/batch_minphase"
if [ ! -x "${BIN}" ]; then
  echo "ERROR: ${BIN} not found or not executable"
  exit 1
fi

"${BIN}" \
  --out-header "${OUT_DIR}/MinPhaseBank.h" \
  --prefix HB \
  --normalize dc \
  --emit-csv \
  --in "${LINEAR_FILES[@]}"

# Move emitted CSVs next to header into out/csv/
# The batch tool emits HB*_min.csv in cwd; collect them:
for f in HB*_min.csv; do
  if [ -f "$f" ]; then
    mv "$f" "${CSV_DIR}/"
  fi
done

echo "Generated header: ${OUT_DIR}/MinPhaseBank.h"
echo "Generated CSVs to: ${CSV_DIR}"
