cmake_minimum_required(VERSION 3.22)
project(Field VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ---- CPM (header-only) ----
include(cmake/CPM.cmake)

# JUCE
CPMAddPackage(
  NAME JUCE
  GITHUB_REPOSITORY juce-framework/JUCE
  GIT_TAG 8.0.0 # update if needed
)

add_subdirectory(Source) 