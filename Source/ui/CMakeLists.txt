# ui bootstrap: extend interface target include path if present
if (TARGET field_ui)
	target_include_directories(field_ui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
