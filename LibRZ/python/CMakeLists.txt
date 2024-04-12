find_package(OpenGL REQUIRED)
find_package(OSMesa REQUIRED)

set_property(SOURCE RayZaler.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE RayZaler.i PROPERTY SWIG_MODULE_NAME RayZaler)
swig_add_library(pyRayZaler
  TYPE MODULE
  LANGUAGE python
  OUTPUT_DIR ${CMAKE_BINARY_DIR}/python/${PYTHON_PROJECT}/RayZaler
  SOURCES RayZaler.i)
add_library(${PROJECT_NAMESPACE}::pyRayZaler ALIAS pyRayZaler)

target_include_directories(pyRayZaler
  PRIVATE
  ../include
  ${Python3_INCLUDE_DIRS}
  )
set_property(TARGET pyRayZaler PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)

# note: macOS is APPLE and also UNIX !
if(APPLE)
  set_target_properties(pyRayZaler PROPERTIES
    SUFFIX ".so"
    INSTALL_RPATH "@loader_path;@loader_path/../../${PYTHON_PROJECT}/.libs"
    )
  set_property(TARGET pyRayZaler APPEND PROPERTY
    LINK_FLAGS "-flat_namespace -undefined suppress"
    )
elseif(UNIX)
  set_target_properties(pyRayZaler PROPERTIES
    INSTALL_RPATH "$ORIGIN:$ORIGIN/../../${PYTHON_PROJECT}/.libs"
    )
endif()

target_link_directories(pyRayZaler PRIVATE ${LIBRZ_LIBDIR} ${OPENGL_INCLUDE_DIRS} ${OSMesa_INCLUDE_DIRS})
target_compile_options(pyRayZaler PRIVATE ${LIBRZ_CFLAGS})
target_link_libraries(pyRayZaler PRIVATE RZ  ${OPENGL_LIBRARIES} ${OSMesa_LIBRARIES})

# Variable PYTHON_LIBRARIES can contains keyword `optimized`
# which won't be interpreted inside a generator expression.
# i.e. we can't use: $<$<PLATFORM_ID:Windows>:${PYTHON_LIBRARIES}>
# see: https://cmake.org/cmake/help/git-stage/command/target_link_libraries.html#command:target_link_libraries
if(MSVC)
  target_link_libraries(pyRayZaler PRIVATE ${Python3_LIBRARIES})
endif()
