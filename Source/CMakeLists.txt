set(SRC
  PluginProcessor.h
  PluginProcessor.cpp
  PluginEditor.h
  PluginEditor.cpp
  FieldLookAndFeel.h
  FieldLookAndFeel.cpp
      KnobCellQuad.h
    KnobCellQuad.cpp
      KnobCellDual.h
    KnobCellDual.cpp
      KnobCell.h
    KnobCell.cpp
    KnobCellMini.h
      IconSystem.h
    IconSystem.cpp
    ui/SpectrumAnalyzer.h
    ui/SpectrumAnalyzer.cpp
    ui/ProcessedSpectrumPane.h
    ui/ImagerPane.h
    ui/StereoFieldEngine.h
    ui/StereoFieldEngine.cpp
    ui/PaneManager.h
    
    dsp/DelayPresetLibrary.h
    dsp/DelayPresetLibrary.cpp
    PresetStore.h
    PresetStore.cpp
    PresetManager.h
    PresetManager.cpp
    
    PresetRegistry.h
    PresetRegistry.cpp
    PresetCommandPalette.h
    PresetCommandPalette.cpp
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