cmake_minimum_required(VERSION 3.20.0)

set(RAYZER_VERSION 0.1)

# Define the project
project(
  RayZaler
  VERSION ${RAYZER_VERSION}
  LANGUAGES CXX)

# The library code is in src
add_subdirectory(lib/Catch2)
add_subdirectory(LibRZ)
add_subdirectory(LibRZTests)
add_subdirectory(RZViewer)
add_subdirectory(RZGUI)
