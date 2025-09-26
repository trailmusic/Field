set(SRC
  Core/PluginProcessor.h
  Core/PluginProcessor.cpp
  Core/PluginEditor.h
  Core/PluginEditor.cpp
  Core/FieldLookAndFeel.h
  Core/FieldLookAndFeel.cpp
    ui/Components/KnobCell.h
    ui/Components/KnobCell.cpp
    ui/Components/KnobCellWithAux.h
    ui/Components/KnobCellWithAux.cpp
    ui/Components/KnobCellDual.h
    ui/Components/KnobCellDual.cpp
    ui/Components/KnobCellQuad.h
    ui/Components/KnobCellQuad.cpp
    ui/Components/KnobCellMini.h
    Core/IconSystem.h
    Core/IconSystem.cpp
    reverb/ReverbParamIDs.h
    reverb/ReverbParameters.h
    reverb/ReverbEngine.h
    reverb/ReverbEngine.cpp
    # legacy visuals removed: DecayCurve/ReverbEQ/ReverbScope
    reverb/ui/ReverbCanvasComponent.h
    reverb/ui/ReverbCanvasComponent.cpp
    reverb/ui/ReverbPanel.h
    reverb/ui/ReverbPanel.cpp
    reverb/ui/ReverbDynEQPane.h
    reverb/ui/ReverbControlsPane2x16.h
    reverb/ui/ReverbTab.h
    ui/SpectrumAnalyzer.h
    ui/SpectrumAnalyzer.cpp
    ui/delay/DelayUiBridge.h
    ui/delay/DelayVisuals.h
    ui/delay/DelayControlsPane.h
    ui/delay/DelayTab.h
    ui/DynEqTab.h
    dynEQ/DynamicEqParamIDs.h
    dynEQ/DynamicEqState.h
    dynEQ/DynamicEqState.cpp
    ui/ImagerPane.h
    ui/ImagerControlsPane.h
    ui/BandControlsPane.h
    ui/ImagerTab.h
    ui/XYControlsPane.h
    ui/XYTab.h
    ui/BandPane.h
    ui/StereoFieldEngine.h
    ui/StereoFieldEngine.cpp
    ui/PaneManager.h
    ui/SimpleSwitchCell.h
    ui/machine/ParamPatch.h
    ui/machine/ProposalCard.h
    ui/machine/ProposalCard.cpp
    ui/machine/WidthDesignerPanel.h
    ui/machine/WidthDesignerPanel.cpp
    ui/machine/MachineEngine.h
    ui/machine/MachineEngine.cpp
    ui/machine/MachinePane.h
    ui/machine/MachinePane.cpp
    
    motion/MotionEngine.h
    motion/MotionPanel.h
    motion/MotionPanel.cpp
    motion/MotionParams.h
    motion/MotionPath.h
    motion/MotionIDs.h
    motion/MotionControlsPane.h
    motion/MotionTab.h
    
    dsp/DelayPresetLibrary.h
    dsp/DelayPresetLibrary.cpp
    Presets/PresetStore.h
    Presets/PresetStore.cpp
    Presets/PresetManager.h
    Presets/PresetManager.cpp
    
    Presets/PresetRegistry.h
    Presets/PresetRegistry.cpp
    Presets/PresetCommandPalette.h
    Presets/PresetCommandPalette.cpp
    Presets/PresetSystem.h
    Presets/PresetTheme.h
)

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

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
)

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

if (MSVC)
  target_compile_options(Field PRIVATE /W4 /permissive-)
else()
  target_compile_options(Field PRIVATE -Wall -Wextra -Wshadow -Wpedantic)
endif() 