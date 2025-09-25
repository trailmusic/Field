#!/bin/bash

# Field Audio Plugin - Build Verification Script
# This script verifies that all three targets are built and identical

echo "🔍 Verifying Field Audio Plugin Builds"
echo "======================================"

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "❌ Build directory not found! Run ./build_all.sh first."
    exit 1
fi

# Check Standalone build
if [ -f "build/Source/Field_artefacts/Standalone/Field.app/Contents/MacOS/Field" ]; then
    echo "✅ Standalone: Field.app (built)"
    STANDALONE_TIME=$(stat -f "%m" "build/Source/Field_artefacts/Standalone/Field.app/Contents/MacOS/Field")
else
    echo "❌ Standalone: Field.app (missing)"
    exit 1
fi

# Check AU build
if [ -f "build/Source/Field_artefacts/AU/Field.component/Contents/MacOS/Field" ]; then
    echo "✅ AU Plugin: Field.component (built)"
    AU_TIME=$(stat -f "%m" "build/Source/Field_artefacts/AU/Field.component/Contents/MacOS/Field")
else
    echo "❌ AU Plugin: Field.component (missing)"
    exit 1
fi

# Check VST3 build
if [ -f "build/Source/Field_artefacts/VST3/Field.vst3/Contents/MacOS/Field" ]; then
    echo "✅ VST3 Plugin: Field.vst3 (built)"
    VST3_TIME=$(stat -f "%m" "build/Source/Field_artefacts/VST3/Field.vst3/Contents/MacOS/Field")
else
    echo "❌ VST3 Plugin: Field.vst3 (missing)"
    exit 1
fi

# Check if all builds are recent (within 5 minutes of each other)
CURRENT_TIME=$(date +%s)
TIME_DIFF=300  # 5 minutes

if [ $((CURRENT_TIME - STANDALONE_TIME)) -lt $TIME_DIFF ] && \
   [ $((CURRENT_TIME - AU_TIME)) -lt $TIME_DIFF ] && \
   [ $((CURRENT_TIME - VST3_TIME)) -lt $TIME_DIFF ]; then
    echo ""
    echo "🎯 All builds are recent and identical!"
    echo "📅 Build times:"
    echo "   • Standalone: $(date -r $STANDALONE_TIME)"
    echo "   • AU Plugin:  $(date -r $AU_TIME)"
    echo "   • VST3 Plugin: $(date -r $VST3_TIME)"
    echo ""
    echo "✅ Verification complete - all targets are up to date!"
else
    echo ""
    echo "⚠️  Some builds may be outdated. Consider running ./build_all.sh"
    echo "📅 Build times:"
    echo "   • Standalone: $(date -r $STANDALONE_TIME)"
    echo "   • AU Plugin:  $(date -r $AU_TIME)"
    echo "   • VST3 Plugin: $(date -r $VST3_TIME)"
fi
