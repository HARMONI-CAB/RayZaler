# Add all the source files needed to build the executable

set(LIBRZ_INCLUDEDIR include)
set(LIBRZ_SRCDIR     src)
set(LIBRZ_GRAMMARDIR grammar)

set(CMAKE_CXX_STANDARD 17)

set(LIBRZ_LIBDIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)

find_package(BISON REQUIRED)
find_package(Python OPTIONAL_COMPONENTS Development)
find_package(Freetype REQUIRED)

# Make sure CMAKE_INSTALL_LIBDIR is defined for all systems
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR lib)
endif()

set(LIBRZ_SOURCES
  ${LIBRZ_SRCDIR}/ApertureStop.cpp
  ${LIBRZ_SRCDIR}/BenchElement.cpp
  ${LIBRZ_SRCDIR}/BlockElement.cpp
  ${LIBRZ_SRCDIR}/Detector.cpp
  ${LIBRZ_SRCDIR}/FlatMirror.cpp
  ${LIBRZ_SRCDIR}/CircularWindow.cpp
  ${LIBRZ_SRCDIR}/CompositeElement.cpp
  ${LIBRZ_SRCDIR}/ConvexLens.cpp
  ${LIBRZ_SRCDIR}/SphericalMirror.cpp
  ${LIBRZ_SRCDIR}/CPURayTracingEngine.cpp
  ${LIBRZ_SRCDIR}/Element.cpp
  ${LIBRZ_SRCDIR}/ExprTkEvaluator.cpp
  ${LIBRZ_SRCDIR}/FT2Facade.cpp
  ${LIBRZ_SRCDIR}/GenericAperture.cpp
  ${LIBRZ_SRCDIR}/GenericCompositeModel.cpp
  ${LIBRZ_SRCDIR}/GLModel.cpp
  ${LIBRZ_SRCDIR}/GLRenderEngine.cpp
  ${LIBRZ_SRCDIR}/GLHelpers.cpp
  ${LIBRZ_SRCDIR}/Helpers.cpp
  ${LIBRZ_SRCDIR}/IdealLens.cpp
  ${LIBRZ_SRCDIR}/IncrementalRotation.cpp
  ${LIBRZ_SRCDIR}/LensletArray.cpp
  ${LIBRZ_SRCDIR}/Library.cpp
  ${LIBRZ_SRCDIR}/Linalg.cpp
  ${LIBRZ_SRCDIR}/Logger.cpp
  ${LIBRZ_SRCDIR}/ModelRenderer.cpp
  ${LIBRZ_SRCDIR}/Obstruction.cpp
  ${LIBRZ_SRCDIR}/OMModel.cpp
  ${LIBRZ_SRCDIR}/OpticalElement.cpp
  ${LIBRZ_SRCDIR}/ParabolicMirror.cpp
  ${LIBRZ_SRCDIR}/ParserContext.cpp
  ${LIBRZ_SRCDIR}/PhaseScreen.cpp
  ${LIBRZ_SRCDIR}/Random.cpp
  ${LIBRZ_SRCDIR}/RayBeamElement.cpp
  ${LIBRZ_SRCDIR}/RayProcessors.cpp
  ${LIBRZ_SRCDIR}/RayTracingEngine.cpp
  ${LIBRZ_SRCDIR}/Recipe.cpp
  ${LIBRZ_SRCDIR}/RectangularStop.cpp
  ${LIBRZ_SRCDIR}/ReferenceFrame.cpp
  ${LIBRZ_SRCDIR}/RodElement.cpp
  ${LIBRZ_SRCDIR}/RotatedFrame.cpp
  ${LIBRZ_SRCDIR}/RZGLModel.cpp
  ${LIBRZ_SRCDIR}/Singleton.cpp
  ${LIBRZ_SRCDIR}/StlMesh.cpp
  ${LIBRZ_SRCDIR}/TopLevelModel.cpp
  ${LIBRZ_SRCDIR}/Tripod.cpp
  ${LIBRZ_SRCDIR}/TripodFrame.cpp
  ${LIBRZ_SRCDIR}/TranslatedFrame.cpp
  ${LIBRZ_SRCDIR}/TubeElement.cpp
  ${LIBRZ_SRCDIR}/WorldFrame.cpp
  ${LIBRZ_SRCDIR}/Zernike.cpp
  
  ${LIBRZ_SRCDIR}/RayProcessors/ApertureStop.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/CircularWindow.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/FlatMirror.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/IdealLens.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/InfiniteMirror.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/LensletArray.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/Obstruction.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/ParabolicMirror.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/PassThrough.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/PhaseScreen.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/RectangularStop.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/SphericalLens.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/SphericalMirror.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/SquareFlatSurface.cpp

  ${LIBRZ_SRCDIR}/Apertures/Array.cpp
  ${LIBRZ_SRCDIR}/Apertures/Circular.cpp
  ${LIBRZ_SRCDIR}/Apertures/Parabolic.cpp
  ${LIBRZ_SRCDIR}/Apertures/Rectangular.cpp
  ${LIBRZ_SRCDIR}/Apertures/Spherical.cpp

  ${LIBRZ_SRCDIR}/Samplers/Circular.cpp
  ${LIBRZ_SRCDIR}/Samplers/Point.cpp
  ${LIBRZ_SRCDIR}/Samplers/Ring.cpp
  ${LIBRZ_SRCDIR}/Samplers/Sampler.cpp)

set(LIBRZ_HEADERS
  ${LIBRZ_INCLUDEDIR}/ApertureStop.h
  ${LIBRZ_INCLUDEDIR}/BenchElement.h
  ${LIBRZ_INCLUDEDIR}/BlockElement.h
  ${LIBRZ_INCLUDEDIR}/FlatMirror.h
  ${LIBRZ_INCLUDEDIR}/CircularWindow.h
  ${LIBRZ_INCLUDEDIR}/CompositeElement.h
  ${LIBRZ_INCLUDEDIR}/ConvexLens.h
  ${LIBRZ_INCLUDEDIR}/SphericalMirror.h
  ${LIBRZ_INCLUDEDIR}/Detector.h
  ${LIBRZ_INCLUDEDIR}/CPURayTracingEngine.h
  ${LIBRZ_INCLUDEDIR}/Element.h
  ${LIBRZ_INCLUDEDIR}/ExprTkEvaluator.h
  ${LIBRZ_INCLUDEDIR}/exprtk.hpp
  ${LIBRZ_INCLUDEDIR}/FT2Facade.h
  ${LIBRZ_INCLUDEDIR}/GenericAperture.h
  ${LIBRZ_INCLUDEDIR}/GenericCompositeModel.h
  ${LIBRZ_INCLUDEDIR}/GLHelpers.h
  ${LIBRZ_INCLUDEDIR}/GLModel.h
  ${LIBRZ_INCLUDEDIR}/GLRenderEngine.h
  ${LIBRZ_INCLUDEDIR}/Helpers.h
  ${LIBRZ_INCLUDEDIR}/IdealLens.h
  ${LIBRZ_INCLUDEDIR}/IncrementalRotation.h
  ${LIBRZ_INCLUDEDIR}/LensletArray.h
  ${LIBRZ_INCLUDEDIR}/Linalg.h
  ${LIBRZ_INCLUDEDIR}/Logger.h
  ${LIBRZ_INCLUDEDIR}/Matrix.h
  ${LIBRZ_INCLUDEDIR}/ModelRenderer.h
  ${LIBRZ_INCLUDEDIR}/Obstruction.h
  ${LIBRZ_INCLUDEDIR}/OMModel.h
  ${LIBRZ_INCLUDEDIR}/OpticalElement.h
  ${LIBRZ_INCLUDEDIR}/ParabolicMirror.h
  ${LIBRZ_INCLUDEDIR}/PhaseScreen.h
  ${LIBRZ_INCLUDEDIR}/ParserContext.h
  ${LIBRZ_INCLUDEDIR}/Random.h
  ${LIBRZ_INCLUDEDIR}/RayBeamElement.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors.h
  ${LIBRZ_INCLUDEDIR}/RayTracingEngine.h
  ${LIBRZ_INCLUDEDIR}/Recipe.h
  ${LIBRZ_INCLUDEDIR}/RectangularStop.h
  ${LIBRZ_INCLUDEDIR}/ReferenceFrame.h
  ${LIBRZ_INCLUDEDIR}/RodElement.h
  ${LIBRZ_INCLUDEDIR}/RotatedFrame.h
  ${LIBRZ_INCLUDEDIR}/RZGLModel.h
  ${LIBRZ_INCLUDEDIR}/Singleton.h
  ${LIBRZ_INCLUDEDIR}/StlMesh.h
  ${LIBRZ_INCLUDEDIR}/TopLevelModel.h
  ${LIBRZ_INCLUDEDIR}/Tripod.h
  ${LIBRZ_INCLUDEDIR}/TripodFrame.h
  ${LIBRZ_INCLUDEDIR}/TranslatedFrame.h
  ${LIBRZ_INCLUDEDIR}/TubeElement.h
  ${LIBRZ_INCLUDEDIR}/WorldFrame.h
  ${LIBRZ_INCLUDEDIR}/Vector.h
  ${LIBRZ_INCLUDEDIR}/Zernike.h

  ${LIBRZ_INCLUDEDIR}/RayProcessors/ApertureStop.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/CircularWindow.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/FlatMirror.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/IdealLens.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/InfiniteMirror.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/LensletArray.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/Obstruction.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/ParabolicMirror.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/PassThrough.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/PhaseScreen.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/RectangularStop.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/SphericalLens.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/SphericalMirror.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/SquareFlatSurface.h
  
  ${LIBRZ_INCLUDEDIR}/Apertures/Array.h
  ${LIBRZ_INCLUDEDIR}/Apertures/Circular.h
  ${LIBRZ_INCLUDEDIR}/Apertures/Parabolic.h
  ${LIBRZ_INCLUDEDIR}/Apertures/Rectangular.h
  ${LIBRZ_INCLUDEDIR}/Apertures/Spherical.h
  
  ${LIBRZ_INCLUDEDIR}/Samplers/Circular.h
  ${LIBRZ_INCLUDEDIR}/Samplers/Point.h
  ${LIBRZ_INCLUDEDIR}/Samplers/Ring.h
  ${LIBRZ_INCLUDEDIR}/Samplers/Sampler.h)

if(Python_FOUND)
  set(LIBRZ_SOURCES
    ${LIBRZ_SOURCES}
    ${LIBRZ_SRCDIR}/PythonAPI.cpp
    ${LIBRZ_SRCDIR}/ScriptLoader.cpp)

  set(LIBRZ_HEADERS
    ${LIBRZ_HEADERS}
    ${LIBRZ_INCLUDEDIR}/ScriptLoader.h)
endif()

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

if(Python_FOUND)
  set(LIBRZ_LOCAL_CFLAGS "-DPYTHON_SCRIPT_SUPPORT")
  target_compile_options(RZ PRIVATE ${LIBRZ_LOCAL_CFLAGS})
  target_include_directories(RZ PRIVATE ${Python_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})
  target_link_libraries(RZ PRIVATE ${Python_LIBRARIES} ${FREETYPE_LIBRARIES})
endif()

set(LIBRZ_CFLAGS ${LIBRZ_LOCAL_CFLAGS} PARENT_SCOPE)
target_include_directories(RZ PRIVATE ${LIBRZ_INCLUDEDIR} ${PNG++_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(RZ PRIVATE ${PNG++_LIBRARIES})

install(FILES ${LIBRZ_HEADERS} DESTINATION include/RZ)
install(TARGETS RZ DESTINATION ${CMAKE_INSTALL_LIBDIR})
