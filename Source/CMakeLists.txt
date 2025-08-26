set(SRC
  PluginProcessor.h
  PluginProcessor.cpp
  PluginEditor.h
  PluginEditor.cpp
  FieldLookAndFeel.h
  FieldLookAndFeel.cpp
      IconSystem.h
    IconSystem.cpp
    PresetSystem.h
    PresetSystem.cpp
)

juce_add_plugin(Field
    COMPANY_NAME "Yucca Tree"
    BUNDLE_ID com.yuccatree.field
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