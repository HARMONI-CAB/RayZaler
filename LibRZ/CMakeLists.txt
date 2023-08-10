# Add all the source files needed to build the executable

set(LIBRZ_INCLUDEDIR include)
set(LIBRZ_SRCDIR     src)
set(LIBRZ_GRAMMARDIR grammar)

set(LIBRZ_LIBDIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)

find_package(BISON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")
include(FindPNG++)

set(LIBRZ_SOURCES
  ${LIBRZ_SRCDIR}/BenchElement.cpp
  ${LIBRZ_SRCDIR}/BlockElement.cpp
  ${LIBRZ_SRCDIR}/Detector.cpp
  ${LIBRZ_SRCDIR}/CircularMirror.cpp
  ${LIBRZ_SRCDIR}/CompositeElement.cpp
  ${LIBRZ_SRCDIR}/ConcaveMirror.cpp
  ${LIBRZ_SRCDIR}/CPURayTracingEngine.cpp
  ${LIBRZ_SRCDIR}/Element.cpp
  ${LIBRZ_SRCDIR}/ExprTkEvaluator.cpp
  ${LIBRZ_SRCDIR}/GenericCompositeModel.cpp
  ${LIBRZ_SRCDIR}/GLHelpers.cpp
  ${LIBRZ_SRCDIR}/Library.cpp
  ${LIBRZ_SRCDIR}/Linalg.cpp
  ${LIBRZ_SRCDIR}/OMModel.cpp
  ${LIBRZ_SRCDIR}/OpticalElement.cpp
  ${LIBRZ_SRCDIR}/ParserContext.cpp
  ${LIBRZ_SRCDIR}/RayBeamElement.cpp
  ${LIBRZ_SRCDIR}/RayProcessors.cpp
  ${LIBRZ_SRCDIR}/RayTracingEngine.cpp
  ${LIBRZ_SRCDIR}/Recipe.cpp
  ${LIBRZ_SRCDIR}/ReferenceFrame.cpp
  ${LIBRZ_SRCDIR}/RotatedFrame.cpp
  ${LIBRZ_SRCDIR}/Singleton.cpp
  ${LIBRZ_SRCDIR}/TopLevelModel.cpp
  ${LIBRZ_SRCDIR}/TranslatedFrame.cpp
  ${LIBRZ_SRCDIR}/WorldFrame.cpp)

set(LIBRZ_HEADERS
  ${LIBRZ_INCLUDEDIR}/BenchElement.h
  ${LIBRZ_INCLUDEDIR}/BlockElement.h
  ${LIBRZ_INCLUDEDIR}/CircularMirror.h
  ${LIBRZ_INCLUDEDIR}/CompositeElement.h
  ${LIBRZ_INCLUDEDIR}/ConcaveMirror.h
  ${LIBRZ_INCLUDEDIR}/Detector.h
  ${LIBRZ_INCLUDEDIR}/CPURayTracingEngine.h
  ${LIBRZ_INCLUDEDIR}/Element.h
  ${LIBRZ_INCLUDEDIR}/ExprTkEvaluator.h
  ${LIBRZ_INCLUDEDIR}/GenericCompositeModel.h
  ${LIBRZ_INCLUDEDIR}/GLHelpers.h
  ${LIBRZ_INCLUDEDIR}/Linalg.h
  ${LIBRZ_INCLUDEDIR}/Matrix.h
  ${LIBRZ_INCLUDEDIR}/OMModel.h
  ${LIBRZ_INCLUDEDIR}/OpticalElement.h
  ${LIBRZ_INCLUDEDIR}/ParserContext.h
  ${LIBRZ_INCLUDEDIR}/RayBeamElement.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors.h
  ${LIBRZ_INCLUDEDIR}/RayTracingEngine.h
  ${LIBRZ_INCLUDEDIR}/Recipe.h
  ${LIBRZ_INCLUDEDIR}/ReferenceFrame.h
  ${LIBRZ_INCLUDEDIR}/RotatedFrame.h
  ${LIBRZ_INCLUDEDIR}/Singleton.h
  ${LIBRZ_INCLUDEDIR}/TopLevelModel.h
  ${LIBRZ_INCLUDEDIR}/TranslatedFrame.h
  ${LIBRZ_INCLUDEDIR}/WorldFrame.h
  ${LIBRZ_INCLUDEDIR}/Vector.h)

BISON_TARGET(
  OMFParser 
  ${LIBRZ_GRAMMARDIR}/model.yy
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h)

add_library(
  RZ
  SHARED
  ${LIBRZ_SOURCES}
  ${LIBRZ_HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/parser.h)

target_include_directories(RZ PRIVATE ${LIBRZ_INCLUDEDIR} ${PNG++_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(RZ PRIVATE ${PNG++_LIBRARIES})
