# engines bootstrap: extend interface target include path if present
if (TARGET field_engines)
	target_include_directories(field_engines INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
