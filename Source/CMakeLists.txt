set(SRC
  processor/PluginProcessor.h
  processor/PluginProcessor.cpp
  processor/PluginEditor.h
  processor/PluginEditor.cpp
  shared/Core/FieldLookAndFeel.h
  shared/Core/FieldLookAndFeel.cpp
  shared/Core/FieldTheme.h
  shared/Core/FieldMetallic.h
  shared/Core/FieldMetallic.cpp
  shared/Core/FieldRendering.h
  shared/Core/FieldRendering.cpp
    shared/ui/Components/KnobCell.h
    shared/ui/Components/KnobCell.cpp
    shared/ui/Components/KnobCellWithAux.h
    shared/ui/Components/KnobCellWithAux.cpp
    shared/ui/Components/BypassButton.h
    shared/ui/Components/ButtonSwitch.h
    shared/ui/Components/ButtonSwitchFactory.h
    shared/ui/Components/ThemedIconButton.h
    shared/ui/Components/AuditionButton.h
    shared/ui/Components/ABButton.h
    shared/ui/Components/PhaseModeButton.h
    shared/ui/Components/QualityButton.h
    shared/ui/Components/TooltipsButton.h
    shared/ui/Components/HelpButton.h
    shared/ui/Components/TooltipBubble.h
    shared/ui/Components/VerticalDivider.h
    shared/ui/Components/HorizontalDivider.h
    features/xy/XYPad.h
    features/xy/XYPad.cpp
    shared/ui/Components/VerticalSlider3D.h
    shared/ui/Components/VerticalSlider3D.cpp
    shared/ui/Components/ToggleSwitch.h
    shared/ui/Components/ToggleSwitch.cpp
    shared/ui/Components/CorrelationMeter.h
    shared/ui/Components/CorrelationMeter.cpp
    shared/ui/Components/MonoSlopeSwitch.h
    shared/ui/Components/MonoSlopeSwitch.cpp
    shared/ui/Components/ShadeOverlay.h
    shared/ui/Components/VerticalLRMeters.h
    shared/ui/Components/IOGainMeters.h
    shared/ui/Components/SwitchCell.h
    shared/ui/Components/Segmented3Control.h
    shared/ui/Components/SimpleIconButtons.h
    shared/ui/Components/ComplexIconButtons.h
    shared/ui/Components/PresetArrowButton.h
    shared/ui/Components/GainSlider.h
    shared/ui/Components/PanSlider.h
    shared/ui/Components/ControlContainer.h
    shared/ui/Components/ControlContainer.cpp
    shared/ui/Components/VizEQ.h
    shared/ui/Components/VizEQ.cpp
    shared/ui/Components/TransportClock.h
    shared/ui/Components/TransportClock.cpp
    shared/ui/Components/TintMenuLNFEx.h
    shared/ui/Components/TintMenuLNFEx.cpp
    shared/ui/Components/UIHelpers.h
    shared/ui/Layout/LayoutManager.h
    shared/ui/Layout/LayoutManager.cpp
    shared/ui/Events/EventManager.h
    shared/ui/Events/EventManager.cpp
            shared/ui/Managers/AttachmentManager.h
            shared/ui/Managers/AttachmentManager.cpp
    shared/ui/Managers/CleanupManager.h
    shared/ui/Managers/CleanupManager.cpp
    shared/ui/Managers/PaintManager.h
    shared/ui/Managers/PaintManager.cpp
    shared/ui/Managers/StateManager.h
    shared/ui/Managers/StateManager.cpp
    shared/ui/Managers/MeterManager.h
    shared/ui/Managers/MeterManager.cpp
    shared/ui/Managers/SliderManager.h
    shared/ui/Managers/SliderManager.cpp
    shared/ui/Managers/ButtonManager.h
    shared/ui/Managers/ButtonManager.cpp
    shared/ui/Utilities/ComponentGreyout.h
    shared/ui/Utilities/ComponentGreyout.cpp
    shared/Core/IconSystem.h
    shared/Core/IconSystem.cpp
    # Reverb Core
    features/reverb/Core/ReverbEngine.h
    features/reverb/Core/ReverbEngine.cpp
    features/reverb/Core/ReverbTypes.h
    features/reverb/Core/FieldReverbConfig.h
    
    # Reverb UI
    features/reverb/UI/ReverbTab.h
    features/reverb/UI/ReverbGraphics.h
    features/reverb/UI/ReverbGraphics.cpp
    features/reverb/UI/ReverbVisuals.h
    features/reverb/UI/ReverbVisuals.cpp
    features/reverb/UI/ReverbControlsPane.h
    features/reverb/UI/ReverbControlsPane.cpp
    features/reverb/UI/ReverbScopeComponent.h
    features/reverb/UI/ReverbScopeComponent.cpp
    features/reverb/UI/DuckingFloat.h
    features/reverb/UI/DuckingFloat.cpp
    features/reverb/UI/DecayRateFloat.h
    features/reverb/UI/DecayRateFloat.cpp
    
    # Reverb DSP
    # features/reverb/DSP/ReverbParamIDs.h (removed; use core/params/ParamIDs.h)
    engines/reverb/Presets/ReverbParameters.h
    engines/reverb/Presets/ReverbParameters.cpp
    engines/reverb/DSP/ReverbEQ.h
    engines/reverb/DSP/ReverbEQ.cpp
    # features/reverb/DSP/ReverbEQParamIDs.h (removed; use core/params/ParamIDs.h)
    engines/reverb/DSP/DecayRateEQ.h
    engines/reverb/DSP/DecayRateEQ.cpp
    engines/reverb/DSP/DecayLossDesigner.h
    engines/reverb/DSP/ReverbFDN.h
    # features/reverb/DSP/ReverbProcessorGlue.* removed; replaced by modules APIs
    engines/reverb/DSP/SimdBiquad.h
    
    # Reverb Presets
    features/reverb/Presets/ReverbPresetManager.h
    features/reverb/Presets/ReverbPresetManager.cpp
    features/reverb/Presets/ReverbParamMap.h
    features/reverb/Presets/ReverbParamMap.cpp
    features/reverb/Presets/ReverbPresetLoader.h
    features/reverb/Presets/ReverbPresetLoader.cpp
    features/reverb/Presets/ReverbPresetIntegration.h
    features/reverb/Presets/ReverbPresetIntegration.cpp
    features/reverb/Presets/ModelMacros.h
    
    # Reverb Testing
    features/reverb/Testing/ReverbIRExportTest.cpp
    shared/ui/Engines/SpectrumAnalyzer.h
    shared/ui/Engines/SpectrumAnalyzer.cpp
    features/delay/DelayUiBridge.h
    features/delay/DelayVisuals.h
    features/delay/DelayControlsPane.h
    features/delay/DelayTab.h
    features/dynEq/DynEqTab.h
    features/dynEq/ProcessedSpectrumPane.h
    features/dynEq/DynamicEqParamIDs.h
    features/dynEq/DynamicEqState.h
    features/dynEq/DynamicEqState.cpp
    features/imager/ImagerPane.h
    features/imager/ImagerControlsPane.h
    features/band/BandControlsPane.h
    features/imager/ImagerTab.h
    features/xy/XYControlsPane.h
    features/xy/XYTab.h
    features/band/BandTab.h
    features/band/BandGraphics.h
    features/band/BandGraphics.cpp
    shared/ui/Engines/StereoFieldEngine.h
    shared/ui/Engines/StereoFieldEngine.cpp
    shared/ui/Managers/PaneManager.h
    features/phase/PhaseTab.h
    features/phase/PhaseTab.cpp
    features/machine/ParamPatch.h
    features/machine/ProposalCard.h
    features/machine/ProposalCard.cpp
    engines/machine/MachineEngine.h
    engines/machine/MachineEngine.cpp
    features/machine/MachineTab.h
    features/machine/MachineTab.cpp
    
    engines/motion/MotionEngine.h
    features/motion/MotionGraphics.h
    features/motion/MotionGraphics.cpp
    features/motion/MotionParams.h
    features/motion/MotionPath.h
    features/motion/MotionIDs.h
    features/motion/MotionControlsPane.h
    features/motion/MotionTab.h
    features/motion/MotionSlot.h
    
    engines/delay/DelayPresetLibrary.h
    engines/delay/DelayPresetLibrary.cpp
    engines/phase/PhaseAlignmentEngine.h
    engines/phase/PhaseAlignmentEngine.cpp
    engines/phase/MinPhaseBankIntegration.h
    engines/phase/MinPhaseBankIntegration.cpp
    ../Ranger/MinPhaseBank.h
    ../Ranger/MinPhaseBank.cpp
    shared/Presets/PresetStore.h
    shared/Presets/PresetStore.cpp
    shared/Presets/PresetManager.h
    shared/Presets/PresetManager.cpp
    
    shared/Presets/PresetRegistry.h
    shared/Presets/PresetRegistry.cpp
    shared/Presets/PresetCommandPalette.h
    shared/Presets/PresetCommandPalette.cpp
    shared/Presets/PresetSystem.h
    shared/Presets/PresetTheme.h
)

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Layered interface targets for phased migration (no sources yet)
add_library(field_core      INTERFACE)
add_library(field_engines   INTERFACE)
add_library(field_modules   INTERFACE)
add_library(field_ui        INTERFACE)
add_library(field_presets   INTERFACE)

target_include_directories(field_core INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/shared"
    "${CMAKE_CURRENT_SOURCE_DIR}/features"
)
target_include_directories(field_engines INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/engines"
)
target_include_directories(field_modules INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/modules"
)
target_include_directories(field_ui INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/ui"
)
target_include_directories(field_presets INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/presets"
)

juce_add_plugin(Field
    COMPANY_NAME "Trail"
    BUNDLE_ID com.trail.field
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE YTtr
    PLUGIN_CODE Fld1
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Field"
)

# Generate JUCE header and configure VST3
juce_generate_juce_header(Field)

# Configure VST3 without VST2 dependencies
target_compile_definitions(Field PRIVATE 
    JUCE_DISABLE_VST2_LEGACY=1
    JUCE_DISABLE_VST2=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_STRICT_REFCOUNTEDPTR=1
    FIELD_POISON_DSP_RUNTIME_CONFIG=1
)

# Enable Dev HUD in internal builds by default (can be overridden per-config)
add_compile_definitions(FIELD_DEV_HUD=1)

# Configure VST3 to use only VST3 headers
set_target_properties(Field PROPERTIES
    JUCE_VST3_SDK_PATH "/dev/null"
)

target_sources(Field PRIVATE ${SRC})

target_link_libraries(Field PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_gui_basics
    juce::juce_core)

# ================================================================
# 🎛️ QUALITY SYSTEM BUILD PREREQUISITES (JANUARY 2025)
# ================================================================
# CRITICAL: These compiler flags are required for professional-grade
# oversampling and phase processing. DO NOT REMOVE without consulting the team.
# ================================================================

if (MSVC)
  # MSVC optimization flags for professional audio processing
  target_compile_options(Field PRIVATE 
    /W4 /permissive-
    /O2 /fp:fast /arch:AVX2 /Oi /Ot
    /D_USE_MATH_DEFINES
  )
  # Enable DAZ/FTZ for denormal handling
  target_compile_definitions(Field PRIVATE 
    JUCE_DISABLE_DENORMALS=1
  )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # Clang/GCC optimization flags for professional audio processing
  target_compile_options(Field PRIVATE 
    -Wall -Wextra -Wshadow -Wpedantic
    -O3 -ffast-math -fno-math-errno -funroll-loops
    -mavx2 -mfma
    -D_USE_MATH_DEFINES
  )
  # Enable DAZ/FTZ for denormal handling
  target_compile_definitions(Field PRIVATE 
    JUCE_DISABLE_DENORMALS=1
  )
  
  # ARM optimization for Apple Silicon
  if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    target_compile_options(Field PRIVATE -mcpu=native)
  endif()
endif()

# ================================================================
# 🎛️ END QUALITY SYSTEM BUILD PREREQUISITES
# ================================================================

# --- CI Tripwires: forbid legacy includes (features/reverb/DSP, shared/dsp) ---
add_custom_target(ci_tripwire_legacy_includes ALL
    COMMAND ${CMAKE_COMMAND} -E env LC_ALL=C LANG=C
      bash -c "(! grep -R --line-number '#include\\s*\"features/reverb/DSP/' Source || (echo '❌ Legacy features/reverb/DSP include found' && false)) && \
               (! grep -R --line-number '#include\\s*\"shared/dsp/' Source || (echo '❌ shared/dsp include found' && false)) && \
               (! grep -R --line-number '\\bDspRuntimeConfig\\b' Source || (echo '❌ DspRuntimeConfig usage found' && false)) && \
               (! grep -R --line-number '#include\\s*\".*(features/|shared/).*\"' Source/engines || (echo '❌ engines code must not include features/ or shared/' && false))"
) 