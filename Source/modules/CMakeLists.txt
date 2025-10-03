add_library(field_modules
    FieldChain.cpp FieldChain.h
    FieldNodes/NodeLatency.h
    FieldNodes/Node_Reverb.h
    FieldNodes/Node_Delay.h
    FieldNodes/Node_DynEq.h
    FieldNodes/Node_Phase.h
    FieldNodes/Node_Imager.h
    Mixing/Node_Gain.h
    Mixing/Node_MSMatrix.h
    Mixing/Node_Meter.h
)

target_include_directories(field_modules PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(field_modules PUBLIC field_core juce_dsp)
