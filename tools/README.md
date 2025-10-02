# Min-Phase FIR Toolchain

This directory contains the complete min-phase FIR toolchain for generating professional-quality minimum-phase filters from linear-phase designs.

## 📁 Directory Structure

```
tools/
├── docs/                           # Documentation
│   ├── README.md                   # Complete user manual
│   ├── QUICKSTART.md               # Quick start guide
│   ├── CI_SETUP.md                 # GitHub Actions CI/CD setup
│   ├── UI_SPECIFICATION.md          # Desktop application specification
│   └── UI_INTEGRATION_NOTES.md     # Field plugin integration guide
├── minphase/                       # Single converter tool
│   ├── main.cpp
│   ├── fft_real.h
│   └── CMakeLists.txt
├── batch_minphase/                 # Batch generator tool
│   ├── batch_minphase.cpp
│   ├── fft_real.h
│   └── CMakeLists.txt
├── examples/                       # Example linear-phase FIR files
│   ├── HB63_linear.csv
│   ├── HB95_linear.csv
│   └── HB127_linear.csv
├── baseline/                       # Reference data for CI testing
├── scripts/                        # CI/CD automation scripts
│   ├── build_batch.sh
│   ├── compare_csv.py
│   └── plot_bank.py
└── .github/workflows/              # GitHub Actions CI/CD
    └── minphase.yml
```

## 🚀 Quick Start

1. **Read the documentation**: Start with [`docs/QUICKSTART.md`](docs/QUICKSTART.md)
2. **Build the tools**: Follow the build instructions in [`docs/README.md`](docs/README.md)
3. **Set up CI/CD**: Configure automated testing with [`docs/CI_SETUP.md`](docs/CI_SETUP.md)
4. **Desktop UI**: Build a visual interface using [`docs/UI_SPECIFICATION.md`](docs/UI_SPECIFICATION.md)
5. **Plugin Integration**: Integrate with Field plugin using [`docs/UI_INTEGRATION_NOTES.md`](docs/UI_INTEGRATION_NOTES.md)

## 📚 Documentation

- **[Complete User Manual](docs/README.md)** - Comprehensive guide to the toolchain
- **[Quick Start Guide](docs/QUICKSTART.md)** - Get up and running quickly
- **[CI/CD Setup](docs/CI_SETUP.md)** - Automated testing and validation
- **[Desktop UI Specification](docs/UI_SPECIFICATION.md)** - JUCE application design
- **[Field Plugin Integration](docs/UI_INTEGRATION_NOTES.md)** - UI integration guide

## 🛠️ Tools

- **`minphase`** - Convert single linear-phase FIR to minimum-phase
- **`batch_minphase`** - Generate `MinPhaseBank.h` from multiple designs
- **CI/CD Pipeline** - Automated testing, validation, and artifact generation
- **Desktop Application** - Visual interface for drag-and-drop conversion
- **Plugin Integration** - Seamless integration with Field's UI system

## 🎯 Features

- **Professional Quality**: Industry-standard min-phase FIR generation
- **Visual Feedback**: Impulse, Step, and Magnitude plots with overlays
- **Baseline Comparison**: Visual diff against reference taps
- **Export Integration**: Generate `MinPhaseBank.h` and individual CSVs
- **Cross-Platform**: macOS, Windows, Linux support
- **Accessibility**: Full VoiceOver/Narrator support
- **Theme Integration**: Seamless integration with Field's Look & Feel system

---

**Ready to generate professional min-phase FIR filters?** Start with the [Quick Start Guide](docs/QUICKSTART.md)!
