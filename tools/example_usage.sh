#!/bin/bash

# Example usage script for Min-Phase FIR Toolchain
# This script demonstrates how to use the tools with sample data

set -e

echo "🎯 Min-Phase FIR Toolchain - Example Usage"
echo "=========================================="

# Create example data directory
mkdir -p examples
cd examples

echo "📝 Creating example linear-phase FIR data..."

# Create a simple halfband filter (63 taps, symmetric)
cat > HB63_linear.csv << 'EOF'
0.000123
-0.002345
0.004567
-0.008901
0.012345
-0.015678
0.018901
-0.021234
0.023456
-0.025678
0.027890
-0.030123
0.032345
-0.034567
0.036789
-0.039012
0.041234
-0.043456
0.045678
-0.047890
0.050123
-0.052345
0.054567
-0.056789
0.059012
-0.061234
0.063456
-0.065678
0.067890
-0.070123
0.072345
-0.074567
0.5
-0.074567
0.072345
-0.070123
0.067890
-0.065678
0.063456
-0.061234
0.059012
-0.056789
0.054567
-0.052345
0.050123
-0.047890
0.045678
-0.043456
0.041234
-0.039012
0.036789
-0.034567
0.032345
-0.030123
0.027890
-0.025678
0.023456
-0.021234
0.018901
-0.015678
0.012345
-0.008901
0.004567
-0.002345
0.000123
EOF

# Create another example (95 taps)
cat > HB95_linear.csv << 'EOF'
0.000045
-0.000123
0.000234
-0.000456
0.000789
-0.001234
0.001567
-0.001890
0.002123
-0.002456
0.002789
-0.003123
0.003456
-0.003789
0.004123
-0.004456
0.004789
-0.005123
0.005456
-0.005789
0.006123
-0.006456
0.006789
-0.007123
0.007456
-0.007789
0.008123
-0.008456
0.008789
-0.009123
0.009456
-0.009789
0.010123
-0.010456
0.010789
-0.011123
0.011456
-0.011789
0.012123
-0.012456
0.012789
-0.013123
0.013456
-0.013789
0.014123
-0.014456
0.014789
-0.015123
0.5
-0.015123
0.014789
-0.014456
0.014123
-0.013789
0.013456
-0.013123
0.012789
-0.012456
0.012123
-0.011789
0.011456
-0.011123
0.010789
-0.010456
0.010123
-0.009789
0.009456
-0.009123
0.008789
-0.008456
0.008123
-0.007789
0.007456
-0.007123
0.006789
-0.006456
0.006123
-0.005789
0.005456
-0.005123
0.004789
-0.004456
0.004123
-0.003789
0.003456
-0.003123
0.002789
-0.002456
0.002123
-0.001890
0.001567
-0.001234
0.000789
-0.000456
0.000234
-0.000123
0.000045
EOF

echo "✅ Created example linear-phase FIR data"
echo ""

# Build the tools if they don't exist
if [ ! -f "../build/minphase" ] || [ ! -f "../build/batch_minphase" ]; then
    echo "🔨 Building tools..."
    cd ..
    ./build_tools.sh
    cd examples
fi

echo "🛠️  Running single converter example..."
echo "======================================"
echo "Converting HB63_linear.csv to minimum-phase..."

../build/minphase --in HB63_linear.csv --out-base HB63 --normalize dc

echo ""
echo "📁 Generated files:"
echo "  • HB63_min.csv - Minimum-phase taps"
echo "  • HB63_min.h - C++ header with constexpr array"
echo ""

echo "🛠️  Running batch converter example..."
echo "====================================="
echo "Converting multiple files to MinPhaseBank.h..."

../build/batch_minphase \
    --out-header MinPhaseBank.h \
    --prefix HB \
    --normalize dc \
    --emit-csv \
    --in HB63_linear.csv HB95_linear.csv

echo ""
echo "📁 Generated files:"
echo "  • HB63_min.csv - Individual minimum-phase taps"
echo "  • HB95_min.csv - Individual minimum-phase taps"
echo "  • MinPhaseBank.h - Combined header with registry"
echo ""

echo "📊 File sizes:"
ls -lh *.csv *.h 2>/dev/null || true

echo ""
echo "🎯 Example integration code:"
echo "============================"
cat << 'EOF'
// In your plugin's oversampler factory:
#include "MinPhaseBank.h"
using namespace MinPhaseBank;

static const TapSet* findByOrder(int order) {
    for (int i = 0; i < registryCount; ++i)
        if (registry[i].order == order) return &registry[i];
    return nullptr;
}

// When creating MinPhaseFIR oversampler:
if (const auto* taps = findByOrder(63)) {
    const double* data = taps->data;
    const int length = taps->length;
    // Load taps into your FIR convolver
}
EOF

echo ""
echo "✅ Example usage completed!"
echo "📚 See README.md for full documentation"
