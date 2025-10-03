# presets bootstrap: extend interface target include path if present
if (TARGET field_presets)
	target_include_directories(field_presets INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
