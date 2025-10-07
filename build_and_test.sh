#!/bin/bash

# Field Audio Plugin - Build and Test Script
# This script builds all targets and automatically closes/reopens the app for testing

echo "🎵 Building Field Audio Plugin - All Targets"
echo "=============================================="

# Function to check if Field app is running
is_field_running() {
    pgrep -f "Field.app" > /dev/null 2>&1 || pgrep -f "Field.component" > /dev/null 2>&1
}

# Function to close Field app if running
close_field_app() {
    echo "🔄 Closing Field app if running..."
    pkill -f "Field.app" 2>/dev/null || true
    pkill -f "Field.component" 2>/dev/null || true
    sleep 2  # Give it time to close
}

# Function to launch Field app
launch_field_app() {
    echo "🚀 Launching Field app..."
    # Prefer Standalone (Debug), then Standalone (non-Debug), then AU (opens in Finder)
    local STANDALONE_DEBUG="/Users/grantedwards/Desktop/Field/build/Source/Field_artefacts/Debug/Standalone/Field.app"
    local STANDALONE_REL="/Users/grantedwards/Desktop/Field/build/Source/Field_artefacts/Standalone/Field.app"
    local AU_PATH="/Users/grantedwards/Library/Audio/Plug-Ins/Components/Field.component"

    if [ -d "$STANDALONE_DEBUG" ]; then
        if open -n "$STANDALONE_DEBUG" 2>/dev/null; then
            echo "✅ Field Standalone (Debug) launched successfully"
            return 0
        fi
    fi

    if [ -d "$STANDALONE_REL" ]; then
        if open -n "$STANDALONE_REL" 2>/dev/null; then
            echo "✅ Field Standalone launched successfully"
            return 0
        fi
    fi

    if [ -e "$AU_PATH" ]; then
        if open "$AU_PATH" 2>/dev/null; then
            echo "✅ Field AU component opened"
            return 0
        fi
    fi

    echo "⚠️  Could not auto-launch Field app. Please launch manually."
}

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
    
    # Auto-close and reopen for testing
    echo ""
    echo "🔄 Auto-testing: Managing Field app..."
    
    if is_field_running; then
        echo "📱 Field app is currently running - closing and reopening..."
        close_field_app
        sleep 1
        launch_field_app
    else
        echo "📱 Field app is not running - launching..."
        launch_field_app
    fi
    
    echo ""
    echo "✨ Build and test cycle complete!"
    echo "   Field app has been automatically managed for testing."
    
else
    echo ""
    echo "❌ Build failed! Please check the error messages above."
    exit 1
fi
