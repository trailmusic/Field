cmake_minimum_required(VERSION 3.18)
project(MinPhaseTools LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch KissFFT
include(FetchContent)
FetchContent_Declare(
  kissfft
  GIT_REPOSITORY https://github.com/mborgerding/kissfft.git
  GIT_TAG        131.1.0  # or latest release tag
)
FetchContent_MakeAvailable(kissfft)

# Single converter
add_executable(minphase
    minphase/main.cpp
    minphase/fft_real.h
)

target_link_libraries(minphase PRIVATE kissfft::kissfft)
target_include_directories(minphase PRIVATE ${kissfft_SOURCE_DIR})

# Batch generator
add_executable(batch_minphase
    batch_minphase.cpp
    minphase/fft_real.h
)

target_link_libraries(batch_minphase PRIVATE kissfft::kissfft)
target_include_directories(batch_minphase PRIVATE ${kissfft_SOURCE_DIR})

# Precision switch (default: float)
# For double precision, uncomment the line below:
# target_compile_definitions(minphase PRIVATE KISS_FFT_DOUBLE)
# target_compile_definitions(batch_minphase PRIVATE KISS_FFT_DOUBLE)

# Field Ranger Desktop Application is now in parent directory

# Optimization flags
if (MSVC)
  target_compile_options(minphase PRIVATE /O2 /fp:fast)
  target_compile_options(batch_minphase PRIVATE /O2 /fp:fast)
else()
  target_compile_options(minphase PRIVATE -O3 -ffast-math -fno-math-errno -funroll-loops)
  target_compile_options(batch_minphase PRIVATE -O3 -ffast-math -fno-math-errno -funroll-loops)
endif()
