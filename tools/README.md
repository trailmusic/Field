# Field Ranger

**Patrol quality, phase, and oversampling.**

![Build](https://img.shields.io/github/actions/workflow/status/trailmusic/Field/tools-ci.yml?branch=feature)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platforms](https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux-666)

Field Ranger is a professional desktop application for designing and generating minimum-phase FIR filters for the Field audio plugin. It provides visual feedback, baseline comparison, and seamless integration with Field's Look & Feel system.

> **What is this?**
> These tools convert textbook-clean **linear-phase** filters into punchier **minimum-phase** versions with the *same tone* (same frequency shape) but **no pre-echo**. You can preview impulse/step/magnitude, compare with a baseline, then export headers your plugin uses for **oversampling/anti-alias** stages.
> TL;DR — it helps your reverbs, saturators, and dynamics sound clean at high quality **without** eating your CPU.

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

- **User Manual:** [`docs/README.md`](docs/README.md)
- **Quick Start:** [`docs/QUICKSTART.md`](docs/QUICKSTART.md)
- **UI Spec (Desktop Tool):** [`docs/UI_SPECIFICATION.md`](docs/UI_SPECIFICATION.md)
- **Field Plugin UI Integration:** [`docs/UI_INTEGRATION_NOTES.md`](docs/UI_INTEGRATION_NOTES.md)
- **CI/CD Setup:** [`docs/CI_SETUP.md`](docs/CI_SETUP.md)
- **Glossary:** [`docs/GLOSSARY.md`](docs/GLOSSARY.md)
- **Troubleshooting:** [`docs/TROUBLESHOOTING.md`](docs/TROUBLESHOOTING.md)
- **Changelog:** [`docs/CHANGELOG.md`](docs/CHANGELOG.md)

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
