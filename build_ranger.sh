#!/bin/bash

# Field Ranger Build Script with Auto App Management
# =================================================

set -e  # Exit on any error

echo "🎯 Building Field Ranger Desktop Application"
echo "============================================="

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Error: Must be run from Field project root directory"
    exit 1
fi

# Build Field Ranger console tools
echo "🔨 Building Field Ranger console tools..."
cd Ranger/console && ./build_tools.sh
echo "✅ Console tools built successfully"

# Check if Field Ranger app exists (placeholder for now)
RANGER_APP="/Applications/Field Ranger.app"
if [ -d "$RANGER_APP" ]; then
    echo "🔄 Auto-testing: Managing Field Ranger app..."
    
    # Check if Field Ranger is running
    if pgrep -f "Field Ranger" > /dev/null; then
        echo "📱 Field Ranger app is running - closing..."
        pkill -f "Field Ranger" || true
        sleep 2
    else
        echo "📱 Field Ranger app is not running"
    fi
    
    # Launch Field Ranger
    echo "🚀 Launching Field Ranger app..."
    open "$RANGER_APP" || {
        echo "❌ Failed to launch Field Ranger app"
        exit 1
    }
    
    echo "✅ Field Ranger app launched successfully"
else
    echo "📝 Field Ranger app not found - console tools available in /tools"
    echo "🛠️  Available tools:"
    echo "   • minphase - Convert single linear-phase FIR to minimum-phase"
    echo "   • batch_minphase - Generate MinPhaseBank.h from multiple designs"
    echo ""
    echo "💡 To test console tools:"
    echo "   cd tools && ./build/minphase --help"
    echo "   cd tools && ./build/batch_minphase --help"
fi

echo ""
echo "✨ Build and test cycle complete!"
echo "   Field Ranger tools are ready for use."

# Show available tools
echo ""
echo "🛠️  Available Field Ranger Tools:"
echo "   • Console Tools: /Users/grantedwards/Desktop/Field/Ranger/console/build/"
echo "   • Documentation: /Users/grantedwards/Desktop/Field/Ranger/docs/"
echo "   • Examples: /Users/grantedwards/Desktop/Field/Ranger/console/examples/"
echo ""
echo "🎯 Next Steps:"
echo "   1. Test console tools with example files"
echo "   2. Generate MinPhaseBank.h for Field plugin integration"
echo "   3. Implement Field Ranger desktop application"
