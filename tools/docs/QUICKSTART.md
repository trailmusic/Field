# Min-Phase FIR Toolchain - Quick Start Guide

## 🚀 Get Started in 5 Minutes

### 1. Build the Tools

```bash
cd tools
./build_tools.sh
```

### 2. Run the Example

```bash
./example_usage.sh
```

This will:
- Create sample linear-phase FIR data
- Convert to minimum-phase using both tools
- Generate C++ headers and CSV files
- Show integration examples

### 3. Visualize Results (Optional)

```bash
# Install matplotlib if needed
pip3 install matplotlib numpy

# Visualize the generated data
python3 visualize_results.py examples/HB63_linear.csv examples/HB63_min.csv
```

## 📁 What You Get

After running the example, you'll have:

```
tools/
├── build/
│   ├── minphase          # Single converter
│   └── batch_minphase    # Batch generator
├── examples/
│   ├── HB63_linear.csv   # Input data
│   ├── HB63_min.csv      # Generated min-phase
│   ├── HB95_linear.csv
│   ├── HB95_min.csv
│   └── MinPhaseBank.h    # Combined header
└── README.md             # Full documentation
```

## 🔧 Quick Commands

### Single Converter
```bash
./build/minphase --in my_filter.csv --out-base MyFilter --normalize dc
```

### Batch Converter
```bash
./build/batch_minphase \
  --out-header MyBank.h \
  --prefix HB \
  --normalize dc \
  --emit-csv \
  --in filter1.csv filter2.csv filter3.csv
```

## 🎯 Integration Example

```cpp
#include "MinPhaseBank.h"
using namespace MinPhaseBank;

// Find taps by order
if (const auto* taps = findByOrder(63)) {
    const double* data = taps->data;
    const int length = taps->length;
    // Load into your FIR convolver
}
```

## 📚 Next Steps

- Read the full [README.md](README.md) for detailed documentation
- Check the [Quality System Implementation Plan](../docs/audits/Quality_System_Implementation_Plan.md) for integration details
- Use the visualization script to verify your results

## 🆘 Need Help?

- Check the troubleshooting section in README.md
- Verify your input CSV has odd-length, linear-phase taps
- Use `--normalize dc` for halfband filters
- Run the example script to see expected output format
