# core bootstrap: extend interface target include path if present
if (TARGET field_core)
    target_include_directories(field_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    target_sources(field_core PRIVATE params/ParamLayout.cpp)
    # headers included for indexing/IDE friendliness
    target_sources(field_core PRIVATE telemetry/LiveSwapHUD.h)
endif()
