# modules bootstrap: extend interface target include path if present
if (TARGET field_modules)
	target_include_directories(field_modules INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
