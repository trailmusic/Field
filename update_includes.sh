#!/bin/bash

# Update include paths for reverb directory restructuring
cd /Users/grantedwards/Desktop/Field/Source/features/reverb

# Update includes in all files
find . -name "*.h" -o -name "*.cpp" | while read file; do
    echo "Updating $file..."
    
    # Update ReverbEngine includes
    sed -i '' 's|#include "ReverbEngine\.h"|#include "Core/ReverbEngine.h"|g' "$file"
    sed -i '' 's|#include "ReverbTypes\.h"|#include "Core/ReverbTypes.h"|g' "$file"
    sed -i '' 's|#include "FieldReverbConfig\.h"|#include "Core/FieldReverbConfig.h"|g' "$file"
    
    # Update UI includes
    sed -i '' 's|#include "ReverbGraphics\.h"|#include "UI/ReverbGraphics.h"|g' "$file"
    sed -i '' 's|#include "ReverbVisuals\.h"|#include "UI/ReverbVisuals.h"|g' "$file"
    sed -i '' 's|#include "ReverbControlsPane\.h"|#include "UI/ReverbControlsPane.h"|g' "$file"
    sed -i '' 's|#include "ReverbTab\.h"|#include "UI/ReverbTab.h"|g' "$file"
    sed -i '' 's|#include "DuckingFloat\.h"|#include "UI/DuckingFloat.h"|g' "$file"
    sed -i '' 's|#include "ReverbScopeComponent\.h"|#include "UI/ReverbScopeComponent.h"|g' "$file"
    
    # Update DSP includes
    sed -i '' 's|#include "ReverbEQ\.h"|#include "DSP/ReverbEQ.h"|g' "$file"
    sed -i '' 's|#include "ReverbEQParamIDs\.h"|#include "DSP/ReverbEQParamIDs.h"|g' "$file"
    sed -i '' 's|#include "DecayRateEQ\.h"|#include "DSP/DecayRateEQ.h"|g' "$file"
    sed -i '' 's|#include "ReverbParameters\.h"|#include "DSP/ReverbParameters.h"|g' "$file"
    sed -i '' 's|#include "ReverbParamIDs\.h"|#include "DSP/ReverbParamIDs.h"|g' "$file"
    sed -i '' 's|#include "ReverbFDN\.h"|#include "DSP/ReverbFDN.h"|g' "$file"
    sed -i '' 's|#include "DecayLossDesigner\.h"|#include "DSP/DecayLossDesigner.h"|g' "$file"
    sed -i '' 's|#include "ReverbProcessorGlue\.h"|#include "DSP/ReverbProcessorGlue.h"|g' "$file"
    sed -i '' 's|#include "SimdBiquad\.h"|#include "DSP/SimdBiquad.h"|g' "$file"
    
    # Update Preset includes
    sed -i '' 's|#include "ReverbPresetManager\.h"|#include "Presets/ReverbPresetManager.h"|g' "$file"
    sed -i '' 's|#include "ReverbPresetLoader\.h"|#include "Presets/ReverbPresetLoader.h"|g' "$file"
    sed -i '' 's|#include "ReverbParamMap\.h"|#include "Presets/ReverbParamMap.h"|g' "$file"
    sed -i '' 's|#include "ModelMacros\.h"|#include "Presets/ModelMacros.h"|g' "$file"
    
    # Update Testing includes
    sed -i '' 's|#include "ReverbIRExportTest\.h"|#include "Testing/ReverbIRExportTest.h"|g' "$file"
done

echo "Include paths updated successfully!"
