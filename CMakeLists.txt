cmake_minimum_required(VERSION 3.20.0)

set(RAYZER_VERSION 0.1)

# Define the project
project(
  RayZaler
  VERSION ${RAYZER_VERSION}
  LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

include(FindPNG++)

if(WIN32)
  message(STATUS "Getting SWIG: ...")
  include(swig)
  message(STATUS "Getting SWIG: ...DONE")
endif()

# The library code is in src
add_subdirectory(lib/Catch2)
add_subdirectory(LibRZ)
add_subdirectory(LibRZTests)
add_subdirectory(RZViewer)
add_subdirectory(RZGUI)

include(python)
include(FindNumPy)
