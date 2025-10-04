add_executable(test_null_unity test_null_unity.cpp)
target_link_libraries(test_null_unity PRIVATE field_modules field_core juce_dsp)

add_executable(test_latency_probe test_latency_probe.cpp)
target_link_libraries(test_latency_probe PRIVATE field_modules field_core juce_dsp)

add_executable(test_rebuild_gate test_rebuild_gate.cpp)
target_link_libraries(test_rebuild_gate PRIVATE field_modules field_core juce_dsp)

add_executable(test_dualchain_xfade test_dualchain_xfade.cpp)
target_link_libraries(test_dualchain_xfade PRIVATE field_modules field_core juce_dsp)

add_executable(test_dualchain_midblock test_dualchain_midblock.cpp)
target_link_libraries(test_dualchain_midblock PRIVATE field_modules field_core juce_dsp)

add_executable(test_statesanity test_statesanity.cpp)
target_link_libraries(test_statesanity PRIVATE field_core juce_dsp)

add_executable(test_latency_ci test_latency_ci.cpp)
target_link_libraries(test_latency_ci PRIVATE field_modules field_core juce_dsp)

add_executable(test_dualchain_voicing_swap test_dualchain_voicing_swap.cpp)
target_link_libraries(test_dualchain_voicing_swap PRIVATE field_modules field_core juce_dsp)

add_executable(test_param_latency_map test_param_latency_map.cpp)
target_link_libraries(test_param_latency_map PRIVATE field_modules field_core juce_dsp)

# Golden determinism tests
add_executable(test_dualchain_voicing_golden test_dualchain_voicing_golden.cpp TestUtils_Golden.h)
target_link_libraries(test_dualchain_voicing_golden PRIVATE field_modules field_core juce_dsp)

add_executable(test_dualchain_midblock_golden test_dualchain_midblock_golden.cpp TestUtils_Golden.h)
target_link_libraries(test_dualchain_midblock_golden PRIVATE field_modules field_core juce_dsp)

# WO-18 smoke matrix and tail cache tests
add_executable(test_latency_smoke_matrix
    test_latency_smoke_matrix.cpp
    TestUtils_Golden.h
    TestUtils_Params.h)
target_link_libraries(test_latency_smoke_matrix PRIVATE field_modules field_core juce_dsp)

add_executable(test_tail_guard_cache test_tail_guard_cache.cpp)
target_link_libraries(test_tail_guard_cache PRIVATE field_core)

# WO-19 processor latency/tail smoke
add_executable(test_processor_latency_tail_smoke
    test_processor_latency_tail_smoke.cpp
    TestUtils_APVTS.h)
target_link_libraries(test_processor_latency_tail_smoke
    PRIVATE field_processor field_modules field_core juce_audio_processors juce_dsp)

# WO-20 ParamChangeBus ⇄ Processor glue
add_executable(test_param_bus_processor_glue
    test_param_bus_processor_glue.cpp
    TestUtils_APVTS.h)
target_link_libraries(test_param_bus_processor_glue
    PRIVATE field_processor field_modules field_core juce_audio_processors juce_dsp)

# WO-37 build-only guard
add_executable(test_no_dsp_runtime_config test_no_dsp_runtime_config.cpp)
target_link_libraries(test_no_dsp_runtime_config PRIVATE field_core)
