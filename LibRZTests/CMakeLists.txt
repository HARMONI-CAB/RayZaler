find_package(Catch2 CONFIG REQUIRED)
include(CTest)
include(Catch) # contains some cmake macros that will help with 

find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)

# These tests can use the Catch2-provided main
add_executable(
  RZTests
  src/ReferenceFrameTests.cpp
  src/RayTracerTests.cpp
  src/ElementTests.cpp)

target_link_directories(RZTests PRIVATE ${LIBRZ_LIBDIR})

target_include_directories(
  RZTests
  PRIVATE ../LibRZ/include
  ${OPENGL_INCLUDE_DIRS}
  ${GLUT_INCLUDE_DIRS})

target_link_libraries(
  RZTests
  PRIVATE RZ
  Catch2::Catch2WithMain
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES})

catch_discover_tests(RZTests)

