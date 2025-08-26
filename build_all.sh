#!/bin/bash

# Field Audio Plugin - Build All Targets Script
# This script builds all three targets to ensure consistency

echo "🎵 Building Field Audio Plugin - All Targets"
echo "=============================================="

# Navigate to build directory
cd build

# Build all three targets
echo "🔨 Building Standalone, AU, and VST3 targets..."
cmake --build . --target Field_Standalone Field_AU Field_VST3 --config Debug -- -j 8

# Check build status
if [ $? -eq 0 ]; then
    echo ""
    echo "✅ All builds completed successfully!"
    echo ""
    echo "📦 Build Results:"
    echo "   • Standalone: Field.app"
    echo "   • AU Plugin: Field.component (installed to ~/Library/Audio/Plug-Ins/Components/)"
    echo "   • VST3 Plugin: Field.vst3 (installed to ~/Library/Audio/Plug-Ins/VST3/)"
    echo ""
    echo "🎯 All three targets are now identical and up to date!"
else
    echo ""
    echo "❌ Build failed! Please check the error messages above."
    exit 1
fi
