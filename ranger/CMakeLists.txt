cmake_minimum_required(VERSION 3.18)
project(FieldRanger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find JUCE using the same method as the main Field project
include(FetchContent)
FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        7.0.12
)
FetchContent_MakeAvailable(juce)

# Create Field Ranger GUI application
juce_add_gui_app(FieldRanger
    PRODUCT_NAME "Field Ranger"
    BUNDLE_ID "com.trail.fieldranger"
    VERSION "1.0.0"
)

# Generate JUCE header
juce_generate_juce_header(FieldRanger)

target_sources(FieldRanger PRIVATE
    RangerMain.cpp
    RangerWindow.cpp
    RangerDesigner.cpp
    RangerFilePane.cpp
    RangerPlotPane.cpp
    RangerSettingsPane.cpp
    RangerInstructionsPane.cpp
)

target_link_libraries(FieldRanger PRIVATE
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_events
    juce::juce_gui_extra
)

# Set up app bundle for macOS
if(APPLE)
    set_target_properties(FieldRanger PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Field Ranger"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
        MACOSX_BUNDLE_IDENTIFIER "com.trail.fieldranger"
    )
endif()

# Optimization flags
if (MSVC)
    target_compile_options(FieldRanger PRIVATE /O2 /fp:fast)
else()
    target_compile_options(FieldRanger PRIVATE -O3 -ffast-math)
endif()
