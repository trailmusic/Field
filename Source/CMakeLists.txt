set(SRC
  shared/Core/PluginProcessor.h
  shared/Core/PluginProcessor.cpp
  shared/Core/PluginEditor.h
  shared/Core/PluginEditor.cpp
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
    features/reverb/ReverbParamIDs.h
    features/reverb/ReverbParameters.h
    features/reverb/ReverbParameters.cpp
    features/reverb/ReverbEngine.h
    features/reverb/ReverbEngine.cpp
    features/reverb/ReverbCanvasComponent.h
    features/reverb/ReverbCanvasComponent.cpp
    features/reverb/ReverbGraphics.h
    features/reverb/ReverbGraphics.cpp
    features/reverb/ReverbVisuals.h
    features/reverb/ReverbVisuals.cpp
    features/reverb/DuckingFloat.h
    features/reverb/DuckingFloat.cpp
    features/reverb/ReverbEQParamIDs.h
    features/reverb/ReverbEQ.h
    features/reverb/ReverbEQ.cpp
    features/reverb/DecayRateEQ.h
    features/reverb/DecayRateEQ.cpp
    features/reverb/ReverbDynEQPane.h
    features/reverb/ReverbControlsPane.h
    features/reverb/ReverbTab.h
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
    shared/ui/Controls/SimpleSwitchCell.h
    features/machine/ParamPatch.h
    features/machine/ProposalCard.h
    features/machine/ProposalCard.cpp
    features/machine/MachineEngine.h
    features/machine/MachineEngine.cpp
    features/machine/MachineTab.h
    features/machine/MachineTab.cpp
    
    features/motion/MotionEngine.h
    features/motion/MotionGraphics.h
    features/motion/MotionGraphics.cpp
    features/motion/MotionParams.h
    features/motion/MotionPath.h
    features/motion/MotionIDs.h
    features/motion/MotionControlsPane.h
    features/motion/MotionTab.h
    features/motion/MotionSlot.h
    
    shared/dsp/DelayPresetLibrary.h
    shared/dsp/DelayPresetLibrary.cpp
    shared/dsp/PhaseAlignmentEngine.h
    shared/dsp/PhaseAlignmentEngine.cpp
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