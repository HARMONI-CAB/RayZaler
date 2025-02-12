set(Catch2_DIR "${PROJECT_SOURCE_DIR}/cmake/modules")

find_package(Catch2 CONFIG REQUIRED)
include(CTest)
include(Catch) # contains some cmake macros that will help with 

find_package(OpenGL REQUIRED)
find_package(OSMesa REQUIRED)
set(CMAKE_CXX_STANDARD 17)

# These tests can use the Catch2-provided main
add_executable(
  RZTests
  src/ReferenceFrameTests.cpp
  src/RayTracerTests.cpp
  src/ElementTests.cpp
  src/TopLevelTests.cpp
  src/HitTests.cpp)

target_link_directories(RZTests PRIVATE ${LIBRZ_LIBDIR})

target_compile_options(RZTests PRIVATE ${LIBRZ_CFLAGS})
target_include_directories(
  RZTests
  PRIVATE ../LibRZ/include
  ${OPENGL_INCLUDE_DIRS}
  ${OSMesa_INCLUDE_DIRS})

target_link_libraries(
  RZTests
  PRIVATE RZ
  Catch2::Catch2WithMain
  ${OPENGL_LIBRARIES}
  ${OSMesa_LIBRARIES})

catch_discover_tests(RZTests)

