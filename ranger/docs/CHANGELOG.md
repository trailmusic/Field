# Changelog

All notable changes to the Min-Phase FIR Toolchain will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2025-01-01

### Added
- Initial release of Min-Phase FIR Toolchain
- Console tools: `minphase` (single converter) and `batch_minphase` (batch generator)
- KissFFT wrapper for cross-platform FFT operations
- Complete documentation suite with user manual, quick start, and integration guides
- GitHub Actions CI/CD pipeline with automated testing and validation
- Desktop UI specification for JUCE-based visual application
- Field plugin integration notes for seamless UI integration
- Example linear-phase FIR files (HB63, HB95, HB127)
- Python visualization and analysis scripts
- Professional build prerequisites and denormal handling
- SR-aware oversampling and realtime/offline separation
- Comprehensive testing and validation framework

### Features
- Professional-quality min-phase FIR generation
- Visual feedback with impulse, step, and magnitude plots
- Baseline comparison and visual diff capabilities
- Export integration for MinPhaseBank.h and individual CSVs
- Cross-platform support (macOS, Windows, Linux)
- Full accessibility support with VoiceOver/Narrator
- Seamless Field plugin integration with existing Look & Feel system
- Automated CI/CD with numeric validation and artifact generation

### Documentation
- Complete user manual with build instructions and usage examples
- Quick start guide for immediate usage
- CI/CD setup guide for automated testing
- Desktop UI specification for visual application development
- Field plugin integration guide for seamless UI integration
- Troubleshooting guide for common issues
- Comprehensive glossary of technical terms
- Changelog for version tracking

### Technical
- Min-phase FIR conversion using cepstral spectral factorization
- FFT backend with KissFFT for cross-platform compatibility
- Batch processing with registry-based lookup system
- Theme integration with Field's Look & Feel system
- Performance optimization with shadow caching
- Accessibility support for custom controls
- Professional build system with compiler optimizations
