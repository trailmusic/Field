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
    # Ranger Application Files
    RangerMain.cpp
    RangerWindow.cpp
    RangerDesigner.cpp
    RangerFilePane.cpp
    RangerPlotPane.cpp
    RangerSettingsPane.cpp
    RangerInstructionsPane.cpp
    RangerLogo.cpp
    SimpleFieldTheme.h
    
    # Field Components (commented out for now - will integrate later)
    # Components/ControlContainer.cpp
    # Components/TooltipBubble.cpp
    # Components/HelpButton.cpp
    # Components/QualityButton.cpp
    # Components/PhaseModeButton.cpp
    # Components/ToggleSwitch.cpp
    # Components/ShadeOverlay.cpp
    # Components/TransportClock.cpp
    # Components/CorrelationMeter.cpp
    # Components/GainSlider.cpp
    # Components/PanSlider.cpp
    # Components/KnobCell.cpp
    # Components/KnobCellWithAux.cpp
    # Components/SwitchCell.cpp
    # Components/SimpleIconButtons.cpp
    # Components/ComplexIconButtons.cpp
    # Components/PresetArrowButton.cpp
    # Components/ButtonSwitch.cpp
    # Components/ButtonSwitchFactory.cpp
    # Components/BypassButton.cpp
    # Components/AuditionButton.cpp
    # Components/ABButton.cpp
    # Components/ThemedIconButton.cpp
    # Components/TooltipsButton.cpp
    # Components/TintMenuLNFEx.cpp
    # Components/VerticalSlider3D.cpp
    # Components/VizEQ.cpp
    # Components/MonoSlopeSwitch.cpp
    # Components/SimpleSwitchCell.cpp
    # Components/ZoomControl.cpp
    # Components/HelpFAQComponent.cpp
    # Components/BottomChevronLNF.cpp
    
    # Field Managers (commented out for now)
    # Managers/AttachmentManager.cpp
    # Managers/CleanupManager.cpp
    # Managers/PaintManager.cpp
    # Managers/StateManager.cpp
    # Managers/MeterManager.cpp
    # Managers/SliderManager.cpp
    # Managers/ButtonManager.cpp
    
    # Field Utilities (commented out for now)
    # Utilities/ComponentGreyout.cpp
    
    # Field Engines (commented out for now)
    # Engines/SpectrumAnalyzer.cpp
    # Engines/StereoFieldEngine.cpp
    
    # Field Events (commented out for now)
    # Events/EventManager.cpp
    
    # Field Layout (commented out for now)
    # Layout/LayoutManager.cpp
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