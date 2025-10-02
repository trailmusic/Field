#!/bin/bash

# Min-Phase FIR Toolchain Build Script
# Builds the minphase and batch_minphase utilities

set -e

echo "🔧 Building Min-Phase FIR Toolchain..."
echo "======================================"

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "📋 Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the tools
echo "🔨 Building tools..."
cmake --build . --config Release

echo ""
echo "✅ Build completed successfully!"
echo ""
echo "📦 Available tools:"
echo "  • minphase - Single converter (linear → min-phase)"
echo "  • batch_minphase - Batch generator (multiple → MinPhaseBank.h)"
echo ""
echo "📖 Usage examples:"
echo "  ./minphase --in HB63_linear.csv --out-base HB63 --normalize dc"
echo "  ./batch_minphase --out-header MinPhaseBank.h --prefix HB --normalize dc --emit-csv --in HB63_linear.csv HB95_linear.csv"
echo ""
echo "📚 See README.md for full documentation"
