set(SRC
  Core/PluginProcessor.h
  Core/PluginProcessor.cpp
  Core/PluginEditor.h
  Core/PluginEditor.cpp
  Core/FieldLookAndFeel.h
  Core/FieldLookAndFeel.cpp
  Core/FieldTheme.h
  Core/FieldMetallic.h
  Core/FieldMetallic.cpp
  Core/FieldRendering.h
  Core/FieldRendering.cpp
    ui/Components/KnobCell.h
    ui/Components/KnobCell.cpp
    ui/Components/KnobCellWithAux.h
    ui/Components/KnobCellWithAux.cpp
    ui/Components/BypassButton.h
    ui/Components/ButtonSwitch.h
    ui/Components/ButtonSwitchFactory.h
    ui/Components/ThemedIconButton.h
    ui/Components/AuditionButton.h
    ui/Components/ABButton.h
    ui/Components/PhaseModeButton.h
    ui/Components/QualityButton.h
    ui/Components/TooltipsButton.h
    ui/Components/HelpButton.h
    ui/Components/TooltipBubble.h
    ui/Components/VerticalDivider.h
    ui/Components/HorizontalDivider.h
    xy/XYPad.h
    xy/XYPad.cpp
    ui/Components/VerticalSlider3D.h
    ui/Components/VerticalSlider3D.cpp
    ui/Components/ToggleSwitch.h
    ui/Components/ToggleSwitch.cpp
    ui/Components/CorrelationMeter.h
    ui/Components/CorrelationMeter.cpp
    ui/Components/MonoSlopeSwitch.h
    ui/Components/MonoSlopeSwitch.cpp
    ui/Components/ShadeOverlay.h
    ui/Components/VerticalLRMeters.h
    ui/Components/IOGainMeters.h
    ui/Components/SwitchCell.h
    ui/Components/Segmented3Control.h
    ui/Components/SimpleIconButtons.h
    ui/Components/ComplexIconButtons.h
    ui/Components/PresetArrowButton.h
    ui/Components/GainSlider.h
    ui/Components/PanSlider.h
    ui/Components/ControlContainer.h
    ui/Components/ControlContainer.cpp
    ui/Components/VizEQ.h
    ui/Components/VizEQ.cpp
    ui/Components/TintMenuLNFEx.h
    ui/Components/TintMenuLNFEx.cpp
    ui/Components/UIHelpers.h
    ui/Layout/LayoutManager.h
    ui/Layout/LayoutManager.cpp
    ui/Events/EventManager.h
    ui/Events/EventManager.cpp
            ui/Managers/AttachmentManager.h
            ui/Managers/AttachmentManager.cpp
    ui/Managers/CleanupManager.h
    ui/Managers/CleanupManager.cpp
    ui/Managers/PaintManager.h
    ui/Managers/PaintManager.cpp
    ui/Managers/StateManager.h
    ui/Managers/StateManager.cpp
    ui/Managers/MeterManager.h
    ui/Managers/MeterManager.cpp
    ui/Managers/SliderManager.h
    ui/Managers/SliderManager.cpp
    ui/Managers/ButtonManager.h
    ui/Managers/ButtonManager.cpp
    Core/IconSystem.h
    Core/IconSystem.cpp
    reverb/ReverbParamIDs.h
    reverb/ReverbParameters.h
    reverb/ReverbEngine.h
    reverb/ReverbEngine.cpp
    reverb/ReverbCanvasComponent.h
    reverb/ReverbCanvasComponent.cpp
    reverb/ReverbGraphics.h
    reverb/ReverbGraphics.cpp
    reverb/ReverbDynEQPane.h
    reverb/ReverbControlsPane.h
    reverb/ReverbTab.h
    ui/Engines/SpectrumAnalyzer.h
    ui/Engines/SpectrumAnalyzer.cpp
    delay/DelayUiBridge.h
    delay/DelayVisuals.h
    delay/DelayControlsPane.h
    delay/DelayTab.h
    dynEq/DynEqTab.h
    dynEq/ProcessedSpectrumPane.h
    dynEQ/DynamicEqParamIDs.h
    dynEQ/DynamicEqState.h
    dynEQ/DynamicEqState.cpp
    imager/ImagerPane.h
    imager/ImagerControlsPane.h
    band/BandControlsPane.h
    imager/ImagerTab.h
    xy/XYControlsPane.h
    xy/XYTab.h
    band/BandTab.h
    band/BandGraphics.h
    band/BandGraphics.cpp
    ui/Engines/StereoFieldEngine.h
    ui/Engines/StereoFieldEngine.cpp
    ui/Managers/PaneManager.h
    phase/PhaseTab.h
    phase/PhaseTab.cpp
    ui/Controls/SimpleSwitchCell.h
    machine/ParamPatch.h
    machine/ProposalCard.h
    machine/ProposalCard.cpp
    machine/MachineEngine.h
    machine/MachineEngine.cpp
    machine/MachineTab.h
    machine/MachineTab.cpp
    
    motion/MotionEngine.h
    motion/MotionGraphics.h
    motion/MotionGraphics.cpp
    motion/MotionParams.h
    motion/MotionPath.h
    motion/MotionIDs.h
    motion/MotionControlsPane.h
    motion/MotionTab.h
    motion/MotionSlot.h
    
    dsp/DelayPresetLibrary.h
    dsp/DelayPresetLibrary.cpp
    dsp/PhaseAlignmentEngine.h
    dsp/PhaseAlignmentEngine.cpp
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